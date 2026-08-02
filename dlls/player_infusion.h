//=========================================================
// player_infusion.h
//
// The Infusion -- the healing a Health Syringe starts, which
// runs for its own duration and is gone when it ends.
//
// Not to be confused with a passive regeneration Skill: an
// Infusion is started deliberately, runs on a clock, and shows
// a status icon while it does.  Vocabulary follows CONTEXT.md.
//
// The Syringe is proactive where the medkit is reactive, and two
// rules carry that: it may be used at full health, and a second
// one is refused while an Infusion is already running.  See
// docs/adr/0007-the-infusion-is-one-at-a-time.md.
//=========================================================

#pragma once

class CBasePlayer;

//=========================================================
// CPlayerInfusion
//
// Stored directly inside CBasePlayer, next to m_pulse.
//
// m_bActive is its own bool rather than being inferred from a
// zeroed timer, for the same reason CPlayerPulse keeps its flags:
// FIELD_TIME rebases on restore, so a zero stored as a sentinel
// comes back as gpGlobals->time and reads as "just expired".
//=========================================================
struct CPlayerInfusion
{
	// ---- Persistent data (save/restore) ----

	bool  m_bActive    = false; // an Infusion is running right now
	float m_flEndTime  = 0;     // when it stops
	float m_flNextTick = 0;     // when the next HP lands

	// Fractional HP carried between ticks.  The tick interval is fixed and the
	// rate is a cvar, so a rate that does not divide evenly into whole HP per
	// tick would otherwise be silently rounded away.
	float m_flAccum    = 0;

	// ---- Transient ----

	// Whether the client has been told to show the icon.  Not saved: the client
	// wipes its icon list on every ResetHUD, and ForgetSentIcon() re-syncs it.
	bool  m_bIconSent  = false;

	void Clear(CBasePlayer* pPlayer);

	// Makes the next Think re-send the icon.  Called when the client's HUD is
	// reset, which wipes CHudStatusIcons and would otherwise leave an Infusion
	// healing invisibly.
	void ForgetSentIcon() { m_bIconSent = false; }

	bool Active() const { return m_bActive; }

	// Starts an Infusion, or refuses if one is already running.  Refusing is
	// what stops a Stack being chugged in one go, and is why the caller must
	// not consume the Syringe when this returns false.
	bool TryStart(CBasePlayer* pPlayer);

	// Lands the ticks that are due and ends the Infusion when its time is up.
	// Call each frame.
	void Think(CBasePlayer* pPlayer);

private:
	void SetIcon(CBasePlayer* pPlayer, bool on);
};

//=========================================================
// Save/restore helpers -- implemented in player_infusion.cpp.
// CSave/CRestore are forward-declared so saverestore.h is not needed here.
//=========================================================
class CSave;
class CRestore;

bool InfusionSave(CPlayerInfusion& infusion, CSave& save);
bool InfusionRestore(CPlayerInfusion& infusion, CRestore& restore);

//=========================================================
// Sounds the Infusion uses.  Called from ClientPrecache().
//=========================================================
void InfusionPrecache();

//=========================================================
// Using a Health Syringe: starts an Infusion and makes its noise.
// Returns false if one is already running, in which case the
// Syringe is NOT spent.
//=========================================================
bool InfusionUseSyringe(CBasePlayer* pPlayer);

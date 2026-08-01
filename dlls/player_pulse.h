//=========================================================
// player_pulse.h
//
// The Pulse -- the HEV suit's brief defensive discharge.
//
// Pressing the Pulse raises a Shield for the Pulse Window,
// during which qualifying damage is negated, followed by a
// Recharge.  Suit hardware, not a Skill: anyone wearing the
// suit has it, and Skills only evolve it.
//
// Vocabulary follows CONTEXT.md.  The decisions that are not
// obvious from the code are recorded in:
//   docs/adr/0005-the-shield-negates-a-curated-damage-list.md
//   docs/adr/0006-the-discharge-vents-at-the-crosshair.md
//=========================================================

#pragma once

class CBasePlayer;

//=========================================================
// CPlayerPulse
//
// Stored directly inside CBasePlayer, next to m_skills.
//
// Note that "a Shield is up" and "a Recharge is running" are
// each their own bool rather than being inferred from a zeroed
// timer.  Half-Life's FIELD_TIME rebases on restore, so a zero
// stored as a sentinel comes back as gpGlobals->time -- which
// would read as "the window just ended" and start a phantom
// Recharge on every save load.
//=========================================================
struct CPlayerPulse
{
	// ---- Persistent data (save/restore) ----

	bool  m_bShieldUp       = false; // a Shield is standing right now
	float m_flShieldEndTime = 0;     // when it falls

	bool  m_bAbsorbed       = false; // this window has negated something

	bool  m_bRecharging     = false; // a Recharge is running
	float m_flReadyTime     = 0;     // when it completes

	// Rebounds banked. Spent when a window that deflected something closes,
	// which skips its Recharge entirely; all of them are restored by sitting
	// through a normal Recharge. A count rather than a bool so a later Skill
	// can raise the ceiling without reshaping any of this.
	int   m_iRebounds       = 0;

	// When the Follow-Up primed by a deflect expires. Zero means none is
	// primed -- safe as a sentinel here because it is only ever compared
	// against gpGlobals->time, and FIELD_TIME rebasing a zero to "now" reads
	// as already expired, which is exactly right.
	float m_flFollowUpUntil = 0;

	// ---- Transient ----

	// Guards against a Discharge causing damage that is itself negated and
	// discharges again -- venting into an explosive barrel at point blank
	// would otherwise recurse.  Deliberately not saved.
	bool m_bDischarging = false;

	// A deflect this frame, and what the player's punchangle was before the
	// attacker got to set it. Monsters assign punchangle *after* TakeDamage
	// has already refused the damage, so it cannot be suppressed at deflect
	// time -- only scaled back once the value is final. Deliberately not saved.
	bool m_bDampenPunch = false;
	Vector m_vecPunchBefore;

	// Last state pushed to the client, so only changes are sent.
	// -1 forces the next Think to send. Not saved: the client's HUD is reset
	// on load anyway, and ForgetSentState() re-syncs it.
	int m_iSentState = -1;

	// Takes the player because Rebounds start full, and how many that is
	// depends on their Skills.
	void Clear(CBasePlayer* pPlayer);

	// Makes the next sync send unconditionally.  Called when the client's HUD
	// is reset, so the bar cannot be left showing a stale state.
	void ForgetSentState() { m_iSentState = -1; }

	// A Shield is standing and has not yet fallen.
	bool ShieldActive() const { return m_bShieldUp; }

	// Nothing is standing and nothing is recharging.
	bool Ready() const { return !m_bShieldUp && !m_bRecharging; }

	// Would a Shield turn this damage away right now?  Pure query, no side
	// effects -- CBasePlayer::TraceAttack asks before spawning blood, because
	// blood is spawned before the damage reaches TakeDamage to be negated.
	bool WouldNegate(int bitsDamageType) const;

	// ---- The three entry points ----

	// Raises a Shield if the player has the suit and is Ready.
	// Returns false (and plays the denied sound) otherwise.
	bool TryPulse(CBasePlayer* pPlayer);

	// Closes a Shield whose window has run out and starts the Recharge,
	// then completes the Recharge when its time comes.  Call each frame.
	void Think(CBasePlayer* pPlayer);

	// Answers "does the Shield turn this away?".  When it does, this has
	// already fired the Discharge and made its noise -- the caller only has
	// to refuse the damage.
	bool TryNegate(CBasePlayer* pPlayer, float flDamage, int bitsDamageType);

	// Scales back the view kick an attacker applied for a blow the Shield
	// turned away. Call once per frame from UpdateClientData, which runs after
	// every entity has thought and so sees the final punchangle -- calling it
	// any earlier gets the value a frame before the attacker writes it.
	void DampenDeflectPunch(CBasePlayer* pPlayer);

private:
	// Pushes the current state to the client if it has changed.
	void SyncClient(CBasePlayer* pPlayer);
};

//=========================================================
// What the client's Pulse bar is showing.  Sent as a byte;
// mirrored in cl_dll/hud_pulse.cpp.
//=========================================================
enum EPulseState
{
	PULSE_READY = 0,      // armed, nothing running
	PULSE_SHIELD = 1,     // a Shield is standing
	PULSE_RECHARGING = 2, // waiting to be armed again
};

//=========================================================
// Save/restore helpers -- implemented in player_pulse.cpp.
// CSave/CRestore are forward-declared so saverestore.h is not needed here.
//=========================================================
class CSave;
class CRestore;

bool PulseSave(CPlayerPulse& pulse, CSave& save);
bool PulseRestore(CPlayerPulse& pulse, CRestore& restore);

//=========================================================
// Sprites and sounds the Pulse uses.  Called from ClientPrecache().
//=========================================================
void PulsePrecache();

//=========================================================
// The Follow-Up: a crowbar swing empowered by having just deflected.
//
// Split in two because the damage has to be scaled BEFORE the hit is dealt and
// the knockback applied AFTER -- the target may not survive the hit, and a
// corpse is still worth throwing.
//
// Both live here rather than in crowbar.cpp so the Pulse's rules and its cvars
// stay in one place; crowbar.cpp only calls them, from inside its existing
// server-only block.
//=========================================================
class CBaseEntity;

// Scales flDamage if a Follow-Up is primed and the Skill is held, consuming it.
// Returns true if it fired, which is the caller's cue to knock the target back.
bool PulseTakeCrowbarFollowUp(CBasePlayer* pPlayer, float& flDamage);

// Throws the target, if it is something worth throwing. Call after the damage
// has been applied, so it works on a body as well as on a live one.
void PulseCrowbarFollowUpKnockback(CBaseEntity* pTarget, const Vector& vecDir);

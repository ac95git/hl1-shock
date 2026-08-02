//=========================================================
// player_regen.h
//
// Passive regeneration from the Regeneration and Battery Regen
// Skills.
//
// Deliberately NOT an Infusion, and CONTEXT.md says so: an
// Infusion is started, runs on a clock, shows an icon and ends.
// This has no duration, no icon and no start -- it is simply true
// while the Skill is held.  It borrows the Infusion's fractional
// accumulator and nothing else.
//=========================================================

#pragma once

class CBasePlayer;

//=========================================================
// CPlayerRegen
//
// Stored directly inside CBasePlayer, next to m_infusion.
//
// One tick clock, two accumulators.  Health and armour are
// separate Skills and separate rates, but sharing the clock keeps
// them landing on the same beat rather than drifting against each
// other, which would read as two unrelated effects.
//=========================================================
struct CPlayerRegen
{
	// ---- Persistent data (save/restore) ----

	float m_flNextTick    = 0; // FIELD_TIME, so it rebases across a save

	// Fractional health/armour carried between ticks.  The rates are cvars and
	// the interval is fixed, so a rate that does not divide evenly into whole
	// points per tick would otherwise be silently rounded away -- at a slow
	// rate that means rounded away to nothing.
	float m_flHealthAccum = 0;
	float m_flArmorAccum  = 0;

	void Clear();

	// Lands whatever regeneration is due.  Call each frame.
	void Think(CBasePlayer* pPlayer);
};

//=========================================================
// Save/restore helpers -- implemented in player_regen.cpp.
// CSave/CRestore are forward-declared so saverestore.h is not needed here.
//=========================================================
class CSave;
class CRestore;

bool RegenSave(CPlayerRegen& regen, CSave& save);
bool RegenRestore(CPlayerRegen& regen, CRestore& restore);

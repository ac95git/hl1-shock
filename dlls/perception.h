/***
*
*	Perception -- how a monster comes to notice the player.
*
*	The whole model, and the base SDK behaviour it sits on top of, is in
*	docs/PERCEPTION.md.  Where the meter gates and why nothing downstream of
*	acquisition changes is docs/adr/0009-suspicion-gates-the-relationship-bits.md.
*
*	In one paragraph: Concealment is what the player IS, Suspicion is what a
*	monster HOLDS.  Concealment sets the rate at which Suspicion fills; it does
*	not decide whether it fills.  When a monster's Suspicion reaches the
*	acquisition threshold, everything below that line happens exactly as
*	Half-Life already does it.
*
****/

#pragma once

class CBaseEntity;
class CBaseMonster;

/**
*	@brief How well one KIND of monster perceives.
*
*	Reached through CBaseMonster::GetPerceptionProfile(), a virtual rather than
*	a member set in Spawn(), for the two reasons CanBackstab() is one: Spawn()
*	does not re-run on restore (dlls/cbase.cpp:380 -- only FCAP_MUST_SPAWN
*	entities get one), so anything set there and not saved comes back
*	default-constructed after every load; and a profile is a property of the
*	monster's type that can never differ between two instances, so paying
*	save-game bytes for it would be wrong twice over.
*
*	The scales are constants rather than cvars because the RATIO between two
*	profiles is a design statement -- a grunt notices sooner than a zombie --
*	while the absolute rate is the tuning knob, and that lives in suspicion_fill
*	and suspicion_drain.  Promote them if the ratio itself needs dialling in.
*/
struct PerceptionProfile
{
	float flFillScale;	// scales the rate Suspicion fills at.  Higher notices sooner.
	float flDrainScale; // scales the rate it drains at.  Higher forgets sooner.

	/**
	*	@brief false means this monster ignores the meter entirely and acquires
	*	the player the instant it sees them, exactly as the base game does.
	*	For the things that should never be sneaked past.  "Dumber" is a worse
	*	profile, never a bypass -- a dark room works on a zombie too, just less.
	*/
	bool bUsesSuspicion;
};

// The profiles themselves.  Defined here as C++17 inline variables rather than
// in perception.cpp because GetPerceptionProfile() is an inline virtual, so the
// CLIENT dll -- which compiles the weapon sources and therefore emits
// CBaseMonster's vtable -- references the default one and would otherwise fail
// to link against a definition that only exists in the server build.

//! The conservative default.  Every monster gets this unless it says otherwise.
inline constexpr PerceptionProfile g_ProfileDefault{1.0f, 1.0f, true};

//! Quicker to notice, slower to forget.  Human grunts, assassins, alien grunts
//! and alien slaves -- the four primaries stealth is tuned against.
inline constexpr PerceptionProfile g_ProfileTrained{1.5f, 0.5f, true};

//! Machines, aircraft, and the things with no eyes to fool.  Vanilla acquisition.
inline constexpr PerceptionProfile g_ProfileAlwaysAware{1.0f, 1.0f, false};

/**
*	@brief Debug view.  Records one monster's live meter for the periodic
*	readout, and prints it when the interval is up.  Off unless debug_suspicion
*	is set.  Shares the screen centre with the debug_damage readout, so do not
*	run both at once.
*/
void DebugSuspicionNote(CBaseMonster* pMonster, float flConcealment);

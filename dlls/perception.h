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
*	@brief What the player is told about how well they are hidden.
*
*	Three states rather than a number, for two reasons.  A meter invites the
*	player to read the meter instead of the room, and a quantised value only
*	has to cross the wire when it changes -- three messages for a whole
*	approach rather than one every frame.  Mirrored by #defines in
*	cl_dll/hud_conceal.cpp.
*
*	It is a WARNING, not a mirror: it reports the highest Suspicion held by any
*	monster that can currently perceive the player, including monsters the
*	player cannot see.  Being told you have been noticed by something behind you
*	is the entire value of it.
*/
enum EConcealState
{
	CONCEAL_UNSEEN = 0, //!< nothing is filling a meter on you
	CONCEAL_NOTICED,	//!< something is past suspicion_notice -- break contact now
	CONCEAL_SPOTTED,	//!< something has acquired you, or is about to
};

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

	/**
	*	@brief true means this monster hears a Disturbance -- a body -- and can
	*	be sent to look at it.  Soldiers, not zombies: bodies mean something to
	*	the four Trained primaries and nothing to a headcrab, and keeping the
	*	rest deaf keeps the 64-entry sound pool clear during a Xen fight.  A
	*	future NPC opts in by picking a profile and nothing else.
	*/
	bool bListensForDisturbance;
};

// The profiles themselves.  Defined here as C++17 inline variables rather than
// in perception.cpp because GetPerceptionProfile() is an inline virtual, so the
// CLIENT dll -- which compiles the weapon sources and therefore emits
// CBaseMonster's vtable -- references the default one and would otherwise fail
// to link against a definition that only exists in the server build.

//! The conservative default.  Every monster gets this unless it says otherwise.
inline constexpr PerceptionProfile g_ProfileDefault{1.0f, 1.0f, true, false};

//! Quicker to notice, slower to forget, and the only ones who go looking at a
//! body.  Human grunts, assassins, alien grunts and alien slaves -- the four
//! primaries stealth is tuned against.
inline constexpr PerceptionProfile g_ProfileTrained{1.5f, 0.5f, true, true};

//! Machines, aircraft, and the things with no eyes to fool.  Vanilla acquisition.
inline constexpr PerceptionProfile g_ProfileAlwaysAware{1.0f, 1.0f, false, false};

class CBasePlayer;

/**
*	@brief Debug view.  Centre-prints the monster under the player's crosshair
*	-- state, squad role, schedule and task, meter, Concealment and floor --
*	four times a second while debug_schedule is set, plus the last kill and
*	the last Search dispatch for a few seconds after each.  For watching a
*	Search happen.  Same screen centre as the other debug_* readouts; run one
*	at a time.  Replaced debug_suspicion on 2026-09-17.
*/
void DebugScheduleReport(CBasePlayer* pPlayer);

//! The two events the readout carries; each also goes to the console.
void DebugScheduleNoteKill(CBaseMonster* pVictim, int cWitnesses, bool bSilentKill);
void DebugScheduleNoteSearch(CBaseMonster* pDispatcher, CBaseMonster* pSearcher);

/**
*	@brief The Concealment Stat nodes' multiplier on the rate a monster's
*	Suspicion fills, 1.0 with none held.  Broken out of UpdateSuspicion so the
*	Status page can report the same number a monster's fill rate actually
*	uses, rather than recomputing it from the raw Stat count.  Never negative
*	-- a player cannot make themselves fill a monster's meter backwards.
*/
float PlayerConcealmentScale(CBasePlayer* pPlayer);

/**
*	@brief The name of the SDK's investigate schedule, which is the Search.
*	Compared by name rather than by pointer because the table lives in
*	defaultai.cpp and the Search is recognised from three other files.
*/
inline constexpr const char* SEARCH_SCHEDULE_NAME = "InvestigateSound";

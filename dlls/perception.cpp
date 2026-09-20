/***
*
*	Perception -- Concealment, Suspicion, and the Perception Profile.
*
*	See perception.h for the model in a paragraph, docs/PERCEPTION.md for the
*	whole of it, and docs/adr/0009-suspicion-gates-the-relationship-bits.md for
*	why the gate sits where it does.
*
*	The gate itself is three lines in CBaseMonster::Look (dlls/monsters.cpp).
*	Everything that decides what those lines answer is here.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "scripted.h"
#include "player.h"
#include "game.h"
#include "perception.h"
#include "soundent.h"
#include "squadmonster.h"
#include "UserMessages.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

// The profiles themselves live in perception.h -- see the note there on why.

//=========================================================
// Interpolate from flAtZero (at flFrac 0) to 1 (at flFrac 1).
//
// Every Concealment term has this shape: a worst case that is never zero, so
// that no single term can make the player literally impossible to see, rising
// to full exposure.  A term that could reach zero would make the product zero
// and stealth absolute, which is a bug rather than a build.
//=========================================================
static float ExposureTerm(float flAtZero, float flFrac)
{
	if (flFrac < 0.0f)
		flFrac = 0.0f;
	else if (flFrac > 1.0f)
		flFrac = 1.0f;

	if (flAtZero < 0.0f)
		flAtZero = 0.0f;
	else if (flAtZero > 1.0f)
		flAtZero = 1.0f;

	return flAtZero + (1.0f - flAtZero) * flFrac;
}

//=========================================================
// PlayerConcealmentScale -- the Concealment Stat nodes' multiplier on the
// rate a monster's Suspicion fills.  Read by UpdateSuspicion, and exposed
// here so the Status page can show the player the same number later.
//=========================================================
float PlayerConcealmentScale(CBasePlayer* pPlayer)
{
	if (!pPlayer)
		return 1.0f;

	const int iCount = pPlayer->m_skills.CountStat(EStat::Concealment);
	float flScale = 1.0f - iCount * skill_stat_concealment.value;

	// Shroud: the roads over again in one node, multiplied rather than added
	// so it is worth the same whatever else is held.
	if (pPlayer->m_skills.HasSkill(ESkillId::Shroud))
		flScale *= std::max(0.0f, skill_shroud_scale.value);

	return flScale > 0.0f ? flScale : 0.0f;
}

//=========================================================
// SuspicionJump -- set the meter to at least flValue.  A witness's 0.75, and
// (from 5b) the give-up's.  Never lowers it, and never touches the floor:
// the floor is a separate write by whoever decides the room is primed.
//=========================================================
void CBaseMonster::SuspicionJump(float flValue)
{
	if (flValue > 1.0f)
		flValue = 1.0f;

	if (m_flSuspicion < flValue)
		m_flSuspicion = flValue;

	m_flSuspicionTime = gpGlobals->time;
}

//=========================================================
// SuspicionNoticePropagate -- 5e, the captain's channel (docs/PERCEPTION.md,
// "The captain's channel", built 2026-09-20).  Called on the member whose own
// meter just crossed suspicion_notice.  Its leader lifts every living member
// below the line up to it -- a lift, never a lowering, through SuspicionJump
// -- and says so.  Loners have no leader and never do this; a squad whose
// leader has died is loners from that frame (SquadRemove nulls every handle
// and the SDK has no promotion), which is what makes the captain the first
// target.  No floor is set: the room is primed only by a kill.
//=========================================================
void CBaseMonster::SuspicionNoticePropagate()
{
	CSquadMonster* pSquad = MySquadMonsterPointer();

	if (pSquad == NULL || !pSquad->InSquad())
		return;

	CSquadMonster* pLeader = pSquad->MySquadLeader();

	if (pLeader == NULL || !pLeader->IsLeader() || !pLeader->IsAlive())
		return;

	int iLifted = 0;

	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		CSquadMonster* pMember = pLeader->MySquadMember(i);

		if (pMember == NULL || pMember == pSquad || !pMember->IsAlive())
			continue;
		if (pMember->m_IdealMonsterState == MONSTERSTATE_DEAD)
			continue;

		if (pMember->m_flSuspicion < suspicion_notice.value)
		{
			pMember->SuspicionJump(suspicion_notice.value);
			iLifted++;
		}
	}

	if (debug_schedule.value != 0)
	{
		ALERT(at_console, "captain: %s crossed notice, leader lifts %d\n",
			STRING(pev->classname), iLifted);
	}

	if (iLifted > 0)
		pLeader->OnSquadAlerted();
}

//=========================================================
// ConcealmentOf -- how hidden pTarget is from THIS monster right now.
// 0 is fully exposed, 1 is invisible.
//
// Four terms, each an exposure fraction, multiplied.  Multiplicative rather
// than additive so that being good at one thing cannot be undone by being bad
// at another: crouching, in the dark, at the edge of the cone compounds into
// something far better than any one of them.
//=========================================================
float CBaseMonster::ConcealmentOf(CBaseEntity* pTarget)
{
	if (!pTarget)
		return 1.0f;

	const Vector vecTarget = pTarget->pev->origin;
	const Vector vecDelta = vecTarget - pev->origin;

	// --- Angle -------------------------------------------------------------
	// Measured against this monster's OWN cone, so a 157-degree grunt and a
	// 120-degree zombie each get the term scaled to what they can actually
	// see.  Computed the same 2D way FInViewCone does, and deliberately
	// without UTIL_MakeVectors: Look calls FInViewCone immediately before
	// this, and stomping gpGlobals->v_forward from underneath it is exactly
	// the trap FInRearArc was written to avoid.
	const float flYaw = pev->angles.y * (M_PI / 180.0f);
	const Vector2D vec2Forward(cos(flYaw), sin(flYaw));

	Vector2D vec2LOS = vecDelta.Make2D();
	vec2LOS = vec2LOS.Normalize();

	const float flDot = DotProduct(vec2LOS, vec2Forward);

	// m_flFieldOfView is a dot product and can be -1 (360 degrees), so the
	// span is never zero and this cannot divide by it.
	const float flEdge = m_flFieldOfView < 0.99f ? m_flFieldOfView : 0.99f;
	float flExposure = ExposureTerm(conceal_angle_edge.value, (flDot - flEdge) / (1.0f - flEdge));

	// --- Distance ----------------------------------------------------------
	// As a fraction of this monster's own sight range, for the same reason.
	const float flLook = m_flDistLook > 1.0f ? m_flDistLook : 1.0f;
	flExposure *= ExposureTerm(conceal_dist_far.value, 1.0f - (vecDelta.Length() / flLook));

	// --- Stance ------------------------------------------------------------
	// The deliberate half of the noise model's accident.  UpdatePlayerSound
	// already makes slow movement quiet as a side effect of velocity; this
	// makes it hard to see as a decision.  Standing still reads as walking on
	// purpose -- a player who has stopped moving is not the one giving
	// themselves away.
	if ((pTarget->pev->flags & FL_DUCKING) != 0)
	{
		flExposure *= conceal_stance_duck.value;
	}
	else
	{
		const float flRunSpeed = pTarget->pev->maxspeed > 0.0f ? pTarget->pev->maxspeed : 320.0f;
		if (pTarget->pev->velocity.Length2D() <= flRunSpeed * 0.5f)
			flExposure *= conceal_stance_walk.value;
	}

	// --- Light -------------------------------------------------------------
	// Wired from the first commit and deliberately not dominant: vanilla maps
	// are lit for readability rather than for hiding, so this term does almost
	// nothing until there are maps with dark places in them (docs/ROADMAP.md,
	// Maps).  The muzzle flash arrives free -- CBasePlayer::Illumination()
	// already folds m_iWeaponFlash in, so firing in the dark lights the player
	// for about a second and nothing here has to know that.
	//
	// Nightfall halves the dark end's worst-case exposure rather than adding a
	// second term, so "darkness conceals twice as much" stays exactly what the
	// light term already means -- a Skill tunes the existing knob, it does not
	// open a new one.
	float flDarkEnd = conceal_light_dark.value;

	if (pTarget->IsPlayer())
	{
		CBasePlayer* pPlayerTarget = static_cast<CBasePlayer*>(pTarget);

		if (pPlayerTarget->m_skills.HasSkill(ESkillId::Nightfall))
			flDarkEnd *= skill_nightfall_scale.value;
	}

	flExposure *= ExposureTerm(flDarkEnd, pTarget->Illumination() / 255.0f);

	if (flExposure < 0.0f)
		flExposure = 0.0f;
	else if (flExposure > 1.0f)
		flExposure = 1.0f;

	return 1.0f - flExposure;
}

//=========================================================
// UpdateSuspicion -- advance the meter one think, and answer whether the
// player may be acquired.
//
// pTarget is the player when this monster can currently see one and is
// hostile to them, NULL when it cannot.  Called exactly once per Look either
// way, because a meter that only moves while the player is visible would
// never drain.
//=========================================================
bool CBaseMonster::UpdateSuspicion(CBaseEntity* pTarget)
{
	const PerceptionProfile& profile = GetPerceptionProfile();

	// The master switch and the opt-out profiles share a path: pin the meter
	// full so that flipping either at runtime cannot leave a stale value
	// behind, and answer yes.  This is vanilla acquisition, restored exactly.
	if (suspicion_enable.value == 0 || !profile.bUsesSuspicion)
	{
		m_flSuspicion = 1.0f;
		m_flSuspicionTime = gpGlobals->time;
		return true;
	}

	// A mapper can say a set piece has to fire.  Half-Life's pacing leans on
	// monsters spawning into a fight that is going to happen, and authored
	// intent has to be able to win.
	if ((pev->spawnflags & SF_MONSTER_IGNORE_CONCEALMENT) != 0)
	{
		m_flSuspicion = 1.0f;
		m_flSuspicionTime = gpGlobals->time;
		return true;
	}

	const float flNow = gpGlobals->time;

	// Look runs once per think, and think intervals vary by monster and by
	// state, so the meter advances against real time rather than per call.
	// Clamped at both ends: a monster outside the player's PVS does not run
	// Look at all (dlls/monsterstate.cpp:82) and would otherwise come back
	// with an arbitrarily large step, and a restore can hand back a stamp in
	// the future.
	float flDelta = flNow - m_flSuspicionTime;
	if (flDelta < 0.0f)
		flDelta = 0.0f;
	else if (flDelta > 0.5f)
		flDelta = 0.5f;

	m_flSuspicionTime = flNow;

	// Freeze while scripted: performing, not perceiving.  The gate still
	// answers from whatever the meter already holds, so a script that wants a
	// monster hostile can still have one, and when the script releases it the
	// normal model applies from where it left off.
	if (m_pCine != NULL)
		return m_flSuspicion >= suspicion_acquire.value;

	// An acquired enemy pins the meter full, seen or not.
	//
	// This is the line that keeps the model PRE-ACQUISITION ONLY, and it is
	// load-bearing rather than lazy: past this point the meter has no effect on
	// anything, so everything downstream of acquisition is byte-for-byte the
	// base game.  Dropping an enemy is de-escalation, which is deferred to the
	// post-aggro step along with the rest of the combat work -- see
	// docs/ROADMAP.md.  The meter must not quietly become a give-up timer under
	// a monster that is actively being shot at.
	if (m_hEnemy != NULL && m_hEnemy->IsPlayer())
	{
		m_flSuspicion = 1.0f;
		return true;
	}

	if (pTarget != NULL)
	{
		const float flConcealment = ConcealmentOf(pTarget);

		// The Concealment Stat nodes scale only the player's OWN fill rate,
		// scoped by IsPlayer() the same way the rest of Suspicion is
		// (adr/0009) -- a Skill on the player never touches how fast a
		// monster learns about some other hostile.
		float flFillScale = profile.flFillScale;

		if (pTarget->IsPlayer())
			flFillScale *= PlayerConcealmentScale(static_cast<CBasePlayer*>(pTarget));

		const float flBefore = m_flSuspicion;

		m_flSuspicion += (1.0f - flConcealment) * suspicion_fill.value * flFillScale * flDelta;

		if (m_flSuspicion > 1.0f)
			m_flSuspicion = 1.0f;

		m_bSuspicionHadTarget = true;

		// 5e, the captain's channel: the moment a squad member crosses the
		// notice line by its own eyes, its leader lifts the rest of the squad
		// to that line.  The crossing, not the level, so a member the leader
		// lifted (which sits exactly at the line) does not fire it again.
		if (flBefore < suspicion_notice.value && m_flSuspicion >= suspicion_notice.value)
			SuspicionNoticePropagate();
	}
	else
	{
		// Slip Away -- the seen -> not-seen transition, caught here because
		// this is the one place that transition is visible: pTarget goes NULL
		// the instant Look stops passing this monster one.  Checked before the
		// ordinary drain below so the drop is a step the player can see happen,
		// not folded invisibly into this frame's decay.  The early returns
		// above (scripted, acquired, the opt-out profiles) leave the flag
		// holding its last value rather than clearing it, so a monster that
		// saw the player, was scripted for a while, and comes out of the
		// script without them fires this on that first unseen Look -- a
		// break of contact by any reading.  The acquired branch never reaches
		// here at all today: nothing drops an enemy until the post-aggro step.
		if (m_bSuspicionHadTarget && m_flSuspicion >= suspicion_notice.value && m_flSuspicion < suspicion_acquire.value)
		{
			// UTIL_PlayerByIndex(1): this mod is single-player, and the only
			// entity that could have been the seen target is player 1.
			CBaseEntity* pPlayer = UTIL_PlayerByIndex(1);

			if (pPlayer != NULL && pPlayer->IsPlayer() && static_cast<CBasePlayer*>(pPlayer)->m_skills.HasSkill(ESkillId::SlipAway))
			{
				const float flBefore = m_flSuspicion;
				float flFraction = 1.0f - skill_slip_away_fraction.value;

				if (flFraction < 0.0f)
					flFraction = 0.0f;

				m_flSuspicion *= flFraction;

				if (debug_schedule.value != 0)
				{
					ALERT(at_console, "Slip Away: %s suspicion %.2f -> %.2f\n",
						STRING(pev->classname), flBefore, m_flSuspicion);
				}
			}
		}

		m_flSuspicion -= suspicion_drain.value * profile.flDrainScale * flDelta;

		// The floor: a monster that has seen a kill (or given up a chase)
		// never forgets all the way for the rest of the level.  Applied to
		// the drain only -- a fresh monster has a floor of 0 and this is a
		// no-op for it.
		if (m_flSuspicion < m_flSuspicionFloor)
			m_flSuspicion = m_flSuspicionFloor;

		if (m_flSuspicion < 0.0f)
			m_flSuspicion = 0.0f;

		m_bSuspicionHadTarget = false;
	}

	return m_flSuspicion >= suspicion_acquire.value;
}

//=========================================================
// PerceptionOnKilled -- the cost of a kill.  docs/PERCEPTION.md, "Death,
// witnesses, and the Disturbance".
//
// Player-dealt kills only, for the same reason the damage rule and the gate
// are scoped that way -- and more so here: every marines-versus-aliens set
// piece would otherwise fill the 64-entry sound pool with grunts searching
// bodies they shot themselves.
//
// Two mechanisms doing two jobs.  SEEING the kill is a direct write, once, to
// whoever had a line to the victim as it died.  FINDING the body is a sound
// that lasts a while and that only the Trained profiles hear; the Search
// that answers it is dispatched from GetSchedule when it is heard.
//
// Called from CBaseMonster::Killed, so THIS is the victim and its origin is
// still where it fell.  The victim's own meter is what it was before the
// killing hit -- TakeDamage calls Killed before SuspicionFromDamage -- which
// is the same reading Ambush took on the way in.
//=========================================================
void CBaseMonster::PerceptionOnKilled(entvars_t* pevAttacker)
{
	if (!pevAttacker || suspicion_enable.value == 0)
		return;

	CBaseEntity* pAttacker = CBaseEntity::Instance(pevAttacker);

	if (!pAttacker || !pAttacker->IsPlayer())
		return;

	CBasePlayer* pPlayer = static_cast<CBasePlayer*>(pAttacker);

	// --- Seeing the kill ---------------------------------------------------
	// A witness is any hostile with a meter that can see the VICTIM, squad or
	// not.  The test is on the victim rather than the player on purpose: a
	// witness that saw the body drop but cannot see the player sits at 0.75
	// with nothing filling it, which is the "sufficiently concealed" case the
	// loop is made of.
	int cWitnesses = 0;

	CBaseEntity* pEnt = NULL;
	while ((pEnt = UTIL_FindEntityInSphere(pEnt, pev->origin, 2048)) != NULL)
	{
		if (pEnt == this || pEnt->IsPlayer())
			continue;

		CBaseMonster* pMonster = pEnt->MyMonsterPointer();

		if (!pMonster || !pMonster->IsAlive())
			continue;

		// Killed this same frame -- a grenade's other victims -- and still
		// passing IsAlive until its death task runs.  Not a witness.
		if (pMonster->m_IdealMonsterState == MONSTERSTATE_DEAD)
			continue;

		// A monster that does not run the meter has nothing to jump; a
		// scripted one is performing, not perceiving; one already hunting
		// the player is pinned full anyway.
		if (!pMonster->GetPerceptionProfile().bUsesSuspicion)
			continue;
		if ((pMonster->pev->spawnflags & SF_MONSTER_IGNORE_CONCEALMENT) != 0)
			continue;
		if (pMonster->m_pCine != NULL)
			continue;
		if (pMonster->m_hEnemy == pPlayer)
			continue;

		const int iRelationship = pMonster->IRelationship(pPlayer);

		if (iRelationship != R_NM && iRelationship != R_HT && iRelationship != R_DL)
			continue;

		if (!pMonster->FVisible(this))
			continue;

		pMonster->SuspicionJump(suspicion_witness.value);

		if (pMonster->m_flSuspicionFloor < suspicion_floor.value)
			pMonster->m_flSuspicionFloor = suspicion_floor.value;

		// It knows where the body is, and turns to it.  No enemy is set, so
		// the LKP is a note rather than a target until acquisition.
		pMonster->m_vecEnemyLKP = pev->origin;
		pMonster->MakeIdealYaw(pev->origin);

		pMonster->OnWitnessedKill();
		cWitnesses++;
	}

	// --- Finding the body --------------------------------------------------
	// Silent Kill is about the ears only: on a victim below Spotted no sound
	// is inserted, so a squadmate around the corner never knows.  A squadmate
	// in sight reacted in full above; soldiers are not blind, and the answer
	// to the one who saw is the silenced headshot.  The weapon's own noise is
	// untouched -- a gunshot is a sound that led to the kill, not one that
	// results from it.
	const bool bSilentKill = pPlayer->m_skills.HasSkill(ESkillId::SilentKill) &&
							 m_flSuspicion < suspicion_acquire.value;

	if (!bSilentKill)
	{
		CSoundEnt::InsertSound(bits_SOUND_DISTURBANCE, pev->origin,
			(int)disturbance_volume.value, disturbance_duration.value);
	}

	DebugScheduleNoteKill(this, cWitnesses, bSilentKill);
}

//=========================================================
// The Search -- who answers a Disturbance.
//
// A squad sends one: the leader picks its nearest free member, and the rest
// hold where they are, turned toward the body by the ordinary hear-and-turn.
// A loner goes itself, so a leaderless group arrives as a mob -- deliberate,
// docs/adr/0014.  Either way one Disturbance is answered once: the dispatcher
// remembers where it last sent someone and refuses the same spot again while
// the sound could still be in the list.
//=========================================================
bool CBaseMonster::IsSearching()
{
	return m_pSchedule != NULL && m_pSchedule->pName != NULL &&
		   0 == strcmp(m_pSchedule->pName, SEARCH_SCHEDULE_NAME);
}

static bool SearchAlreadyAnswered(const CBaseMonster* pDispatcher, const Vector& vecDisturbance)
{
	if (pDispatcher->m_flLastSearchTime <= 0.0f)
		return false;

	if (gpGlobals->time - pDispatcher->m_flLastSearchTime > disturbance_duration.value)
		return false;

	return (pDispatcher->m_vecLastSearch - vecDisturbance).Length() < 64.0f;
}

bool CBaseMonster::TryClaimSearch(const Vector& vecDisturbance)
{
	CSquadMonster* pSquad = MySquadMonsterPointer();

	if (pSquad == NULL || !pSquad->InSquad())
	{
		// A loner.  Goes itself, once per body.
		if (SearchAlreadyAnswered(this, vecDisturbance))
		{
			DebugScheduleNoteRefusal(this, "loner, already answered");
			return false;
		}

		m_vecLastSearch = vecDisturbance;
		m_flLastSearchTime = gpGlobals->time;
		m_vecSearchTarget = vecDisturbance;
		m_flSearchTargetTime = gpGlobals->time;
		return true;
	}

	CSquadMonster* pSent = pSquad->MySquadLeader()->SquadDispatchSearch(vecDisturbance, pSquad);

	if (pSent != pSquad)
		DebugScheduleNoteRefusal(this, pSent == NULL ? "squad, nobody sent" : "squad, another sent");

	return pSent == pSquad;
}

//=========================================================
// PAudibleSoundOfType -- a sound of the given type in this think's audible
// list, or NULL.  PBestSound answers "nearest", which is the wrong question
// for a body: the player's own footstep is often nearer.
//=========================================================
CSound* CBaseMonster::PAudibleSoundOfType(int iType)
{
	int iSound = m_iAudibleList;

	// The audible chain runs through fields on the shared sound pool that
	// every monster's Listen rewrites, so a stale list can loop.  Bounded by
	// the pool size: no valid chain is longer.
	for (int cSteps = 0; iSound != SOUNDLIST_EMPTY && cSteps < MAX_WORLD_SOUNDS; cSteps++)
	{
		CSound* pSound = CSoundEnt::SoundPointerForIndex(iSound);

		if (pSound == NULL)
			break;

		if ((pSound->m_iType & iType) != 0)
			return pSound;

		iSound = pSound->m_iNextAudible;
	}

	return NULL;
}

//=========================================================
// SuspicionFromDamage -- being shot is proof, not a hint.
//
// The second exception to "only sight fills the meter", and it is not in the
// design because the design did not anticipate the base game's behaviour here:
// CBaseMonster::TakeDamage sets the last known position and turns the monster
// toward the attack, but it never sets m_hEnemy.  Acquisition comes only
// through Look and GetEnemy.  So with the gate in place and nothing else, a
// monster the player shot would stand and take it for however long its meter
// needed -- seconds, in a dark room at range.
//
// Player-dealt only, for the same reason the gate is: every other hostile is
// acquired instantly anyway.
//=========================================================
void CBaseMonster::SuspicionFromDamage(entvars_t* pevAttacker)
{
	if (!pevAttacker)
		return;

	CBaseEntity* pAttacker = CBaseEntity::Instance(pevAttacker);

	if (!pAttacker || !pAttacker->IsPlayer())
		return;

	m_flSuspicion = 1.0f;
	m_flSuspicionTime = gpGlobals->time;
}

//=========================================================
// CBasePlayer::SyncConcealState -- the readout.
//
// Derived server-side from the highest Suspicion held by any monster that can
// currently perceive the player, INCLUDING monsters the player cannot see.
// That is the whole value of it: a warning rather than a mirror.  Being told
// something behind you has started to notice is information the player cannot
// get any other way, and it is what makes breaking contact a decision instead
// of a guess.
//
// Quantised to three states so the wire sees threshold crossings rather than a
// value every frame -- gmsgPulse's precedent.
//=========================================================
void CBasePlayer::SyncConcealState()
{
	if (gmsgConceal == 0)
		return;

	if (gpGlobals->time < m_flNextConcealThink)
		return;

	m_flNextConcealThink = gpGlobals->time + 0.1f;

	float flHighest = 0.0f;

	// m_flDistLook's default is 2048 and nothing in the SDK raises it, so
	// nothing outside that radius can have been filling a meter on us.
	CBaseEntity* pEnt = NULL;
	while ((pEnt = UTIL_FindEntityInSphere(pEnt, pev->origin, 2048)) != NULL)
	{
		if (pEnt == this || pEnt->IsPlayer())
			continue;

		CBaseMonster* pMonster = pEnt->MyMonsterPointer();

		if (!pMonster || !pMonster->IsAlive())
			continue;

		const int iRelationship = pMonster->IRelationship(this);

		if (iRelationship != R_NM && iRelationship != R_HT && iRelationship != R_DL)
			continue;

		// A monster that does not run the meter -- an opted-out profile, or the
		// whole model switched off -- sits pinned at 1.0 forever, so reading its
		// meter would report SPOTTED from a turret three rooms away.  Those
		// count only once they have actually taken the player as an enemy.
		const bool bUsesMeter = suspicion_enable.value != 0 && pMonster->GetPerceptionProfile().bUsesSuspicion;

		if (!bUsesMeter)
		{
			if (pMonster->m_hEnemy == this)
				flHighest = 1.0f;

			continue;
		}

		// Look does not run at all for a monster with no client in its PVS
		// (dlls/monsterstate.cpp:82), which means its meter is frozen rather
		// than draining.  A stale value is not perception, so require that the
		// monster has actually looked recently.  This is the filter that makes
		// "can currently perceive the player" true rather than approximate.
		if (gpGlobals->time - pMonster->m_flSuspicionTime > 0.5f)
			continue;

		if (pMonster->m_flSuspicion > flHighest)
			flHighest = pMonster->m_flSuspicion;
	}

	int iState = CONCEAL_UNSEEN;

	if (flHighest >= suspicion_acquire.value)
		iState = CONCEAL_SPOTTED;
	else if (flHighest >= suspicion_notice.value)
		iState = CONCEAL_NOTICED;

	if (iState == m_iConcealSentState)
		return;

	m_iConcealSentState = iState;

	MESSAGE_BEGIN(MSG_ONE, gmsgConceal, NULL, pev);
	WRITE_BYTE(iState);
	MESSAGE_END();
}

//=========================================================
// The debug view -- debug_schedule.
//
// Built alongside the model rather than after it, because a meter nobody can
// see is a meter nobody can tune -- and every number above is a first guess.
// The first version, debug_suspicion, printed the four highest meters in the
// room; it was decommissioned on 2026-09-17 for this one, which reads the
// monster under the crosshair instead and carries the two events the room
// view could not: the last kill (witnesses, Disturbance or not) and the last
// Search dispatch.  Throwaway diagnostic.
//=========================================================

//=========================================================
// Strip the "monster_" that every one of these classnames starts with, so
// a reading fits on a line of a centre print.
//=========================================================
static const char* ShortMonsterName(const char* pszClassname)
{
	if (0 == strncmp(pszClassname, "monster_", 8))
		return pszClassname + 8;

	return pszClassname;
}

// The two events, kept for a few seconds so they can be read after the
// moment has passed.  Whole-world, like the sound list they describe.
#define SCHEDULE_DEBUG_EVENT_SHOWN 8.0f

struct ScheduleDebugEvent
{
	float flTime;
	char szText[44];
};

static ScheduleDebugEvent g_LastKill;
static ScheduleDebugEvent g_LastSearch;

void DebugScheduleNoteKill(CBaseMonster* pVictim, int cWitnesses, bool bSilentKill)
{
	if (debug_schedule.value == 0 || !pVictim)
		return;

	g_LastKill.flTime = gpGlobals->time;
	snprintf(g_LastKill.szText, sizeof(g_LastKill.szText), "kill: %.10s, %d wit, %s",
		ShortMonsterName(STRING(pVictim->pev->classname)), cWitnesses,
		bSilentKill ? "silent" : "Disturbance");

	ALERT(at_console, "%s\n", g_LastKill.szText);
}

void DebugScheduleNoteSearch(CBaseMonster* pDispatcher, CBaseMonster* pSearcher)
{
	if (debug_schedule.value == 0 || !pDispatcher || !pSearcher)
		return;

	g_LastSearch.flTime = gpGlobals->time;
	snprintf(g_LastSearch.szText, sizeof(g_LastSearch.szText), "search: #%d -> #%d%s",
		pDispatcher->entindex(), pSearcher->entindex(),
		pDispatcher == pSearcher ? " (self)" : "");

	ALERT(at_console, "%s\n", g_LastSearch.szText);
}

// Why a monster that heard a body did not go.  Console only, and it repeats:
// hear-and-turn brings the monster back here every few seconds for as long
// as the body is audible.  Added 2026-09-18 for C7.
void DebugScheduleNoteRefusal(CBaseMonster* pMonster, const char* pszWhy)
{
	if (debug_schedule.value == 0 || !pMonster)
		return;

	ALERT(at_console, "no search: %s #%d, %s\n", ShortMonsterName(STRING(pMonster->pev->classname)),
		pMonster->entindex(), pszWhy);
}

//=========================================================
// DebugScheduleReport -- the monster under the crosshair, and the events.
//
// Built for watching the Search: who was sent, what it is running, which
// task it is on, and what its meter reads, without reading the AI console.
// Squad role is the thing the squad code never shows -- the beret marks a
// leader, nothing marks a loner, and loners are the ones that mob a body.
//=========================================================
void DebugScheduleReport(CBasePlayer* pPlayer)
{
	if (debug_schedule.value == 0 || !pPlayer)
		return;

	static float s_flNextPrint;

	// New map: the clock restarts, so a stale stamp would park this in the
	// future.  Same guard DebugSuspicionNote uses.
	if (s_flNextPrint > gpGlobals->time + 1.0f)
		s_flNextPrint = 0.0f;

	if (gpGlobals->time < s_flNextPrint)
		return;

	s_flNextPrint = gpGlobals->time + 0.25f;

	UTIL_MakeVectors(pPlayer->pev->v_angle);
	const Vector vecSrc = pPlayer->GetGunPosition();

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecSrc + gpGlobals->v_forward * 4096, dont_ignore_monsters, pPlayer->edict(), &tr);

	CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
	CBaseMonster* pMonster = pHit ? pHit->MyMonsterPointer() : NULL;

	if (pMonster != NULL && pMonster->IsPlayer())
		pMonster = NULL;

	// ClientPrint sends a user message and the engine caps one at 192 bytes.
	// Overflow does not truncate -- it drops the server with SZ_GetSpace.
	// Bounded by the buffer, so an extra field can only truncate.
	char szReport[176];
	szReport[0] = '\0';

	if (pMonster != NULL)
	{
		static const char* pStateNames[] = {"None", "Idle", "Combat", "Alert", "Hunt", "Prone", "Scripted", "PlayDead", "Dead"};
		const char* pszState = (int)pMonster->m_MonsterState < (int)ARRAYSIZE(pStateNames) ? pStateNames[pMonster->m_MonsterState] : "?";

		char szRole[16];
		CSquadMonster* pSquad = pMonster->MySquadMonsterPointer();
		if (pSquad == NULL)
			strcpy(szRole, "-");
		else if (!pSquad->InSquad())
			strcpy(szRole, "loner");
		else if (pSquad->IsLeader())
			snprintf(szRole, sizeof(szRole), "leader/%d", pSquad->SquadCount());
		else
			strcpy(szRole, "member");

		const char* pszSchedule = "no schedule";
		int iTask = -1;
		int cTasks = 0;
		if (pMonster->m_pSchedule)
		{
			pszSchedule = pMonster->m_pSchedule->pName ? pMonster->m_pSchedule->pName : "unnamed";
			Task_t* pTask = pMonster->GetTask();
			iTask = pTask ? pTask->iTask : -1;
			cTasks = pMonster->m_pSchedule->cTasks;
		}

		// Concealment as THIS monster computes it for the player right now,
		// which is what the old room view showed as c0.38.  Zero when the
		// monster is dead: a corpse computes nothing.
		const float flConcealment = pMonster->IsAlive() ? pMonster->ConcealmentOf(pPlayer) : 0.0f;

		snprintf(szReport, sizeof(szReport),
			"%.10s  %s  %s\n%.28s  #%d/%d task %d\nsusp %.2f  c%.2f  floor %.2f%s\n",
			ShortMonsterName(STRING(pMonster->pev->classname)), pszState, szRole,
			pszSchedule, pMonster->m_iScheduleIndex + 1, cTasks, iTask,
			pMonster->m_flSuspicion, flConcealment, pMonster->m_flSuspicionFloor,
			pMonster->m_hEnemy != NULL ? "  enemy" : "");

		// The nearest live Disturbance against this monster's ears: distance,
		// the range it can hear it at, and whether its last Listen did.  Added
		// 2026-09-18 for C7 -- leaderless grunts not walking to a body -- where
		// the question is whether they ever hear it at all.
		const Vector vecEar = pMonster->EarPosition();
		CSound* pNearestBody = NULL;
		float flNearestBody = 0.0f;

		for (int iSound = CSoundEnt::ActiveList(), cSteps = 0;
			 iSound != SOUNDLIST_EMPTY && cSteps < MAX_WORLD_SOUNDS; cSteps++)
		{
			CSound* pSound = CSoundEnt::SoundPointerForIndex(iSound);

			if (pSound == NULL)
				break;

			if ((pSound->m_iType & bits_SOUND_DISTURBANCE) != 0)
			{
				const float flDist = (pSound->m_vecOrigin - vecEar).Length();

				if (pNearestBody == NULL || flDist < flNearestBody)
				{
					pNearestBody = pSound;
					flNearestBody = flDist;
				}
			}

			iSound = pSound->m_iNext;
		}

		if (pNearestBody != NULL)
		{
			char szBody[48];
			snprintf(szBody, sizeof(szBody), "body %.0f/%.0f  heard %s\n",
				flNearestBody, pNearestBody->m_iVolume * pMonster->HearingSensitivity(),
				(pMonster->m_afSoundTypes & bits_SOUND_DISTURBANCE) != 0 ? "yes" : "no");
			strncat(szReport, szBody, sizeof(szReport) - strlen(szReport) - 1);
		}
	}

	// The events, while fresh, whether or not anything is under the crosshair.
	if (g_LastKill.flTime > 0.0f && gpGlobals->time - g_LastKill.flTime < SCHEDULE_DEBUG_EVENT_SHOWN)
	{
		strncat(szReport, g_LastKill.szText, sizeof(szReport) - strlen(szReport) - 1);
		strncat(szReport, "\n", sizeof(szReport) - strlen(szReport) - 1);
	}
	if (g_LastSearch.flTime > 0.0f && gpGlobals->time - g_LastSearch.flTime < SCHEDULE_DEBUG_EVENT_SHOWN)
	{
		strncat(szReport, g_LastSearch.szText, sizeof(szReport) - strlen(szReport) - 1);
	}

	if (szReport[0] != '\0')
		ClientPrint(pPlayer->pev, HUD_PRINTCENTER, szReport);
}


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
#include "game.h"
#include "perception.h"

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
	flExposure *= ExposureTerm(conceal_light_dark.value, pTarget->Illumination() / 255.0f);

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
		m_flSuspicion += (1.0f - flConcealment) * suspicion_fill.value * profile.flFillScale * flDelta;

		if (m_flSuspicion > 1.0f)
			m_flSuspicion = 1.0f;

		DebugSuspicionNote(this, flConcealment);
	}
	else
	{
		m_flSuspicion -= suspicion_drain.value * profile.flDrainScale * flDelta;

		if (m_flSuspicion < 0.0f)
			m_flSuspicion = 0.0f;
	}

	return m_flSuspicion >= suspicion_acquire.value;
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
// The debug view.
//
// Built alongside the meter rather than after it, because a meter nobody can
// see is a meter nobody can tune -- and every number in the model above is a
// first guess.  Throwaway diagnostic; delete it with debug_suspicion.
//
// Monsters think at different times and Look is called from each of them, so
// this cannot print from one place at one moment.  Instead each monster leaves
// its latest reading in a small table keyed by entity index, and whichever
// call finds the interval expired renders the whole table at once.
//=========================================================
#define SUSPICION_DEBUG_SLOTS 8
#define SUSPICION_DEBUG_INTERVAL 0.25f
#define SUSPICION_DEBUG_STALE 1.0f

struct SuspicionDebugEntry
{
	int iEntIndex;
	float flSuspicion;
	float flConcealment;
	float flStamp;
	char szName[20];
};

static SuspicionDebugEntry g_rgSuspicionDebug[SUSPICION_DEBUG_SLOTS];
static float g_flNextSuspicionPrint;

//=========================================================
// Strip the "monster_" that every one of these classnames starts with, so
// four readings fit on four lines of a centre print.
//=========================================================
static const char* ShortMonsterName(const char* pszClassname)
{
	if (0 == strncmp(pszClassname, "monster_", 8))
		return pszClassname + 8;

	return pszClassname;
}

static void SuspicionDebugPrint()
{
	CBaseEntity* pPlayer = UTIL_PlayerByIndex(1);

	if (!pPlayer)
		return;

	// Highest first, so the monster closest to acquiring the player is the one
	// at the top of the readout whichever order the table happens to be in.
	int rgOrder[SUSPICION_DEBUG_SLOTS];
	int cShown = 0;

	for (int i = 0; i < SUSPICION_DEBUG_SLOTS; i++)
	{
		if (g_rgSuspicionDebug[i].iEntIndex == 0 ||
			gpGlobals->time - g_rgSuspicionDebug[i].flStamp > SUSPICION_DEBUG_STALE)
			continue;

		int j = cShown++;
		while (j > 0 && g_rgSuspicionDebug[rgOrder[j - 1]].flSuspicion < g_rgSuspicionDebug[i].flSuspicion)
		{
			rgOrder[j] = rgOrder[j - 1];
			j--;
		}
		rgOrder[j] = i;
	}

	if (cShown == 0)
		return;

	if (cShown > 4)
		cShown = 4;

	char szReport[320];
	szReport[0] = '\0';

	for (int i = 0; i < cShown; i++)
	{
		const SuspicionDebugEntry& entry = g_rgSuspicionDebug[rgOrder[i]];

		char szBar[11];
		const int cFilled = (int)(entry.flSuspicion * 10.0f + 0.5f);
		for (int c = 0; c < 10; c++)
			szBar[c] = c < cFilled ? '=' : '.';
		szBar[10] = '\0';

		char szLine[80];
		snprintf(szLine, sizeof(szLine), "%-14s [%s] %.2f  cnc %.2f%s\n",
			ShortMonsterName(entry.szName),
			szBar,
			entry.flSuspicion,
			entry.flConcealment,
			entry.flSuspicion >= suspicion_notice.value ? "  NOTICED" : "");

		strncat(szReport, szLine, sizeof(szReport) - strlen(szReport) - 1);
	}

	ClientPrint(pPlayer->pev, HUD_PRINTCENTER, szReport);
}

void DebugSuspicionNote(CBaseMonster* pMonster, float flConcealment)
{
	if (debug_suspicion.value == 0 || !pMonster)
		return;

	const int iEntIndex = ENTINDEX(pMonster->edict());

	// gpGlobals->time restarts on a new map, which would otherwise park the
	// next print arbitrarily far in the future and leave every stale entry
	// looking fresh (time - stamp goes negative).  Start over instead.
	if (g_flNextSuspicionPrint > gpGlobals->time + SUSPICION_DEBUG_INTERVAL)
	{
		g_flNextSuspicionPrint = 0.0f;
		memset(g_rgSuspicionDebug, 0, sizeof(g_rgSuspicionDebug));
	}

	// One slot per monster, so a crowd cannot push the interesting reading out
	// of the table between prints.  Falling back to the stalest slot means the
	// monsters still being looked at survive and the ones that walked off do
	// not.
	int iSlot = -1;
	int iStalest = 0;

	for (int i = 0; i < SUSPICION_DEBUG_SLOTS; i++)
	{
		if (g_rgSuspicionDebug[i].iEntIndex == iEntIndex)
		{
			iSlot = i;
			break;
		}

		if (g_rgSuspicionDebug[i].flStamp < g_rgSuspicionDebug[iStalest].flStamp)
			iStalest = i;
	}

	if (iSlot == -1)
		iSlot = iStalest;

	SuspicionDebugEntry& entry = g_rgSuspicionDebug[iSlot];
	entry.iEntIndex = iEntIndex;
	entry.flSuspicion = pMonster->m_flSuspicion;
	entry.flConcealment = flConcealment;
	entry.flStamp = gpGlobals->time;
	strncpy(entry.szName, STRING(pMonster->pev->classname), sizeof(entry.szName) - 1);
	entry.szName[sizeof(entry.szName) - 1] = '\0';

	if (gpGlobals->time >= g_flNextSuspicionPrint)
	{
		g_flNextSuspicionPrint = gpGlobals->time + SUSPICION_DEBUG_INTERVAL;
		SuspicionDebugPrint();
	}
}

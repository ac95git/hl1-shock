/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "gamerules.h"

#include "skill_tuning.h"
#include <algorithm>

// This file is compiled into the client too, for weapon prediction. Melee
// Force, the Melee Damage Stat nodes and the Backstab node are decided
// server-side alone, so their cvars come from game.h under the guard; Melee
// Reach and Melee Speed are predicted, so they read through skill_tuning.h,
// which resolves the same cvar from either DLL.
#ifndef CLIENT_DLL
#include "game.h"
#endif


#define CROWBAR_BODYHIT_VOLUME 128
#define CROWBAR_WALLHIT_VOLUME 512

LINK_ENTITY_TO_CLASS(weapon_crowbar, CCrowbar);

void CCrowbar::Spawn()
{
	Precache();
	m_iId = WEAPON_CROWBAR;
	SET_MODEL(ENT(pev), "models/w_crowbar.mdl");
	m_iClip = -1;

	FallInit(); // get ready to fall down.
}


void CCrowbar::Precache()
{
	PRECACHE_MODEL("models/v_crowbar.mdl");
	PRECACHE_MODEL("models/w_crowbar.mdl");
	PRECACHE_MODEL("models/p_crowbar.mdl");
	PRECACHE_SOUND("weapons/cbar_hit1.wav");
	PRECACHE_SOUND("weapons/cbar_hit2.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod1.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod2.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod3.wav");
	PRECACHE_SOUND("weapons/cbar_miss1.wav");

	// Backstab cue. Placeholder -- see docs/ART_DEBT.md.
	PRECACHE_SOUND("debris/bustflesh1.wav");
	PRECACHE_SOUND("debris/bustflesh2.wav");

	m_usCrowbar = PRECACHE_EVENT(1, "events/crowbar.sc");

	// Cleave's sweep, client-side from an event like the katana's crescent;
	// the beam sprite it draws with is precached here because the client
	// cannot.  Its sound is the miss pitched down, precached above.
	m_usCleave = PRECACHE_EVENT(1, "events/cleave.sc");
	PRECACHE_MODEL("sprites/shockwave.spr");
}

bool CCrowbar::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 0;
	p->iId = WEAPON_CROWBAR;
	p->iWeight = CROWBAR_WEIGHT;
	return true;
}



bool CCrowbar::Deploy()
{
	return DefaultDeploy("models/v_crowbar.mdl", "models/p_crowbar.mdl", CROWBAR_DRAW, "crowbar");
}

#ifndef CLIENT_DLL
float CCrowbar::BaseDamage()
{
	return gSkillData.plrDmgCrowbar;
}

float CCrowbar::BackstabScale()
{
	// The roster's all-rounder takes the plain Backstab. A weapon that leans
	// on it (the knife) overrides this; the Backstab node multiplies whatever
	// this returns.
	return std::max(1.0f, backstab_damage_scale.value);
}

void CCrowbar::LeechHeal(float flDamage)
{
	// The figure is the swing's damage before Weapon Mastery and the
	// victim's hitgroup, which is what the player can reason about from the
	// tree; the exact number that landed is not known here. TakeHealth
	// refuses at full health, so a Leech at full costs nothing and does
	// nothing.
	if (!m_pPlayer->m_skills.HasSkill(ESkillId::Leech))
		return;
	const float flHeal = flDamage * std::max(0.0f, skill_leech_fraction.value);
	if (flHeal > 0.0f)
		m_pPlayer->TakeHealth(flHeal, DMG_GENERIC);
}

bool CCrowbar::ReprisalUnhurt(CBaseMonster* pVictim)
{
	// "Had not been hurt": at its full health, read before the hit lands.  A
	// monster whose spawn never set max_health reads as unhurt, which errs
	// toward the refill.
	return pVictim && pVictim->IsAlive() && pVictim->pev->health >= pVictim->pev->max_health;
}

void CCrowbar::ReprisalRefill(CBaseMonster* pVictim, bool bWasUnhurt)
{
	// Reprisal (Shinobi): a single melee hit that kills a monster nothing had
	// hurt refills a Dash.  A Backstab kill counts, a Cleave opener counts
	// once per victim, finishing a wounded grunt does not.
	if (!bWasUnhurt || !pVictim || pVictim->IsAlive())
		return;
	if (!m_pPlayer->m_skills.HasSkill(ESkillId::Reprisal))
		return;
	m_pPlayer->DashRefill();
}

int CCrowbar::HitDamageType(CBaseEntity* pVictim)
{
	// A monster takes the weapon's type; that is where a type means
	// something (plating, the Gargantua's filter, the Energy Route's bonus
	// to come). Everything else is struck: func_breakable keys its crowbar
	// rules on DMG_CLUB (instant on a crowbar-sensitive one, double damage),
	// and a blade through a crate should still be a blow.
	if (pVictim && pVictim->MyMonsterPointer() != nullptr)
		return SwingDamageType();
	return DMG_CLUB;
}

float CCrowbar::SwingDamage(bool bCleaveSwing)
{
	float flDamage = BaseDamage() * std::max(0.0f, BladeDamageScale());

	// Melee Force and the Melee Damage Stat nodes. Before the Follow-Up, so a
	// primed swing multiplies the already-stronger hit rather than a base one.
	// One function, so the Status page reads the same number.
	flDamage *= PlayerMeleeScale(m_pPlayer);

	// Cleave's scale, on every hit of a Cleave swing.
	if (bCleaveSwing)
		flDamage *= std::max(1.0f, cleave_damage_scale.value);

	return flDamage;
}

int CCrowbar::CleaveArc(const Vector& vecSrc, float flDamage, bool bBackstabNode)
{
	const float flRadius = std::max(32.0f, cleave_radius.value);
	const float flArcDot = cleave_arc_dot.value;

	// A primed Follow-Up multiplies every victim and is spent once, whether
	// the arc finds one victim or six. Its sound plays once too.
	const bool bFollowUp = m_pPlayer->m_skills.HasSkill(ESkillId::FollowUp) && PulseFollowUpPrimed(m_pPlayer);
	if (bFollowUp)
	{
		PulseSpendFollowUp(m_pPlayer);
		flDamage *= PulseFollowUpScale();
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, FollowUpSound(), 1.0, ATTN_NORM, 0, 90);
	}

	int nHit = 0;

	// gpGlobals->v_forward is still the aim from the swing; nothing in this
	// loop touches it.
	CBaseEntity* pEntity = nullptr;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != nullptr)
	{
		// Exactly what the line trace could have damaged: anything that takes
		// damage, a crate as much as a zombie. Not the player.
		if (pEntity == m_pPlayer || pEntity->pev->takedamage == DAMAGE_NO)
			continue;

		CBaseMonster* pMonster = pEntity->MyMonsterPointer();
		if (pMonster && !pMonster->IsAlive())
			continue;

		// In front, within the arc.  The point tested is the nearest point of
		// the victim's box to the eyes, not its centre: the drawn wave is the
		// region's edge, and a body the wave visibly reaches must be hit.  On
		// the centre, a monster at the edge or the side was touched by the
		// wave and missed by the test.  Inside the box altogether -- the
		// player is standing in it -- counts, straight ahead.
		Vector vecPoint;
		for (int k = 0; k < 3; ++k)
			vecPoint[k] = std::clamp(vecSrc[k], pEntity->pev->absmin[k], pEntity->pev->absmax[k]);
		Vector vecTo = vecPoint - vecSrc;
		const float flDist = vecTo.Length();
		if (flDist > flRadius)
			continue;
		if (flDist > 1.0f)
		{
			vecTo = vecTo / flDist;
			if (DotProduct(vecTo, gpGlobals->v_forward) < flArcDot)
				continue;
		}
		else
		{
			vecTo = gpGlobals->v_forward;
		}

		// And not behind a wall or another thing: a Cleave does not reach
		// through what it hits.
		TraceResult tr;
		UTIL_TraceLine(vecSrc, pEntity->Center(), dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);
		if (tr.flFraction < 1.0f && tr.pHit != pEntity->edict())
			continue;

		// The Backstab is per victim, monsters only: one of them can be
		// facing away.
		float flHit = flDamage;
		const bool bBackstab = pMonster && pMonster->CanBackstab() &&
			pMonster->FInRearArc(m_pPlayer->pev->origin, backstab_arc_dot.value);
		if (bBackstab)
		{
			flHit *= BackstabScale();
			if (bBackstabNode)
				flHit *= std::max(1.0f, skill_backstab_bonus_scale.value);
		}

		// Phantom: a Backstab KILL on a victim below Noticed buys the Shinobi
		// link's silent, faster window (a Backstab hit alone was the first
		// shape; Andrei tightened it to the kill on 2026-09-16).  The tier is
		// read off THIS victim's own meter before the hit, the same tier
		// Ambush reads (player_skills.cpp PlayerAmbushScale): uses the meter
		// at all (bUsesSuspicion), not opted out of it
		// (SF_MONSTER_IGNORE_CONCEALMENT).  Whether it was a kill is known
		// only after the damage, below.
		const bool bPhantomEligible = bBackstab && pMonster->GetPerceptionProfile().bUsesSuspicion &&
			!FBitSet(pMonster->pev->spawnflags, SF_MONSTER_IGNORE_CONCEALMENT) &&
			pMonster->m_flSuspicion < suspicion_notice.value &&
			m_pPlayer->m_skills.HasSkill(ESkillId::Phantom);

		DebugDamageDetail("cleave -> %s %.0f%s%s%s",
			STRING(pEntity->pev->classname), flHit,
			bBackstab ? "  xBACKSTAB" : "",
			bBackstab && bBackstabNode ? "  xNode" : "",
			bFollowUp ? "  xFollowUp" : "");

		const bool bUnhurt = ReprisalUnhurt(pMonster);

		ClearMultiDamage();
		pEntity->TraceAttack(m_pPlayer->pev, flHit, vecTo, &tr, HitDamageType(pEntity));
		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
		++nHit;

		// Leech, per living victim: a Cleave through a crowd heals for each.
		if (pMonster)
			LeechHeal(flHit);

		ReprisalRefill(pMonster, bUnhurt);

		// A Cleave can Backstab several victims in one swing; the first kill
		// that qualifies is enough to start the window, so this is a start
		// call with nothing to double-spend.
		if (bPhantomEligible && !pMonster->IsAlive())
			m_pPlayer->PhantomStart();

		// After the damage, so a headcrab the hit killed is still thrown.
		if (bFollowUp)
			PulseCrowbarFollowUpKnockback(pEntity, vecTo);
	}

	return nHit;
}
#endif

void CCrowbar::Holster()
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	SendWeaponAnim(CROWBAR_HOLSTER);
}


void FindHullIntersection(const Vector& vecSrc, TraceResult& tr, const Vector& mins, const Vector& maxs, edict_t* pEntity)
{
	int i, j, k;
	float distance;
	const Vector* minmaxs[2] = {&mins, &maxs};
	TraceResult tmpTrace;
	Vector vecHullEnd = tr.vecEndPos;
	Vector vecEnd;

	distance = 1e6f;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc) * 2);
	UTIL_TraceLine(vecSrc, vecHullEnd, dont_ignore_monsters, pEntity, &tmpTrace);
	if (tmpTrace.flFraction < 1.0)
	{
		tr = tmpTrace;
		return;
	}

	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < 2; j++)
		{
			for (k = 0; k < 2; k++)
			{
				vecEnd.x = vecHullEnd.x + minmaxs[i]->x;
				vecEnd.y = vecHullEnd.y + minmaxs[j]->y;
				vecEnd.z = vecHullEnd.z + minmaxs[k]->z;

				UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, pEntity, &tmpTrace);
				if (tmpTrace.flFraction < 1.0)
				{
					float thisDistance = (tmpTrace.vecEndPos - vecSrc).Length();
					if (thisDistance < distance)
					{
						tr = tmpTrace;
						distance = thisDistance;
					}
				}
			}
		}
	}
}


void CCrowbar::PrimaryAttack()
{
	if (!Swing(true))
	{
		SetThink(&CCrowbar::SwingAgain);
		pev->nextthink = gpGlobals->time + 0.1;
	}
}


float CCrowbar::CleaveRecovery(float flSpeed, float flStock)
{
	// The swipe is one length on every model that carries it, so the weapon's
	// own swing scale stays out of it; Melee Speed still applies, as it does
	// to every swing.  Both sides: this sets a predicted delay.
	const float flTime = g_tuneCleaveSwingTime.Value();
	if (flTime <= 0.0f)
		return flStock;
	return flTime * flSpeed;
}


void CCrowbar::Smack()
{
	DecalGunshot(&m_trHit, BULLET_PLAYER_CROWBAR);
}


void CCrowbar::SwingAgain()
{
	Swing(false);
}


bool CCrowbar::Swing(bool fFirst)
{
	bool fDidHit = false;

	TraceResult tr;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();

	// Melee Reach. Applied on BOTH sides. The server still decides whether a
	// hit landed; the client's copy of this trace only picks which swing
	// animation plays, and it has to reach as far as the server's or an unlocked
	// player sees a miss animation for a hit that landed.
	float flRange = 32.0f;
	if (m_pPlayer->m_skills.HasSkill(ESkillId::MeleeReach))
		flRange *= std::max(1.0f, g_tuneMeleeReach.Value());

	// Melee Speed. Both sides too: the delay it shortens is predicted. Applied
	// to the miss and the hit alike, so the Skill reads as "faster" whatever
	// the swing meets.
	float flSpeed = 1.0f;
	if (m_pPlayer->m_skills.HasSkill(ESkillId::MeleeSpeed))
		flSpeed = std::max(0.1f, g_tuneMeleeSpeed.Value());

	Vector vecEnd = vecSrc + gpGlobals->v_forward * flRange;

	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

#ifndef CLIENT_DLL
	if (tr.flFraction >= 1.0)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);
		if (tr.flFraction < 1.0)
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
			if (!pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict());
			vecEnd = tr.vecEndPos; // This is the point on the actual surface (the hull could have hit space)
		}
	}
#endif

	// Which swing this is.  Cleave-ready is known on both sides -- the server
	// from the cooldown's time, the client from the flag clientdata carries
	// (CBasePlayer::CleaveReady on each) -- so the predicted animation picks
	// the Cleave sequence on the same frame.  The Follow-Up's primed state is
	// still server-only, so on the client it stays false and the stock
	// animation plays, which is what its hook falls back to anyway.
	//
	// Cleave, the Melee major. While it is ready the swing IS the arc: spent
	// on the swing whether or not anything is there, so the player has to
	// see the difference without a live target (the swipe, the sweep and the
	// sound), and so a swarm is hit the moment it enters the arc rather than
	// once one of them lines up with the crowbar.
	// Never on a secondary swing: Cleave is the melee verb's major.
	const bool bCleaveSwing = fFirst && !IsSecondarySwing() && m_pPlayer->CleaveReady();
	bool bFollowUpSwing = false;

	if (fFirst)
	{
		// iparam1: the swing's own sequence, so the miss animation -- which
		// only the event plays -- is the Cleave's on a Cleave swing.
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usCrowbar,
			0.0, g_vecZero, g_vecZero, 0, 0,
			(bCleaveSwing && CleaveSequence() >= 0) ? CleaveSequence() : 0,
			0.0, 0, 0.0);
	}

#ifdef CLIENT_DLL
	// The client's share of the spend: clear the synced flag so a re-run of
	// these commands does not cleave again before the server's word arrives.
	if (bCleaveSwing)
		m_pPlayer->CleaveSpend();
#else
	bFollowUpSwing = PulseFollowUpPrimed(m_pPlayer);

	if (bCleaveSwing)
	{
		// The air shock and its sound, one event, to everyone including the
		// host: this block is not predicted. The region's radius and
		// half-angle ride along so the drawn wave dies where the hit test
		// ends, and the weapon's style picks its look.
		PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usCleave,
			0.0, m_pPlayer->pev->origin, m_pPlayer->pev->angles,
			std::max(32.0f, cleave_radius.value),
			acosf(std::max(-1.0f, std::min(1.0f, cleave_arc_dot.value))) * (180.0f / M_PI),
			CleaveSweepStyle(), 0, 0, 0);

		// The arc is the hit: everything in it, now, whatever the line trace
		// below finds. Then the cooldown, hit or not.
		CleaveArc(vecSrc, SwingDamage(true), m_pPlayer->m_skills.HasSkill(ESkillId::Backstab));
		m_pPlayer->CleaveSpend();
	}
#endif

	// The animation the swing plays when it lands: the stock three in turn,
	// or a roster weapon's own Follow-Up or Cleave swing when it has one.
	// The Follow-Up's wins when a swing is both.
	auto HitSequence = [&]() -> int
	{
		if (bFollowUpSwing && FollowUpSequence() >= 0)
			return FollowUpSequence();
		if (bCleaveSwing && CleaveSequence() >= 0)
			return CleaveSequence();
		switch (((m_iSwing++) % 2) + 1)
		{
		case 1:  return CROWBAR_ATTACK2HIT;
		case 2:  return CROWBAR_ATTACK3HIT;
		default: return CROWBAR_ATTACK1HIT;
		}
	};

	if (tr.flFraction >= 1.0)
	{
		if (fFirst)
		{
			// miss
			m_flNextPrimaryAttack = GetNextAttackDelay(bCleaveSwing ? CleaveRecovery(flSpeed, 0.5 * SwingDelayScale() * flSpeed) : 0.5 * SwingDelayScale() * flSpeed);

			// player "shoot" animation
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		}
	}
	else
	{
		SendWeaponAnim(HitSequence());

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

#ifndef CLIENT_DLL

		// hit
		fDidHit = true;
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		ClearMultiDamage();

		// Every swing does full damage. Valve halved any swing within about a
		// second of the last one ("subsequent swings do half"); the Melee Route
		// dropped that on 2026-09-13 because it made the tree unreadable (a
		// player checking Force sees 15, then 7), made a speed Skill dishonest
		// (it only ever bought faster half-hits), and keyed on the left-click
		// timer alone. Sustained melee damage roughly doubles for a player
		// holding the button; that is absorbed by base damage and swing time,
		// both cvars.
		float flDamage = SwingDamage(bCleaveSwing);
		const float flBaseDamage = BaseDamage();
		const bool bForce = m_pPlayer->m_skills.HasSkill(ESkillId::MeleeForce);
		const int iStat = m_pPlayer->m_skills.CountStat(EStat::MeleeDamage);

		// On a Cleave swing the arc has already dealt with everything that
		// takes damage, this entity included if it does; what is left of the
		// line trace is the wall response below.
		const bool bDamageHere = !bCleaveSwing && pEntity && pEntity->pev->takedamage != DAMAGE_NO;

		// The Backstab. Purely positional -- whether the victim has noticed the
		// player does not enter into it, so this lands mid-fight on anything
		// you can get behind. See docs/adr/0010-the-backstab-is-positional.md.
		//
		// Sits between Force and the Follow-Up for the same reason Force sits
		// before the Follow-Up: each stage multiplies the already-stronger hit,
		// so the biggest number the player can produce is every bonus at once
		// rather than whichever happens to be last.
		//
		// FInRearArc deliberately does not touch gpGlobals->v_forward, which
		// still holds the player's aim vector and is needed below.
		CBaseMonster* pVictim = bDamageHere ? pEntity->MyMonsterPointer() : nullptr;
		const bool bBackstab =
			pVictim &&
			pVictim->IsAlive() &&
			pVictim->CanBackstab() &&
			pVictim->FInRearArc(m_pPlayer->pev->origin, backstab_arc_dot.value);

		// The weapon's own Backstab base (the knife's lean, when it exists),
		// then the Backstab node on top of it.
		const bool bBackstabNode = bBackstab && m_pPlayer->m_skills.HasSkill(ESkillId::Backstab);
		if (bBackstab)
			flDamage *= BackstabScale();
		if (bBackstabNode)
			flDamage *= std::max(1.0f, skill_backstab_bonus_scale.value);

		// Phantom: the Shinobi link, on a Backstab KILL of an unaware victim.
		// Eligibility is read here, before the hit fills the meter; whether
		// the hit killed is known after ApplyMultiDamage below.  See
		// CleaveArc's copy of this check for why the three tests.
		const bool bPhantomEligible = bBackstab && pVictim->GetPerceptionProfile().bUsesSuspicion &&
			!FBitSet(pVictim->pev->spawnflags, SF_MONSTER_IGNORE_CONCEALMENT) &&
			pVictim->m_flSuspicion < suspicion_notice.value &&
			m_pPlayer->m_skills.HasSkill(ESkillId::Phantom);

		// A deflect primes the next melee HIT. Consumed here rather than in
		// PrimaryAttack so a swing that connects with nothing costs nothing.
		// A Cleave swing spends it in the arc instead.
		const bool bFollowUp = bDamageHere && PulseTakeCrowbarFollowUp(m_pPlayer, flDamage);
		if (bFollowUp)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, FollowUpSound(), 1.0, ATTN_NORM, 0, 90);

		if (bDamageHere)
		{
			// Damage debug readout. Stashed only when the target will actually
			// reach TakeDamage, so a swing into a wall cannot leave a stale
			// breakdown for the next hit to pick up. The figure here is
			// PRE-Weapon-Mastery: that lands later, at ApplyMultiDamage, and the
			// "landed" number in the report is what finally arrived.
			if (pVictim)
			{
				DebugDamageDetail("%.0f%s  +%d stat%s%s%s = %.0f",
					flBaseDamage,
					bForce ? "  xForce" : "",
					iStat,
					bBackstab ? "  xBACKSTAB" : "",
					bBackstabNode ? "  xNode" : "",
					bFollowUp ? "  xFollowUp" : "",
					flDamage);
			}

			const bool bUnhurt = ReprisalUnhurt(pVictim);

			pEntity->TraceAttack(m_pPlayer->pev, flDamage, gpGlobals->v_forward, &tr, HitDamageType(pEntity));
			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

			// Leech: a hit on a living monster heals. Crates do not bleed.
			if (pVictim)
				LeechHeal(flDamage);

			ReprisalRefill(pVictim, bUnhurt);

			if (bPhantomEligible && !pVictim->IsAlive())
				m_pPlayer->PhantomStart();

			// After the damage, so a headcrab the hit killed is still thrown.
			if (bFollowUp)
				PulseCrowbarFollowUpKnockback(pEntity, gpGlobals->v_forward);
		}

#endif

		m_flNextPrimaryAttack = GetNextAttackDelay(bCleaveSwing ? CleaveRecovery(flSpeed, 0.25 * SwingDelayScale() * flSpeed) : 0.25 * SwingDelayScale() * flSpeed);

#ifndef CLIENT_DLL
		// play thwack, smack, or dong sound
		float flVol = 1.0;
		bool fHitWorld = true;

		if (pEntity)
		{
			if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
			{
				// play thwack or smack sound
				switch (RANDOM_LONG(0, 2))
				{
				case 0:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hitbod1.wav", 1, ATTN_NORM);
					break;
				case 1:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hitbod2.wav", 1, ATTN_NORM);
					break;
				case 2:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hitbod3.wav", 1, ATTN_NORM);
					break;
				}

				// The Backstab gets its own cue, on a different channel and in a
				// different timbre from the crowbar's own body-hit sound, which
				// lands in the same instant. ART_DEBT.md records why that
				// matters: the Pulse's first sound set failed not because the
				// samples were bad but because the deflect shared a timbre with
				// the Pulse and arrived a fraction of a second later, so it was
				// simply not heard. This would fail the same way.
				//
				// EMIT_SOUND does not reach CSoundEnt, so this tells the player
				// what happened without telling any monster. Weapon noise stays
				// entirely owned by m_iWeaponVolume below -- see
				// docs/PERCEPTION.md.
				if (bBackstab)
				{
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_VOICE,
						RANDOM_LONG(0, 1) == 0 ? "debris/bustflesh1.wav" : "debris/bustflesh2.wav",
						1.0, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 10));
				}

				m_pPlayer->m_iWeaponVolume = CROWBAR_BODYHIT_VOLUME;
				if (!pEntity->IsAlive())
					return true;
				else
					flVol = 0.1;

				fHitWorld = false;
			}
		}

		// play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

		if (fHitWorld)
		{
			float fvolbar = TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);

			if (g_pGameRules->IsMultiplayer())
			{
				// override the volume here, cause we don't play texture sounds in multiplayer,
				// and fvolbar is going to be 0 from the above call.

				fvolbar = 1;
			}

			// also play crowbar strike
			switch (RANDOM_LONG(0, 1))
			{
			case 0:
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hit1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				break;
			case 1:
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hit2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				break;
			}

			// delay the decal a bit
			m_trHit = tr;
		}

		m_pPlayer->m_iWeaponVolume = flVol * CROWBAR_WALLHIT_VOLUME;
#endif
		SetThink(&CCrowbar::Smack);
		pev->nextthink = gpGlobals->time + 0.2;
	}
	return fDidHit;
}

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
#include "hornet.h"
#include "gamerules.h"
#include "UserMessages.h"

// player_skills.h comes in through player.h.  skill_tuning.h is this file's
// own include: Hive Attack Speed is predicted (it sets m_flNextPrimaryAttack
// / m_flNextSecondaryAttack, and this file compiles into both DLLs), so it
// reads its cvar the way Melee Speed and Quick Draw do rather than through
// game.h.  debug_damage is server-only, guarded at its one use in Reload().
#include "skill_tuning.h"
#include "game.h"
#include <algorithm>

static float GetRechargeTime()
{
	if (gpGlobals->maxClients > 1)
	{
		return 0.3f;
	}
	return 0.5f;
}

enum firemode_e
{
	FIREMODE_TRACK = 0,
	FIREMODE_FAST
};

// Hive Attack Speed multiplies the hivehand's fire interval.  Both DLLs
// compile this file for prediction, so this reads through skill_tuning.h
// the way CBasePlayerWeapon::ReloadTimeScale/DrawTimeScale do
// (weapons_shared.cpp) rather than naming the cvar directly.
static float HiveAttackSpeedScale(CBasePlayer* pPlayer)
{
	if (!pPlayer || !pPlayer->m_skills.HasSkill(ESkillId::HiveAttackSpeed))
		return 1.0f;
	return std::max(0.05f, g_tuneHiveAttackSpeed.Value());
}


LINK_ENTITY_TO_CLASS(weapon_hornetgun, CHgun);

bool CHgun::IsUseable()
{
	return true;
}

void CHgun::Spawn()
{
	Precache();
	m_iId = WEAPON_HORNETGUN;
	SET_MODEL(ENT(pev), "models/w_hgun.mdl");

	m_iDefaultAmmo = HIVEHAND_DEFAULT_GIVE;
	m_iFirePhase = 0;

	FallInit(); // get ready to fall down.
}


void CHgun::Precache()
{
	PRECACHE_MODEL("models/v_hgun.mdl");
	PRECACHE_MODEL("models/w_hgun.mdl");
	PRECACHE_MODEL("models/p_hgun.mdl");

	m_usHornetFire = PRECACHE_EVENT(1, "events/firehornet.sc");

	UTIL_PrecacheOther("hornet");
}

void CHgun::AddToPlayer(CBasePlayer* pPlayer)
{
#ifndef CLIENT_DLL
	if (g_pGameRules->IsMultiplayer())
	{
		// in multiplayer, all hivehands come full -- to THIS player's
		// ceiling, so Hive Capacity is not silently worth less on a
		// fresh spawn than it is after the first regrow.
		m_iDefaultAmmo = PlayerHornetMaxCarry(pPlayer);
	}
#endif

	CBasePlayerWeapon::AddToPlayer(pPlayer);
}

bool CHgun::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Hornets";

	// The BASE ceiling only.  This fills CBasePlayerItem::ItemInfoArray, a
	// registry shared by every player and populated once from a template
	// entity with no owner (UTIL_PrecacheOtherWeapon, called by worldspawn
	// before any player exists) -- there is no player here to ask
	// PlayerHornetMaxCarry about, and the registry copy would not see a
	// later answer anyway.  The ceiling players actually play against is
	// enforced where a real player is available: CHgun::Reload and
	// CHgun::AddToPlayer's multiplayer refill, both through
	// PlayerHornetMaxCarry.  This registry value still governs ammo BOXES
	// (CBasePlayerWeapon::ExtractAmmo/AddPrimaryAmmo) and the world pickup
	// gate (CGameRules::CanHaveAmmo) -- Hive Capacity does not raise those,
	// a known gap, see the Alien table in docs/SKILL_TREE.md.
	p->iMaxAmmo1 = HORNET_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 3;
	p->iPosition = 3;
	p->iId = m_iId = WEAPON_HORNETGUN;
	p->iFlags = ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_NOAUTORELOAD;
	p->iWeight = HORNETGUN_WEIGHT;

	return true;
}


bool CHgun::Deploy()
{
	return DefaultDeploy("models/v_hgun.mdl", "models/p_hgun.mdl", HGUN_UP, "hive");
}

void CHgun::Holster()
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	SendWeaponAnim(HGUN_DOWN);

	//!!!HACKHACK - can't select hornetgun if it's empty! no way to get ammo for it, either.
	if (0 == m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()])
	{
		m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()] = 1;
	}
}


void CHgun::PrimaryAttack()
{
	Reload();

	if (m_pPlayer->ammo_hornets <= 0)
	{
		return;
	}

#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	CBaseEntity* pHornet = CBaseEntity::Create("hornet", m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -12, m_pPlayer->pev->v_angle, m_pPlayer->edict());
	pHornet->pev->velocity = gpGlobals->v_forward * 300;

	m_flRechargeTime = gpGlobals->time + GetRechargeTime();
#endif

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;


	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

	int flags;
#if defined(CLIENT_WEAPONS)
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usHornetFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, 0, 0);



	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	// Hive Attack Speed.  Only the delay before the next shot is allowed
	// shortens -- the fire event and the player's attack animation are not
	// sped up, so an unlocked player can out-fire them; a visible gap, see
	// the Alien table in docs/SKILL_TREE.md.
	const float flSpeed = HiveAttackSpeedScale(m_pPlayer);
	m_flNextPrimaryAttack = GetNextAttackDelay(0.25 * flSpeed);

	if (m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.25 * flSpeed;
	}
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 0)
	{
		m_flNextPrimaryAttack += GetRechargeTime();
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}



void CHgun::SecondaryAttack()
{
	Reload();

	if (m_pPlayer->ammo_hornets <= 0)
	{
		return;
	}

	//Wouldn't be a bad idea to completely predict these, since they fly so fast...
#ifndef CLIENT_DLL
	CBaseEntity* pHornet;
	Vector vecSrc;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -12;

	m_iFirePhase++;
	switch (m_iFirePhase)
	{
	case 1:
		vecSrc = vecSrc + gpGlobals->v_up * 8;
		break;
	case 2:
		vecSrc = vecSrc + gpGlobals->v_up * 8;
		vecSrc = vecSrc + gpGlobals->v_right * 8;
		break;
	case 3:
		vecSrc = vecSrc + gpGlobals->v_right * 8;
		break;
	case 4:
		vecSrc = vecSrc + gpGlobals->v_up * -8;
		vecSrc = vecSrc + gpGlobals->v_right * 8;
		break;
	case 5:
		vecSrc = vecSrc + gpGlobals->v_up * -8;
		break;
	case 6:
		vecSrc = vecSrc + gpGlobals->v_up * -8;
		vecSrc = vecSrc + gpGlobals->v_right * -8;
		break;
	case 7:
		vecSrc = vecSrc + gpGlobals->v_right * -8;
		break;
	case 8:
		vecSrc = vecSrc + gpGlobals->v_up * 8;
		vecSrc = vecSrc + gpGlobals->v_right * -8;
		m_iFirePhase = 0;
		break;
	}

	pHornet = CBaseEntity::Create("hornet", vecSrc, m_pPlayer->pev->v_angle, m_pPlayer->edict());
	pHornet->pev->velocity = gpGlobals->v_forward * 1200;
	pHornet->pev->angles = UTIL_VecToAngles(pHornet->pev->velocity);

	pHornet->SetThink(&CHornet::StartDart);

	m_flRechargeTime = gpGlobals->time + GetRechargeTime();
#endif

	int flags;
#if defined(CLIENT_WEAPONS)
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usHornetFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, 0, 0);


	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	// Hive Attack Speed, same as PrimaryAttack.
	const float flSpeed = HiveAttackSpeedScale(m_pPlayer);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.1 * flSpeed;
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 0)
	{
		m_flRechargeTime = gpGlobals->time + 0.5;
		m_flNextSecondaryAttack += 0.5;
		m_flNextPrimaryAttack += 0.5;
	}
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}


void CHgun::Reload()
{
#ifndef CLIENT_DLL
	// Hive Capacity: the ceiling this player actually plays against, not
	// the base HORNET_MAX_CARRY -- see the comment on it in GetItemInfo.
	const int maxCarry = PlayerHornetMaxCarry(m_pPlayer);
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= maxCarry)
		return;

	// Hive Replenish and the Hornet Replenish Stat nodes: rate 1 is the
	// base cadence (GetRechargeTime), so the interval divides by it.
	const float rate = std::max(0.05f, PlayerHornetReplenishScale(m_pPlayer));

	while (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] < maxCarry && m_flRechargeTime < gpGlobals->time)
	{
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]++;
		const float interval = GetRechargeTime() / rate;
		m_flRechargeTime += interval;

		// The Ricochet lesson: a Skill whose effect is a number quietly
		// changing needs a readout, not a code-derived guess.
		if (debug_damage.value != 0)
		{
			ALERT(at_console, "hive replenish: regrew a hornet, interval %.2fs (rate x%.2f), now %d/%d\n",
				interval, rate, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType], maxCarry);
		}
	}

	m_pPlayer->TabulateAmmo();
#endif
}


void CHgun::WeaponIdle()
{
	Reload();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	int iAnim;
	float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);
	if (flRand <= 0.75)
	{
		iAnim = HGUN_IDLE1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 30.0 / 16 * (2);
	}
	else if (flRand <= 0.875)
	{
		iAnim = HGUN_FIDGETSWAY;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 40.0 / 16.0;
	}
	else
	{
		iAnim = HGUN_FIDGETSHAKE;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 35.0 / 16.0;
	}
	SendWeaponAnim(iAnim);
}

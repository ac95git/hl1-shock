// The Carbon Pickaxe.
//
// The mining tool, and the heavy end of the melee roster: CCrowbar with
// different numbers and different models, the same shape the katana took.
// 25 damage at ~0.75s against the crowbar's 10 at 0.5s -- better burst and
// better sustained damage, because the crowbar is free and this costs Cells.
//
// The one thing it does that nothing else does is break a crystal deposit
// (docs/ROADMAP.md, "Mining and crystal shards").  That is answered by
// IsMiningTool() on the item rather than by a damage bit: func_deposit asks
// the striking player's active item, so a second mining tool later is one
// override and no bit out of the damage word.
//
// The viewmodel and world model are the crowbar's in black metal
// (E:\CustomAssets\scripts\pickaxe_black.py), a stand-in agreed in the grill
// of 2026-09-18; v_pickaxe.mdl keeps the mod's crowbar's sequences and glove
// families.  Still borrowed, and recorded in ART_DEBT.md: the player model,
// the sounds, the HUD sprite.  Like crowbar.cpp this file compiles into both
// DLLs, for prediction.

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "gamerules.h"

#include "skill_tuning.h"
#include <algorithm>

#ifndef CLIENT_DLL
#include "game.h"
#endif

LINK_ENTITY_TO_CLASS(weapon_pickaxe, CPickaxe);

void CPickaxe::Spawn()
{
	Precache();
	m_iId = WEAPON_PICKAXE;
	SET_MODEL(ENT(pev), "models/w_pickaxe.mdl");
	m_iClip = -1;

	FallInit(); // get ready to fall down.
}

void CPickaxe::Precache()
{
	PRECACHE_MODEL("models/v_pickaxe.mdl");
	PRECACHE_MODEL("models/w_pickaxe.mdl");
	PRECACHE_MODEL("models/p_crowbar.mdl");

	// The crowbar's own set, for now.
	PRECACHE_SOUND("weapons/cbar_hit1.wav");
	PRECACHE_SOUND("weapons/cbar_hit2.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod1.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod2.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod3.wav");
	PRECACHE_SOUND("weapons/cbar_miss1.wav");
	PRECACHE_SOUND("debris/bustflesh1.wav");
	PRECACHE_SOUND("debris/bustflesh2.wav");

	// The crowbar's event drives the miss animation unchanged, as the
	// katana's does: whatever model this weapon ends up with keeps the
	// crowbar's sequence order, which is why the roster subclasses at all.
	m_usCrowbar = PRECACHE_EVENT(1, "events/crowbar.sc");

	// Cleave's air shock, which CCrowbar::Precache would have registered had
	// this one called it.
	m_usCleave = PRECACHE_EVENT(1, "events/cleave.sc");
	PRECACHE_MODEL("sprites/shockwave.spr");
}

bool CPickaxe::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 2; // the melee bucket, after the crowbar and the katana
	p->iId = WEAPON_PICKAXE;
	p->iWeight = PICKAXE_WEIGHT;
	return true;
}

bool CPickaxe::Deploy()
{
	return DefaultDeploy("models/v_pickaxe.mdl", "models/p_crowbar.mdl", CROWBAR_DRAW, "crowbar");
}

#ifndef CLIENT_DLL
float CPickaxe::BaseDamage()
{
	return gSkillData.plrDmgPickaxe;
}
#endif

float CPickaxe::SwingDelayScale()
{
	// Heavier than the crowbar and lighter than the katana's slash.  Read on
	// both sides: the delay it scales is predicted.
	return std::max(0.1f, g_tunePickaxeSwing.Value());
}

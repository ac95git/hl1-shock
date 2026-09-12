// The Gauss Katana, v1.
//
// A melee weapon on the crowbar's swing: same trace, same Backstab, same
// Skills, same Follow-Up.  What differs is in the two hooks below -- it hits
// far harder and swings far slower -- and in the models.  Everything else it
// inherits, which is the point: one melee rule set, two weapons.
//
// The gauss arcs it is named for are not here.  ROADMAP.md records what they
// are (a beam, the gunshot decal, and a six-second glow, client-side from an
// event, exactly as the gauss gun does it) and that they want an event of
// their own on the swing.
//
// Placeholders, all recorded in ART_DEBT.md: the crowbar's p_ model, the
// crowbar's sounds, the crowbar's HUD icon.  This file, like crowbar.cpp,
// compiles into both DLLs for prediction.

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "gamerules.h"

#include "skill_tuning.h"
#include <algorithm>

LINK_ENTITY_TO_CLASS(weapon_katana, CKatana);

void CKatana::Spawn()
{
	Precache();
	m_iId = WEAPON_KATANA;
	SET_MODEL(ENT(pev), "models/w_katana.mdl");
	m_iClip = -1;

	FallInit(); // get ready to fall down.
}

void CKatana::Precache()
{
	PRECACHE_MODEL("models/v_katana.mdl");
	PRECACHE_MODEL("models/w_katana.mdl");
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

	// The crowbar's event plays the miss swing on the local viewmodel by
	// sequence index.  v_katana.mdl keeps the crowbar's sequence order for
	// exactly this reason, so the event is shared as-is.
	m_usCrowbar = PRECACHE_EVENT(1, "events/crowbar.sc");
}

bool CKatana::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 1; // beside the crowbar in the melee bucket
	p->iId = WEAPON_KATANA;
	p->iWeight = KATANA_WEIGHT;
	return true;
}

bool CKatana::Deploy()
{
	return DefaultDeploy("models/v_katana.mdl", "models/p_crowbar.mdl", CROWBAR_DRAW, "crowbar");
}

#ifndef CLIENT_DLL
float CKatana::BaseDamage()
{
	return gSkillData.plrDmgKatana;
}
#endif

float CKatana::SwingDelayScale()
{
	// A first guess to be judged in play, which is why it is a cvar.  Read on
	// both sides: the delay it scales is predicted.
	return std::max(0.1f, g_tuneKatanaSwing.Value());
}

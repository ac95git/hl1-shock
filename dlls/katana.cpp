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

// The wave's cvars are server-only, like Crowbar Force's: nothing the client
// predicts depends on them.
#ifndef CLIENT_DLL
#include "game.h"
#endif

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

	// The arcs.  Client-side, from an event, the way the gauss gun does its
	// wall hits (cl_dll/ev_hldm.cpp, EV_KatanaArc); the server only says
	// "a swing happened here, facing this way".  Sprites and sounds the
	// event uses are precached here because the client cannot.
	m_usKatanaArc = PRECACHE_EVENT(1, "events/katana_arc.sc");
	PRECACHE_MODEL("sprites/laserbeam.spr");
	PRECACHE_MODEL("sprites/hotglow.spr");
	PRECACHE_SOUND("weapons/electro4.wav");
	PRECACHE_SOUND("weapons/electro5.wav");
}

void CKatana::PrimaryAttack()
{
	CCrowbar::PrimaryAttack();

	// Origin and angles as the gauss passes them; the client turns them into
	// the gun position and the aim vector.  FEV_NOTHOST because this file is
	// predicted: the local client reaches this line itself.
	PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usKatanaArc,
		0.0, m_pPlayer->pev->origin, m_pPlayer->pev->angles, 0.0, 0.0, 0, 0, 0, 0);

#ifndef CLIENT_DLL
	WaveAttack();
#endif
}

#ifndef CLIENT_DLL
void CKatana::WaveAttack()
{
	const float flRange = std::max(0.0f, katana_wave_range.value);
	const float flScale = std::max(0.0f, katana_wave_damage_scale.value);
	if (flRange <= 0.0f || flScale <= 0.0f)
		return;

	// Start beyond the blade: what the blade reaches is the blade's, at the
	// blade's damage, and must not be hit twice in one swing.
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	const Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 32.0f;
	const Vector vecEnd = vecSrc + gpGlobals->v_forward * flRange;

	// A line first; a wave is tall but thin, and a line is what the player
	// aimed.  Then the small hull, so a headcrab just under the line is not
	// missed by a hair.  Never the large hull: aimed down, it finds the floor
	// before the target, which is exactly the shot this exists to make land.
	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);
	CBaseEntity* pEntity = tr.flFraction < 1.0f ? CBaseEntity::Instance(tr.pHit) : nullptr;
	if (pEntity == nullptr || pEntity->pev->takedamage == DAMAGE_NO)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);
		pEntity = tr.flFraction < 1.0f ? CBaseEntity::Instance(tr.pHit) : nullptr;
	}
	if (pEntity == nullptr || pEntity->pev->takedamage == DAMAGE_NO)
		return; // a wall: the client is already burning it

	// Falls off with distance the way the drawn wave fades, so the two agree
	// about how much wave arrived.
	const float flDist = (tr.vecEndPos - vecSrc).Length();
	const float flFalloff = std::max(0.0f, 1.0f - flDist / flRange);
	const float flDamage = BaseDamage() * flScale * flFalloff;
	if (flDamage <= 0.0f)
		return;

	ClearMultiDamage();
	pEntity->TraceAttack(m_pPlayer->pev, flDamage, gpGlobals->v_forward, &tr, DMG_ENERGYBEAM);
	ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
}
#endif

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

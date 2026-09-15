// The Gauss Katana, v1.
//
// A melee weapon on the crowbar's swing: same trace, same Backstab, same
// Skills, same Follow-Up.  What differs is in the hooks below -- it hits
// far harder and swings far slower -- and in the models.  Everything else it
// inherits, which is the point: one melee rule set, two weapons.
//
// Two clicks (docs/ROADMAP.md, the katana rework).  The left is the slash,
// blade only, heavy and slow: burst.  The right swings the blade light and
// fast and throws the wave: CKatanaWave below, an unseen projectile that
// carries the damage along the flight of the crescent the client draws
// (EV_KatanaArc, ev_hldm.cpp), so what the player sees arrive and what
// arrives are one thing.  The wave spends uranium: DPS, metered.
//
// Placeholders, all recorded in ART_DEBT.md: the crowbar's p_ model, the
// crowbar's sounds, the crowbar's HUD icon.  This file, like crowbar.cpp,
// compiles into both DLLs for prediction; the wave entity is server-only.

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "gamerules.h"

#include "skill_tuning.h"
#include <algorithm>
#include <cmath>

// The wave's damage cvars are server-only, like Crowbar Force's; the cost
// and Energy Efficiency come through skill_tuning.h because the ammo check
// gates a predicted swing.
#ifndef CLIENT_DLL
#include "game.h"
#include "saverestore.h"
#endif

LINK_ENTITY_TO_CLASS(weapon_katana, CKatana);

//=========================================================
// CKatanaWave -- the wave in flight.
//
// No model, no solid, no engine movement: it steps itself forward each
// frame along the aim, from the blade's reach to katana_wave_range, at
// katana_wave_speed, and each step is swept twice -- a line, which is what
// the player aimed, then the small hull, so a headcrab a hair under the
// line is not missed.  Everything damageable the sweep meets is struck once
// (m_hHit remembers), at full damage out to katana_wave_full_range and
// falling off to nothing at the range, so a crowd on the path is a crowd
// hit.  The wave dies where the crescent's belly does: on a wall.  A floor
// or ceiling the line meets is scraped, not hit, and the wave flies on,
// because aiming down at a headcrab used to end it on the first floor tile;
// a step spent entirely inside the world ends it quietly.
//=========================================================
#ifndef CLIENT_DLL
constexpr int KATANA_WAVE_MAX_HITS = 12;

class CKatanaWave : public CBaseEntity
{
public:
	static CKatanaWave* Create(CBasePlayer* pOwner, const Vector& vecStart, const Vector& vecDir, float flDamage);

	void Spawn() override;
	void EXPORT FlyThink();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	// A wave in flight does not follow the player through a level change.
	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

private:
	// One sweep of one step.  Strikes each new damageable thing on the way,
	// stops at the first thing it cannot damage.  Returns true if that thing
	// was a wall the wave should die on: only for the line, and only for a
	// surface that is not a floor or ceiling.
	bool Sweep(CBasePlayer* pOwner, const Vector& vecFrom, const Vector& vecTo, const Vector& vecDir, int iHull, bool bWallsCount);
	void Strike(CBasePlayer* pOwner, CBaseEntity* pVictim, const Vector& vecDir, TraceResult& tr, float flDist);
	bool AlreadyHit(CBaseEntity* pEntity);
	void RememberHit(CBaseEntity* pEntity);
	float Falloff(float flDist) const;

	float m_flDamage = 0.0f;     // before falloff and the player's Skills
	float m_flTravelled = 0.0f;  // from birth, along the aim
	float m_flLastThink = 0.0f;
	EHANDLE m_hHit[KATANA_WAVE_MAX_HITS];
};

LINK_ENTITY_TO_CLASS(katana_wave, CKatanaWave);

TYPEDESCRIPTION CKatanaWave::m_SaveData[] =
{
	DEFINE_FIELD(CKatanaWave, m_flDamage, FIELD_FLOAT),
	DEFINE_FIELD(CKatanaWave, m_flTravelled, FIELD_FLOAT),
	DEFINE_FIELD(CKatanaWave, m_flLastThink, FIELD_TIME),
	DEFINE_ARRAY(CKatanaWave, m_hHit, FIELD_EHANDLE, KATANA_WAVE_MAX_HITS),
};

IMPLEMENT_SAVERESTORE(CKatanaWave, CBaseEntity);

CKatanaWave* CKatanaWave::Create(CBasePlayer* pOwner, const Vector& vecStart, const Vector& vecDir, float flDamage)
{
	CKatanaWave* pWave = GetClassPtr((CKatanaWave*)NULL);
	pWave->pev->classname = MAKE_STRING("katana_wave");
	pWave->pev->owner = pOwner->edict();
	pWave->pev->origin = vecStart;
	// The direction and the speed in one, the way a bolt carries its flight,
	// and saved with the entity for free.
	pWave->pev->velocity = vecDir * std::max(1.0f, katana_wave_speed.value);
	pWave->m_flDamage = flDamage;
	pWave->Spawn();
	return pWave;
}

void CKatanaWave::Spawn()
{
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;
	pev->effects = EF_NODRAW;
	UTIL_SetOrigin(pev, pev->origin);
	UTIL_SetSize(pev, g_vecZero, g_vecZero);

	m_flLastThink = gpGlobals->time;
	SetThink(&CKatanaWave::FlyThink);
	pev->nextthink = gpGlobals->time;
}

float CKatanaWave::Falloff(float flDist) const
{
	const float flFull = std::max(0.0f, katana_wave_full_range.value);
	const float flRange = std::max(0.0f, katana_wave_range.value);
	if (flDist <= flFull)
		return 1.0f;
	if (flRange <= flFull)
		return 0.0f;
	return std::max(0.0f, 1.0f - (flDist - flFull) / (flRange - flFull));
}

bool CKatanaWave::AlreadyHit(CBaseEntity* pEntity)
{
	for (EHANDLE& h : m_hHit)
	{
		if (static_cast<CBaseEntity*>(h) == pEntity)
			return true;
	}
	return false;
}

void CKatanaWave::RememberHit(CBaseEntity* pEntity)
{
	for (EHANDLE& h : m_hHit)
	{
		if (static_cast<CBaseEntity*>(h) == nullptr)
		{
			h = pEntity;
			return;
		}
	}
	// Full: the thirteenth thing on one wave's path is hit again if the next
	// sweep meets it.  Accepted; a corridor that long and that full is not
	// the case this was built for.
}

void CKatanaWave::Strike(CBasePlayer* pOwner, CBaseEntity* pVictim, const Vector& vecDir, TraceResult& tr, float flDist)
{
	const float flFalloff = Falloff(flDist);
	const float flDamage = m_flDamage * flFalloff;
	if (flDamage <= 0.0f)
		return;

	DebugDamageDetail("wave -> %s %.0f at %.0f units (x%.2f)",
		STRING(pVictim->pev->classname), flDamage, flDist, flFalloff);

	// The wave is the inflictor and the player the attacker, the bolt's
	// shape, so the Energy Route's scaling at ApplyMultiDamage sees a
	// player's energy hit.
	ClearMultiDamage();
	pVictim->TraceAttack(pOwner->pev, flDamage, vecDir, &tr, DMG_ENERGYBEAM);
	ApplyMultiDamage(pev, pOwner->pev);
}

bool CKatanaWave::Sweep(CBasePlayer* pOwner, const Vector& vecFrom, const Vector& vecTo, const Vector& vecDir, int iHull, bool bWallsCount)
{
	edict_t* pentIgnore = pOwner->edict();
	Vector vecStart = vecFrom;

	// Bounded: each pass either reaches the end, stops at a wall, or moves
	// past one more thing.
	for (int i = 0; i < KATANA_WAVE_MAX_HITS + 2; ++i)
	{
		TraceResult tr;
		if (iHull < 0)
			UTIL_TraceLine(vecStart, vecTo, dont_ignore_monsters, pentIgnore, &tr);
		else
			UTIL_TraceHull(vecStart, vecTo, dont_ignore_monsters, iHull, pentIgnore, &tr);

		// A step spent entirely inside the world -- the wave has gone into a
		// floor it scraped -- is over.  The line decides that; the hull,
		// wider, is inside things the line is not.
		if (0 != tr.fAllSolid)
			return bWallsCount;

		if (tr.flFraction >= 1.0f)
			return false;

		CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
		if (pHit == nullptr || pHit->pev->takedamage == DAMAGE_NO)
		{
			// The world, or something that cannot be hurt.  For the line:
			// a wall ends the wave, a floor or ceiling is scraped and flown
			// through.  For the hull: neither means anything, the line has
			// the say.
			if (!bWallsCount)
				return false;
			return fabsf(tr.vecPlaneNormal.z) <= 0.7f;
		}

		if (pHit != pOwner && !AlreadyHit(pHit))
		{
			const float flDist = m_flTravelled + (tr.vecEndPos - vecFrom).Length();
			Strike(pOwner, pHit, vecDir, tr, flDist);
			RememberHit(pHit);
		}

		// On through it.  Ignoring what was just met, and a hair forward in
		// case the trace started inside it and reported no progress.
		pentIgnore = pHit->edict();
		vecStart = tr.vecEndPos;
		if (tr.flFraction <= 0.0f)
			vecStart = vecStart + vecDir;
	}
	return false;
}

void CKatanaWave::FlyThink()
{
	CBaseEntity* pOwnerEnt = CBaseEntity::Instance(pev->owner);
	if (pOwnerEnt == nullptr || !pOwnerEnt->IsPlayer())
	{
		UTIL_Remove(this);
		return;
	}
	CBasePlayer* pOwner = static_cast<CBasePlayer*>(pOwnerEnt);

	const float flRange = std::max(0.0f, katana_wave_range.value);
	const float flSpeed = pev->velocity.Length();
	if (flSpeed <= 0.0f || flRange <= 0.0f || m_flTravelled >= flRange)
	{
		UTIL_Remove(this);
		return;
	}
	const Vector vecDir = pev->velocity / flSpeed;

	// This frame's step, clipped to what is left of the range.
	const float flDt = std::max(0.0f, gpGlobals->time - m_flLastThink);
	m_flLastThink = gpGlobals->time;
	float flStep = flSpeed * flDt;
	if (flStep > flRange - m_flTravelled)
		flStep = flRange - m_flTravelled;

	if (flStep > 0.0f)
	{
		const Vector vecFrom = pev->origin;
		const Vector vecTo = vecFrom + vecDir * flStep;

		// The line first, which decides the wall; then the hull for what
		// sits just off the line.
		const bool bWall = Sweep(pOwner, vecFrom, vecTo, vecDir, -1, true);
		Sweep(pOwner, vecFrom, vecTo, vecDir, head_hull, false);

		if (bWall)
		{
			UTIL_Remove(this);
			return;
		}

		m_flTravelled += flStep;
		UTIL_SetOrigin(pev, vecTo);
	}

	if (m_flTravelled >= flRange)
	{
		UTIL_Remove(this);
		return;
	}
	pev->nextthink = gpGlobals->time;
}
#endif

//=========================================================
// CKatana
//=========================================================
void CKatana::Spawn()
{
	Precache();
	m_iId = WEAPON_KATANA;
	SET_MODEL(ENT(pev), "models/w_katana.mdl");
	m_iClip = -1;
	// Uranium for the wave, handed over on pickup like the gauss's.
	m_iDefaultAmmo = KATANA_DEFAULT_GIVE;

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

	// The heat and the arcs.  Client-side, from two events, the way the gauss
	// gun does its wall hits (cl_dll/ev_hldm.cpp, EV_KatanaSwing and
	// EV_KatanaArc); the server only says "a swing happened here, facing
	// this way".  Sprites and sounds the events use are precached here
	// because the client cannot.
	m_usKatanaSwing = PRECACHE_EVENT(1, "events/katana_swing.sc");
	m_usKatanaArc = PRECACHE_EVENT(1, "events/katana_arc.sc");
	PRECACHE_MODEL("sprites/laserbeam.spr");
	PRECACHE_MODEL("sprites/hotglow.spr");
	PRECACHE_SOUND("weapons/electro4.wav");
	PRECACHE_SOUND("weapons/electro5.wav");

	// Cleave's air shock, which the crowbar's Precache would have registered
	// had this one called it.  It does not, so here.
	m_usCleave = PRECACHE_EVENT(1, "events/cleave.sc");
	PRECACHE_MODEL("sprites/shockwave.spr");
}

void CKatana::PrimaryAttack()
{
	// The slash.  Blade only, full share; the heat, always.
	m_bWaveSwing = false;
	CCrowbar::PrimaryAttack();

	// Origin and angles as the gauss passes them; the client turns them into
	// the gun position and the aim vector.  FEV_NOTHOST because this file is
	// predicted: the local client reaches this line itself.
	PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usKatanaSwing,
		0.0, m_pPlayer->pev->origin, m_pPlayer->pev->angles, 0.0, 0.0, 0, 0, 0, 0);

	// One cadence for both clicks, so they cannot be alternated faster than
	// either swings.
	m_flNextSecondaryAttack = m_flNextPrimaryAttack;
}

int CKatana::WaveCost()
{
	float flCost = std::max(0.0f, g_tuneKatanaWaveCost.Value());
	if (flCost <= 0.0f)
		return 0;
	// Energy Efficiency divides it: at 1.33, five becomes four.  Never below
	// one; a wave that costs anything costs at least a unit.
	if (m_pPlayer->m_skills.HasSkill(ESkillId::EnergyEfficiency))
		flCost /= std::max(0.1f, g_tuneEnergyEfficiency.Value());
	return std::max(1, static_cast<int>(roundf(flCost)));
}

void CKatana::SecondaryAttack()
{
	// The wave spends uranium.  Without it the right click is refused
	// outright -- no swing, the empty click -- rather than falling back to a
	// slash, so the two clicks stay two verbs.  Both sides: the swing this
	// gates is predicted.
	const int iCost = WaveCost();
	if (iCost > 0 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] < iCost)
	{
		PlayEmptySound();
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5f;
		return;
	}

	// The wave.  The old swing entire: the blade at its reduced share, then
	// the crescent and the wave itself.  The lore: a swing heats the energy
	// in the blade at no loss, a thrown wave spends some of it.
	m_bWaveSwing = true;
	CCrowbar::PrimaryAttack();

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= iCost;

#ifndef CLIENT_DLL
	// Overdraw: the wave's charge also drains armour, at half the uranium it
	// just spent -- iCost, so Energy Efficiency's discount is already in it.
	OverdrawSpendArmor(m_pPlayer, iCost);
#endif

	PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usKatanaArc,
		0.0, m_pPlayer->pev->origin, m_pPlayer->pev->angles, 0.0, 0.0, 0, 0, 0, 0);

#ifndef CLIENT_DLL
	ThrowWave();
#endif

	m_flNextSecondaryAttack = m_flNextPrimaryAttack;
}

#ifndef CLIENT_DLL
float CKatana::BladeDamageScale()
{
	// The right click's blade is a plain number (katana_wave_blade_damage),
	// tuned apart from the slash; the swing wants it as a share of base.
	if (!m_bWaveSwing)
		return 1.0f;
	return std::max(0.0f, katana_wave_blade_damage.value) / std::max(1.0f, BaseDamage());
}

void CKatana::ThrowWave()
{
	const float flDamage = std::max(0.0f, katana_wave_damage.value);
	if (flDamage <= 0.0f || katana_wave_range.value <= 0.0f)
		return;

	// Born beyond the blade: what the blade reaches is the blade's, at the
	// blade's damage, and should not be hit twice in one swing.
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	const Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 32.0f;
	CKatanaWave::Create(m_pPlayer, vecSrc, gpGlobals->v_forward, flDamage);
}
#endif

bool CKatana::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	// Uranium, the egon's and the gauss's, for the wave.  The slash needs
	// none, hence the two flags: the HUD may select it empty and the game
	// does not switch away from it at zero.
	p->pszAmmo1 = "uranium";
	p->iMaxAmmo1 = URANIUM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 1; // beside the crowbar in the melee bucket
	p->iId = WEAPON_KATANA;
	p->iWeight = KATANA_WEIGHT;
	p->iFlags = ITEM_FLAG_SELECTONEMPTY | ITEM_FLAG_NOAUTOSWITCHEMPTY;
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
	// Per click: the slash swings slow, the wave at the crowbar's rate --
	// burst and DPS.  Read on both sides: the delay it scales is predicted.
	// The two clicks still share one cadence in the sense that either click
	// waits out the swing the other just made.
	return std::max(0.1f, m_bWaveSwing ? g_tuneKatanaWaveSwing.Value() : g_tuneKatanaSwing.Value());
}

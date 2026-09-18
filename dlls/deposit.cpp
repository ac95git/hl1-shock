// func_deposit -- a crystal deposit (docs/ROADMAP.md, "Mining and crystal
// shards").
//
// A brush entity the mapper carves where a vein should be: a secret that
// happens to be made of crystal.  Placed by hand, a few dozen in the game,
// and it never comes back once broken.  Only a mining tool breaks one --
// today the Carbon Pickaxe, answered by IsMiningTool() on the striking
// player's active item -- and everything else sparks off it: a blade, a
// bullet, a grenade, the katana's wave.  The Prompt says why, in its state
// line, so the rule teaches itself the first time a player swings the wrong
// thing at one.  Explosives are deliberately not a second way in: a deposit
// is optional content rather than a gate, and a grenade-priced bypass
// would make explosives the mining meta.
//
// Broken, it scatters its yield as loose item_shard pickups, one entity per
// Shard, on the side the blow came from -- walk-over like every pickup
// (ADR-0011), so a full Grid leaves them on the floor rather than
// destroying them.
//
// It is CBreakable underneath, which is where the damage, the break sound,
// the gibs and the fired target all come from.  Two things CBreakable does
// are worth knowing: a DMG_CLUB hit counts double on every breakable, so a
// pickaxe's 25 lands as 50; and Die() is not virtual, so the Shards are
// spawned after the base class's break rather than inside it.
//
// An UNSTABLE deposit (spawnflag 8) also discharges, on a free-running
// cycle, in the mod's one telegraph language: a flash, then the discharge
// (docs/ROADMAP.md, "The shape of the game").  The flash is small arcs
// crawling over the vein and a light rising around it; the discharge is one
// arc to every living thing in reach that the vein can see, and the arc is
// the damage -- what is seen is exactly what hurts, and cover works.  With
// nobody in reach it arcs into the walls instead, so it still performs.
// Player and monsters alike: a hazard on nobody's side, so a player who knows
// the rhythm can walk a zombie past it.  It runs whether or not anyone is
// watching, because the rhythm has to be learnable before the player commits
// to it: this is the parry's first tutor, and energy is on the Shield's list.
// Breaking the vein stops it.
//
// Server only.

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "func_break.h"
#include "decals.h"
#include "player.h"
#include "weapons.h"
#include "game.h"

#include <algorithm>

// Our own flag word.  CBreakable's flags (trigger-only, touch, pressure,
// instant-crowbar) all describe other ways to break a thing, and a deposit
// has exactly one, so they are cleared on spawn in case a mapper converted
// a func_breakable and kept them.
#define SF_DEPOSIT_BREAKABLE_FLAGS (SF_BREAK_TRIGGER_ONLY | SF_BREAK_TOUCH | SF_BREAK_PRESSURE | SF_BREAK_CROWBAR)

// Ours.  8 because it collides with none of the above.
#define SF_DEPOSIT_UNSTABLE 8

// The arcs.  Vanilla's lightning sprite and zaps; the colour is the
// Shard's amber, pushed toward white, so crystal and its discharge read as
// one substance (ART_DEBT.md: the whole game's crystal hue is unsettled).
#define DEPOSIT_ARC_SPRITE "sprites/lgtning.spr"
constexpr int k_ArcR = 255, k_ArcG = 200, k_ArcB = 120;

// How often the cycle is stepped.  Fine enough for the crawl to read as
// continuous; the cycle's own length is a cvar.
constexpr float DEPOSIT_ARC_STEP = 0.1f;

// Points on the vein's exposed surface, found once by tracing in from
// outside: where arcs start, so no arc ever appears to leave solid rock.
constexpr int DEPOSIT_MAX_SURFACE = 16;

// Health when the mapper sets none: three pickaxe hits at 50 each, two with
// Melee Force.
constexpr float DEPOSIT_DEFAULT_HEALTH = 120.0f;

// Shards when the mapper sets no yield.
constexpr int DEPOSIT_DEFAULT_YIELD = 3;
// And a ceiling, so a typo cannot fill a room with entities.
constexpr int DEPOSIT_MAX_YIELD = 20;

class CDeposit : public CBreakable
{
public:
	void Spawn() override;
	void Precache() override;
	bool KeyValue(KeyValueData* pkvd) override;

	// A +use cap so the Prompt considers it at all (FindLookedAtPickup only
	// looks at usable things and pickups); the press itself does nothing,
	// and the Prompt shows a state line rather than offering one.
	int ObjectCaps() override { return CBreakable::ObjectCaps() | FCAP_IMPULSE_USE; }
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;

	// Chips, never breakage marks: struck crystal does not crack like a pane.
	int DamageDecal(int bitsDamageType) override { return DECAL_BPROOF1; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	// The unstable cycle.  A MOVETYPE_PUSH entity thinks on pev->ltime, as
	// CBreakable's own Die does.
	void EXPORT ArcThink();

private:
	// Scatter the yield, once, from the side pAttacker stands on.
	void ScatterShards(CBaseEntity* pAttacker);

	void FindSurface();
	Vector NearestSurface(const Vector& vecTarget) const;
	void Arc(const Vector& vecFrom, const Vector& vecTo, int iWidth, int iNoise, float flLife, int iBrightness);
	void Telegraph(float flProgress);
	void Discharge();

	int m_iYield = DEPOSIT_DEFAULT_YIELD;
	bool m_bShardsDropped = false;

	// Seconds since the last discharge.  Saved; a vein keeps its place in
	// the rhythm across a save.
	float m_flCycle = 0.0f;

	// Rebuilt on first think after a spawn or a restore, since a vein never
	// moves; not worth saving sixteen vectors per deposit.
	Vector m_vecSurface[DEPOSIT_MAX_SURFACE];
	int m_nSurface = 0;
	int m_iArcSprite = 0;
};

LINK_ENTITY_TO_CLASS(func_deposit, CDeposit);

TYPEDESCRIPTION CDeposit::m_SaveData[] =
{
	DEFINE_FIELD(CDeposit, m_iYield, FIELD_INTEGER),
	DEFINE_FIELD(CDeposit, m_bShardsDropped, FIELD_BOOLEAN),
	DEFINE_FIELD(CDeposit, m_flCycle, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CDeposit, CBreakable);

// Is this hit a mining tool's?  The attacker is a player whose active item
// mines, and the attacker struck directly -- a grenade thrown before a swap
// to the pickaxe has the grenade as its inflictor and does not count.
static bool IsMiningHit(entvars_t* pevInflictor, entvars_t* pevAttacker)
{
	if (!pevAttacker || (pevInflictor && pevInflictor != pevAttacker))
		return false;

	CBaseEntity* pAttacker = CBaseEntity::Instance(pevAttacker);
	if (!pAttacker || !pAttacker->IsPlayer())
		return false;

	CBasePlayer* pPlayer = static_cast<CBasePlayer*>(pAttacker);
	return pPlayer->m_pActiveItem != nullptr && pPlayer->m_pActiveItem->IsMiningTool();
}

bool CDeposit::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "yield"))
	{
		m_iYield = std::clamp(atoi(pkvd->szValue), 0, DEPOSIT_MAX_YIELD);
		return true;
	}
	return CBreakable::KeyValue(pkvd);
}

void CDeposit::Precache()
{
	CBreakable::Precache();
	UTIL_PrecacheOther("item_shard");

	if (FBitSet(pev->spawnflags, SF_DEPOSIT_UNSTABLE))
	{
		m_iArcSprite = PRECACHE_MODEL(DEPOSIT_ARC_SPRITE);
		PRECACHE_SOUND("debris/zap1.wav");
		PRECACHE_SOUND("debris/zap4.wav");
		PRECACHE_SOUND("weapons/electro4.wav");
		PRECACHE_SOUND("weapons/electro5.wav");
		PRECACHE_SOUND("weapons/electro6.wav");
	}
}

void CDeposit::Spawn()
{
	pev->spawnflags &= ~SF_DEPOSIT_BREAKABLE_FLAGS;

	if (pev->health <= 0)
		pev->health = DEPOSIT_DEFAULT_HEALTH;

	// CBreakable::Spawn precaches, links the brush in and sets the touch;
	// the touch does nothing without the flags cleared above.
	CBreakable::Spawn();

	if (FBitSet(pev->spawnflags, SF_DEPOSIT_UNSTABLE))
	{
		// Out of step with every other vein in the map, so a room of them
		// does not pulse as one.
		m_flCycle = RANDOM_FLOAT(0.0f, 0.8f) * std::max(1.0f, deposit_arc_period.value);
		SetThink(&CDeposit::ArcThink);
		pev->nextthink = pev->ltime + 0.5f;
	}
}

void CDeposit::FindSurface()
{
	m_nSurface = 0;

	const Vector vecCenter = Center();
	const float flReach = pev->size.Length() * 0.5f + 16.0f;

	for (int iTry = 0; iTry < 96 && m_nSurface < DEPOSIT_MAX_SURFACE; ++iTry)
	{
		Vector vecDir(RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1));
		if (vecDir.Length() < 0.1f)
			continue;
		vecDir = vecDir.Normalize();

		// From outside, inward: the first thing met is the vein only where the
		// vein is exposed.  A direction that starts in the wall, or meets the
		// wall first, is the rock side and is dropped.
		TraceResult tr;
		UTIL_TraceLine(vecCenter + vecDir * flReach, vecCenter, ignore_monsters, nullptr, &tr);
		if (0 != tr.fStartSolid || 0 != tr.fAllSolid || tr.pHit != edict())
			continue;

		m_vecSurface[m_nSurface++] = tr.vecEndPos + tr.vecPlaneNormal * 2.0f;
	}

	// A vein buried on every side still discharges -- from its middle.
	if (m_nSurface == 0)
		m_vecSurface[m_nSurface++] = vecCenter;
}

Vector CDeposit::NearestSurface(const Vector& vecTarget) const
{
	int iBest = 0;
	float flBest = 1e9f;
	for (int i = 0; i < m_nSurface; ++i)
	{
		const float flDist = (m_vecSurface[i] - vecTarget).Length();
		if (flDist < flBest)
		{
			flBest = flDist;
			iBest = i;
		}
	}
	return m_vecSurface[iBest];
}

void CDeposit::Arc(const Vector& vecFrom, const Vector& vecTo, int iWidth, int iNoise, float flLife, int iBrightness)
{
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecFrom);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(vecFrom.x);
	WRITE_COORD(vecFrom.y);
	WRITE_COORD(vecFrom.z);
	WRITE_COORD(vecTo.x);
	WRITE_COORD(vecTo.y);
	WRITE_COORD(vecTo.z);
	WRITE_SHORT(m_iArcSprite);
	WRITE_BYTE(0);                                            // start frame
	WRITE_BYTE(10);                                           // frame rate
	WRITE_BYTE(std::clamp(static_cast<int>(flLife * 10.0f), 1, 255)); // life, 0.1 s units
	WRITE_BYTE(iWidth);
	WRITE_BYTE(iNoise);
	WRITE_BYTE(k_ArcR);
	WRITE_BYTE(k_ArcG);
	WRITE_BYTE(k_ArcB);
	WRITE_BYTE(iBrightness);
	WRITE_BYTE(30); // scroll
	MESSAGE_END();
}

// The flash.  flProgress runs 0 to 1 across deposit_arc_warn: the crawl
// thickens and brightens and the light around the vein swells, so the
// moment of the discharge can be read from how far along the flash is.
void CDeposit::Telegraph(float flProgress)
{
	if (m_nSurface >= 2)
	{
		const int nArcs = flProgress < 0.5f ? 1 : 2;
		for (int i = 0; i < nArcs; ++i)
		{
			const int a = RANDOM_LONG(0, m_nSurface - 1);
			int b = RANDOM_LONG(0, m_nSurface - 2);
			if (b >= a)
				++b;
			Arc(m_vecSurface[a], m_vecSurface[b], 6 + static_cast<int>(10 * flProgress), 40,
				0.15f, 90 + static_cast<int>(140 * flProgress));
		}
	}

	const Vector vecCenter = Center();
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecCenter);
	WRITE_BYTE(TE_DLIGHT);
	WRITE_COORD(vecCenter.x);
	WRITE_COORD(vecCenter.y);
	WRITE_COORD(vecCenter.z);
	WRITE_BYTE(4 + static_cast<int>(14 * flProgress)); // radius, 10-unit steps
	WRITE_BYTE(k_ArcR);
	WRITE_BYTE(k_ArcG);
	WRITE_BYTE(k_ArcB);
	WRITE_BYTE(2);  // life, 0.1 s
	WRITE_BYTE(0);  // decay
	MESSAGE_END();
}

void CDeposit::Discharge()
{
	const Vector vecCenter = Center();
	const float flRadius = std::max(16.0f, deposit_arc_radius.value);
	const float flDamage = std::max(0.0f, deposit_arc_damage.value);

	int nStruck = 0;
	CBaseEntity* pEntity = nullptr;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecCenter, flRadius)) != nullptr)
	{
		if (pEntity == this || pEntity->pev->takedamage == DAMAGE_NO || !pEntity->IsAlive())
			continue;
		// Victims are the living -- the player and monsters.  A crate in reach
		// is not arced to; it would read as noise, not as a rule.
		if (!pEntity->IsPlayer() && pEntity->MyMonsterPointer() == nullptr)
			continue;

		const Vector vecTarget = pEntity->BodyTarget(vecCenter);
		const Vector vecFrom = NearestSurface(vecTarget);

		// Line of sight from the vein's own surface: cover works.
		TraceResult tr;
		UTIL_TraceLine(vecFrom, vecTarget, dont_ignore_monsters, edict(), &tr);
		if (tr.flFraction < 1.0f && tr.pHit != pEntity->edict())
			continue;

		Arc(vecFrom, vecTarget, 30, 70, 0.25f, 255);
		if (flDamage > 0.0f)
			pEntity->TakeDamage(pev, pev, flDamage, DMG_ENERGYBEAM);
		++nStruck;
	}

	// Nobody in reach: a few arcs into the walls, so the vein still performs
	// and its reach can be seen before anyone walks into it.
	if (nStruck == 0)
	{
		const int nArcs = RANDOM_LONG(2, 3);
		for (int i = 0; i < nArcs; ++i)
		{
			const Vector vecFrom = m_vecSurface[RANDOM_LONG(0, m_nSurface - 1)];
			Vector vecDir = (vecFrom - vecCenter);
			vecDir = vecDir.Length() > 1.0f ? vecDir.Normalize() : Vector(0, 0, 1);
			vecDir = (vecDir + Vector(RANDOM_FLOAT(-0.6f, 0.6f), RANDOM_FLOAT(-0.6f, 0.6f), RANDOM_FLOAT(-0.6f, 0.6f))).Normalize();

			TraceResult tr;
			UTIL_TraceLine(vecFrom, vecFrom + vecDir * flRadius * 0.6f, ignore_monsters, edict(), &tr);
			if (tr.flFraction >= 1.0f)
				continue;
			Arc(vecFrom, tr.vecEndPos, 20, 60, 0.2f, 220);
			UTIL_Sparks(tr.vecEndPos);
		}
	}

	static const char* const k_Discharge[] = {"weapons/electro4.wav", "weapons/electro5.wav", "weapons/electro6.wav"};
	EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, k_Discharge[RANDOM_LONG(0, 2)], 1.0f, ATTN_NORM, 0, 95 + RANDOM_LONG(0, 15));

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecCenter);
	WRITE_BYTE(TE_DLIGHT);
	WRITE_COORD(vecCenter.x);
	WRITE_COORD(vecCenter.y);
	WRITE_COORD(vecCenter.z);
	WRITE_BYTE(24);
	WRITE_BYTE(k_ArcR);
	WRITE_BYTE(k_ArcG);
	WRITE_BYTE(k_ArcB);
	WRITE_BYTE(3);
	WRITE_BYTE(80);
	MESSAGE_END();
}

void CDeposit::ArcThink()
{
	pev->nextthink = pev->ltime + DEPOSIT_ARC_STEP;

	if (m_nSurface == 0)
		FindSurface();

	const float flPeriod = std::max(1.0f, deposit_arc_period.value);
	const float flWarn = std::clamp(deposit_arc_warn.value, 0.2f, flPeriod - 0.2f);

	const float flBefore = m_flCycle;
	m_flCycle += DEPOSIT_ARC_STEP;

	if (m_flCycle >= flPeriod)
	{
		Discharge();
		m_flCycle = 0.0f;
		return;
	}

	const float flWarnStart = flPeriod - flWarn;
	if (m_flCycle >= flWarnStart)
	{
		// The flash begins: one crackle, so the ear gets the warning too.
		if (flBefore < flWarnStart)
			EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, RANDOM_LONG(0, 1) ? "debris/zap1.wav" : "debris/zap4.wav", 0.7f, ATTN_NORM, 0, 90 + RANDOM_LONG(0, 20));
		Telegraph((m_flCycle - flWarnStart) / flWarn);
	}
}

void CDeposit::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// A player's +use press: nothing.  The Prompt has already said what will
	// break it.
	if (pCaller && pCaller->IsPlayer())
		return;

	// Anything else is a mapper's trigger -- a cave-in, a scripted blast --
	// and breaks it the way a triggered func_breakable breaks, Shards and all.
	if (pev->solid == SOLID_NOT)
		return;
	CBreakable::Use(pActivator, pCaller, useType, value);
	ScatterShards(pActivator);
}

void CDeposit::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	// The inflictor is not known here; TakeDamage has the final word.  This
	// only decides what the hit looks like: a mining tool bites, anything
	// else glances off in a spark and a ricochet, and adds no damage at all.
	if (IsMiningHit(pevAttacker, pevAttacker))
	{
		CBreakable::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
		return;
	}

	UTIL_Ricochet(ptr->vecEndPos, RANDOM_FLOAT(0.5f, 1.0f));
}

bool CDeposit::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	// Explosions and projectiles reach here without a TraceAttack, so the
	// rule is enforced here too.
	if (!IsMiningHit(pevInflictor, pevAttacker))
		return false;

	const bool bResult = CBreakable::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);

	// CBreakable::Die sets SOLID_NOT as it breaks, and is not virtual; this
	// is the one place a damage break can be seen from.
	if (pev->solid == SOLID_NOT)
		ScatterShards(CBaseEntity::Instance(pevAttacker));

	return bResult;
}

void CDeposit::ScatterShards(CBaseEntity* pAttacker)
{
	if (m_bShardsDropped)
		return;
	m_bShardsDropped = true;

	// From the point of the deposit nearest whoever broke it, stepped a
	// little toward them: a vein cut into a wall breaks outward into the
	// room, never into the rock behind it.  Kept no higher than the
	// attacker's middle, because an item falls at most 256 units to find a
	// floor and a vein high on a wall would otherwise lose its Shards.
	Vector vecSpot = Center();
	if (pAttacker)
	{
		const Vector vecEye = pAttacker->EyePosition();
		for (int k = 0; k < 3; ++k)
			vecSpot[k] = std::clamp(vecEye[k], pev->absmin[k], pev->absmax[k]);

		Vector vecToward = vecEye - vecSpot;
		const float flDist = vecToward.Length();
		if (flDist > 1.0f)
			vecSpot = vecSpot + vecToward * (std::min(flDist, 24.0f) / flDist);

		vecSpot.z = std::min(vecSpot.z, pAttacker->pev->origin.z + 8.0f);
	}

	for (int i = 0; i < m_iYield; ++i)
	{
		const Vector vecOrigin = vecSpot + Vector(RANDOM_FLOAT(-4, 4), RANDOM_FLOAT(-4, 4), 0);
		CBaseEntity* pShard = CBaseEntity::Create("item_shard", vecOrigin,
			Vector(0, RANDOM_FLOAT(0, 360), 0), nullptr);
		if (!pShard)
			break;

		pShard->pev->velocity = Vector(RANDOM_FLOAT(-80, 80), RANDOM_FLOAT(-80, 80), RANDOM_FLOAT(100, 200));
	}
}

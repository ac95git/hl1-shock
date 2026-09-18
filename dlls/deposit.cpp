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
// Server only.

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "func_break.h"
#include "decals.h"
#include "player.h"
#include "weapons.h"

#include <algorithm>

// Our own flag word.  CBreakable's flags (trigger-only, touch, pressure,
// instant-crowbar) all describe other ways to break a thing, and a deposit
// has exactly one, so they are cleared on spawn in case a mapper converted
// a func_breakable and kept them.
#define SF_DEPOSIT_BREAKABLE_FLAGS (SF_BREAK_TRIGGER_ONLY | SF_BREAK_TOUCH | SF_BREAK_PRESSURE | SF_BREAK_CROWBAR)

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

private:
	// Scatter the yield, once, from the side pAttacker stands on.
	void ScatterShards(CBaseEntity* pAttacker);

	int m_iYield = DEPOSIT_DEFAULT_YIELD;
	bool m_bShardsDropped = false;
};

LINK_ENTITY_TO_CLASS(func_deposit, CDeposit);

TYPEDESCRIPTION CDeposit::m_SaveData[] =
{
	DEFINE_FIELD(CDeposit, m_iYield, FIELD_INTEGER),
	DEFINE_FIELD(CDeposit, m_bShardsDropped, FIELD_BOOLEAN),
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
}

void CDeposit::Spawn()
{
	pev->spawnflags &= ~SF_DEPOSIT_BREAKABLE_FLAGS;

	if (pev->health <= 0)
		pev->health = DEPOSIT_DEFAULT_HEALTH;

	// CBreakable::Spawn precaches, links the brush in and sets the touch;
	// the touch does nothing without the flags cleared above.
	CBreakable::Spawn();
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

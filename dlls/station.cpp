// func_station -- a Station (docs/ROADMAP.md, "Stations").
//
// A brush the mapper builds into a wall, on func_recharge's shape: a use
// press, a finite budget, a sound.  What is new is that the press is a
// transaction against the Inventory: Crystal Shards in, something the
// player cannot find enough of out.  Which recipe is the "stationtype"
// keyvalue, a row of game_shared/station_defs.h; the Prompt states the
// trade, so a recipe is known, never guessed.
//
// One press is one trade.  Everything that can refuse it is checked before
// a single Shard is taken, so a Station can never consume what it then
// fails to pay for:
//
//   - spent: nothing happens but the refusal;
//   - too few Shards: refused, and the centre line says how many it wants;
//   - an ammunition output that would not fit ENTIRELY: refused.  Partly
//     fitting is refused too, deliberately -- a trade that pays out half of
//     what it charged is the same class of bug as one that pays nothing,
//     only smaller, and the player can always spend the ammunition and
//     come back.
//
// A Skill Point is banked, not carried, so it never needs room.
//
// The server owns the Inventory (ADR-0004), so this is server code calling
// straight into it: no new authority question and no new message.
//
// Server only.

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "player_inventory.h"
#include "player_skills.h"
#include "weapons.h"
#include "station.h"

#include <algorithm>

// A press within this long of the last is ignored, so a held key or a
// double tap cannot buy two by accident.
constexpr float STATION_PRESS_INTERVAL = 0.6f;

class CStation : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	int ObjectCaps() override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_IMPULSE_USE; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	const StationDef& Def() const { return GetStationDef(m_iType); }
	bool Spent() const { return m_iUses == 0; }

private:
	// Refuses, with the reason on the centre line (nullptr for none).
	void Refuse(CBasePlayer* pPlayer, const char* pszWhy);

	int m_iType = static_cast<int>(EStationType::FuelProcessor);
	int m_iUses = -2; // -2: not set by the mapper, take the row's default
	float m_flNextPress = 0.0f;
};

LINK_ENTITY_TO_CLASS(func_station, CStation);

TYPEDESCRIPTION CStation::m_SaveData[] =
{
	DEFINE_FIELD(CStation, m_iType, FIELD_INTEGER),
	DEFINE_FIELD(CStation, m_iUses, FIELD_INTEGER),
	DEFINE_FIELD(CStation, m_flNextPress, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CStation, CBaseEntity);

EPromptClass StationPromptClass(CBaseEntity* pEnt)
{
	CStation* pStation = dynamic_cast<CStation*>(pEnt);
	if (!pStation)
		return EPromptClass::None;

	switch (pStation->Def().type)
	{
	case EStationType::FuelProcessor:
		return pStation->Spent() ? EPromptClass::StationFuelSpent : EPromptClass::StationFuel;
	case EStationType::AmmoUranium:
		return pStation->Spent() ? EPromptClass::StationAmmoEmpty : EPromptClass::StationUranium;
	case EStationType::AmmoCores:
		return pStation->Spent() ? EPromptClass::StationAmmoEmpty : EPromptClass::StationCores;
	default:
		return EPromptClass::Generic;
	}
}

bool CStation::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "stationtype"))
	{
		m_iType = std::clamp(atoi(pkvd->szValue), 0, static_cast<int>(EStationType::_Count) - 1);
		return true;
	}
	if (FStrEq(pkvd->szKeyName, "uses"))
	{
		m_iUses = std::max(-1, atoi(pkvd->szValue));
		return true;
	}
	return CBaseEntity::KeyValue(pkvd);
}

void CStation::Precache()
{
	// func_recharge's own pair: the charger's accept and refuse.
	PRECACHE_SOUND("items/suitchargeok1.wav");
	PRECACHE_SOUND("items/suitchargeno1.wav");
	PRECACHE_SOUND("items/gunpickup2.wav");
}

void CStation::Spawn()
{
	Precache();

	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;
	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model));

	// The row's budget, unless the mapper set one and the row allows it.  A
	// Fuel Processor is always exactly one: "one Skill Point, once".  Zero
	// from the mapper is honoured -- a Station placed already spent.
	const StationDef& def = Def();
	if (m_iUses == -2 || def.usesFixed)
		m_iUses = def.defaultUses;
}

void CStation::Refuse(CBasePlayer* pPlayer, const char* pszWhy)
{
	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/suitchargeno1.wav", 0.85, ATTN_NORM);
	if (pszWhy)
		ClientPrint(pPlayer->pev, HUD_PRINTCENTER, pszWhy);
}

void CStation::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// Only a player's own press trades; a trigger has no Inventory.
	if (!pActivator || !pActivator->IsPlayer())
		return;
	CBasePlayer* pPlayer = static_cast<CBasePlayer*>(pActivator);

	if (gpGlobals->time < m_flNextPress)
		return;
	m_flNextPress = gpGlobals->time + STATION_PRESS_INTERVAL;

	const StationDef& def = Def();

	if (Spent())
	{
		Refuse(pPlayer, nullptr);
		return;
	}

	// Every refusal before a single Shard is taken.
	const int iHave = pPlayer->m_inventory.TotalOfItem(static_cast<int>(EItemTypeId::Shard));
	if (iHave < def.shards)
	{
		char szWhy[96];
		snprintf(szWhy, sizeof(szWhy), "Needs %d Crystal Shards - you have %d.\n", def.shards, iHave);
		Refuse(pPlayer, szWhy);
		return;
	}

	int iAmmoMax = 0;
	if (def.output == EStationOutput::Ammo)
	{
		// The carry ceilings are weapons.h's, and the table is compiled into
		// the client too, so they are looked up here rather than copied into it.
		if (FStrEq(def.ammoName, "uranium"))
			iAmmoMax = URANIUM_MAX_CARRY;
		else if (FStrEq(def.ammoName, "Cores"))
			iAmmoMax = CORE_MAX_CARRY;

		const int iIndex = CBasePlayer::GetAmmoIndex(def.ammoName);
		const int iHeld = iIndex >= 0 ? pPlayer->AmmoInventory(iIndex) : 0;
		if (iAmmoMax <= 0 || iHeld + def.amount > iAmmoMax)
		{
			char szWhy[96];
			snprintf(szWhy, sizeof(szWhy), "No room for %d %s.\n", def.amount, def.ammoName);
			Refuse(pPlayer, szWhy);
			return;
		}
	}

	// The trade.
	if (InventoryTakeItem(pPlayer, EItemTypeId::Shard, def.shards) != def.shards)
	{
		// Unreachable after the count above; refused rather than half-paid if
		// it ever is.
		Refuse(pPlayer, nullptr);
		return;
	}

	switch (def.output)
	{
	case EStationOutput::SkillPoint:
		pPlayer->m_skills.AddSkillPoints(def.amount);
		ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "Skill Point acquired.\n");
		EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);
		SendSkillTreeToClient(pPlayer);
		break;

	case EStationOutput::Ammo:
		pPlayer->GiveAmmo(def.amount, def.ammoName, iAmmoMax);
		break;
	}

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/suitchargeok1.wav", 0.85, ATTN_NORM);

	if (m_iUses > 0)
		--m_iUses;

	// Each trade fires the target: a light going out on a spent processor, a
	// door, a global the endings will one day read.
	SUB_UseTargets(pPlayer, USE_TOGGLE, 0);
}

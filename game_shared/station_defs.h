#pragma once

// ---------------------------------------------------------
// Stations: world entities that take items in and give items out
// (docs/ROADMAP.md, "Stations").  The input is always Crystal Shards; the
// output is what the player cannot find enough of.  The choice a Station
// poses is ammunition now, or power for good -- which is why there are
// exactly these two kinds to begin with.
//
// A func_station names one row by its "stationtype" keyvalue; the recipe
// lives here rather than as free-form keys on the entity, because
// free-form recipes would be a crafting system and nobody asked for one.
// One press of use is one trade, and the Prompt states the trade, so a
// recipe is known rather than discovered.
//
// Compiled into both DLLs: the server trades from it, and the Prompt's
// rows (prompt_defs.h) are built from its strings, so the price shown and
// the price charged are one line apart and cannot drift further.
// ---------------------------------------------------------

enum class EStationType : int
{
	FuelProcessor = 0, // Shards into one Skill Point, once
	AmmoUranium   = 1, // Shards into uranium
	AmmoCores     = 2, // Shards into Cores
	_Count,
};

enum class EStationOutput : int
{
	SkillPoint, // banked, like item_skillpoint; never needs room
	Ammo,       // amount of ammoName, refused unless it all fits
};

struct StationDef
{
	EStationType   type;
	int            shards;      // Crystal Shards one trade takes
	EStationOutput output;
	const char*    ammoName;    // Half-Life's ammo name, for EStationOutput::Ammo
	int            amount;      // how much one trade gives
	int            defaultUses; // trades before it is spent; -1 is unlimited
	bool           usesFixed;   // true: the mapper's "uses" is ignored
	const char*    promptTitle;
	const char*    promptAction; // follows the use key: "[E] Insert ..."
	const char*    promptSpent;  // the state line once it is spent
};

// First guesses, all of them, from the grill of 2026-09-18.  One full Stack
// of Shards (ten, inventory_defs.h) is one Skill Point; three Shards are a
// gauss's pickup of uranium or two Cores.  Keep each promptAction in step
// with the numbers on its own row.
inline constexpr StationDef k_StationDefs[] =
{
	//  type                          shards output                       ammo        amt uses fixed title                 action                                spent
	{ EStationType::FuelProcessor,  10, EStationOutput::SkillPoint,     nullptr,    1,  1,   true,  "Fuel processor",      "Insert 10 Shards for a Skill Point", "Spent" },
	{ EStationType::AmmoUranium,    3,  EStationOutput::Ammo,           "uranium",  20, 3,   false, "Ammunition station",  "Insert 3 Shards for 20 uranium",     "Empty" },
	{ EStationType::AmmoCores,      3,  EStationOutput::Ammo,           "Cores",    2,  3,   false, "Ammunition station",  "Insert 3 Shards for 2 Cores",        "Empty" },
};

static_assert(sizeof(k_StationDefs) / sizeof(k_StationDefs[0]) == static_cast<int>(EStationType::_Count),
	"k_StationDefs needs one row per EStationType");

inline const StationDef& GetStationDef(int type)
{
	if (type < 0 || type >= static_cast<int>(EStationType::_Count))
		return k_StationDefs[0];
	return k_StationDefs[type];
}

//=========================================================
// inventory_defs.h
//
// The single definition of what can be in an Inventory, and
// of the Grid it sits in.  Compiled into BOTH the server and
// the client -- see docs/adr/0002-two-identity-spaces-for-weapons-and-items.md.
//
// Vocabulary here follows CONTEXT.md: Entry, Stack, Item Type,
// Cell, Row.  Header-only so neither project file needs to change.
//=========================================================

#pragma once

#include <cstdint>

// ---------------------------------------------------------
// Grid geometry
//
// The width is frozen: a Cell's linear index (row * width + col)
// would silently change meaning if it ever moved, scrambling
// saved layouts with no error.  See
// docs/adr/0003-fixed-grid-width-rows-only-growth.md.
// ---------------------------------------------------------
inline constexpr int INV_GRID_WIDTH = 12;

// Hard ceiling on Rows, used only to size arrays.  The *playable*
// ceiling is the inv_rows_max cvar, which must never exceed this.
//
// Set generously and then left alone: it sizes the DEFINE_ARRAY fields in
// the save table, so changing it changes the save format and invalidates
// existing saves.  Costs ~2.9 KB per player in the save file at this size.
inline constexpr int INV_ROWS_ABSOLUTE_MAX = 12;

inline constexpr int INV_MAX_CELLS = INV_GRID_WIDTH * INV_ROWS_ABSOLUTE_MAX;

// Maximum Entries an Inventory can hold.  Every Entry occupies at
// least one Cell, so this can never be exceeded in practice; it is
// the fixed array size the save system requires.
inline constexpr int INV_MAX_ENTRIES = INV_MAX_CELLS;

// Every weapon occupies this many Cells.  INV_GRID_WIDTH is a
// multiple of it so no Cell is ever stranded at the end of a Row.
inline constexpr int INV_WEAPON_CELL_WIDTH = 3;

// Entries per sync message.  A GoldSrc user message caps out around
// 192 bytes; at 5 bytes per Entry plus a 5-byte header this leaves
// generous headroom, so a full Inventory is sent as several messages.
inline constexpr int INV_SYNC_CHUNK = 20;

static_assert(INV_GRID_WIDTH % INV_WEAPON_CELL_WIDTH == 0,
	"Grid width must be a multiple of the weapon width, or Cells strand at row ends");

// ---------------------------------------------------------
// What an Entry refers to.
//
// Weapons keep Half-Life's own WeaponId; items use EItemTypeId.
// Two identity spaces is deliberate -- see ADR-0002.
// ---------------------------------------------------------
enum class EEntryKind : uint8_t
{
	Empty  = 0,
	Weapon = 1, // id is a WeaponId from cdll_dll.h
	Item   = 2, // id is an EItemTypeId
};

// ---------------------------------------------------------
// EItemTypeId
//
// Ids are FROZEN once written to a save.  Adding is free; reordering
// or reusing corrupts old saves.
//
// The values deliberately match the legacy ITEM_* constants in
// dlls/weapons.h so they remain usable as m_rgItems[] indices while
// that array still exists.
// ---------------------------------------------------------
enum class EItemTypeId : int
{
	None     = 0,
	Medkit   = 1, // == ITEM_HEALTHKIT
	Antidote = 2, // == ITEM_ANTIDOTE
	Keycard  = 3, // == ITEM_SECURITY
	Battery  = 4, // == ITEM_BATTERY

	// The first Item Type that is ours rather than Half-Life's, and so the
	// first with no legacy ITEM_* twin: MAX_ITEMS is 5, so m_rgItems[] has no
	// slot for this and SyncLegacyItemCount skips it.  That is correct, not an
	// oversight -- the Inventory is the only record of a Syringe.
	Syringe  = 5,

	_Count   = 6, // keep last
};

inline constexpr int k_MaxItemTypes = static_cast<int>(EItemTypeId::_Count);

// ---------------------------------------------------------
// Item Type definition record
// ---------------------------------------------------------
struct ItemTypeDef
{
	EItemTypeId id;
	const char* classname;   // world entity to spawn on drop; nullptr = cannot be dropped
	const char* displayName; // shown in the UI
	const char* spriteName;  // HUD sprite name; nullptr = renders without an icon
	int         cellWidth;   // Cells occupied
	int         maxStack;    // Stack ceiling; 1 means unique
	// Whether Use does anything.  The client offers the verb from this rather
	// than from a list of its own -- it had one, and the first item added after
	// it was written silently shipped with no Use button.
	bool        usable;
};

inline constexpr ItemTypeDef k_ItemTypes[k_MaxItemTypes] =
{
	//  id                      classname         display     sprite            cells  stack  usable
	{ EItemTypeId::None,        nullptr,          "",         nullptr,          1,     0,     false },
	{ EItemTypeId::Medkit,      "item_healthkit", "Medkit",   "item_healthkit", 1,     5,     true  },
	// Carried, not consumed -- deliberately have no Use.
	{ EItemTypeId::Antidote,    "item_antidote",  "Antidote", nullptr,          1,     5,     false },
	{ EItemTypeId::Keycard,     "item_security",  "Keycard",  nullptr,          1,     1,     false },
	{ EItemTypeId::Battery,     "item_battery",   "Battery",  "item_battery",   1,     5,     true  },
	// "cross" is deliberately the same sprite the Infusion's status icon uses:
	// the player sees a cross in the Grid, uses it, and a cross appears at the
	// screen edge.  Placeholder art -- see docs/ART_DEBT.md.
	{ EItemTypeId::Syringe,     "item_syringe",   "Health Syringe", "cross",    1,     3,     true  },
};

// Returns nullptr for None or any out-of-range id.
inline const ItemTypeDef* GetItemType(int id)
{
	if (id <= 0 || id >= k_MaxItemTypes)
		return nullptr;
	return &k_ItemTypes[id];
}

inline const ItemTypeDef* GetItemType(EItemTypeId id)
{
	return GetItemType(static_cast<int>(id));
}

// Cells occupied by an Entry of the given kind and id.
// Returns 0 for anything unrecognised, which callers must treat as "cannot be placed".
inline int InvEntryCellWidth(EEntryKind kind, int id)
{
	switch (kind)
	{
	case EEntryKind::Weapon:
		return INV_WEAPON_CELL_WIDTH;
	case EEntryKind::Item:
	{
		const ItemTypeDef* def = GetItemType(id);
		return def ? def->cellWidth : 0;
	}
	default:
		return 0;
	}
}

// Stack ceiling for an Entry. Weapons are always unique.
inline int InvEntryMaxStack(EEntryKind kind, int id)
{
	switch (kind)
	{
	case EEntryKind::Weapon:
		return 1;
	case EEntryKind::Item:
	{
		const ItemTypeDef* def = GetItemType(id);
		return def ? def->maxStack : 0;
	}
	default:
		return 0;
	}
}

// Look up an Item Type by the classname of its world entity.
// Returns EItemTypeId::None if the classname is not an Item Type.
inline EItemTypeId ItemTypeFromClassname(const char* classname)
{
	if (!classname || classname[0] == '\0')
		return EItemTypeId::None;

	for (int i = 1; i < k_MaxItemTypes; ++i)
	{
		const char* c = k_ItemTypes[i].classname;
		if (!c)
			continue;

		// Deliberately a manual compare: this header is included by both
		// DLLs and pulling <cstring> in for one call is not worth it.
		const char* a = c;
		const char* b = classname;
		while (*a != '\0' && *a == *b) { ++a; ++b; }
		if (*a == '\0' && *b == '\0')
			return k_ItemTypes[i].id;
	}

	return EItemTypeId::None;
}

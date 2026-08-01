//=========================================================
// player_inventory.h
//
// The player's Inventory.  The server owns both the contents
// and where each Entry sits -- see
// docs/adr/0004-the-server-owns-the-inventory.md.
//
// Vocabulary follows CONTEXT.md.
//=========================================================

#pragma once

#include "inventory_defs.h"

//=========================================================
// CPlayerInventory
//
// Stored directly inside CBasePlayer, next to m_skills.
//
// Entries are kept COMPACT: indices 0..m_iEntryCount-1 are live
// and removing one shifts the rest down.  Callers that hold an
// index across a mutation must re-check it -- every command that
// names an index also names the kind and id it expects, so a
// stale index is detected rather than acted on.
//
// Parallel arrays rather than an array of structs because
// Half-Life's save system only serialises basic field types.
//=========================================================
struct CPlayerInventory
{
	// ---- Persistent data (save/restore) ----

	int m_iEntryCount  = 0;

	// Rows the player started with, captured once from inv_rows_start.
	// Stored rather than read live so that lowering the cvar mid-playthrough
	// cannot shrink a Grid that already has things in it.
	int m_iRowsBase    = 0;

	// Rows granted since, clamped at grant time and never re-clamped.
	int m_iRowsGranted = 0;

	// Grid width this layout was written with.  If it ever differs from
	// INV_GRID_WIDTH the layout is re-packed on restore instead of being
	// silently misread.
	int m_iSavedWidth  = INV_GRID_WIDTH;

	int m_rgKind [INV_MAX_ENTRIES] = {}; // EEntryKind
	int m_rgId   [INV_MAX_ENTRIES] = {}; // WeaponId or EItemTypeId
	int m_rgCount[INV_MAX_ENTRIES] = {};
	int m_rgCol  [INV_MAX_ENTRIES] = {};
	int m_rgRow  [INV_MAX_ENTRIES] = {};

	// ---- Setup ----

	// Captures m_iRowsBase from inv_rows_start if it has not been set yet.
	// Safe to call repeatedly; does nothing once initialised.
	void EnsureInitialised();

	void Clear();

	// ---- Queries ----

	// Rows currently usable.  Never shrinks for a given player.
	int Rows() const;

	// Rows to draw, including un-granted ones shown greyed out.
	// Never less than Rows(), so a lowered cvar cannot hide occupied Cells.
	int RowsToDraw() const;

	int EntryCount() const { return m_iEntryCount; }

	bool IsValidIndex(int i) const { return i >= 0 && i < m_iEntryCount; }

	EEntryKind KindAt(int i) const
	{
		return IsValidIndex(i) ? static_cast<EEntryKind>(m_rgKind[i]) : EEntryKind::Empty;
	}
	int IdAt   (int i) const { return IsValidIndex(i) ? m_rgId[i]    : 0; }
	int CountAt(int i) const { return IsValidIndex(i) ? m_rgCount[i] : 0; }
	int ColAt  (int i) const { return IsValidIndex(i) ? m_rgCol[i]   : 0; }
	int RowAt  (int i) const { return IsValidIndex(i) ? m_rgRow[i]   : 0; }

	// Confirms the Entry at 'index' is what the caller thinks it is.
	// Guards against a client acting on an index that shifted underneath it.
	bool EntryMatches(int index, EEntryKind kind, int id) const;

	// First Entry of this kind and id, or -1.
	int FindEntry(EEntryKind kind, int id) const;

	// Total held of an Item Type across every Stack.
	int TotalOfItem(int itemTypeId) const;

	// Can an Entry 'cellWidth' wide sit at (col,row)?
	// 'ignoreIndex' excludes an Entry from the occupancy test, so an
	// Entry can be tested against a position overlapping where it already is.
	bool CanPlaceAt(int col, int row, int cellWidth, int ignoreIndex = -1) const;

	// First free run of 'cellWidth' Cells, scanning left-to-right,
	// top-to-bottom.  Returns false if the Grid has no room.
	bool FindFirstFree(int cellWidth, int& outCol, int& outRow, int ignoreIndex = -1) const;

	// ---- Mutations ----

	// Adds up to 'count' of an Item Type, topping up partial Stacks before
	// creating new ones.  Returns how many were ACCEPTED, which may be
	// zero or less than requested -- partial retrieval falls out of this.
	int TryAddItem(int itemTypeId, int count);

	// Claims a run of Cells for a weapon.  Returns false when the Grid is
	// full, which is what makes a weapon pickup refusable.
	// Returns true if the weapon is already held (nothing to do).
	bool TryAddWeapon(int weaponId);

	// Removes one Entry, shifting later Entries down.
	bool RemoveAt(int index);

	// Removes up to 'count' from the Stack at 'index', removing the Entry
	// entirely when it empties.  Returns how many were removed.
	int RemoveCountAt(int index, int count);

	// Moves an Entry to (col,row) if it fits there. Returns false and
	// changes nothing otherwise.
	bool MoveEntry(int index, int col, int row);

	// Grants Rows, clamped at grant time to inv_rows_max.
	// Returns how many Rows were actually added.
	int GrantRows(int rows);

	// Re-places every Entry first-fit, preserving order.  Used when the
	// Grid width changed under a save.  Entries that no longer fit are
	// dropped from the Inventory and their number returned.
	int Repack();
};

//=========================================================
// Save/restore helpers -- implemented in player_inventory.cpp.
// CSave/CRestore are forward-declared so saverestore.h is not needed here.
//=========================================================
class CSave;
class CRestore;

bool InventorySave(CPlayerInventory& inv, CSave& save);
bool InventoryRestore(CPlayerInventory& inv, CRestore& restore);

//=========================================================
// Sends the full Inventory to a single player, in chunks small
// enough for a GoldSrc user message.
//=========================================================
class CBasePlayer;
void SendInventoryToClient(CBasePlayer* pPlayer);

//=========================================================
// Gives a player up to 'count' of an Item Type.
//
// Returns how many were ACCEPTED -- zero means the Grid was full and
// nothing changed, so a pickup entity must stay in the world.
//
// Keeps the legacy m_rgItems[] counters in step for the code that
// still reads them, and re-syncs the client.
//=========================================================
int InventoryGiveItem(CBasePlayer* pPlayer, EItemTypeId type, int count = 1);

//=========================================================
// Removes 'count' of an Item Type, taking from the last Stack first so
// partial Stacks are consumed before full ones are broken into.
// Returns how many were removed.
//=========================================================
int InventoryTakeItem(CBasePlayer* pPlayer, EItemTypeId type, int count = 1);

//=========================================================
// The three verbs.  Each validates against the server's own state and
// re-syncs the client on success -- see docs/PILLARS.md.
//
// 'expectedKind'/'expectedId' guard against the client naming an index
// that shifted underneath it; a mismatch is ignored rather than acted on.
//=========================================================
bool InventoryUseEntry(CBasePlayer* pPlayer, int index, EEntryKind expectedKind, int expectedId);
// dropAll drops a whole Stack, one world entity per item, scattered so they
// do not pile up in a single point. Weapons are always a single Entry.
bool InventoryDropEntry(CBasePlayer* pPlayer, int index, EEntryKind expectedKind, int expectedId,
	bool dropAll = false);
bool InventoryMoveEntry(CBasePlayer* pPlayer, int index, EEntryKind expectedKind, int expectedId,
	int col, int row);

//=========================================================
// Acquisition -- the Pickup Prompt and taking what it names.
//=========================================================
class CBaseEntity;

// What the player is currently looking at, if it can be taken.
struct LookedAtPickup
{
	EEntryKind   kind    = EEntryKind::Empty;
	int          id      = 0;  // WeaponId or EItemTypeId
	CBaseEntity* pEntity = nullptr;

	bool Valid() const { return pEntity != nullptr && kind != EEntryKind::Empty; }
};

// The single answer to "what would a use press take?".
//
// Deliberately also considers ordinary usable entities (buttons, doors,
// chargers) so that when one of those wins the aim test this returns nothing.
// The Pickup Prompt and the take therefore cannot disagree, and pressing use
// near a button never silently grabs a medkit instead.
LookedAtPickup FindLookedAtPickup(CBasePlayer* pPlayer);

// Sends the Pickup Prompt when the looked-at pickup changes. Call each frame.
void UpdatePickupPrompt(CBasePlayer* pPlayer);

// Takes whatever the prompt is naming. Returns false if there is nothing to
// take or the Grid has no room.
bool TryTakeLookedAtPickup(CBasePlayer* pPlayer);

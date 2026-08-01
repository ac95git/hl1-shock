#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "items.h"
#include "skill.h"
#include "game.h"
#include "player_inventory.h"
#include "UserMessages.h"
#include <algorithm>

//=========================================================
// Local save/restore descriptor table
//=========================================================
static TYPEDESCRIPTION g_InventorySaveData[] =
{
	DEFINE_FIELD(CPlayerInventory, m_iEntryCount,  FIELD_INTEGER),
	DEFINE_FIELD(CPlayerInventory, m_iRowsBase,    FIELD_INTEGER),
	DEFINE_FIELD(CPlayerInventory, m_iRowsGranted, FIELD_INTEGER),
	DEFINE_FIELD(CPlayerInventory, m_iSavedWidth,  FIELD_INTEGER),
	DEFINE_ARRAY(CPlayerInventory, m_rgKind,  FIELD_INTEGER, INV_MAX_ENTRIES),
	DEFINE_ARRAY(CPlayerInventory, m_rgId,    FIELD_INTEGER, INV_MAX_ENTRIES),
	DEFINE_ARRAY(CPlayerInventory, m_rgCount, FIELD_INTEGER, INV_MAX_ENTRIES),
	DEFINE_ARRAY(CPlayerInventory, m_rgCol,   FIELD_INTEGER, INV_MAX_ENTRIES),
	DEFINE_ARRAY(CPlayerInventory, m_rgRow,   FIELD_INTEGER, INV_MAX_ENTRIES),
};

//=========================================================
// Cvar readers
//
// Both are clamped into a sane range here so a typo in the console
// cannot produce a zero-row or oversized Grid.
//=========================================================
static int CvarRowsStart()
{
	int v = static_cast<int>(inv_rows_start.value);
	return std::max(1, std::min(v, INV_ROWS_ABSOLUTE_MAX));
}

static int CvarRowsMax()
{
	int v = static_cast<int>(inv_rows_max.value);
	return std::max(CvarRowsStart(), std::min(v, INV_ROWS_ABSOLUTE_MAX));
}

//=========================================================
// Setup
//=========================================================
void CPlayerInventory::EnsureInitialised()
{
	if (m_iRowsBase <= 0)
	{
		m_iRowsBase = CvarRowsStart();
		m_iSavedWidth = INV_GRID_WIDTH;
	}
}

void CPlayerInventory::Clear()
{
	m_iEntryCount = 0;
	m_iRowsGranted = 0;
	m_iRowsBase = 0;
	m_iSavedWidth = INV_GRID_WIDTH;

	memset(m_rgKind,  0, sizeof(m_rgKind));
	memset(m_rgId,    0, sizeof(m_rgId));
	memset(m_rgCount, 0, sizeof(m_rgCount));
	memset(m_rgCol,   0, sizeof(m_rgCol));
	memset(m_rgRow,   0, sizeof(m_rgRow));
}

//=========================================================
// Queries
//=========================================================
int CPlayerInventory::Rows() const
{
	// Deliberately NOT clamped against inv_rows_max: a Row once granted
	// stays granted even if the cvar is lowered afterwards.
	int rows = m_iRowsBase + m_iRowsGranted;
	if (rows < 1)
		rows = 1;
	if (rows > INV_ROWS_ABSOLUTE_MAX)
		rows = INV_ROWS_ABSOLUTE_MAX;
	return rows;
}

int CPlayerInventory::RowsToDraw() const
{
	return std::max(Rows(), CvarRowsMax());
}

bool CPlayerInventory::EntryMatches(int index, EEntryKind kind, int id) const
{
	if (!IsValidIndex(index))
		return false;
	return static_cast<EEntryKind>(m_rgKind[index]) == kind && m_rgId[index] == id;
}

int CPlayerInventory::FindEntry(EEntryKind kind, int id) const
{
	for (int i = 0; i < m_iEntryCount; ++i)
	{
		if (static_cast<EEntryKind>(m_rgKind[i]) == kind && m_rgId[i] == id)
			return i;
	}
	return -1;
}

int CPlayerInventory::TotalOfItem(int itemTypeId) const
{
	int total = 0;
	for (int i = 0; i < m_iEntryCount; ++i)
	{
		if (static_cast<EEntryKind>(m_rgKind[i]) == EEntryKind::Item && m_rgId[i] == itemTypeId)
			total += m_rgCount[i];
	}
	return total;
}

bool CPlayerInventory::CanPlaceAt(int col, int row, int cellWidth, int ignoreIndex) const
{
	if (cellWidth < 1 || cellWidth > INV_GRID_WIDTH)
		return false;
	if (col < 0 || row < 0)
		return false;
	if (row >= Rows())
		return false;
	if (col + cellWidth > INV_GRID_WIDTH)
		return false;

	for (int i = 0; i < m_iEntryCount; ++i)
	{
		if (i == ignoreIndex)
			continue;
		if (m_rgRow[i] != row)
			continue;

		int otherWidth = InvEntryCellWidth(static_cast<EEntryKind>(m_rgKind[i]), m_rgId[i]);
		if (otherWidth < 1)
			continue;

		// Overlap on a single Row is a 1-D interval test.
		if (col < m_rgCol[i] + otherWidth && m_rgCol[i] < col + cellWidth)
			return false;
	}

	return true;
}

bool CPlayerInventory::FindFirstFree(int cellWidth, int& outCol, int& outRow, int ignoreIndex) const
{
	const int rows = Rows();
	for (int row = 0; row < rows; ++row)
	{
		for (int col = 0; col + cellWidth <= INV_GRID_WIDTH; ++col)
		{
			if (CanPlaceAt(col, row, cellWidth, ignoreIndex))
			{
				outCol = col;
				outRow = row;
				return true;
			}
		}
	}
	return false;
}

//=========================================================
// Mutations
//=========================================================
int CPlayerInventory::TryAddItem(int itemTypeId, int count)
{
	EnsureInitialised();

	const ItemTypeDef* def = GetItemType(itemTypeId);
	if (!def || count <= 0)
		return 0;

	const int maxStack = def->maxStack;
	if (maxStack <= 0)
		return 0;

	int remaining = count;
	int accepted = 0;

	// 1. Top up partial Stacks first, in Entry order.
	for (int i = 0; i < m_iEntryCount && remaining > 0; ++i)
	{
		if (static_cast<EEntryKind>(m_rgKind[i]) != EEntryKind::Item)
			continue;
		if (m_rgId[i] != itemTypeId)
			continue;

		int room = maxStack - m_rgCount[i];
		if (room <= 0)
			continue;

		int take = std::min(room, remaining);
		m_rgCount[i] += take;
		remaining -= take;
		accepted += take;
	}

	// 2. Spill the rest into new Entries while the Grid has room.
	while (remaining > 0)
	{
		if (m_iEntryCount >= INV_MAX_ENTRIES)
			break;

		int col = 0, row = 0;
		if (!FindFirstFree(def->cellWidth, col, row))
			break;

		int take = std::min(maxStack, remaining);

		const int i = m_iEntryCount;
		m_rgKind[i]  = static_cast<int>(EEntryKind::Item);
		m_rgId[i]    = itemTypeId;
		m_rgCount[i] = take;
		m_rgCol[i]   = col;
		m_rgRow[i]   = row;
		++m_iEntryCount;

		remaining -= take;
		accepted += take;
	}

	return accepted;
}

bool CPlayerInventory::TryAddWeapon(int weaponId)
{
	EnsureInitialised();

	if (weaponId <= 0)
		return false;

	// Already carried -- a duplicate pickup only tops up ammo, which is
	// not our concern.
	if (FindEntry(EEntryKind::Weapon, weaponId) >= 0)
		return true;

	if (m_iEntryCount >= INV_MAX_ENTRIES)
		return false;

	int col = 0, row = 0;
	if (!FindFirstFree(INV_WEAPON_CELL_WIDTH, col, row))
		return false;

	const int i = m_iEntryCount;
	m_rgKind[i]  = static_cast<int>(EEntryKind::Weapon);
	m_rgId[i]    = weaponId;
	m_rgCount[i] = 1;
	m_rgCol[i]   = col;
	m_rgRow[i]   = row;
	++m_iEntryCount;

	return true;
}

bool CPlayerInventory::RemoveAt(int index)
{
	if (!IsValidIndex(index))
		return false;

	// Shift later Entries down to keep the array compact.
	for (int i = index; i < m_iEntryCount - 1; ++i)
	{
		m_rgKind[i]  = m_rgKind[i + 1];
		m_rgId[i]    = m_rgId[i + 1];
		m_rgCount[i] = m_rgCount[i + 1];
		m_rgCol[i]   = m_rgCol[i + 1];
		m_rgRow[i]   = m_rgRow[i + 1];
	}

	const int last = m_iEntryCount - 1;
	m_rgKind[last]  = 0;
	m_rgId[last]    = 0;
	m_rgCount[last] = 0;
	m_rgCol[last]   = 0;
	m_rgRow[last]   = 0;

	--m_iEntryCount;
	return true;
}

int CPlayerInventory::RemoveCountAt(int index, int count)
{
	if (!IsValidIndex(index) || count <= 0)
		return 0;

	int removed = std::min(count, m_rgCount[index]);
	m_rgCount[index] -= removed;

	if (m_rgCount[index] <= 0)
		RemoveAt(index);

	return removed;
}

bool CPlayerInventory::MoveEntry(int index, int col, int row)
{
	if (!IsValidIndex(index))
		return false;

	int cellWidth = InvEntryCellWidth(static_cast<EEntryKind>(m_rgKind[index]), m_rgId[index]);
	if (cellWidth < 1)
		return false;

	if (!CanPlaceAt(col, row, cellWidth, index))
		return false;

	m_rgCol[index] = col;
	m_rgRow[index] = row;
	return true;
}

int CPlayerInventory::GrantRows(int rows)
{
	EnsureInitialised();

	if (rows <= 0)
		return 0;

	// Clamped HERE, at grant time, and never again.  That is what lets a
	// lowered inv_rows_max cap future grants without shrinking the Grid.
	const int ceiling = std::max(CvarRowsMax(), Rows());
	const int headroom = ceiling - Rows();
	if (headroom <= 0)
		return 0;

	const int granted = std::min(rows, headroom);
	m_iRowsGranted += granted;
	return granted;
}

int CPlayerInventory::Repack()
{
	// Re-place every Entry first-fit in its current order.  Entries are
	// walked front to back and each is placed into the space left by those
	// before it, so order is preserved even though positions change.
	const int originalCount = m_iEntryCount;
	int written = 0;

	for (int i = 0; i < originalCount; ++i)
	{
		int cellWidth = InvEntryCellWidth(static_cast<EEntryKind>(m_rgKind[i]), m_rgId[i]);
		if (cellWidth < 1)
			continue;

		// Only Entries already written back count as occupied.
		const int savedCount = m_iEntryCount;
		m_iEntryCount = written;

		int col = 0, row = 0;
		const bool placed = FindFirstFree(cellWidth, col, row);

		m_iEntryCount = savedCount;

		if (!placed)
			continue; // no room; this Entry is dropped

		m_rgKind[written]  = m_rgKind[i];
		m_rgId[written]    = m_rgId[i];
		m_rgCount[written] = m_rgCount[i];
		m_rgCol[written]   = col;
		m_rgRow[written]   = row;
		++written;
	}

	for (int i = written; i < originalCount; ++i)
	{
		m_rgKind[i]  = 0;
		m_rgId[i]    = 0;
		m_rgCount[i] = 0;
		m_rgCol[i]   = 0;
		m_rgRow[i]   = 0;
	}

	const int dropped = originalCount - written;
	m_iEntryCount = written;
	m_iSavedWidth = INV_GRID_WIDTH;
	return dropped;
}

//=========================================================
// InventorySave / InventoryRestore
//=========================================================
bool InventorySave(CPlayerInventory& inv, CSave& save)
{
	inv.m_iSavedWidth = INV_GRID_WIDTH;
	return save.WriteFields("INVENTORY", &inv, g_InventorySaveData, ARRAYSIZE(g_InventorySaveData));
}

bool InventoryRestore(CPlayerInventory& inv, CRestore& restore)
{
	if (!restore.ReadFields("INVENTORY", &inv, g_InventorySaveData, ARRAYSIZE(g_InventorySaveData)))
		return false;

	// Guard against a corrupt or hand-edited count walking off the arrays.
	if (inv.m_iEntryCount < 0)
		inv.m_iEntryCount = 0;
	if (inv.m_iEntryCount > INV_MAX_ENTRIES)
		inv.m_iEntryCount = INV_MAX_ENTRIES;

	// A save written with a different Grid width would be silently misread,
	// because a position means something different at another width.
	// Re-place everything instead.
	if (inv.m_iSavedWidth != INV_GRID_WIDTH)
	{
		const int savedWidth = inv.m_iSavedWidth;
		const int dropped = inv.Repack();
		ALERT(at_console,
			"[Inventory] Save written at grid width %d, now %d; layout re-packed, %d entries dropped.\n",
			savedWidth, INV_GRID_WIDTH, dropped);
	}

	return true;
}

//=========================================================
// Legacy counter sync
//
// m_rgItems[] is still read by older code paths.  The Inventory is the
// source of truth; this mirrors the total back into it after a change.
//=========================================================
static void SyncLegacyItemCount(CBasePlayer* pPlayer, int itemTypeId)
{
	if (itemTypeId <= 0 || itemTypeId >= MAX_ITEMS)
		return;

	// Server-side only: the client learns about items from the Inventory
	// sync, not from these counters.
	pPlayer->m_rgItems[itemTypeId] = pPlayer->m_inventory.TotalOfItem(itemTypeId);
}

//=========================================================
// InventoryGiveItem / InventoryTakeItem
//=========================================================
int InventoryGiveItem(CBasePlayer* pPlayer, EItemTypeId type, int count)
{
	if (!pPlayer || count <= 0)
		return 0;

	const int id = static_cast<int>(type);
	const int accepted = pPlayer->m_inventory.TryAddItem(id, count);
	if (accepted <= 0)
		return 0;

	SyncLegacyItemCount(pPlayer, id);
	SendInventoryToClient(pPlayer);
	return accepted;
}

int InventoryTakeItem(CBasePlayer* pPlayer, EItemTypeId type, int count)
{
	if (!pPlayer || count <= 0)
		return 0;

	const int id = static_cast<int>(type);
	CPlayerInventory& inv = pPlayer->m_inventory;

	int remaining = count;
	int removed = 0;

	// Walk backwards so the most recently created (and usually smallest)
	// Stack is emptied first, and so RemoveAt's shifting cannot skip an Entry.
	for (int i = inv.EntryCount() - 1; i >= 0 && remaining > 0; --i)
	{
		if (inv.KindAt(i) != EEntryKind::Item || inv.IdAt(i) != id)
			continue;

		const int took = inv.RemoveCountAt(i, remaining);
		remaining -= took;
		removed += took;
	}

	if (removed > 0)
	{
		SyncLegacyItemCount(pPlayer, id);
		SendInventoryToClient(pPlayer);
	}

	return removed;
}

//=========================================================
// Verb: Use
//
// Item behaviour lives here, keyed by Item Type, because the server
// decides what using something does -- never the client (ADR-0002).
//=========================================================
static bool UseMedkit(CBasePlayer* pPlayer, int index)
{
	if (!pPlayer->TakeHealth(gSkillData.healthkitCapacity, DMG_GENERIC))
		return false; // already at full health; the kit is not spent

	pPlayer->m_inventory.RemoveCountAt(index, 1);
	EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/smallmedkit1.wav", 1, ATTN_NORM);
	return true;
}

static bool UseBattery(CBasePlayer* pPlayer, int index)
{
	if (!pPlayer->HasSuit())
		return false;
	if (pPlayer->pev->armorvalue >= MAX_NORMAL_BATTERY)
		return false;

	pPlayer->pev->armorvalue = V_min(
		pPlayer->pev->armorvalue + gSkillData.batteryCapacity,
		(float)MAX_NORMAL_BATTERY);

	pPlayer->m_inventory.RemoveCountAt(index, 1);

	EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

	// HEV suit charge voice line.
	int pct = (int)((pPlayer->pev->armorvalue * 100.0f) * (1.0f / MAX_NORMAL_BATTERY) + 0.5f);
	pct = (pct / 5);
	if (pct > 0)
		pct--;

	char szcharge[64];
	sprintf(szcharge, "!HEV_%1dP", pct);
	pPlayer->SetSuitUpdate(szcharge, false, SUIT_NEXT_IN_30SEC);

	return true;
}

bool InventoryUseEntry(CBasePlayer* pPlayer, int index, EEntryKind expectedKind, int expectedId)
{
	if (!pPlayer)
		return false;

	CPlayerInventory& inv = pPlayer->m_inventory;
	if (!inv.EntryMatches(index, expectedKind, expectedId))
		return false;

	// Weapons are equipped through Half-Life's own selection path, not here.
	if (expectedKind != EEntryKind::Item)
		return false;

	bool used = false;
	switch (static_cast<EItemTypeId>(expectedId))
	{
	case EItemTypeId::Medkit:
		used = UseMedkit(pPlayer, index);
		break;

	case EItemTypeId::Battery:
		used = UseBattery(pPlayer, index);
		break;

	case EItemTypeId::Antidote:
	case EItemTypeId::Keycard:
		// Carried, not consumed. Deliberately does nothing.
		return false;

	default:
		return false;
	}

	if (used)
	{
		SyncLegacyItemCount(pPlayer, expectedId);
		SendInventoryToClient(pPlayer);
	}

	return used;
}

//=========================================================
// Verb: Drop
//=========================================================
bool InventoryDropEntry(CBasePlayer* pPlayer, int index, EEntryKind expectedKind, int expectedId,
	bool dropAll)
{
	if (!pPlayer)
		return false;

	CPlayerInventory& inv = pPlayer->m_inventory;
	if (!inv.EntryMatches(index, expectedKind, expectedId))
		return false;

	if (expectedKind == EEntryKind::Weapon)
	{
		// DropPlayerItem spawns the world entity and calls RemovePlayerItem,
		// which releases the Cells and re-syncs the client.
		const char* classname = CBasePlayerItem::ItemInfoArray[expectedId].pszName;
		if (!classname || classname[0] == '\0')
			return false;

		pPlayer->DropPlayerItem((char*)classname);
		return true;
	}

	const ItemTypeDef* def = GetItemType(expectedId);
	if (!def || !def->classname)
		return false;

	const int wanted = dropAll ? inv.CountAt(index) : 1;
	if (wanted <= 0)
		return false;

	UTIL_MakeVectors(pPlayer->pev->angles);

	const Vector vecOrigin = pPlayer->pev->origin + gpGlobals->v_forward * 10;

	int dropped = 0;
	for (int i = 0; i < wanted; ++i)
	{
		CBaseEntity* pDropped = CBaseEntity::Create(
			(char*)def->classname, vecOrigin, pPlayer->pev->angles, pPlayer->edict());

		if (!pDropped)
			break;

		pDropped->pev->angles.x = 0;
		pDropped->pev->angles.z = 0;

		// Scatter so a dropped Stack lands as a small pile rather than one
		// model hiding the rest. A Stack is at most a handful of items.
		pDropped->pev->velocity = gpGlobals->v_forward * 200
			+ Vector(RANDOM_FLOAT(-45, 45), RANDOM_FLOAT(-45, 45), RANDOM_FLOAT(0, 80));

		++dropped;
	}

	if (dropped <= 0)
		return false;

	inv.RemoveCountAt(index, dropped);
	SyncLegacyItemCount(pPlayer, expectedId);
	SendInventoryToClient(pPlayer);
	return true;
}

//=========================================================
// Verb: Move
//=========================================================
bool InventoryMoveEntry(CBasePlayer* pPlayer, int index, EEntryKind expectedKind, int expectedId,
	int col, int row)
{
	if (!pPlayer)
		return false;

	CPlayerInventory& inv = pPlayer->m_inventory;
	if (!inv.EntryMatches(index, expectedKind, expectedId))
		return false;

	if (!inv.MoveEntry(index, col, row))
	{
		// Rejected. Re-sync anyway so the client snaps back to the truth.
		SendInventoryToClient(pPlayer);
		return false;
	}

	SendInventoryToClient(pPlayer);
	return true;
}

//=========================================================
// SendInventoryToClient
//
// Chunked: a GoldSrc user message caps out around 192 bytes and a full
// Inventory can carry far more Entries than that allows.  The first
// chunk carries the header and tells the client to reset.
//=========================================================
//=========================================================
// Acquisition
//=========================================================

// Matches PLAYER_SEARCH_RADIUS in player.cpp, so the Pickup Prompt appears at
// exactly the distance a use press would reach.
static constexpr float INV_PICKUP_RADIUS = 64.0f;

// Is this entity something that can be taken, and if so, what?
static bool ClassifyPickup(CBaseEntity* pEnt, EEntryKind& outKind, int& outId)
{
	if (!pEnt)
		return false;

	// A carried weapon is attached to its owner and not drawn. Only things
	// actually lying in the world are pickups.
	if ((pEnt->pev->effects & EF_NODRAW) != 0)
		return false;

	if (auto weapon = dynamic_cast<CBasePlayerItem*>(pEnt); weapon)
	{
		if (weapon->m_iId <= 0)
			return false;
		outKind = EEntryKind::Weapon;
		outId = weapon->m_iId;
		return true;
	}

	if (dynamic_cast<CItem*>(pEnt))
	{
		// Only Item Types are Inventory pickups. The HEV suit and the longjump
		// module are CItems too, but they are not carried, so they keep
		// Half-Life's walk-over behaviour and get no prompt.
		const EItemTypeId type = ItemTypeFromClassname(STRING(pEnt->pev->classname));
		if (type == EItemTypeId::None)
			return false;

		outKind = EEntryKind::Item;
		outId = static_cast<int>(type);
		return true;
	}

	return false;
}

LookedAtPickup FindLookedAtPickup(CBasePlayer* pPlayer)
{
	LookedAtPickup out;

	if (!pPlayer || pPlayer->pev->deadflag != DEAD_NO)
		return out;

	UTIL_MakeVectors(pPlayer->pev->v_angle);

	CBaseEntity* pObject = nullptr;
	float flMaxDot = VIEW_FIELD_NARROW;

	// Mirrors CBasePlayer::PlayerUse's aim test exactly, including considering
	// ordinary usable entities, so the winner here is the thing a use press
	// would actually act on.
	while ((pObject = UTIL_FindEntityInSphere(pObject, pPlayer->pev->origin, INV_PICKUP_RADIUS)) != nullptr)
	{
		const bool usable =
			(pObject->ObjectCaps() & (FCAP_IMPULSE_USE | FCAP_CONTINUOUS_USE | FCAP_ONOFF_USE)) != 0;

		EEntryKind kind = EEntryKind::Empty;
		int id = 0;
		const bool pickup = ClassifyPickup(pObject, kind, id);

		if (!usable && !pickup)
			continue;

		Vector vecLOS = (VecBModelOrigin(pObject->pev) - (pPlayer->pev->origin + pPlayer->pev->view_ofs));
		vecLOS = UTIL_ClampVectorToBox(vecLOS, pObject->pev->size * 0.5);

		const float flDot = DotProduct(vecLOS, gpGlobals->v_forward);
		if (flDot <= flMaxDot)
			continue;

		flMaxDot = flDot;

		// A usable entity that wins means a use press goes to it, not to any
		// pickup behind it -- so report nothing.
		out.pEntity = pickup ? pObject : nullptr;
		out.kind = pickup ? kind : EEntryKind::Empty;
		out.id = pickup ? id : 0;
	}

	return out;
}

void UpdatePickupPrompt(CBasePlayer* pPlayer)
{
	if (!pPlayer || gmsgPickupPrompt == 0)
		return;

	const LookedAtPickup look = FindLookedAtPickup(pPlayer);

	const int kind = look.Valid() ? static_cast<int>(look.kind) : 0;
	const int id = look.Valid() ? look.id : 0;

	// Only on change: the prompt is stable for as long as the player keeps
	// looking at the same thing, so there is nothing to resend each frame.
	if (kind == pPlayer->m_iPromptKind && id == pPlayer->m_iPromptId)
		return;

	pPlayer->m_iPromptKind = kind;
	pPlayer->m_iPromptId = id;

	MESSAGE_BEGIN(MSG_ONE, gmsgPickupPrompt, NULL, pPlayer->pev);
	WRITE_BYTE((unsigned char)kind);
	WRITE_BYTE((unsigned char)id);
	MESSAGE_END();
}

bool TryTakeLookedAtPickup(CBasePlayer* pPlayer)
{
	const LookedAtPickup look = FindLookedAtPickup(pPlayer);
	if (!look.Valid())
		return false;

	if (look.kind == EEntryKind::Weapon)
	{
		// Routed through the normal touch path so every piece of weapon
		// bookkeeping -- ammo extraction, auto-switch, respawn -- still runs.
		// AddPlayerItem refuses it if the Grid is full.
		DispatchTouch(ENT(look.pEntity->pev), ENT(pPlayer->pev));
		return true;
	}

	CItem* pItem = dynamic_cast<CItem*>(look.pEntity);
	if (!pItem)
		return false;

	if (!pItem->AcquireBy(pPlayer))
	{
		ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "No room in inventory.\n");
		return false;
	}

	return true;
}

//=========================================================
// Reconcile
//
// Brings the Inventory back in step with what the player demonstrably
// carries.  Needed because a save written before the Inventory existed
// restores weapons and item counts with no Entries to match, which would
// otherwise show an empty Grid to a fully equipped player.
//
// Cheap and idempotent: when everything already agrees it does nothing.
//=========================================================
static void ReconcileWithCarriedItems(CBasePlayer* pPlayer)
{
	CPlayerInventory& inv = pPlayer->m_inventory;

	// Weapons the player holds but the Grid does not know about.
	for (int slot = 0; slot < MAX_ITEM_TYPES; ++slot)
	{
		for (CBasePlayerItem* pItem = pPlayer->m_rgpPlayerItems[slot]; pItem; pItem = pItem->m_pNext)
		{
			if (pItem->m_iId <= 0)
				continue;
			if (inv.FindEntry(EEntryKind::Weapon, pItem->m_iId) >= 0)
				continue;

			inv.TryAddWeapon(pItem->m_iId);
		}
	}

	// Legacy item counters ahead of the Entries that should back them.
	for (int id = 1; id < k_MaxItemTypes && id < MAX_ITEMS; ++id)
	{
		const int shortfall = pPlayer->m_rgItems[id] - inv.TotalOfItem(id);
		if (shortfall > 0)
			inv.TryAddItem(id, shortfall);

		// Whatever actually fitted is now the truth.
		pPlayer->m_rgItems[id] = inv.TotalOfItem(id);
	}
}

void SendInventoryToClient(CBasePlayer* pPlayer)
{
	if (!pPlayer || gmsgInventory == 0)
		return;

	CPlayerInventory& inv = pPlayer->m_inventory;
	inv.EnsureInitialised();
	ReconcileWithCarriedItems(pPlayer);

	const int total = inv.EntryCount();
	int sent = 0;

	// Always send at least one message, so an emptied Inventory still
	// clears the client.
	do
	{
		const int chunk = std::min(INV_SYNC_CHUNK, total - sent);

		MESSAGE_BEGIN(MSG_ONE, gmsgInventory, NULL, pPlayer->pev);
		WRITE_BYTE(sent == 0 ? 1 : 0);                                  // reset flag
		WRITE_BYTE((unsigned char)INV_GRID_WIDTH);
		WRITE_BYTE((unsigned char)inv.Rows());
		WRITE_BYTE((unsigned char)inv.RowsToDraw());
		WRITE_BYTE((unsigned char)chunk);

		for (int i = 0; i < chunk; ++i)
		{
			const int e = sent + i;
			WRITE_BYTE((unsigned char)inv.m_rgKind[e]);
			WRITE_BYTE((unsigned char)inv.IdAt(e));
			WRITE_BYTE((unsigned char)std::min(inv.CountAt(e), 255));
			WRITE_BYTE((unsigned char)inv.ColAt(e));
			WRITE_BYTE((unsigned char)inv.RowAt(e));
		}

		MESSAGE_END();

		sent += chunk;
	} while (sent < total);
}

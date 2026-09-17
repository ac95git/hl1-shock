#pragma once

#include "record_defs.h"

// =====================================================================
// CPlayerRecords
//   Stored directly inside CBasePlayer, beside CPlayerSkills and the
//   Inventory.  The suit's memory: which Records have been registered.
//
//   The server owns this and the client only mirrors it, for the same
//   reason it owns the Inventory (docs/adr/0004): it is saved state, and
//   a Record can open a lock, so a client that could invent one could
//   open a door it never earned.
//
//   The set is a bitmask rather than a list because a Record's id IS its
//   bit (record_defs.h), which makes "has the player read this?" a test
//   with no search in it and makes the sync message a fixed 64 bytes.
// =====================================================================
struct CPlayerRecords
{
	// ---- Persistent data (save/restore) ----

	// One bit per Record id, sized to the ceiling rather than to whatever
	// records.txt happens to hold today, so the saved array keeps one shape
	// as Records are written.
	unsigned char m_found[k_RecordMaskBytes] = {};

	// ---- Transient ----

	// The Record the reader is currently showing, or k_RecordIdNone.  NOT
	// saved: a save made mid-read reloads with the reader shut, which is
	// the only sane answer -- the world around it has been rebuilt.
	int m_iReaderOpen = k_RecordIdNone;

	// Where the document being read is standing.  Walking away from it
	// shuts the page (record_read_range), which is how a reader that takes
	// no input gets closed by the one input the player always has: moving.
	Vector m_vecReaderOrigin;

	// ---- Queries ----

	bool Has(int id) const { return RecordMaskGet(m_found, id); }

	bool AnyFound() const;

	bool ReaderOpen() const { return m_iReaderOpen != k_RecordIdNone; }

	// ---- Mutations ----

	// Registers a Record in the suit's memory.  Returns true only when it
	// was NOT already held, so the caller can tell a first read from a
	// re-read without asking twice.
	bool Register(int id);

	// Takes one back.  Only Guidance is ever revoked (docs/ROADMAP.md) --
	// a found document never leaves the suit's memory -- but the mask does
	// not know the difference, so the rule lives in record_grant (slice 3).
	void Forget(int id);

	void Clear();
};

// =====================================================================
// Save/restore helpers -- implemented in player_records.cpp, mirroring
// SkillsSave/SkillsRestore.  CSave/CRestore are forward-declared so
// saverestore.h is NOT needed here.
// =====================================================================
class CSave;
class CRestore;

bool RecordsSave(CPlayerRecords& records, CSave& save);
bool RecordsRestore(CPlayerRecords& records, CRestore& restore);

class CBasePlayer;

// =====================================================================
// The glow: an unread Record glows softly and goes dark the moment it is
// registered, so "if it glows, it is unread" is a rule the player can
// rely on without ever being told it.  A record entity keeps no state of
// its own -- the found-set lights it -- so this walks them rather than
// flagging each one, and there is nothing extra for a save to carry.
//
// Implemented in record.cpp, beside the entity that answers for its own
// id.  Called by SyncPlayerRecords; nothing else should need it.
// =====================================================================
void UpdateRecordGlows(CBasePlayer* pPlayer);

// =====================================================================
// Sends the found-set and brings every record entity's glow into line
// with it.  Call after anything that changes the set, and once when a
// client arrives or a save is loaded.
//
// The two jobs are one function on purpose: a glow that disagreed with
// the tab would be the worst kind of bug to notice, since the rule the
// player is taught is "if it glows, it is unread".
// =====================================================================
void SyncPlayerRecords(CBasePlayer* pPlayer);

// =====================================================================
// A use press landed on a Record.
//
// Registers it, syncs, and opens the reader on it -- or shuts the reader
// if it was already showing that same Record, so the key that opened it
// closes it again.  Returns false only for an id records.txt could never
// hold; the client says "record not found" for one it merely does not
// have, because only the client can tell.
//
// 'vecOrigin' is where the document stands, kept so that walking away
// shuts the page (RecordReaderThink).
// =====================================================================
bool PlayerReadRecord(CBasePlayer* pPlayer, int id, const Vector& vecOrigin);

// Shuts the reader if it is open, and tells the client.  Called by damage
// and by death: reading happens in real time in an unsafe world, and the
// Record is already registered, so nothing is lost by closing it.
void CloseRecordReader(CBasePlayer* pPlayer);

// Shuts the reader once the player has walked away from what they were
// reading.  Polled each frame from PlayerPreThink -- it is one distance
// test against a stored origin, and only while a reader is open.
void RecordReaderThink(CBasePlayer* pPlayer);

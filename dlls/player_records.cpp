#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "player.h"
#include "player_records.h"
#include "UserMessages.h"
#include "game.h" // record_read_range
#include <cstring>

// A Record's text lives in records.txt and is read by the client alone
// (game_shared/record_defs.h).  Nothing in this file knows what any Record
// says -- only which ones the suit has registered.

// =====================================================================
// Local save/restore descriptor table
//
// The mask is saved under a name carrying its size, the lesson
// g_SkillsSaveData paid for: CRestore::ReadField copies as many entries
// as the CODE declares, so a save written against a smaller array would
// be over-read into the bytes after it.  k_MaxRecords was sized once and
// generously so that this name never has to change -- but if it ever
// does, rename the field with it.
// =====================================================================
static TYPEDESCRIPTION g_RecordsSaveData[] =
{
    { FIELD_CHARACTER, "m_found512", static_cast<int>(offsetof(CPlayerRecords, m_found)), k_RecordMaskBytes, 0 },

    // Added after the first Records build shipped.  A save written before it
    // simply restores with nothing granted, which means nothing revocable --
    // the safe answer, since the only thing that can be revoked is Guidance
    // and a stale Guidance line is a smaller problem than a lost document.
    { FIELD_CHARACTER, "m_granted512", static_cast<int>(offsetof(CPlayerRecords, m_granted)), k_RecordMaskBytes, 0 },
};

bool RecordsSave(CPlayerRecords& records, CSave& save)
{
    return save.WriteFields("RECORDS", &records, g_RecordsSaveData, ARRAYSIZE(g_RecordsSaveData));
}

bool RecordsRestore(CPlayerRecords& records, CRestore& restore)
{
    return restore.ReadFields("RECORDS", &records, g_RecordsSaveData, ARRAYSIZE(g_RecordsSaveData));
}

// =====================================================================
// CPlayerRecords
// =====================================================================
bool CPlayerRecords::AnyFound() const
{
    for (int i = 0; i < k_RecordMaskBytes; ++i)
    {
        if (m_found[i] != 0)
            return true;
    }
    return false;
}

bool CPlayerRecords::Register(int id)
{
    if (!RecordIdValid(id))
        return false;

    // Reading it makes it the player's own, so a later revoke can no longer
    // take it back. This is the "found" half of "a found document never
    // leaves the suit's memory" -- found means read, whatever put the bit
    // there first.
    RecordMaskSet(m_granted, id, false);

    if (Has(id))
        return false;

    RecordMaskSet(m_found, id, true);
    return true;
}

bool CPlayerRecords::Grant(int id)
{
    if (!RecordIdValid(id) || Has(id))
        return false;

    // Only a grant that actually ADDED something marks it revocable. A grant
    // aimed at a Record the player already has changes nothing at all -- in
    // particular it cannot make a document they read revocable, which would
    // be a back door through the rule Forget exists to enforce.
    RecordMaskSet(m_found, id, true);
    RecordMaskSet(m_granted, id, true);
    return true;
}

bool CPlayerRecords::Forget(int id)
{
    if (!RecordIdValid(id) || !Has(id))
        return false;

    // The whole rule, in one test.
    if (!RecordMaskGet(m_granted, id))
        return false;

    RecordMaskSet(m_found, id, false);
    RecordMaskSet(m_granted, id, false);
    return true;
}

void CPlayerRecords::Clear()
{
    memset(m_found, 0, sizeof(m_found));
    memset(m_granted, 0, sizeof(m_granted));
    m_iReaderOpen = k_RecordIdNone;
}

// =====================================================================
// SyncPlayerRecords
//
// The glow goes out in the same breath as the mask is sent, because the
// rule the player is taught -- if it glows, it is unread -- is only
// worth teaching if the two can never disagree.
// =====================================================================
void SyncPlayerRecords(CBasePlayer* pPlayer)
{
    if (!pPlayer)
        return;

    UpdateRecordGlows(pPlayer);

    if (gmsgRecords == 0)
        return;

    MESSAGE_BEGIN(MSG_ONE, gmsgRecords, NULL, pPlayer->pev);
    for (int i = 0; i < k_RecordMaskBytes; ++i)
        WRITE_BYTE(pPlayer->m_records.m_found[i]);
    MESSAGE_END();
}

// =====================================================================
// The reader
// =====================================================================
static void SendReaderState(CBasePlayer* pPlayer)
{
    if (gmsgRecordRead == 0)
        return;

    MESSAGE_BEGIN(MSG_ONE, gmsgRecordRead, NULL, pPlayer->pev);
    WRITE_SHORT(pPlayer->m_records.m_iReaderOpen);
    MESSAGE_END();
}

void CloseRecordReader(CBasePlayer* pPlayer)
{
    if (!pPlayer || !pPlayer->m_records.ReaderOpen())
        return;

    pPlayer->m_records.m_iReaderOpen = k_RecordIdNone;
    SendReaderState(pPlayer);
}

void RecordReaderThink(CBasePlayer* pPlayer)
{
    if (!pPlayer || !pPlayer->m_records.ReaderOpen())
        return;

    const float flRange = record_read_range.value;
    if (flRange <= 0.0f)
        return;

    if ((pPlayer->pev->origin - pPlayer->m_records.m_vecReaderOrigin).Length() > flRange)
        CloseRecordReader(pPlayer);
}

bool PlayerReadRecord(CBasePlayer* pPlayer, int id, const Vector& vecOrigin)
{
    if (!pPlayer || !RecordIdValid(id))
        return false;

    // Reading is intentional and repeatable: the document stays in the
    // world and re-opens on every use (a deliberate exception to
    // ADR-0011, which is about TAKING).  So the second press on the same
    // Record shuts the reader -- the key that opened it closes it.
    if (pPlayer->m_records.m_iReaderOpen == id)
    {
        CloseRecordReader(pPlayer);
        return true;
    }

    const bool bNew = pPlayer->m_records.Register(id);

    pPlayer->m_records.m_iReaderOpen = id;
    pPlayer->m_records.m_vecReaderOrigin = vecOrigin;
    SendReaderState(pPlayer);

    // Only a first read changes the set, and only a first read is worth a
    // sync -- but the glow has to go out either way, and SyncPlayerRecords
    // is what puts it out.
    if (bNew)
        SyncPlayerRecords(pPlayer);

    return true;
}

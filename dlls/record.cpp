//=========================================================
// record.cpp
//
// The `record` entity: a Record as it stands in the world.
//
// Two forms, one class (docs/ROADMAP.md, "Pillar 1: Records"): a loose
// document, which is a model, and a fixed source -- a terminal, a wall
// notice, a roster -- which is the mapper's brushwork.  Both are read
// with +use, both stay where they are, and both re-open on every press.
//
// Reading is deliberately NOT walk-over.  That is an exception to
// docs/adr/0011-pickups-are-walk-over.md with its own reason: the ADR is
// about TAKING, and a Record is READ -- attention, which is what +use
// already means for a scientist.  A document absorbed by walking over it
// might never be read at all, and exploration is the first-ranked pillar.
// Written down here so nobody fixes it back.
//
// The entity holds no state.  Whether it has been read is the player's
// found-set (dlls/player_records.cpp), which is what lights its glow --
// so there is nothing here for a save to carry and nothing to get out of
// step with the Records tab.
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "player.h"
#include "player_records.h"
#include "effects.h" // CSprite, for the brush form's halo
#include "game.h"    // record_glow_shell / _halo

// Pinned where it was placed rather than settling onto what is under it:
// for a document flat against a wall, or on a shelf the mapper has already
// lined up by eye.
#define SF_RECORD_FIXED 1

class CRecord : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;
	bool KeyValue(KeyValueData* pkvd) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	// FCAP_IMPULSE_USE is what puts it in front of CBasePlayer::PlayerUse,
	// and with it in front of FindLookedAtPickup -- so the Prompt and the
	// press cannot disagree about what a use would act on.
	int ObjectCaps() override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_IMPULSE_USE; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	int RecordId() const { return m_iRecordId; }

	// Shows or hides the unread glow.  Driven by the found-set, never by
	// the entity, which is why it is public and why nothing here is saved
	// beyond the halo's own handle.
	void SetGlow(bool bOn);

	// Re-applies the glow from the cvars, twice a second, so that tuning
	// the look takes effect where it is being looked at.  Without it the
	// cvars only bite on a found-set sync -- which needs sv_cheats and a
	// console command, and is no way to judge a colour (Andrei, 2026-09-18).
	void EXPORT GlowThink();

private:
	void CreateGlow();
	void ApplyGlow();

	int m_iRecordId = k_RecordIdNone;

	// Only a BRUSH Record has one: a loose document glows with a shell on
	// its own model and needs no second entity.
	EHANDLE m_hHalo;

	// Whether this Record is unread, as of the last found-set sync.  Not
	// saved: SyncPlayerRecords sets it on every spawn and every restore, so
	// a saved copy could only ever be a chance to disagree with the mask.
	bool m_bGlowOn = true;
};

// One class, two classnames.  The forms are one entity as designed -- same
// code, same read, same glow -- but an FGD cannot declare one class both
// @PointClass and @SolidClass, and a mapper who has to type a classname by
// hand will not use the feature.  So the split is in the editor's
// vocabulary only; everything below is shared.
LINK_ENTITY_TO_CLASS(record, CRecord);
LINK_ENTITY_TO_CLASS(record_brush, CRecord);

// For the glow walk below. ClassifyUsable (player_inventory.cpp) keeps
// both names in its own table, where every other usable class is listed.
static const char* const k_RecordClassnames[] = {"record", "record_brush"};
static const int k_NumRecordClassnames = 2;

TYPEDESCRIPTION CRecord::m_SaveData[] =
	{
		DEFINE_FIELD(CRecord, m_iRecordId, FIELD_INTEGER),
		DEFINE_FIELD(CRecord, m_hHalo, FIELD_EHANDLE),
};

IMPLEMENT_SAVERESTORE(CRecord, CBaseEntity);

bool CRecord::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "record_id"))
	{
		m_iRecordId = atoi(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

void CRecord::Precache()
{
	if (!FStringNull(pev->model) && STRING(pev->model)[0] != '*')
		PRECACHE_MODEL(STRING(pev->model));

	// Only the brush form uses it, but precaching is cheap and a Record
	// that had to decide its form before Precache would be fragile.
	PRECACHE_MODEL(RECORD_GLOW_SPRITE);
}

void CRecord::Spawn()
{
	// A mapper who forgot the id gets one complaint and an entity that
	// still stands there, rather than a silent prop that reads as nothing.
	if (!RecordIdValid(m_iRecordId))
	{
		ALERT(at_error, "record at (%.0f %.0f %.0f) has no valid record_id (%d)\n",
			pev->origin.x, pev->origin.y, pev->origin.z, m_iRecordId);
	}

	// No model set means the loose form with its stand-in
	// (docs/ART_DEBT.md): a security card is the flattest thing vanilla
	// ships, and it is meant to look wrong until the document model exists.
	if (FStringNull(pev->model))
		pev->model = MAKE_STRING(RECORD_PLACEHOLDER_MODEL);

	Precache();

	const bool bBrush = STRING(pev->model)[0] == '*';

	SET_MODEL(ENT(pev), STRING(pev->model));

	if (bBrush)
	{
		// The fixed source: a terminal, a notice, a roster.  Solid, because
		// it is part of the wall the mapper built it into.
		pev->solid = SOLID_BSP;
		pev->movetype = MOVETYPE_PUSH;
	}
	else
	{
		// The loose document. SOLID_TRIGGER with no touch function is inert
		// -- nothing can be picked up by walking into it -- but it is what
		// puts the entity in the engine's sphere queries, which is how both
		// PlayerUse and the Prompt find it.
		pev->solid = SOLID_TRIGGER;
		UTIL_SetSize(pev, Vector(-8, -8, 0), Vector(8, 8, 8));
		UTIL_SetOrigin(pev, pev->origin);

		if (FBitSet(pev->spawnflags, SF_RECORD_FIXED))
		{
			// Pinned where it was placed: on a shelf the mapper trusts, or
			// flat against a wall with nothing under it.
			pev->movetype = MOVETYPE_NONE;
		}
		else
		{
			// Otherwise it settles onto whatever is beneath it, the way
			// every other placed thing in Half-Life does -- which lands it
			// on the desk as readily as on the floor, and spares the mapper
			// finding the exact height (docs/adr, and the mod rule that a
			// vanilla verb wins).
			pev->movetype = MOVETYPE_TOSS;
			pev->origin.z += 1;
			if (DROP_TO_FLOOR(ENT(pev)) == 0)
			{
				ALERT(at_error, "record %d fell out of the level at (%.0f %.0f %.0f)\n",
					m_iRecordId, pev->origin.x, pev->origin.y, pev->origin.z);
			}
		}
	}

	// Unread until a player's found-set says otherwise; SyncPlayerRecords
	// corrects it the moment one arrives or a save is loaded.
	CreateGlow();
	SetGlow(true);

	// And from here the glow follows its cvars on its own clock, so the
	// look can be tuned by eye in front of the thing being tuned.
	SetThink(&CRecord::GlowThink);
	pev->nextthink = gpGlobals->time + 0.5;
}

//=========================================================
// The glow
//
// An unread Record glows, and goes dark the moment the suit registers it.
// Not a light in the room: a dynamic light was the first attempt -- it is
// what the progression pickups use -- and it lit the whole corner, which
// reads as a beacon (Andrei, 2026-09-18).  "Glint" was the word tried
// next and describes the wrong thing; a glint is a spark catching the
// light, and what this wants is the document itself softly lit.
//
// Two forms, two mechanisms, because no one effect suits both:
//
//  - A loose document is a studio model, so it wears a
//    kRenderFxGlowShell -- the model rendered a second time, pushed out
//    along its normals and tinted.  The object glows, and nothing else
//    does.  It is the buster egon's effect (multiplay_gamerules.cpp), so
//    renderamt 25 is a known-good starting thickness.
//  - A fixed source is brushwork, which never reaches the studio
//    renderer, so it gets a soft halo sprite at its centre.  kRenderGlow
//    occludes behind geometry and, without kRenderFxNoDissipation, fades
//    with distance the way a real haze would.
//=========================================================
void CRecord::CreateGlow()
{
	// A loose document needs no second entity -- its glow is a render flag
	// on itself, applied in SetGlow.
	if (STRING(pev->model)[0] != '*')
		return;

	if (static_cast<CBaseEntity*>(m_hHalo) != nullptr)
		return;

	// Created even with the cvar at zero, so turning the glow back up
	// mid-game works without a map reload: SetGlow re-reads it on every
	// sync of the found-set.
	CSprite* pHalo = CSprite::SpriteCreate(RECORD_GLOW_SPRITE, Center(), false);
	if (!pHalo)
		return;

	pHalo->pev->owner = edict();
	m_hHalo = pHalo;
}

void CRecord::SetGlow(bool bOn)
{
	m_bGlowOn = bOn;
	ApplyGlow();
}

void CRecord::GlowThink()
{
	ApplyGlow();
	pev->nextthink = gpGlobals->time + 0.5;
}

void CRecord::ApplyGlow()
{
	const bool bOn = m_bGlowOn;

	// Pale and cold: paper under a scanner, and deliberately not the cyan
	// and gold the progression pickups own.
	constexpr int k_R = 190, k_G = 225, k_B = 255;

	if (CBaseEntity* pHalo = m_hHalo)
	{
		// The brush form.
		const float scale = record_glow_halo.value;
		if (!bOn || scale <= 0.0f)
		{
			pHalo->pev->effects |= EF_NODRAW;
		}
		else
		{
			static_cast<CSprite*>(pHalo)->SetTransparency(kRenderGlow, k_R, k_G, k_B, 110, kRenderFxNone);
			static_cast<CSprite*>(pHalo)->SetScale(scale);
			pHalo->pev->effects &= ~EF_NODRAW;
		}
		return;
	}

	// The loose form: the shell on the model itself.
	const int shell = static_cast<int>(record_glow_shell.value);
	if (!bOn || shell <= 0)
	{
		pev->renderfx = kRenderFxNone;
		pev->renderamt = 0;
	}
	else
	{
		pev->renderfx = kRenderFxGlowShell;
		pev->renderamt = shell;
		pev->rendercolor = Vector(k_R, k_G, k_B);
	}
}

void CRecord::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBasePlayer* pPlayer = (pActivator != nullptr) ? dynamic_cast<CBasePlayer*>(pActivator) : nullptr;
	if (!pPlayer)
		return;

	// Asked BEFORE the read, because the read is what makes it false.
	const bool bFirstRead = !pPlayer->m_records.Has(m_iRecordId);

	// Center() rather than pev->origin, so a brush Record measures from the
	// middle of the terminal and not from a corner of its bounding box.
	PlayerReadRecord(pPlayer, m_iRecordId, Center());

	// A Record's first read can fire a target -- which is how a remembered
	// global gets set for the endings without any code here knowing that
	// endings exist (docs/ROADMAP.md, "The tab").  First read only: a
	// document that stays in the world is re-readable, and a trigger that
	// fired every time would make "the player has read this" mean nothing.
	if (bFirstRead && !FStringNull(pev->target))
		SUB_UseTargets(pPlayer, USE_TOGGLE, 0);
}

//=========================================================
// UpdateRecordGlows  (declared in player_records.h)
//
// Entirely server-side -- a render flag and a sprite the server owns -- so
// the client is told nothing and has nothing to get wrong.
//=========================================================
void UpdateRecordGlows(CBasePlayer* pPlayer)
{
	if (!pPlayer)
		return;

	for (int i = 0; i < k_NumRecordClassnames; ++i)
	{
		CBaseEntity* pEnt = nullptr;
		while ((pEnt = UTIL_FindEntityByClassname(pEnt, k_RecordClassnames[i])) != nullptr)
		{
			CRecord* pRecord = static_cast<CRecord*>(pEnt);
			pRecord->SetGlow(!pPlayer->m_records.Has(pRecord->RecordId()));
		}
	}
}

//=========================================================
// record_grant
//
// Hands a Record over, or takes one back, at a scripted moment.  This is
// how Guidance works: a Guidance line is an ordinary Record in a pinned
// category, and "done" is a revoke that optionally grants the next one --
// so there is no second objectives system to save, sync and debug
// (docs/ROADMAP.md, "The tab").
//
// NOTHING FAILS.  Granting what is already held, revoking what is not,
// naming an id records.txt has never heard of: all silent.  A mapper
// wiring a chain of these should never have to reason about order, and a
// trigger that could fail would make them.
//
// The same entity hands over any Record, not only Guidance -- a briefing
// the player is given rather than finds.  Only a GRANTED Record can be
// revoked (CPlayerRecords::Forget), so pointing the revoke at a real
// document quietly does nothing rather than erasing it.
//=========================================================
#define SF_RECORD_GRANT_ONCE 1

class CRecordGrant : public CPointEntity
{
public:
	bool KeyValue(KeyValueData* pkvd) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

private:
	int m_iGrantId = k_RecordIdNone;
	int m_iRevokeId = k_RecordIdNone;
	bool m_bSpent = false;
};

LINK_ENTITY_TO_CLASS(record_grant, CRecordGrant);

TYPEDESCRIPTION CRecordGrant::m_SaveData[] =
	{
		DEFINE_FIELD(CRecordGrant, m_iGrantId, FIELD_INTEGER),
		DEFINE_FIELD(CRecordGrant, m_iRevokeId, FIELD_INTEGER),
		DEFINE_FIELD(CRecordGrant, m_bSpent, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CRecordGrant, CPointEntity);

bool CRecordGrant::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "record_id"))
	{
		m_iGrantId = atoi(pkvd->szValue);
		return true;
	}
	if (FStrEq(pkvd->szKeyName, "revoke_id"))
	{
		m_iRevokeId = atoi(pkvd->szValue);
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}

void CRecordGrant::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (m_bSpent)
		return;

	// Single player: whoever triggered this, the suit that remembers is the
	// one player's. Taking the activator when it is a player keeps the door
	// open for anything that later cares.
	CBasePlayer* pPlayer = (pActivator != nullptr) ? dynamic_cast<CBasePlayer*>(pActivator) : nullptr;
	if (!pPlayer)
		pPlayer = static_cast<CBasePlayer*>(UTIL_PlayerByIndex(1));
	if (!pPlayer)
		return;

	// Revoke first, so that "this objective is done, here is the next one"
	// is one entity and reads in that order in the tab.
	bool bChanged = pPlayer->m_records.Forget(m_iRevokeId);
	bChanged = pPlayer->m_records.Grant(m_iGrantId) || bChanged;

	if (bChanged)
		SyncPlayerRecords(pPlayer);

	if (FBitSet(pev->spawnflags, SF_RECORD_GRANT_ONCE))
		m_bSpent = true;

	// Fires whether or not anything changed: a chain must not stall because
	// the player had already been given the line.
	SUB_UseTargets(pPlayer, USE_TOGGLE, 0);
}

//=========================================================
// record_lock
//
// Names a Record and fires its target if the suit has it.  The first
// reusable form of the soft-gate rule (docs/ROADMAP.md): the intended way
// through is knowing something, and the mapper's vent or window is the
// alternative.
//
// NO TYPING.  GoldSrc has no keypad, and VGUI1 text entry is awkward
// enough that the Inventory Panel avoids it -- so knowing the code IS
// entering it.  Accepted knowingly: a code the player remembers from a
// previous run still has to be found again.
//
// Knowledge costs no Cells, where a keycard would tax a scarce Inventory.
// It takes a granted Record as readily as a found one, because the suit's
// memory does not distinguish and neither should a door.
//=========================================================
class CRecordLock : public CBaseEntity
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

	int RecordId() const { return m_iRecordId; }

private:
	int m_iRecordId = k_RecordIdNone;
};

// Two classnames for the same reason `record` has two: an FGD cannot
// declare one class both @PointClass and @SolidClass, and a lock is most
// naturally a keypad the mapper builds out of brushes.
LINK_ENTITY_TO_CLASS(record_lock, CRecordLock);
LINK_ENTITY_TO_CLASS(record_lock_brush, CRecordLock);

const char* const k_RecordLockClassnames[] = {"record_lock", "record_lock_brush"};
const int k_NumRecordLockClassnames = 2;

// The refusal. Vanilla's own access-denied beep, so a locked thing in this
// mod sounds like a locked thing in Half-Life.
static const char* const k_RecordLockDeniedSound = "buttons/button11.wav";

TYPEDESCRIPTION CRecordLock::m_SaveData[] =
	{
		DEFINE_FIELD(CRecordLock, m_iRecordId, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CRecordLock, CBaseEntity);

bool CRecordLock::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "record_id"))
	{
		m_iRecordId = atoi(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

void CRecordLock::Precache()
{
	PRECACHE_SOUND(k_RecordLockDeniedSound);

	if (!FStringNull(pev->model) && STRING(pev->model)[0] != '*')
		PRECACHE_MODEL(STRING(pev->model));
}

void CRecordLock::Spawn()
{
	Precache();

	if (!FStringNull(pev->model))
	{
		SET_MODEL(ENT(pev), STRING(pev->model));

		if (STRING(pev->model)[0] == '*')
		{
			pev->solid = SOLID_BSP;
			pev->movetype = MOVETYPE_PUSH;
			return;
		}
	}

	// A point lock with no model is a bare interaction point -- the keypad
	// is the mapper's brushwork nearby, or there is nothing to see at all.
	// It still needs a size, or the aim test has nothing to aim at.
	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_NONE;
	UTIL_SetSize(pev, Vector(-8, -8, -8), Vector(8, 8, 8));
	UTIL_SetOrigin(pev, pev->origin);
}

void CRecordLock::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBasePlayer* pPlayer = (pActivator != nullptr) ? dynamic_cast<CBasePlayer*>(pActivator) : nullptr;
	if (!pPlayer)
		return;

	if (!pPlayer->m_records.Has(m_iRecordId))
	{
		// The Prompt already said "Code required", so the sound is
		// confirmation rather than news -- but a press that did nothing at
		// all would read as the press having been missed.
		EMIT_SOUND(ENT(pev), CHAN_ITEM, k_RecordLockDeniedSound, 1.0, ATTN_NORM);
		return;
	}

	SUB_UseTargets(pPlayer, USE_TOGGLE, 0);
}

//=========================================================
// RecordLockPromptClass  (declared in player_records.h)
//
// Which of the lock's two faces to show.  It lives here, beside the Use
// that has to agree with it, so the Prompt and the press cannot drift --
// the same reason FindLookedAtPickup mirrors PlayerUse's aim test.
//=========================================================
bool IsRecordLock(CBaseEntity* pEnt)
{
	if (!pEnt)
		return false;

	for (int i = 0; i < k_NumRecordLockClassnames; ++i)
	{
		if (FClassnameIs(pEnt->pev, k_RecordLockClassnames[i]))
			return true;
	}
	return false;
}

bool RecordLockIsOpen(CBaseEntity* pEnt, CBasePlayer* pPlayer)
{
	if (!pPlayer || !IsRecordLock(pEnt))
		return false;

	return pPlayer->m_records.Has(static_cast<CRecordLock*>(pEnt)->RecordId());
}

/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
/*

===== items.cpp ========================================================

  functions governing the selection/use of weapons for players

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"
#include "skill.h"
#include "items.h"
#include "gamerules.h"
#include "UserMessages.h"

class CWorldItem : public CBaseEntity
{
public:
	bool KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	int m_iType;
};

LINK_ENTITY_TO_CLASS(world_items, CWorldItem);

bool CWorldItem::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "type"))
	{
		m_iType = atoi(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

void CWorldItem::Spawn()
{
	CBaseEntity* pEntity = NULL;

	switch (m_iType)
	{
	case 44: // ITEM_BATTERY:
		pEntity = CBaseEntity::Create("item_battery", pev->origin, pev->angles);
		break;
	case 42: // ITEM_ANTIDOTE:
		pEntity = CBaseEntity::Create("item_antidote", pev->origin, pev->angles);
		break;
	case 43: // ITEM_SECURITY:
		pEntity = CBaseEntity::Create("item_security", pev->origin, pev->angles);
		break;
	case 45: // ITEM_SUIT:
		pEntity = CBaseEntity::Create("item_suit", pev->origin, pev->angles);
		break;
	}

	if (!pEntity)
	{
		ALERT(at_console, "unable to create world_item %d\n", m_iType);
	}
	else
	{
		pEntity->pev->target = pev->target;
		pEntity->pev->targetname = pev->targetname;
		pEntity->pev->spawnflags = pev->spawnflags;
	}

	REMOVE_ENTITY(edict());
}


void CItem::Spawn()
{
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;
	UTIL_SetOrigin(pev, pev->origin);
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));
	SetTouch(&CItem::ItemTouch);

	if (DROP_TO_FLOOR(ENT(pev)) == 0)
	{
		ALERT(at_error, "Item %s fell out of level at %f,%f,%f", STRING(pev->classname), pev->origin.x, pev->origin.y, pev->origin.z);
		UTIL_Remove(this);
		return;
	}
}

void CItem::FinishAcquire(CBasePlayer* pPlayer)
{
	SUB_UseTargets(pPlayer, USE_TOGGLE, 0);
	SetTouch(NULL);

	g_pGameRules->PlayerGotItem(pPlayer, this);
	if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_YES)
	{
		Respawn();
	}
	else
	{
		UTIL_Remove(this);
	}
}

bool CItem::AcquireBy(CBasePlayer* pPlayer)
{
	if (!pPlayer)
		return false;

	if (!g_pGameRules->CanHaveItem(pPlayer, this))
		return false;

	if (!MyTouch(pPlayer))
	{
		if (gEvilImpulse101)
			UTIL_Remove(this);
		return false;
	}

	FinishAcquire(pPlayer);
	return true;
}

void CItem::ItemTouch(CBaseEntity* pOther)
{
	// if it's not a player, ignore
	if (!pOther->IsPlayer())
	{
		return;
	}

	CBasePlayer* pPlayer = (CBasePlayer*)pOther;

	// ok, a player is touching this item, but can he have it?
	if (!g_pGameRules->CanHaveItem(pPlayer, this))
	{
		// no? Ignore the touch.
		return;
	}

	// Using it on contact beats carrying it, when nothing would be wasted.
	if (ConsumeOnContact(pPlayer))
	{
		FinishAcquire(pPlayer);
		return;
	}

	// Otherwise Inventory items wait for a deliberate use press; walking over
	// them does nothing. The Pickup Prompt tells the player they can take it.
	if (!AutoPickupOnTouch())
	{
		return;
	}

	AcquireBy(pPlayer);
}

CBaseEntity* CItem::Respawn()
{
	SetTouch(NULL);
	pev->effects |= EF_NODRAW;

	UTIL_SetOrigin(pev, g_pGameRules->VecItemRespawnSpot(this)); // blip to whereever you should respawn.

	SetThink(&CItem::Materialize);
	pev->nextthink = g_pGameRules->FlItemRespawnTime(this);
	return this;
}

void CItem::Materialize()
{
	if ((pev->effects & EF_NODRAW) != 0)
	{
		// changing from invisible state to visible.
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM, 0, 150);
		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	SetTouch(&CItem::ItemTouch);
}

#define SF_SUIT_SHORTLOGON 0x0001

class CItemSuit : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_suit.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_suit.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if (pPlayer->HasSuit())
			return false;

		if ((pev->spawnflags & SF_SUIT_SHORTLOGON) != 0)
			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_A0"); // short version of suit logon,
		else
			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_AAx"); // long version of suit logon

		pPlayer->SetHasSuit(true);
		return true;
	}
};

LINK_ENTITY_TO_CLASS(item_suit, CItemSuit);



class CItemBattery : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_battery.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_battery.mdl");
		PRECACHE_SOUND("items/gunpickup2.wav");
	}
	bool AutoPickupOnTouch() override { return false; }
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if (pPlayer->pev->deadflag != DEAD_NO)
			return false;

		// Require HEV suit so batteries still make sense contextually.
		if (!pPlayer->HasSuit())
			return false;

		// Add to the Inventory instead of charging armor immediately.
		// A full Grid refuses it and the battery stays in the world.
		if (InventoryGiveItem(pPlayer, EItemTypeId::Battery) <= 0)
			return false;

		EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

		// Standard ItemPickup for pickup history HUD.
		MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
		WRITE_STRING(STRING(pev->classname));
		MESSAGE_END();

		return true;
	}
};

LINK_ENTITY_TO_CLASS(item_battery, CItemBattery);


class CItemAntidote : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_antidote.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_antidote.mdl");
	}
	bool AutoPickupOnTouch() override { return false; }
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if (InventoryGiveItem(pPlayer, EItemTypeId::Antidote) <= 0)
			return false;

		pPlayer->SetSuitUpdate("!HEV_DET4", false, SUIT_NEXT_IN_1MIN);
		return true;
	}
};

LINK_ENTITY_TO_CLASS(item_antidote, CItemAntidote);


class CItemSecurity : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_security.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_security.mdl");
	}
	bool AutoPickupOnTouch() override { return false; }
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		return InventoryGiveItem(pPlayer, EItemTypeId::Keycard) > 0;
	}
};

LINK_ENTITY_TO_CLASS(item_security, CItemSecurity);


//=========================================================
// The Health Syringe -- the first Item Type that is ours rather
// than Half-Life's.  Everything genuinely new about it is the
// Infusion it starts; as a pickup it is an ordinary CItem.
//
// w_adrenaline.mdl is a placeholder and lives in valve/models,
// which the mod gets by game-directory fallback.
//=========================================================
class CItemSyringe : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_adrenaline.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_adrenaline.mdl");
		PRECACHE_SOUND("items/smallmedkit1.wav");
	}
	// Use-to-take, never Auto-Consume: waste is not computable for an effect
	// that pays out over time, so the test Auto-Consume relies on cannot exist.
	bool AutoPickupOnTouch() override { return false; }
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		// A full Grid refuses it and the Syringe stays in the world.
		if (InventoryGiveItem(pPlayer, EItemTypeId::Syringe) <= 0)
			return false;

		// Pickup feedback. Borrowed from the medkit as a stand-in -- the Syringe
		// deliberately does not sound like a medkit when USED, so this one is
		// the placeholder most likely to need replacing.
		EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/smallmedkit1.wav", 1, ATTN_NORM);

		return true;
	}
};

LINK_ENTITY_TO_CLASS(item_syringe, CItemSyringe);


//=========================================================
// Progression pickups -- Skill Points, Reset Tokens and Row Grants.
//
// None of the three is an Item Type.  All are banked counters, so none
// occupies a Cell and a full Grid can refuse none of them -- which
// matters, because a progression reward that stays on the floor reads
// as a bug.  All are taken on contact, because none carries a decision
// at the moment of the pickup: a Skill Point is spent from the tree, a
// Reset Token from the Upgrades tab, and a Row Grant spends itself.
//
// Every model below is a placeholder -- see docs/ART_DEBT.md.
//=========================================================

// The three are dressed alike so they read as one family, and apart from
// equipment.  The glow shell is doing work the models cannot: every stand-in
// is a borrowed prop that already means something else in Half-Life, and the
// shell is what says "progression" before the player is near enough to make
// out the shape.  Rank rides on colour -- cyan is common, gold is scarce.
//
// Two things that are not obvious.  renderamt is shell THICKNESS here rather
// than opacity (the precedent is dlls/player.cpp:749), and pev->scale is
// networked (dlls/client.cpp:1490), so a prop built at the wrong size can be
// brought down to pickup scale without an art pass.
static void SetProgressionLook(entvars_t* pev, const Vector& colour, float scale)
{
	pev->renderfx = kRenderFxGlowShell;
	pev->rendercolor = colour;
	pev->renderamt = 25;
	pev->scale = scale;
}

// crystal.mdl is a Xen crystal formation, 54x54x94 units -- taller than the
// player, hence the quarter scale.  It replaces w_longjump.mdl, which was
// actively misleading rather than merely unevocative: the longjump module is
// a real pickup the player can also find, and Modules will add more of them.
class CItemSkillPoint : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/crystal.mdl");
		SetProgressionLook(pev, Vector(0, 200, 255), 0.25f);
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/crystal.mdl");
		PRECACHE_SOUND("items/gunpickup2.wav");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		pPlayer->m_skills.AddSkillPoints(1);

		ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "Skill Point acquired.\n");
		EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

		SendSkillTreeToClient(pPlayer);
		return true;
	}
};

LINK_ENTITY_TO_CLASS(item_skillpoint, CItemSkillPoint);


// sphere.mdl is a small unused orb (8x8x6), scaled up slightly.  A smooth
// ball is deliberately a different shape class from the Skill Point's
// faceted shard, so the two are told apart at a glance -- which the old
// w_security.mdl could not manage, being a literal door key.
class CItemResetToken : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/sphere.mdl");
		SetProgressionLook(pev, Vector(255, 190, 40), 1.5f);
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/sphere.mdl");
		PRECACHE_SOUND("items/gunpickup2.wav");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		pPlayer->m_skills.AddResetTokens(1);

		ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "Reset Token acquired.\n");
		EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

		SendSkillTreeToClient(pPlayer);
		return true;
	}
};

LINK_ENTITY_TO_CLASS(item_resettoken, CItemResetToken);


//=========================================================
// The Row Grant -- a permanent +1 to the Inventory's Row count.
//
// Named item_rowgrant, not item_inventory_upgrade as docs/MAP_BRIEF.md
// proposed: CONTEXT.md settles the term as "Row Grant" and lists
// "upgrade" among the words to avoid for it.
//
// w_isotopebox.mdl is a shipping case with a handle, which is as close as
// Half-Life's model set gets to "you can carry more".  Deliberately NOT
// w_weaponbox.mdl, which is the better metaphor and is already spoken for:
// lootable Boxes will use it, and the two things that grow what a player
// carries must not look identical.
//=========================================================
class CItemRowGrant : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_isotopebox.mdl");
		SetProgressionLook(pev, Vector(80, 255, 80), 1.0f);
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_isotopebox.mdl");
		PRECACHE_SOUND("items/gunpickup2.wav");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		// Rows are the one progression reward with a real ceiling
		// (inv_rows_max), so this is the one pickup that can arrive with
		// nothing left to give.  It refuses and stays standing rather than
		// being consumed for nothing -- a map that trips this has placed
		// more Grants than the ceiling allows, and the author wants to see
		// it still sitting there.
		if (pPlayer->m_inventory.GrantRows(1) <= 0)
		{
			// ItemTouch fires every frame of the overlap, so the refusal
			// is throttled where the success below does not need to be.
			if (gpGlobals->time >= m_flNextRefusalMessage)
			{
				ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "Your Grid is already at its maximum size.\n");
				m_flNextRefusalMessage = gpGlobals->time + 3.0f;
			}
			return false;
		}

		ClientPrint(pPlayer->pev, HUD_PRINTCENTER, "Inventory Row gained.\n");
		EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

		SendInventoryToClient(pPlayer);
		return true;
	}

	// Deliberately not in a save table: it is a message throttle, and the
	// worst a reload can cost is one repeated centre-print.
	float m_flNextRefusalMessage = 0;
};

LINK_ENTITY_TO_CLASS(item_rowgrant, CItemRowGrant);

class CItemLongJump : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_longjump.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_longjump.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if (pPlayer->m_fLongJump)
		{
			return false;
		}

		if (pPlayer->HasSuit())
		{
			pPlayer->m_fLongJump = true; // player now has longjump module

			g_engfuncs.pfnSetPhysicsKeyValue(pPlayer->edict(), "slj", "1");

			MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
			WRITE_STRING(STRING(pev->classname));
			MESSAGE_END();

			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_A1"); // Play the longjump sound UNDONE: Kelly? correct sound?
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_longjump, CItemLongJump);

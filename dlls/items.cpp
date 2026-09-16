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
#include "suit_defs.h"
#include "game.h"

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
		if (item_debug.value != 0)
			ALERT(at_console, "[item] %s touched: game rules refuse\n", STRING(pev->classname));
		return;
	}

	// Using it on contact beats carrying it, when nothing would be wasted.
	if (ConsumeOnContact(pPlayer))
	{
		if (item_debug.value != 0)
			ALERT(at_console, "[item] %s touched: consumed on contact\n", STRING(pev->classname));
		FinishAcquire(pPlayer);
		return;
	}

	// Otherwise walking over it takes it, as in Half-Life. What refuses here
	// waits for a use press, and the Pickup Prompt names it meanwhile.
	//
	// impulse 101 hands its list straight to the player through this touch,
	// so it never waits, or its suit would be left at the player's feet.
	if (!AutoPickupOnTouch(pPlayer) && !gEvilImpulse101)
	{
		if (item_debug.value != 0)
			ALERT(at_console, "[item] %s touched: waits for a use press\n", STRING(pev->classname));
		return;
	}

	const bool taken = AcquireBy(pPlayer);
	if (item_debug.value != 0)
		ALERT(at_console, "[item] %s touched: %s\n", STRING(pev->classname), taken ? "taken" : "refused by MyTouch");
}

void CItem::DisarmUntilClear()
{
	SetTouch(NULL);
	SetThink(&CItem::ArmWhenClear);
	pev->nextthink = gpGlobals->time + 0.1f;
}

void CItem::ArmWhenClear()
{
	pev->nextthink = gpGlobals->time + 0.1f;

	if ((pev->flags & FL_ONGROUND) == 0)
		return;

	// Anyone still standing in it keeps it disarmed: the drop spawns inside
	// the dropper's own box, and a slow toss can land there too.
	for (int i = 1; i <= gpGlobals->maxClients; ++i)
	{
		CBaseEntity* pPlayer = UTIL_PlayerByIndex(i);
		if (pPlayer && pPlayer->Intersects(this))
			return;
	}

	if (item_debug.value != 0)
		ALERT(at_console, "[item] %s dropped: armed\n", STRING(pev->classname));

	SetTouch(&CItem::ItemTouch);
	SetThink(NULL);
}

void CItem::AnnouncePickup(CBasePlayer* pPlayer, bool carried)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
	WRITE_STRING(STRING(pev->classname));
	WRITE_BYTE(carried ? 1 : 0);
	MESSAGE_END();
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

//=========================================================
// The HEV suit, in three Suit Variants.
//
// One class and one keyvalue rather than a second classname: a vanilla
// item_suit with no "variant" is Agility, so stock maps are unchanged.
// pev->skin carries the variant at both ends -- the pickup's own skin
// family while it lies on the floor, and the player's skin once worn,
// which is what the client reads back for the gloves and the HUD.
//
// Walk-over for a player with no suit, exactly as in Half-Life, so the
// locker at Anomalous Materials still works by walking into it.  Switching
// is use-only: the old variant does not drop, so a red suit must never be
// taken by brushing past it.  The Pickup Prompt names the variant on offer
// (dlls/player_inventory.cpp) so the player knows what a press would do.
// Either way it never enters the Grid: it is worn, not carried.
//=========================================================
class CItemSuit : public CItem
{
	bool KeyValue(KeyValueData* pkvd) override
	{
		if (FStrEq(pkvd->szKeyName, "variant"))
		{
			pev->skin = SuitVariantClamp(atoi(pkvd->szValue));
			return true;
		}

		return CItem::KeyValue(pkvd);
	}
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
	bool AutoPickupOnTouch(CBasePlayer* pPlayer) override { return !pPlayer->HasSuit(); }
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		const bool bSwitching = pPlayer->HasSuit();

		// A suit of the variant already worn is refused, exactly as the stock
		// item refuses a second suit.  Nothing is dropped and nothing is lost:
		// the pickup stays where it is.
		if (bSwitching && pPlayer->pev->skin == pev->skin)
			return false;

		// Changing suit is announced with the short line -- the long logon
		// belongs to putting a suit on for the first time.
		if (bSwitching || (pev->spawnflags & SF_SUIT_SHORTLOGON) != 0)
			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_A0"); // short version of suit logon,
		else
			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_AAx"); // long version of suit logon

		// Armour is deliberately untouched: a different suit is a different
		// colour, not a fresh charge.
		pPlayer->pev->skin = pev->skin;
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
	// Charged on the spot when the whole charge fits, as the medkit heals on
	// the spot -- at low armour this is exactly the vanilla battery.  The test
	// is inclusive: a charge that lands exactly on the ceiling is a perfect fit.
	bool ConsumeOnContact(CBasePlayer* pPlayer) override
	{
		if (pPlayer->pev->deadflag != DEAD_NO || !pPlayer->HasSuit())
			return false;

		if (BatteryChargeRoom(pPlayer) < gSkillData.batteryCapacity)
			return false;

		ApplyBatteryCharge(pPlayer);
		AnnouncePickup(pPlayer, false);
		return true;
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if (pPlayer->pev->deadflag != DEAD_NO)
			return false;

		// Require HEV suit so batteries still make sense contextually.
		if (!pPlayer->HasSuit())
			return false;

		// Carried, because using it now would waste some of it.
		// A full Grid refuses it and the battery stays in the world.
		if (InventoryGiveItem(pPlayer, EItemTypeId::Battery) <= 0)
			return false;

		EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);
		AnnouncePickup(pPlayer, true);
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
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if (InventoryGiveItem(pPlayer, EItemTypeId::Antidote) <= 0)
			return false;

		// Announced, but the Antidote has no HUD sprite, so the history shows
		// nothing for it yet -- see docs/ART_DEBT.md.
		AnnouncePickup(pPlayer, true);
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
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if (InventoryGiveItem(pPlayer, EItemTypeId::Keycard) <= 0)
			return false;

		AnnouncePickup(pPlayer, true);
		return true;
	}
};

LINK_ENTITY_TO_CLASS(item_security, CItemSecurity);


//=========================================================
// The Health Syringe -- the first Item Type that is ours rather
// than Half-Life's.  Everything genuinely new about it is the
// Infusion it starts; as a pickup it is an ordinary CItem.
//
// w_syringe.mdl is the mod's own: a one-bone model made by
// E:\CustomAssets\scripts\syringe_world.py (docs/MODEL_WORKFLOW.md),
// lying on the floor with the needle along the entity's forward axis.
//=========================================================
class CItemSyringe : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_syringe.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_syringe.mdl");
		PRECACHE_SOUND("items/smallmedkit1.wav");
	}
	// Always carried, never Auto-Consumed: waste is not computable for an
	// effect that pays out over time, so the test Auto-Consume relies on
	// cannot exist.
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		// A full Grid refuses it and the Syringe stays in the world.
		if (InventoryGiveItem(pPlayer, EItemTypeId::Syringe) <= 0)
			return false;

		// Pickup feedback. Borrowed from the medkit as a stand-in -- the Syringe
		// deliberately does not sound like a medkit when USED, so this one is
		// the placeholder most likely to need replacing.
		EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/smallmedkit1.wav", 1, ATTN_NORM);
		AnnouncePickup(pPlayer, true);

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
// The three models are the mod's own, one-bone props authored by
// E:\CustomAssets\scripts\progression_world.py (docs/MODEL_WORKFLOW.md).
// The pickup sound is still borrowed -- see docs/ART_DEBT.md.
//=========================================================

// The three read as one family, and apart from equipment, by each glowing in
// the dark.  Two halves: a texture flagged additive in the compiled model is
// the LOOK (it is still multiplied by the room's light, so alone it goes dark
// with the room), and the client puts a dynamic light on every visible pickup
// in the family's colour (cl_dll/entity.cpp, ProgressionLight), which is the
// GLOW.  Nothing here on the server.  Rank rides on colour -- cyan is common,
// gold is scarce.  Until 2026-09-13 they wore a kRenderFxGlowShell instead, as
// cover for borrowed props that meant something else in Half-Life; the models
// now carry their own shape and light, and the shell went with the stand-ins.
//
// If a prop ever needs resizing without an art pass: pev->scale is networked
// (dlls/client.cpp:1490).

// w_skillpoint.mdl: a machined hex shard, 18 units tall, standing point-down
// in a dark ring base.  Faceted like the Xen crystal that stood in before it,
// but cut rather than grown, so it is not scenery in a Black Mesa office.  The
// shard itself is the light.
class CItemSkillPoint : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_skillpoint.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_skillpoint.mdl");
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


// w_resettoken.mdl: a thick gold medallion, 14 across and 5 high, with a
// raised rim, a recessed ring of ticks and a centre boss.  The word is Token
// and a coin is what it means; a flat gold disc is a different shape class
// from the Skill Point's upright shard, so the two are told apart at a glance.
// The ring of ticks and the stamp are the light.
class CItemResetToken : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_resettoken.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_resettoken.mdl");
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
// w_rowgrant.mdl: a rack of four open compartments with a carry handle --
// the Grid row it grants, made into a thing.  Open compartments say storage
// without being a box, and deliberately NOT a case like w_weaponbox.mdl,
// which is spoken for: lootable Boxes will use it, and the two things that
// grow what a player carries must not look alike.  The cell grid on the
// divider and a strip along the top bar are the light.
//=========================================================
class CItemRowGrant : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_rowgrant.mdl");
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_rowgrant.mdl");
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

			// The Dash comes with it, and the long jump stays (settled
			// 2026-09-15): found, the Module opens the Shinobi region and
			// arrives with every charge ready.
			pPlayer->m_skills.OpenGate(EGate::DashModule);
			pPlayer->DashFill();
			SendSkillTreeToClient(pPlayer);

			AnnouncePickup(pPlayer, false);

			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_A1"); // Play the longjump sound UNDONE: Kelly? correct sound?
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS(item_longjump, CItemLongJump);

// ---------------------------------------------------------
// The Night Vision Module -- the fifth Module, and the reveal gate for the
// Stealth region (docs/SKILL_TREE.md, "The Night Vision Module"). Adapted
// from Opposing Force's: the flashlight stays until this is found, and then
// this replaces it, closing the flashlight hole in the light term -- no more
// EF_DIMLIGHT, so a lit corridor and a dark one finally conceal the same.
//
// Nothing new is saved. Held is EGate::NightVision being open, which
// CPlayerSkills already saves with the rest of the gate mask; whether the
// device is currently switched on rides EF_NIGHTVISION in pev->effects,
// which the engine saves for free (see FlashlightTurnOn/Off).
// ---------------------------------------------------------
class CItemNightVision : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_silencer.mdl"); // stand-in, reads as a device; see ART_DEBT
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_silencer.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if (pPlayer->m_skills.IsGateOpen(EGate::NightVision))
		{
			return false;
		}

		if (!pPlayer->HasSuit())
		{
			return false;
		}

		// Night Vision replaces the flashlight; if it is lit at the moment
		// of pickup, close it so the player is never holding both at once.
		if (pPlayer->FlashlightIsOn())
		{
			pPlayer->FlashlightTurnOff();
		}

		pPlayer->m_skills.OpenGate(EGate::NightVision);
		SendSkillTreeToClient(pPlayer);

		AnnouncePickup(pPlayer, false);

		EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_A1"); // placeholder, as item_longjump's

		return true;
	}
};

LINK_ENTITY_TO_CLASS(item_nightvision, CItemNightVision);

// ---------------------------------------------------------
// The silencer -- the first Evolution, settled 2026-09-17 (docs/ROADMAP.md,
// "Weapon evolutions"), pulled forward by the Stealth pillar because the
// predator loop's intended play -- stab, swap, silenced headshot on the
// witness -- has no second half without it.  Found, and permanent: the
// pickup sets one saved flag on the player, and every pistol deploys
// silenced from then on.  A pistol in hand plays the attach animation the
// SDK shipped in v_9mmhandgun.mdl and never used.
//
// Needs nothing -- not the suit, not the pistol.  A player who finds it
// before the pistol gets the silencer on the pistol when it comes.
// ---------------------------------------------------------
class CItemSilencer : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_silencer.mdl"); // the real thing, for once
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_silencer.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if (pPlayer->m_bSilencerFound)
		{
			return false;
		}

		pPlayer->m_bSilencerFound = true;

		// In hand: attach it now, with the animation.  Holstered or not yet
		// found: the next Deploy derives the body from the flag.
		CBasePlayerItem* pActive = pPlayer->m_pActiveItem;
		if (pActive != NULL && FClassnameIs(pActive->pev, "weapon_9mmhandgun"))
		{
			((CGlock*)pActive)->AttachSilencer();
		}

		AnnouncePickup(pPlayer, false);

		EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_A1"); // placeholder, as item_nightvision's

		return true;
	}
};

LINK_ENTITY_TO_CLASS(item_silencer, CItemSilencer);

// ---------------------------------------------------------
// The alien Module -- the fourth Module, and the reveal gate for the whole
// Alien Route, Hive nodes included (docs/ROADMAP.md, "The alien Module --
// settled").  It is a platform: what it grants is access to weapons that run
// on Cores, and the summon weapon is the first of them, handed over with it
// so that no verb of the Module reads as broken on the day it is found.
//
// THE STAND-IN.  The design says the freed alien slave hands the Module over
// after a boss fight.  That NPC does not exist, and neither does the fight, so
// this pickup stands in for the hand-over until they do; it is not the shape
// the Module is meant to arrive in and should go when the slave can give it.
//
// Nothing new is saved.  Held is EGate::AlienModule being open, which
// CPlayerSkills already saves with the rest of the gate mask, plus the weapon
// and the Cores, which the player saves like any other.
// ---------------------------------------------------------
class CItemAlienModule : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_sqknest.mdl"); // stand-in, reads alien; see ART_DEBT
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_sqknest.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if (pPlayer->m_skills.IsGateOpen(EGate::AlienModule))
		{
			return false;
		}

		if (!pPlayer->HasSuit())
		{
			return false;
		}

		pPlayer->m_skills.OpenGate(EGate::AlienModule);
		SendSkillTreeToClient(pPlayer);

		// Whole, as the design asks: the Module and its first weapon arrive
		// together, with a starting stock of Cores (the weapon's own
		// SUMMON_DEFAULT_GIVE).  Cores are found in the world after that.
		pPlayer->GiveNamedItem("weapon_summon");

		AnnouncePickup(pPlayer, false);

		EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_A1"); // placeholder, as item_longjump's

		return true;
	}
};

LINK_ENTITY_TO_CLASS(item_alienmodule, CItemAlienModule);

// ---------------------------------------------------------
// The Pulse Module -- the first Module, and the reveal gate for every Pulse
// node and the Defense Matrix (docs/adr/0013-the-pulse-is-a-found-module.md).
// The Pulse was suit hardware until 2026-09-16: it came with the suit, its
// gate was opened on every spawn, and level design could assume it from
// Anomalous Materials on.  Now it is found, early, and assumed only after.
//
// A stand-in pickup like item_alienmodule's: the design says only "found in
// the world, early", so a walk-over item is the whole of it for now.  Nothing
// new is saved: held is EGate::PulseModule being open.
// ---------------------------------------------------------
class CItemPulseModule : public CItem
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_adrenaline.mdl"); // stand-in, a stock model nothing else uses; see ART_DEBT
		CItem::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_adrenaline.mdl");
	}
	bool MyTouch(CBasePlayer* pPlayer) override
	{
		if (pPlayer->m_skills.IsGateOpen(EGate::PulseModule))
		{
			return false;
		}

		if (!pPlayer->HasSuit())
		{
			return false;
		}

		pPlayer->m_skills.OpenGate(EGate::PulseModule);
		SendSkillTreeToClient(pPlayer);

		// The bar appears with the Module: the next sync sends READY instead
		// of NONE, since the sent state no longer matches.
		pPlayer->m_pulse.ForgetSentState();

		AnnouncePickup(pPlayer, false);

		EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_A1"); // placeholder, as item_longjump's

		return true;
	}
};

LINK_ENTITY_TO_CLASS(item_pulsemodule, CItemPulseModule);

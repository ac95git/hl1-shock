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

#pragma once

class CItem : public CBaseEntity
{
public:
	void Spawn() override;
	CBaseEntity* Respawn() override;
	void EXPORT ItemTouch(CBaseEntity* pOther);
	void EXPORT Materialize();

	// Takes this into the player's Inventory. Return false to refuse, which
	// leaves the item in the world.
	virtual bool MyTouch(CBasePlayer* pPlayer) { return false; }

	// Whether walking over this is enough to take it.
	//
	// True for everything, as in Half-Life: a playtest showed that an item
	// which stays on the floor when touched reads as broken, whatever the
	// Pickup Prompt says (docs/adr/0011-pickups-are-walk-over.md). The one
	// override is a suit offered to a player already wearing one, which is
	// irreversible and so waits for a use press.
	virtual bool AutoPickupOnTouch(CBasePlayer* pPlayer) { return true; }

	// Used on the spot instead of being carried, when using it now wastes
	// nothing. Returns true if it was consumed. See docs/PILLARS.md.
	virtual bool ConsumeOnContact(CBasePlayer* pPlayer) { return false; }

	// Runs MyTouch plus the acquire bookkeeping (targets, respawn, removal).
	// This is what a use press calls; ItemTouch calls it for walk-over items.
	bool AcquireBy(CBasePlayer* pPlayer);

	// For an item a player has just dropped: switches touch off until the item
	// has landed and nobody is standing in it, so a drop does not walk straight
	// back into the Inventory. A dropped weapon gets the same for free from
	// CBasePlayerItem::FallInit, which leaves it a point until it lands. A use
	// press still takes it at once -- that path never goes through touch.
	void DisarmUntilClear();
	void EXPORT ArmWhenClear();

protected:
	// Flashes this pickup on the HUD's pickup history. `carried` marks it as
	// having gone into the Inventory rather than been used on the spot, which
	// the history draws as an arrow over the icon -- the player's one signal
	// that a walk-over put something in the Grid.
	void AnnouncePickup(CBasePlayer* pPlayer, bool carried);

private:
	// Shared tail of both acquiring and consuming: fire targets, then respawn
	// or remove according to the game rules.
	void FinishAcquire(CBasePlayer* pPlayer);
};

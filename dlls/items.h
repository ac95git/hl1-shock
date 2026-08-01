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
	// False for anything that goes into the Inventory: those are taken with a
	// deliberate use press so the player is never surprised by what they are
	// carrying. Things that are not Inventory items -- the HEV suit, the
	// longjump module -- keep the original walk-over behaviour.
	virtual bool AutoPickupOnTouch() { return true; }

	// Used on the spot instead of being carried, when using it now wastes
	// nothing. Returns true if it was consumed. See docs/PILLARS.md.
	virtual bool ConsumeOnContact(CBasePlayer* pPlayer) { return false; }

	// Runs MyTouch plus the acquire bookkeeping (targets, respawn, removal).
	// This is what a use press calls; ItemTouch calls it for walk-over items.
	bool AcquireBy(CBasePlayer* pPlayer);

private:
	// Shared tail of both acquiring and consuming: fire targets, then respawn
	// or remove according to the game rules.
	void FinishAcquire(CBasePlayer* pPlayer);
};

//=========================================================
// suit_defs.h
//
// The Suit Variant: which of the mod's three HEV suits the player
// wears.  Compiled into BOTH the server and the client -- the server
// sets it from an item_suit's keyvalue, the client reads it back off
// the player's entity state for the viewmodel gloves, the HUD colour
// and the Pickup Prompt's name for a suit on the floor.
//
// Vocabulary here follows CONTEXT.md, "The Suit".
//=========================================================

#pragma once

// ---------------------------------------------------------
// ESuitVariant
//
// Ids are FROZEN.  They are the player's saved pev->skin, and they are
// the skin family index of the $texturegroup every viewmodel carries
// (E:\CustomAssets\scripts\hev_gloves.py writes cyan first, so 0 is
// what a model shows with no code at all).  Adding is free; reordering
// recolours every save and every compiled model at once.
//
// The names are codenames for specializations the suits do not yet
// have.  They are cosmetic today -- see docs/PILLARS.md -- and the
// names are here so that they survive the day they are not.
// ---------------------------------------------------------
enum class ESuitVariant : int
{
	Agility      = 0, // cyan
	Strength     = 1, // red
	Intelligence = 2, // purple

	_Count       = 3, // keep last
};

inline constexpr int k_SuitVariantCount = static_cast<int>(ESuitVariant::_Count);

struct SuitVariantDef
{
	ESuitVariant id;
	const char*  name;        // "Strength"
	const char*  displayName; // what the Pickup Prompt reads

	// The accent the glove generator paints the light channels and the
	// hand-back readout with.  Taken from VARIANTS in hev_gloves.py rather
	// than picked to match it, so the gloves and the HUD agree by
	// construction instead of by two tables being kept in step by hand.
	unsigned char r, g, b;
};

inline constexpr SuitVariantDef k_SuitVariants[k_SuitVariantCount] =
{
	//  id                            name             prompt                       accent
	{ ESuitVariant::Agility,      "Agility",      "HEV Suit (Agility)",       60, 220, 255 },
	{ ESuitVariant::Strength,     "Strength",     "HEV Suit (Strength)",     255,  70,  50 },
	{ ESuitVariant::Intelligence, "Intelligence", "HEV Suit (Intelligence)", 200,  90, 255 },
};

// Anything out of range is Agility.  Three things legitimately arrive here:
// a vanilla item_suit with no variant keyvalue, a save written before the
// variant existed, and a client that has not yet heard from the server.
inline constexpr int SuitVariantClamp(int variant)
{
	return (variant > 0 && variant < k_SuitVariantCount) ? variant : 0;
}

inline constexpr const SuitVariantDef& GetSuitVariant(int variant)
{
	return k_SuitVariants[SuitVariantClamp(variant)];
}

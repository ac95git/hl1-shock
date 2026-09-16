#pragma once

#include "skill_defs.h"

// ---------------------------------------------------------
// ESlot
//
// Where a Module sits on the suit, as the Status page draws it
// (docs/STATUS_PANEL.md).  A Slot is a place, not a choice: each
// Module always sits in its own, and nothing is put in or taken out.
// Presentation only -- nothing is saved or sent by Slot.
// ---------------------------------------------------------
enum class ESlot : uint8_t
{
	Head     = 0,
	Body     = 1,
	LeftArm  = 2,
	RightArm = 3,
	Legs     = 4, // undecided: no Module fills it yet
	_Count   = 5,
};

// ---------------------------------------------------------
// ModuleDef
//
// Everything about a Module that is the same for every player, the
// way SkillDef is for a Skill.  Whether the player has a Module is its
// gate being open (EGate, sent on gmsgSkillTree), so the gate is the
// Module's identity here and nothing new is networked.
// ---------------------------------------------------------
struct ModuleDef
{
	EGate       gate;        // the gate that is open once this Module is found
	ESlot       slot;        // where it sits on the doll
	const char* name;        // tooltip title
	const char* description; // tooltip body, one sentence
	const char* spriteName;  // HUD sprite standing in for the glyph (docs/ART_DEBT.md)
	const char* command;     // what its key is bound to, looked up live; nullptr for none
};

// Night Vision is built (item_nightvision, dlls/items.cpp): its Slot fills
// when the pickup opens EGate::NightVision, or skill_open_gates does the
// same as a cheat. Its key is the flashlight's, which it replaces.
// The Alien Module is carried as a weapon, so its key is settled when
// it is built (docs/STATUS_PANEL.md, "Open").
inline constexpr ModuleDef k_ModuleDefs[] =
{
	//  gate                  slot              name            description                                                                    sprite           command
	{ EGate::NightVision, ESlot::Head,     "Night Vision", "Sees in the dark. Replaces the flashlight.",                                   "dmg_gas",       "impulse 100" },
	{ EGate::DashModule,  ESlot::Body,     "Dash",         "A burst along the movement keys, from the ground; Air Dash takes it into the air along the aim.", "item_longjump", "impulse 151" },
	{ EGate::PulseModule, ESlot::LeftArm,  "Pulse",        "Raises a Shield for a moment. A hit inside the Pulse Window is negated.",    "autoaim_c",     "+pulse"      },
	{ EGate::AlienModule, ESlot::RightArm, "Alien Module", "A platform for alien weapons that run on Cores.",                              "d_hornet",      nullptr       },
};

inline constexpr int k_NumModuleDefs = static_cast<int>(sizeof(k_ModuleDefs) / sizeof(k_ModuleDefs[0]));

// The Module that sits in 'slot', or nullptr for a Slot nothing fills.
inline const ModuleDef* GetModuleInSlot(ESlot slot)
{
	for (const ModuleDef& def : k_ModuleDefs)
	{
		if (def.slot == slot)
			return &def;
	}
	return nullptr;
}

#pragma once

// ---------------------------------------------------------
// The Prompt: what a usable thing is called, and what a use press does
// to it.
//
// Everything that can be interacted with says so on screen -- a title,
// and the action under it (docs/ROADMAP.md, Pillar 1: Records).  The
// Pickup Prompt is the oldest case and names what it shows from the
// Item Type and weapon tables; this table is for everything else a use
// press can act on.
//
// The server classifies the entity (ClassifyUsable, player_inventory.cpp)
// and sends the class; the client resolves it here, so no strings cross
// the wire.  A class is neither saved nor kept between maps, so the
// values may be renumbered freely.
//
// Compiled into both DLLs, so the two sides cannot disagree.
// ---------------------------------------------------------

#include <cstdint>

#include "station_defs.h"

enum class EPromptClass : uint8_t
{
	None = 0,      // nothing usable is being looked at
	Generic,       // usable, and nothing below names it
	Button,        // func_button, func_rot_button, button_target
	Valve,         // momentary_rot_button
	Door,          // a use-only func_door / func_door_rotating
	HealthStation, // func_healthcharger
	HevCharger,    // func_recharge
	Scientist,     // monster_scientist, monster_sitting_scientist
	Guard,         // monster_barney
	Movable,       // func_pushable
	MountedGun,    // func_tankcontrols
	Record,        // the `record` entity, in either of its two forms

	// A record_lock, in its two states.  The server decides which, because
	// only it holds the found-set -- so a lock can never say "open" to a
	// player who cannot open it.
	RecordLock,       // the suit has the Record it names
	RecordLockSealed, // it does not

	// A func_deposit, by what the player holds.  Both are state lines: a
	// deposit is mined by striking it, never by a use press, so neither
	// offers the key.
	Deposit,       // a mining tool is in hand
	DepositNoTool, // it is not

	// A func_station, by its type and whether it has trades left.  The words
	// are station_defs.h's, so the price shown is the price charged.
	StationFuel,
	StationFuelSpent,
	StationUranium,
	StationCores,
	StationAmmoEmpty,
	StationHoist,
	StationHoistSent,

	// Taken rather than used: the pickups that never reach the Grid, so the
	// Item Type table cannot name them.  Sent as EEntryKind::Pickup's id.
	SkillPoint,   // item_skillpoint
	ResetToken,   // item_resettoken
	RowGrant,     // item_rowgrant
	LongJump,     // item_longjump
	NightVision,  // item_nightvision
	Silencer,     // item_silencer
	AlienModule,  // item_alienmodule
	PulseModule,  // item_pulsemodule
	Core,         // item_core
	Ammo,         // any other CBasePlayerAmmo
	Item,         // any other CItem

	// Ammunition, by what it actually is.  Until 2026-09-18 every box and
	// clip in the game read "Ammunition", which tells the player nothing
	// they could not see (Andrei).  A table rather than a prettified
	// classname, because "ammo_9mmAR" does not prettify into anything worth
	// reading.
	Ammo357,
	Ammo9mmAR,
	Ammo9mmBox,
	Ammo9mmClip,
	AmmoARGrenades,
	AmmoBuckshot,
	AmmoCrossbow,
	AmmoEgonClip,
	AmmoGaussClip,
	AmmoRpgClip,

	// The vanilla pickups that are not Item Types either, and so were
	// falling through to a bare "Item".
	Battery,
	Healthkit,
	AirTank,
	Antidote,
	SecurityCard,

	_Count, // keep last
};

struct PromptClassDef
{
	const char* title;  // nullptr: draw the action alone
	const char* action; // follows the bound use key: "[E] Press"

	// A state line, drawn INSTEAD of the key and action: how a hard gate
	// looks impassable (docs/ROADMAP.md, "The Prompt").  A door that offers
	// "[E] Open" and then does nothing teaches the player that use presses
	// are unreliable; one that says "Code required" teaches them to go and
	// find the code.  nullptr on everything that simply works.
	const char* state = nullptr;
};

// One row per EPromptClass, in enum order.
static const PromptClassDef k_PromptClassDefs[] = {
	{nullptr, nullptr},          // None
	{nullptr, "Use"},            // Generic
	{"Button", "Press"},         // Button
	{"Valve", "Turn"},           // Valve
	{"Door", "Open"},            // Door
	{"Health station", "Heal"},  // HealthStation
	{"HEV charger", "Charge"},   // HevCharger
	{"Scientist", "Talk"},       // Scientist
	{"Security guard", "Talk"},  // Guard
	{"Movable object", "Pull"},  // Movable
	{"Mounted gun", "Operate"},  // MountedGun
	{"Record", "Read"},          // Record
	{"Lock", "Enter code"},                 // RecordLock
	{"Lock", nullptr, "Code required"},     // RecordLockSealed
	{"Crystal deposit", nullptr, "Strike to mine"},         // Deposit
	{"Crystal deposit", nullptr, "Requires a mining tool"}, // DepositNoTool
	{k_StationDefs[0].promptTitle, k_StationDefs[0].promptAction},          // StationFuel
	{k_StationDefs[0].promptTitle, nullptr, k_StationDefs[0].promptSpent},  // StationFuelSpent
	{k_StationDefs[1].promptTitle, k_StationDefs[1].promptAction},          // StationUranium
	{k_StationDefs[2].promptTitle, k_StationDefs[2].promptAction},          // StationCores
	{k_StationDefs[1].promptTitle, nullptr, k_StationDefs[1].promptSpent},  // StationAmmoEmpty
	{k_StationDefs[3].promptTitle, k_StationDefs[3].promptAction},          // StationHoist
	{k_StationDefs[3].promptTitle, nullptr, k_StationDefs[3].promptSpent},  // StationHoistSent
	{"Skill Point", "Take"},         // SkillPoint
	{"Reset Token", "Take"},         // ResetToken
	{"Row Grant", "Take"},           // RowGrant
	{"Long Jump Module", "Take"},    // LongJump
	{"Night Vision Module", "Take"}, // NightVision
	{"Silencer", "Take"},            // Silencer
	{"Alien Module", "Take"},        // AlienModule
	{"Pulse Module", "Take"},        // PulseModule
	{"Core", "Take"},                // Core
	{"Ammunition", "Take"},          // Ammo
	{"Item", "Take"},                // Item

	{".357 rounds", "Take"},         // Ammo357
	{"Rifle magazine", "Take"},      // Ammo9mmAR
	{"9mm box", "Take"},             // Ammo9mmBox
	{"9mm clip", "Take"},            // Ammo9mmClip
	{"Rifle grenades", "Take"},      // AmmoARGrenades
	{"Shotgun shells", "Take"},      // AmmoBuckshot
	{"Crossbow bolts", "Take"},      // AmmoCrossbow
	{"Egon cells", "Take"},          // AmmoEgonClip
	{"Uranium", "Take"},             // AmmoGaussClip
	{"Rocket", "Take"},              // AmmoRpgClip

	{"HEV battery", "Take"},         // Battery
	{"Medkit", "Take"},              // Healthkit
	{"Oxygen tank", "Take"},         // AirTank
	{"Antidote", "Take"},            // Antidote
	{"Security card", "Take"},       // SecurityCard
};

static_assert(sizeof(k_PromptClassDefs) / sizeof(k_PromptClassDefs[0]) == static_cast<int>(EPromptClass::_Count),
	"k_PromptClassDefs needs one row per EPromptClass");

// What every takeable thing says under its name.
static const char* const k_PromptTakeAction = "Take";

// ---------------------------------------------------------
// The mapper's overrides.
//
// A default by class gets a Prompt onto everything for free, but it can
// only ever say what the CLASS is -- and a pump control is not a "Button",
// nor is its action "Press" (docs/ROADMAP.md, "The Prompt").  So any
// entity takes prompt_title and prompt_action, and the server sends the
// override in place of the table's row.
//
// This is the one place strings cross the wire, and it is deliberate: a
// Prompt is only sent when what the player is looking at CHANGES, so the
// cost is a handful of bytes per doorway rather than per frame.  They are
// clamped well under the 192-byte message all the same, because a mapper
// who writes an essay should get a truncated Prompt and not a dropped one.
// ---------------------------------------------------------
inline constexpr int k_PromptOverrideMax = 48;

inline const PromptClassDef& GetPromptClass(int promptClass)
{
	if (promptClass <= 0 || promptClass >= static_cast<int>(EPromptClass::_Count))
		return k_PromptClassDefs[0];
	return k_PromptClassDefs[promptClass];
}

//=========================================================
// skill_defs.h
//
// The single definition of every Skill: what it is called, what
// it does, where it sits in the Skill Tree, what it costs and
// what gates it.  Compiled into BOTH the server and the client --
// see docs/adr/0008-skill-definitions-are-shared-not-networked.md.
//
// Static Skill data is shared rather than networked.  Only
// per-player state -- which Skills are unlocked, and how many
// Skill Points are unspent -- travels over the wire.
//
// Vocabulary here follows CONTEXT.md: Skill, Skill Point, Skill Tree.
// Header-only so neither project file needs to change.
//=========================================================

#pragma once

#include <cstdint>

// ---------------------------------------------------------
// ENodeTier
//
// The visual weight of a node in the client's Skill Tree.
// Presentation only -- nothing in the rules reads it.
// ---------------------------------------------------------
enum class ENodeTier : uint8_t
{
	Minor  = 0, // small node  -- cheap/root Skills
	Medium = 1, // medium node -- mid-tree Skills
	Major  = 2, // large node  -- powerful end-tree Skills
};

// ---------------------------------------------------------
// ESkillId
//
// Ids are FROZEN once written to a save.  They index the unlocked
// array on the server and are the BIT POSITIONS in the sync mask,
// so reordering or reusing one silently reassigns a player's
// unlocked Skills.  Adding is free; retiring is free, and the
// retired id must never be handed to a different Skill.
// ---------------------------------------------------------
enum class ESkillId : int
{
	None = 0,

	// ---- Combat ----
	CrowbarRange        = 1,  // melee reach +25%
	CrowbarDamage       = 2,  // melee damage +50%
	FastReload          = 3,  // reload time -20%
	ExtraDamage         = 4,  // all weapon damage +10%

	// ---- Mobility ----
	HighJump            = 5,  // jump height +30%
	SprintSpeed         = 6,  // movement speed +15%
	FallResistance      = 7,  // fall damage -50%

	// ---- Survivability ----
	MoreHealth          = 8,  // max health +25
	ArmorEfficiency     = 9,  // armor absorbs 10% more damage
	HealthRegen         = 10, // slowly regenerate health out of combat

	CrowbarSpeed        = 11, // crowbar swing speed +30%
	CrowbarFollowUp     = 18, // the swing after a deflect hits harder

	// ---- The Pulse ----
	// Id 12 was CrowbarParry, a placeholder for a mechanic that turned out to
	// be suit hardware rather than a crowbar technique.  The id is reused
	// rather than retired: ids are saved and networked, so keeping 12 keeps
	// old saves loading, and changing its meaning is only safe because no
	// Skill had an effect yet.  Do not treat this as a precedent.
	PulseWindow         = 12, // wider Pulse Window
	PulseRecharge       = 15, // shorter Recharge
	PulseDischarge      = 16, // negated hits vent at the crosshair
	PulseRebound        = 17, // a deflect skips the Recharge, once per charge

	BatteryCapacity     = 13, // +50 max battery
	BatteryRegen        = 14, // passive armor regeneration

	// ---- The Infusion ----
	MedExpert           = 19, // longer Infusion from a Health Syringe

	_Count              = 20, // keep last
};

inline constexpr int k_MaxSkills = static_cast<int>(ESkillId::_Count);

// Bytes needed to carry one bit per Skill in the sync message.
// Both DLLs derive this from the same constant, so they cannot
// disagree about the message length.
inline constexpr int k_SkillMaskBytes = (k_MaxSkills + 7) / 8;

// ---------------------------------------------------------
// SkillDef
//
// Everything about a Skill that is the same for every player.
//
// Two prerequisites, and both are required -- a line drawn in the
// tree always means "you need this".  ESkillId::None in either
// slot means "no gate there"; None in both makes the Skill a root.
// ---------------------------------------------------------
struct SkillDef
{
	ESkillId    id;
	const char* name;        // display name
	const char* description; // hover text
	const char* spriteName;  // HUD sprite name; nullptr renders without an icon
	int         gridCol;     // column in the Skill Tree
	int         gridRow;     // row    in the Skill Tree
	int         cost;        // Skill Points to unlock
	ESkillId    prereq;      // both prerequisites are required
	ESkillId    prereq2;
	ENodeTier   tier;        // visual weight only
};

// Indexed by ESkillId, so entry [n] is always the Skill with id n.
inline constexpr SkillDef k_SkillDefs[k_MaxSkills] =
{
	//  id                        name                description                                        sprite           col row cost prereq                     prereq2            tier
	{ ESkillId::None,            "None",             "",                                                nullptr,          0,  0,  0,  ESkillId::None,            ESkillId::None,    ENodeTier::Minor  },
	{ ESkillId::CrowbarRange,    "Crowbar Reach",    "+25% melee range.",                               "d_crowbar",      1,  0,  1,  ESkillId::None,            ESkillId::None,    ENodeTier::Minor  },
	{ ESkillId::CrowbarDamage,   "Crowbar Force",    "+50% melee damage.",                              "d_crowbar",      1,  1,  2,  ESkillId::CrowbarRange,    ESkillId::None,    ENodeTier::Medium },
	{ ESkillId::FastReload,      "Fast Reload",      "-20% reload time.",                               "d_9mmhandgun",   3,  0,  1,  ESkillId::None,            ESkillId::None,    ENodeTier::Minor  },
	{ ESkillId::ExtraDamage,     "Weapon Mastery",   "+10% weapon damage.",                             "d_9mmar",        3,  1,  3,  ESkillId::FastReload,      ESkillId::None,    ENodeTier::Major  },
	{ ESkillId::HighJump,        "High Jump",        "+30% jump height.",                               nullptr,          5,  0,  1,  ESkillId::None,            ESkillId::None,    ENodeTier::Minor  },
	// Lists itself as its own prerequisite, so it can never become available.
	// Preserved verbatim from the pre-move table: this iteration changes where
	// the data lives and nothing about what it says.  The node is cut in the
	// curation pass, which removes the bug with it.
	{ ESkillId::SprintSpeed,     "Sprint",           "+15% movement speed.",                            nullptr,          5,  1,  2,  ESkillId::SprintSpeed,     ESkillId::None,    ENodeTier::Major  },
	{ ESkillId::FallResistance,  "Fall Resist",      "-50% fall damage.",                               nullptr,          5,  2,  1,  ESkillId::HighJump,        ESkillId::None,    ENodeTier::Medium },
	{ ESkillId::MoreHealth,      "Fortitude",        "+25 max health.",                                 "cross",          7,  0,  2,  ESkillId::None,            ESkillId::None,    ENodeTier::Minor  },
	{ ESkillId::ArmorEfficiency, "Armor Expert",     "Armor absorbs 10% more damage.",                  "suit_full",      7,  1,  2,  ESkillId::MoreHealth,      ESkillId::None,    ENodeTier::Medium },
	{ ESkillId::HealthRegen,     "Regen",            "Slowly regenerate health.",                       "cross",          7,  2,  3,  ESkillId::MoreHealth,      ESkillId::None,    ENodeTier::Major  },
	{ ESkillId::CrowbarSpeed,    "Crowbar Speed",    "+30% crowbar attack speed.",                      "d_crowbar",      0,  2,  2,  ESkillId::CrowbarDamage,   ESkillId::None,    ENodeTier::Medium },
	{ ESkillId::PulseWindow,     "Pulse Window",     "+0.15s Pulse Window.",                            "suit_full",     11,  0,  1,  ESkillId::None,            ESkillId::None,    ENodeTier::Minor  },
	{ ESkillId::BatteryCapacity, "Battery Capacity", "+50 max battery.",                                "suit_full",      9,  0,  2,  ESkillId::None,            ESkillId::None,    ENodeTier::Minor  },
	{ ESkillId::BatteryRegen,    "Battery Regen",    "Regenerate armor over time.",                     "suit_full",      9,  1,  3,  ESkillId::BatteryCapacity, ESkillId::None,    ENodeTier::Major  },
	{ ESkillId::PulseRecharge,   "Pulse Recharge",   "-33% Pulse Recharge.",                            "suit_full",     11,  1,  2,  ESkillId::PulseWindow,     ESkillId::None,    ENodeTier::Medium },
	{ ESkillId::PulseDischarge,  "Pulse Discharge",  "Negated hits vent energy at your crosshair.",     "suit_full",     11,  2,  3,  ESkillId::PulseRecharge,   ESkillId::None,    ENodeTier::Major  },
	{ ESkillId::PulseRebound,    "Pulse Rebound",    "A deflect skips the Recharge. Once, until you sit through a normal one.", "suit_full", 12, 2, 3, ESkillId::PulseRecharge, ESkillId::None, ENodeTier::Major },
	// The second prerequisite this Skill always wanted: it is a crowbar payoff
	// for a Pulse deflect, and now genuinely requires both branches.
	{ ESkillId::CrowbarFollowUp, "Follow-Up",        "After a deflect, your next crowbar hit lands far harder.", "d_crowbar", 2, 2, 3, ESkillId::CrowbarDamage, ESkillId::PulseRecharge, ENodeTier::Major },
	// A root rather than a child of the survivability column: MoreHealth,
	// ArmorEfficiency and HealthRegen are all still inert, so hanging this off
	// one of them would charge points for a node that does nothing purely to
	// reach one that does. PulseWindow set the precedent. Re-parenting later is
	// a data change, not a structural one.
	{ ESkillId::MedExpert,       "Med Expert",       "+5s Infusion duration.",                          "cross",          8,  0,  2,  ESkillId::None,            ESkillId::None,    ENodeTier::Medium },
};

// Returns nullptr for None or any out-of-range id.
inline const SkillDef* GetSkillDef(int id)
{
	if (id <= 0 || id >= k_MaxSkills)
		return nullptr;
	return &k_SkillDefs[id];
}

inline const SkillDef* GetSkillDef(ESkillId id)
{
	return GetSkillDef(static_cast<int>(id));
}

// ---------------------------------------------------------
// The unlocked mask -- one bit per Skill, indexed by id.
//
// This is the whole of what the server tells the client about
// which Skills a player has.
// ---------------------------------------------------------
inline bool SkillMaskGet(const unsigned char* mask, int id)
{
	if (!mask || id <= 0 || id >= k_MaxSkills)
		return false;
	return (mask[id >> 3] & (1 << (id & 7))) != 0;
}

inline void SkillMaskSet(unsigned char* mask, int id, bool value)
{
	if (!mask || id <= 0 || id >= k_MaxSkills)
		return;
	const unsigned char bit = static_cast<unsigned char>(1 << (id & 7));
	if (value)
		mask[id >> 3] |= bit;
	else
		mask[id >> 3] &= static_cast<unsigned char>(~bit);
}

// ---------------------------------------------------------
// SkillPrereqMet
//
// The one implementation of the gating rule, shared by the server
// (which asks about its own unlocked array) and the client (which
// asks about the mask it was sent).  'has' is any callable taking
// an ESkillId and returning bool.
//
// Both prerequisites are required.  A None prerequisite is not a
// gate, so a Skill with None in both slots is a root.
// ---------------------------------------------------------
template <typename HasSkillFn>
inline bool SkillPrereqMet(int id, HasSkillFn has)
{
	const SkillDef* def = GetSkillDef(id);
	if (!def)
		return false;

	if (def->prereq != ESkillId::None && !has(def->prereq))
		return false;
	if (def->prereq2 != ESkillId::None && !has(def->prereq2))
		return false;

	return true;
}

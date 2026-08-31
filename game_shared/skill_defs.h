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

	// ---- Melee ----
	CrowbarRange        = 1,  // melee reach +25%
	CrowbarDamage       = 2,  // melee damage +50%
	CrowbarFollowUp     = 18, // the swing after a deflect hits harder

	// ---- Armaments ----
	FastReload          = 3,  // reload delay -20%
	ExtraDamage         = 4,  // all weapon damage +10%

	// ---- Survivability ----
	FallResistance      = 7,  // fall damage -50%
	MoreHealth          = 8,  // max health +25
	ArmorEfficiency     = 9,  // armor absorbs 10% more damage
	HealthRegen         = 10, // slowly regenerate health out of combat

	// ---- Reserved: cut from the tree pending movement prediction ----
	// Both change how the player MOVES, which pm_shared/ owns and neither
	// m_skills nor the weapon prediction path reaches. They keep their ids
	// and return unchanged when that work happens; they simply have no row
	// in k_SkillDefs meanwhile.
	//
	// FastReload used to sit here for a different reason -- weapon-side
	// prediction -- which is fixed, so it is in the tree above.
	HighJump            = 5,  // jump height +30%
	SprintSpeed         = 6,  // movement speed +15%

	// Reserved for a different reason: swing speed is predicted and reachable
	// now, but the crowbar's attack cadence is entangled with the first-swing
	// /follow-up damage rule at crowbar.cpp, which reads m_flNextPrimaryAttack
	// to decide which it was. Retuning one retunes the other.
	CrowbarSpeed        = 11, // crowbar swing speed +30%

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

	// ---- Reserved: the alien column ----
	// Held for a branch that stays hidden until the player carries an alien
	// weapon, so its existence is not spoiled by reading the tree. Half-Life
	// has only two alien weapons, and the Hivehand carries both knobs worth
	// having -- how many hornets it holds, and how fast they come back.
	// See docs/PILLARS.md pillar 4.
	HiveCapacity        = 20, // Hivehand holds more hornets
	HiveRegrowth        = 21, // hornets replenish faster

	_Count              = 22, // keep last
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

// A reserved id: it exists, nothing in the tree points at it, and it is not
// buyable.  Used for Skills cut pending other work and for ones held for a
// branch that has not opened yet.  RebuildNodeList and SpentPoints both key
// off the null name, so a Skill moved to this state disappears from the tree
// and refunds itself on the next load.
#define SKILL_RESERVED(idName) \
	{ ESkillId::idName, nullptr, nullptr, nullptr, 0, 0, 0, ESkillId::None, ESkillId::None, ENodeTier::Minor }

// Indexed by ESkillId, so entry [n] is always the Skill with id n.
//
// Seven columns, three rows.  Melee sits beside the Pulse deliberately:
// Follow-Up is gated on both, and adjacent columns keep that cross-link short
// instead of dragging a connector the width of the tree.
//
//   MELEE      PULSE    (fork)   SUIT     ARMS    SURVIVAL  (fork)
//   col0       col1     col2     col3     col4     col5     col6
//
// r0 Reach     Window            Capacity Mastery Fortitude
// r1 Force     Recharge          BattRegen Reload ArmorExp  FallResist
// r2 Follow-Up Discharge Rebound                  Regen     MedExpert
//
// Total cost is 35 points, which is the target for how many Skill Points get
// placed across a campaign -- see docs/PILLARS.md pillar 4.
inline constexpr SkillDef k_SkillDefs[k_MaxSkills] =
{
	//  id                        name                description                                                    sprite           col row cost prereq                     prereq2                  tier
	{ ESkillId::None,            "None",             "",                                                            nullptr,          0,  0,  0,  ESkillId::None,            ESkillId::None,          ENodeTier::Minor  },

	// 1-2: melee, col 0
	{ ESkillId::CrowbarRange,    "Crowbar Reach",    "Your crowbar connects from 25% further away.",                "d_crowbar",      0,  0,  1,  ESkillId::None,            ESkillId::None,          ENodeTier::Minor  },
	{ ESkillId::CrowbarDamage,   "Crowbar Force",    "Crowbar hits land 50% harder.",                               "d_skull",        0,  1,  2,  ESkillId::CrowbarRange,    ESkillId::None,          ENodeTier::Medium },

	// 3-4: armaments, col 4. Mastery is the root and Fast Reload hangs off it,
	// rather than the reverse: the cheap utility Skill should not be the toll
	// gate in front of the column's headline effect.
	{ ESkillId::FastReload,      "Fast Reload",      "Every magazine you feed goes in 20% quicker.",                "d_9mmhandgun",   4,  1,  2,  ESkillId::ExtraDamage,     ESkillId::None,          ENodeTier::Medium },
	{ ESkillId::ExtraDamage,     "Weapon Mastery",   "Every weapon you carry deals 10% more damage.",               "d_9mmAR",        4,  0,  3,  ESkillId::None,            ESkillId::None,          ENodeTier::Major  },

	SKILL_RESERVED(HighJump),
	// Also carried a self-prerequisite bug, which goes away with the row.
	SKILL_RESERVED(SprintSpeed),

	// 7-10: survivability, cols 5-6
	{ ESkillId::FallResistance,  "Sure Footing",     "Falls deal half as much damage.",                             "item_longjump",  6,  1,  1,  ESkillId::MoreHealth,      ESkillId::None,          ENodeTier::Minor  },
	{ ESkillId::MoreHealth,      "Fortitude",        "+25 maximum health.",                                         "item_healthkit", 5,  0,  2,  ESkillId::None,            ESkillId::None,          ENodeTier::Minor  },
	{ ESkillId::ArmorEfficiency, "Armor Expert",     "A tenth less damage gets past your armor.",                   "suit_full",      5,  1,  2,  ESkillId::MoreHealth,      ESkillId::None,          ENodeTier::Medium },
	{ ESkillId::HealthRegen,     "Regeneration",     "Wounds slowly close on their own.",                           "cross",          5,  2,  3,  ESkillId::ArmorEfficiency, ESkillId::None,          ENodeTier::Major  },

	SKILL_RESERVED(CrowbarSpeed),

	// 12: the Pulse, col 1
	{ ESkillId::PulseWindow,     "Pulse Window",     "The Shield stands 0.15s longer.",                             "autoaim_c",      1,  0,  1,  ESkillId::None,            ESkillId::None,          ENodeTier::Minor  },

	// 13-14: the suit, col 3
	{ ESkillId::BatteryCapacity, "Battery Capacity", "The suit holds 50 more armor.",                               "item_battery",   3,  0,  2,  ESkillId::None,            ESkillId::None,          ENodeTier::Minor  },
	{ ESkillId::BatteryRegen,    "Battery Regen",    "The suit slowly rebuilds its own armor.",                     "suit_empty",     3,  1,  3,  ESkillId::BatteryCapacity, ESkillId::None,          ENodeTier::Major  },

	// 15-17: the Pulse continued, cols 1-2
	{ ESkillId::PulseRecharge,   "Pulse Recharge",   "The wait between Pulses is a third shorter.",                 "flash_empty",    1,  1,  2,  ESkillId::PulseWindow,     ESkillId::None,          ENodeTier::Medium },
	{ ESkillId::PulseDischarge,  "Pulse Discharge",  "Negated hits vent energy at your crosshair.",                 "d_egon",         1,  2,  3,  ESkillId::PulseRecharge,   ESkillId::None,          ENodeTier::Major  },
	{ ESkillId::PulseRebound,    "Pulse Rebound",    "A deflect skips the Recharge. Once, until you sit through a normal one.", "flash_beam", 2, 2, 3, ESkillId::PulseRecharge, ESkillId::None,     ENodeTier::Major  },

	// 18: melee payoff, col 0. Gated on both branches: it is a crowbar payoff
	// for a Pulse deflect, and does nothing for a player who never deflects.
	{ ESkillId::CrowbarFollowUp, "Follow-Up",        "After a deflect, your next crowbar hit lands far harder.",     "d_gauss",        0,  2,  3,  ESkillId::CrowbarDamage,   ESkillId::PulseRecharge, ENodeTier::Major  },

	// 19: medical, col 6. Re-parented off root now that the whole survival
	// line is shipping -- Regeneration is the node it belongs behind.
	{ ESkillId::MedExpert,       "Med Expert",       "An Infusion runs 5 seconds longer.",                          "flash_full",     6,  2,  2,  ESkillId::HealthRegen,     ESkillId::None,          ENodeTier::Medium },

	// 20-21: the alien column, held until it opens
	SKILL_RESERVED(HiveCapacity),
	SKILL_RESERVED(HiveRegrowth),
};

#undef SKILL_RESERVED

// The table is indexed by id, and a row out of position silently reassigns
// Skills rather than failing anywhere visible -- a save's unlocked bits would
// simply start meaning different abilities.  Checked at compile time because
// the grouping that reads most naturally to a human is exactly the mistake.
constexpr bool SkillDefsAreIdOrdered()
{
	for (int i = 0; i < k_MaxSkills; ++i)
	{
		if (static_cast<int>(k_SkillDefs[i].id) != i)
			return false;
	}
	return true;
}

static_assert(SkillDefsAreIdOrdered(),
	"k_SkillDefs must be ordered by id: entry [n] is the Skill with id n");

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

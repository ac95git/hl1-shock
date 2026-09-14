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
// Presentation only -- nothing in the rules reads it, nothing
// saves it and nothing sends it, which is why the values could be
// renumbered when Stat was added below Minor on 2026-09-14.
// ---------------------------------------------------------
enum class ENodeTier : uint8_t
{
	Stat   = 0, // smallest -- a Stat node: one flat bonus, the roads between Skills (docs/SKILL_TREE.md)
	Minor  = 1, // small node  -- cheap/root Skills
	Medium = 2, // medium node -- mid-tree Skills
	Major  = 3, // large node  -- powerful end-tree Skills
	_Count = 4,
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

	// ---- The Melee Route (docs/SKILL_TREE.md) ----
	// Named for the crowbar until 2026-09-14; the enumerators and display
	// names changed for the melee roster, the ids did not.
	MeleeReach          = 1,  // swings connect from further      (was CrowbarRange)
	MeleeForce          = 2,  // hits land harder                 (was CrowbarDamage)
	MeleeSpeed          = 11, // swings come faster               (was CrowbarSpeed, reserved 2026-08 to 2026-09-14)
	FollowUp            = 18, // the hit after a deflect lands far harder (was CrowbarFollowUp)
	Backstab            = 33, // the rear-arc multiplier climbs
	Cleave              = 34, // the major: the first hit after a cooldown hits everything in its arc, harder

	// ---- Armaments ----
	FastReload          = 3,  // reload delay -20%
	ExtraDamage         = 4,  // all weapon damage +10%

	// ---- Survivability ----
	FallResistance      = 7,  // fall damage -50%
	MoreHealth          = 8,  // max health +25
	ArmorEfficiency     = 9,  // armor absorbs 10% more damage

	// ---- Cut, 2026-09-13.  Ids reserved forever; see docs/SKILL_TREE.md ----
	// Four Skills were cut when the tree became Routes, for two reasons.
	// Regeneration and Battery Regen rewarded standing still.  High Jump and
	// Sprint Speed altered the normal movement rules, which is a Module's job
	// (Dash, Hook) rather than a stat's.  Their ids are never reused.
	HighJump            = 5,
	SprintSpeed         = 6,
	HealthRegen         = 10,
	BatteryRegen        = 14,

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

	// ---- Reserved: the stealth column ----
	// The never-noticed Backstab tier is its first node.  See docs/SKILL_TREE.md.
	StealthReserved1    = 22,
	StealthReserved2    = 23,

	// ---- The Melee Route's Stat nodes ----
	// The roads between the Melee Skills: one flat bonus each, one point
	// each, one id each.  Numbered by id, not by position; the position is
	// in the table.
	StatMelee01         = 24,
	StatMelee02         = 25,
	StatMelee03         = 26,
	StatMelee04         = 27,
	StatMelee05         = 28,
	StatMelee06         = 29,
	StatMelee07         = 30,
	StatMelee08         = 31,
	StatMelee09         = 32,
	// 33 is Backstab and 34 is Cleave, above.

	_Count              = 35, // keep last
};

// ---------------------------------------------------------
// EStat
//
// What a Stat node grants.  A Skill has EStat::None; a Stat node
// has one of these and nothing else, and every node of the same
// stat adds the same amount (a cvar in game.cpp), so the effect
// is a count of unlocked nodes of that stat.  Only the stats a
// built Route uses are here -- the planned set, one per Route, is
// in docs/SKILL_TREE.md, and adding one is adding it to this enum
// and to the place its effect is computed.
// ---------------------------------------------------------
enum class EStat : uint8_t
{
	None        = 0,
	MeleeDamage = 1, // melee hits land harder, +skill_stat_melee_damage each
};

// How many ids have a row in k_SkillDefs.  Bounds every walk over the
// definition table.  Grows by one with each Skill added.
inline constexpr int k_MaxSkills = static_cast<int>(ESkillId::_Count);

// ---------------------------------------------------------
// The id ceiling
//
// The unlocked array in a save and the mask on the wire are sized
// to THIS, not to _Count.  CRestore::ReadField reads as many array
// entries as the code declares, not as many as the save holds, so
// an array sized by _Count changed shape with every Skill added and
// an older save's unlocked bits were followed by the bytes of the
// next field.  Sized once, to a ceiling, saves and the sync message
// stop changing shape.  Ids at or past the ceiling do not exist;
// the static_assert below is where that is enforced.
//
// Do not lower this: the save format is written against it.
//
// Raised from 96 to 256 on 2026-09-14 for the matrix tree, whose
// Stat nodes each take an id (docs/SKILL_TREE.md).  The saved array
// changed name with it (dlls/player_skills.cpp), so a 96-entry save
// resets its tree rather than over-reading.
// ---------------------------------------------------------
inline constexpr int k_SkillIdCeiling = 256;

static_assert(k_MaxSkills <= k_SkillIdCeiling,
	"ESkillId has grown past k_SkillIdCeiling; raise the ceiling (a save-format change) rather than "
	"working around it");

// Bytes needed to carry one bit per id up to the ceiling in the sync
// message.  Both DLLs derive this from the same constant, so they
// cannot disagree about the message length.
inline constexpr int k_SkillMaskBytes = (k_SkillIdCeiling + 7) / 8;

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
	EStat       stat;        // None for a Skill; what a Stat node grants
};

// A reserved id: it exists, nothing in the tree points at it, and it is not
// buyable.  Used for Skills cut pending other work and for ones held for a
// branch that has not opened yet.  RebuildNodeList and SpentPoints both key
// off the null name, so a Skill moved to this state disappears from the tree
// and refunds itself on the next load.
#define SKILL_RESERVED(idName) \
	{ ESkillId::idName, nullptr, nullptr, nullptr, 0, 0, 0, ESkillId::None, ESkillId::None, ENodeTier::Minor, EStat::None }

// A Melee Damage Stat node: the road material of the Melee Route.  Same
// name, text, icon and stat on every one; only the cell and the road differ.
#define STAT_MELEE(idName, col, row, prereqName) \
	{ ESkillId::idName, "Melee Damage", "Melee hits land 5% harder. Every Melee Damage node adds another 5%.", \
	  "d_crowbar", col, row, 1, ESkillId::prereqName, ESkillId::None, ENodeTier::Stat, EStat::MeleeDamage }

// Indexed by ESkillId, so entry [n] is always the Skill with id n.
//
// Every node costs ONE point: under the matrix (docs/SKILL_TREE.md) the price
// of a Skill is the road of Stat nodes to it, and a static_assert below holds
// every row to that.
//
// The Melee Route is built and occupies columns 0-3; the six other columns
// are the pre-Routes tree shifted right, each waiting for its Route to give
// it roads.  Follow-Up sits at the seam because it is gated on both Melee
// Force and Pulse Recharge, and a cross-link wants its two parents adjacent.
//
//     MELEE ROUTE                  PULSE      (fork)  SUIT     ARMS     SURVIVAL (fork)
//     col0     col1     col2       col3       col4    col5     col6     col7     col8      col9
//
// r0  S01      Reach    S02        .          Window          Capacity Mastery  Fortitude
// r1  Speed    .        Force      Follow-Up  Recharge                 Reload   ArmorExp  FallResist
// r2  S03      .        S04        .          Discharge Rebound                          MedExpert
// r3  S05      .        S06
// r4  S07      .        Backstab
// r5  S08      Cleave   S09
//
// Reach is the root.  The left road (S01, Speed, S03, S05, S07, S08) and the
// right road (S02, Force, S04, S06, Backstab, S09) meet at Cleave, the Melee
// major, which needs both S08 and S09.  Speed costs 3 points from nothing,
// Force 3, Backstab 6, Cleave 14.
inline constexpr SkillDef k_SkillDefs[k_MaxSkills] =
{
	//  id                        name                description                                                    sprite           col row cost prereq                     prereq2                  tier              stat
	{ ESkillId::None,            "None",             "",                                                            nullptr,          0,  0,  0,  ESkillId::None,            ESkillId::None,          ENodeTier::Minor,  EStat::None },

	// 1-2: the Melee Route's root and the right road's Skill
	{ ESkillId::MeleeReach,      "Melee Reach",      "Your melee swings connect from 25% further away.",            "d_tripmine",     1,  0,  1,  ESkillId::None,            ESkillId::None,          ENodeTier::Minor,  EStat::None },
	{ ESkillId::MeleeForce,      "Melee Force",      "Melee hits land 50% harder.",                                 "d_skull",        2,  1,  1,  ESkillId::StatMelee02,     ESkillId::None,          ENodeTier::Medium, EStat::None },

	// 3-4: armaments, col 7. Mastery is the root and Fast Reload hangs off it,
	// rather than the reverse: the cheap utility Skill should not be the toll
	// gate in front of the column's headline effect.
	{ ESkillId::FastReload,      "Fast Reload",      "Every magazine you feed goes in 20% quicker.",                "d_9mmhandgun",   7,  1,  1,  ESkillId::ExtraDamage,     ESkillId::None,          ENodeTier::Medium, EStat::None },
	{ ESkillId::ExtraDamage,     "Weapon Mastery",   "Every weapon you carry deals 10% more damage.",               "d_9mmAR",        7,  0,  1,  ESkillId::None,            ESkillId::None,          ENodeTier::Major,  EStat::None },

	// 5-6: cut (movement rules)
	SKILL_RESERVED(HighJump),
	SKILL_RESERVED(SprintSpeed),

	// 7-9: survivability, cols 8-9
	{ ESkillId::FallResistance,  "Sure Footing",     "Falls deal half as much damage.",                             "item_longjump",  9,  1,  1,  ESkillId::MoreHealth,      ESkillId::None,          ENodeTier::Minor,  EStat::None },
	{ ESkillId::MoreHealth,      "Fortitude",        "+25 maximum health.",                                         "item_healthkit", 8,  0,  1,  ESkillId::None,            ESkillId::None,          ENodeTier::Minor,  EStat::None },
	{ ESkillId::ArmorEfficiency, "Armor Expert",     "A tenth less damage gets past your armor.",                   "suit_full",      8,  1,  1,  ESkillId::MoreHealth,      ESkillId::None,          ENodeTier::Medium, EStat::None },

	// 10: cut (rewarded idling)
	SKILL_RESERVED(HealthRegen),

	// 11: the left road's Skill. Back from reserve with the Melee Route, which
	// dropped the rapid-swing halving that had kept it out.
	{ ESkillId::MeleeSpeed,      "Melee Speed",      "Melee swings come 30% faster.",                               "d_357",          0,  1,  1,  ESkillId::StatMelee01,     ESkillId::None,          ENodeTier::Medium, EStat::None },

	// 12: the Pulse, col 4
	{ ESkillId::PulseWindow,     "Pulse Window",     "The Shield stands 0.15s longer.",                             "autoaim_c",      4,  0,  1,  ESkillId::None,            ESkillId::None,          ENodeTier::Minor,  EStat::None },

	// 13: the suit, col 6.  One node until the Juggernaut Route fills the column.
	{ ESkillId::BatteryCapacity, "Battery Capacity", "The suit holds 50 more armor.",                               "item_battery",   6,  0,  1,  ESkillId::None,            ESkillId::None,          ENodeTier::Minor,  EStat::None },

	// 14: cut (rewarded idling)
	SKILL_RESERVED(BatteryRegen),

	// 15-17: the Pulse continued, cols 4-5
	{ ESkillId::PulseRecharge,   "Pulse Recharge",   "The wait between Pulses is a third shorter.",                 "flash_empty",    4,  1,  1,  ESkillId::PulseWindow,     ESkillId::None,          ENodeTier::Medium, EStat::None },
	{ ESkillId::PulseDischarge,  "Pulse Discharge",  "Negated hits vent energy at your crosshair.",                 "d_egon",         4,  2,  1,  ESkillId::PulseRecharge,   ESkillId::None,          ENodeTier::Major,  EStat::None },
	{ ESkillId::PulseRebound,    "Pulse Rebound",    "A deflect skips the Recharge. Once, until you sit through a normal one.", "flash_beam", 5, 2, 1, ESkillId::PulseRecharge, ESkillId::None,     ENodeTier::Major,  EStat::None },

	// 18: the Melee x Juggernaut link, at the seam. Gated on both: it is a
	// melee payoff for a Pulse deflect, and does nothing for a player who
	// never deflects.
	{ ESkillId::FollowUp,        "Follow-Up",        "After a deflect, your next melee hit lands far harder.",       "d_gauss",        3,  1,  1,  ESkillId::MeleeForce,      ESkillId::PulseRecharge, ENodeTier::Major,  EStat::None },

	// 19: medical, col 9. A root again: it was gated on Regeneration, which is
	// cut, and it is the root of the Medical Route in docs/SKILL_TREE.md.
	{ ESkillId::MedExpert,       "Med Expert",       "An Infusion runs 5 seconds longer.",                          "flash_full",     9,  2,  1,  ESkillId::None,            ESkillId::None,          ENodeTier::Medium, EStat::None },

	// 20-21: the alien column, held until it opens
	SKILL_RESERVED(HiveCapacity),
	SKILL_RESERVED(HiveRegrowth),

	// 22-23: the stealth column, held
	SKILL_RESERVED(StealthReserved1),
	SKILL_RESERVED(StealthReserved2),

	// 24-32: the Melee Route's roads.  Left road down column 0 from Reach
	// through Speed; right road down column 2 from Reach through Force and
	// Backstab.  S08 and S09 are the two ends Cleave will need.
	STAT_MELEE(StatMelee01, 0, 0, MeleeReach),
	STAT_MELEE(StatMelee02, 2, 0, MeleeReach),
	STAT_MELEE(StatMelee03, 0, 2, MeleeSpeed),
	STAT_MELEE(StatMelee04, 2, 2, MeleeForce),
	STAT_MELEE(StatMelee05, 0, 3, StatMelee03),
	STAT_MELEE(StatMelee06, 2, 3, StatMelee04),
	STAT_MELEE(StatMelee07, 0, 4, StatMelee05),
	STAT_MELEE(StatMelee08, 0, 5, StatMelee07),
	STAT_MELEE(StatMelee09, 2, 5, Backstab),

	// 33: the Backstab node, end of the right road before Cleave
	{ ESkillId::Backstab,        "Backstab",         "Hits from behind land half again as hard as a plain Backstab.", "d_crossbow",    2,  4,  1,  ESkillId::StatMelee06,     ESkillId::None,          ENodeTier::Medium, EStat::None },

	// 34: Cleave, the Melee major, where the two roads meet
	{ ESkillId::Cleave,          "Cleave",           "When Cleave is ready, your next melee hit strikes everything in front of you, and harder. Then it needs a moment.", "d_handgrenade", 1, 5, 1, ESkillId::StatMelee08, ESkillId::StatMelee09, ENodeTier::Major, EStat::None },
};

#undef SKILL_RESERVED
#undef STAT_MELEE

// Every node costs one.  The price of a Skill is the road to it, and a row
// that says otherwise is a row that would be drawn with no cost on it and
// charge something else -- so it is a compile error rather than a surprise.
constexpr bool SkillDefsCostOne()
{
	for (int i = 1; i < k_MaxSkills; ++i)
	{
		const SkillDef& def = k_SkillDefs[i];
		if (def.name && def.name[0] && def.cost != 1)
			return false;
	}
	return true;
}

static_assert(SkillDefsCostOne(), "every node in the Skill Tree costs one point; see docs/SKILL_TREE.md");

// No two rows in one cell.  With 180 hand-placed rows this is the mistake
// that will be made, and it draws as one node hiding another.
constexpr bool SkillDefsOnePerCell()
{
	for (int i = 1; i < k_MaxSkills; ++i)
	{
		const SkillDef& a = k_SkillDefs[i];
		if (!a.name || !a.name[0])
			continue;
		for (int j = i + 1; j < k_MaxSkills; ++j)
		{
			const SkillDef& b = k_SkillDefs[j];
			if (!b.name || !b.name[0])
				continue;
			if (a.gridCol == b.gridCol && a.gridRow == b.gridRow)
				return false;
		}
	}
	return true;
}

static_assert(SkillDefsOnePerCell(), "two Skill Tree rows share a cell");

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
// The unlocked mask -- one bit per id up to the ceiling.
//
// This is the whole of what the server tells the client about
// which Skills a player has.  Bounded by the ceiling, not by
// _Count: the mask is storage, and what it holds is decided by
// whoever walks the definition table.
// ---------------------------------------------------------
inline bool SkillMaskGet(const unsigned char* mask, int id)
{
	if (!mask || id <= 0 || id >= k_SkillIdCeiling)
		return false;
	return (mask[id >> 3] & (1 << (id & 7))) != 0;
}

inline void SkillMaskSet(unsigned char* mask, int id, bool value)
{
	if (!mask || id <= 0 || id >= k_SkillIdCeiling)
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

//=========================================================
// skill_defs.h
//
// The single definition of every Skill: what it is called, what
// it does, where it sits on the Skill Tree's board, and what it
// costs.  Compiled into BOTH the server and the client --
// see docs/adr/0008-skill-definitions-are-shared-not-networked.md.
//
// Static Skill data is shared rather than networked.  Only
// per-player state -- which Skills are unlocked, and how many
// Skill Points are unspent -- travels over the wire.
//
// Since 2026-09-15 the tree has ONE START and OPEN ROADS
// (docs/adr/0012-the-skill-tree-has-one-start-and-open-roads.md):
// the suit at the centre is held from the first moment, and any
// node opens from any owned orthogonal neighbour.  There are no
// prerequisites; empty cells are the walls.  The board is
// docs/SKILL_MAP.md, cell by cell.
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
// Presentation only -- nothing in the rules reads it except the
// cost-one assert's exemption for the Suit.  Nothing saves it and
// nothing sends it, which is why the values could be renumbered
// when Stat was added below Minor on 2026-09-14 and Suit above
// Major on 2026-09-15.
// ---------------------------------------------------------
enum class ENodeTier : uint8_t
{
	Stat   = 0, // smallest -- a Stat node: one flat bonus, the roads between Skills (docs/SKILL_TREE.md)
	Minor  = 1, // small chip  -- the hub's rim Skills and each Route's entry
	Medium = 2, // medium chip -- mid-region Skills
	Major  = 3, // large chip  -- a region's Major, and the four big Skills kept large by decision
	Suit   = 4, // the processor -- the one start, held from spawn, never bought
	_Count = 5,
};

// ---------------------------------------------------------
// EGate
//
// What a node waits on before it is shown.  A gated node is drawn as a
// blank pad and cannot be bought until its Module is found; the
// server owns which gates are open.  None means always shown.
// ---------------------------------------------------------
enum class EGate : uint8_t
{
	None        = 0,
	PulseModule = 1, // the Pulse's nodes.  Open until the Pulse becomes a Module: the Pulse is suit hardware today
	DashModule  = 2, // the Shinobi region
	AlienModule = 3, // the Alien region
	NightVision = 4, // the Stealth region
	_Count      = 5, // keep last; bounds the skill_open_gates/skill_close_gates "all" loop
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
	MeleeForce          = 2,  // hits land harder; on the hub's west rim since 2026-09-15
	MeleeSpeed          = 11, // swings come faster               (was CrowbarSpeed, reserved 2026-08 to 2026-09-14)
	FollowUp            = 18, // the hit after a deflect lands far harder; the hub's south-west corner
	Backstab            = 33, // the rear-arc multiplier climbs
	Cleave              = 34, // the major: the first hit after a cooldown hits everything in its arc, harder

	// ---- Armaments ----
	FastReload          = 3,  // reload delay -20%
	ExtraDamage         = 4,  // all weapon damage +10%  (Weapon Mastery)

	// ---- Survivability: the hub's Skills ----
	FallResistance      = 7,  // fall damage -50%   (Sure Footing)
	MoreHealth          = 8,  // max health +25     (Fortitude, the hub's north rim)
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

	BatteryCapacity     = 13, // +50 max battery (the hub's south rim)

	// ---- The Infusion ----
	MedExpert           = 19, // longer Infusion from a Health Syringe

	// ---- The Alien region (docs/SKILL_TREE.md), gate EGate::AlienModule ----
	HiveCapacity        = 20, // Hivehand holds more hornets.  The region's root
	HiveRegrowth        = 21, // hornets replenish faster; display name "Hive Replenish"

	// ---- The Stealth region (docs/SKILL_TREE.md), gate EGate::NightVision ----
	// Renamed from StealthReserved1/2 on 2026-09-15 when the region was built;
	// the numbers did not move.
	Ambush              = 22, // the all-weapons unaware damage multiplier
	Phantom             = 23, // a Backstab on an Unseen monster buys silent seconds

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

	// ---- The Weapon Specialist Route (docs/SKILL_TREE.md) ----
	// Fast Reload (3) and Weapon Mastery (4) belong to it too; their ids
	// predate it.  Built node by node from 2026-09-14.
	Marksman            = 35, // bullets hit harder; the hub's east rim since 2026-09-15
	QuickDraw           = 36, // weapons come up faster
	Demolitions         = 37, // explosives dealt up, explosives taken down
	Headhunter          = 38, // head hits land harder
	SwapSurge           = 39, // the major: a window after a swap where everything lands harder
	// The Route's Stat nodes, its roads.
	StatBullet01        = 40,
	StatBullet02        = 41,
	StatBullet03        = 42,
	StatBullet04        = 43,
	StatBullet05        = 44,
	StatBullet06        = 45,
	StatBullet07        = 46,

	// ---- The Medical Route (docs/SKILL_TREE.md) ----
	// Med Expert (19) is its entry; the id predates it.  No passive healing
	// anywhere in it.
	Overheal            = 47, // a Syringe at full health heals above the maximum, decaying back
	Leech               = 48, // melee hits heal a fraction of the damage dealt
	LastStand           = 49, // the major: a killing hit spends a Syringe; Infusions doubled below 50
	// The Route's Stat nodes, its roads: Potency's ranks became these.
	StatHeal01          = 50,
	StatHeal02          = 51,
	StatHeal03          = 52,
	StatHeal04          = 53,

	// ---- The Energy Route (docs/SKILL_TREE.md) ----
	// Energy is DMG_ENERGYBEAM and nothing else: the katana, the egon, the
	// Discharge.
	EnergyDamage        = 54, // energy hits harder; the entry
	EgonFocus           = 55, // secondary fire unlocks the egon's narrow beam
	EnergyEfficiency    = 56, // uranium drains slower; was Egon Efficiency until the katana's wave spent uranium too
	QuickCharge         = 57, // CUT 2026-09-15: the wave has no charge to quicken.  Reserved forever
	Insulation          = 58, // less energy and shock damage taken
	EnergyMajor         = 59, // Overdraw, the major: energy attacks drain armour for bonus damage
	// The Route's Stat nodes, its roads: Energy Damage's ranks became these.
	StatEnergy01        = 60,
	StatEnergy02        = 61,
	StatEnergy03        = 62,
	StatEnergy04        = 63,

	// ---- The Juggernaut Route (docs/SKILL_TREE.md) ----
	// Fortitude (8), Armor Expert (9), Battery Capacity (13) are the hub's
	// now; the four Pulse Skills are the region's far side.
	Ricochet            = 64, // a chance per bullet to bounce it back at the attacker

	// ---- The suit: the one start (ADR-0012), 2026-09-15 ----
	Suit                = 65, // held from spawn and through a Reset, never bought

	// ---- The board's extra roads, 2026-09-15 (docs/SKILL_MAP.md) ----
	// The 15x15 board has more Stat cells per region than the 2026-09-14
	// columns did.  Numbered as they were placed, never by position.
	StatMelee10         = 66,
	StatMelee11         = 67,
	StatBullet08        = 68,
	StatBullet09        = 69,
	StatBullet10        = 70,
	StatBullet11        = 71,
	StatBullet12        = 72,
	StatBullet13        = 73,
	StatHeal05          = 74,
	StatHeal06          = 75,
	StatHeal07          = 76,
	StatHeal08          = 77,
	StatHeal09          = 78,
	StatHeal10          = 79,
	StatEnergy05        = 80,
	StatEnergy06        = 81,
	StatEnergy07        = 82,
	StatEnergy08        = 83,
	StatEnergy09        = 84,
	StatEnergy10        = 85,

	// ---- The hub's Stat nodes: the generic suit stats around the processor ----
	StatHealth01        = 86,
	StatHealth02        = 87,
	StatHealth03        = 88,
	StatArmour01        = 89,
	StatArmour02        = 90,
	StatArmour03        = 91,
	StatArmour04        = 92,
	StatMeleeHub        = 93, // the hub's one melee cell, west of the suit
	StatBulletHub       = 94, // the hub's one bullet cell, east of the suit

	// ---- The Juggernaut region's roads: Max Armour continuing the hub's ----
	StatArmour05        = 95,
	StatArmour06        = 96,
	StatArmour07        = 97,
	StatArmour08        = 98,
	StatArmour09        = 99,
	StatArmour10        = 100,
	StatArmour11        = 101,
	StatArmour12        = 102,
	StatArmour13        = 103,
	StatArmour14        = 104,

	// ---- The keystone, 2026-09-15 (docs/SKILL_TREE.md, "The keystone") ----
	// Max health becomes 50 -- the ceiling AFTER every other health bonus --
	// and Last Stand is armed permanently, no damage multiplier of any kind.
	// Drawn Major-sized with a RED frame by the client.
	GlassCannon         = 105,

	// ---- Melee's top-road Stat cell, un-islanded by Glass Cannon ----
	// (1,5) was deliberately left empty until Glass Cannon existed to fill
	// (0,5) beside it; see docs/SKILL_MAP.md.
	StatMelee12         = 106,

	// ---- The Shinobi region, 2026-09-15 (docs/SKILL_MAP.md), gate EGate::DashModule ----
	DashReach           = 107, // the Dash goes further
	DashRecovery        = 108, // the Skill at (4,4); the Dash comes back sooner
	SecondWind          = 109, // a second Dash charge
	Reprisal            = 110, // a one-shot melee kill refills a Dash
	Phase               = 111, // no damage taken during the Dash itself
	AirDash             = 112, // the major: the Dash works in the air, upward included

	// The region's roads: the Dash Recovery Stat nodes.
	StatDash01          = 113,
	StatDash02          = 114,
	StatDash03          = 115,
	StatDash04          = 116,
	StatDash05          = 117,
	StatDash06          = 118,
	StatDash07          = 119,
	StatDash08          = 120,
	StatDash09          = 121,
	StatDash10          = 122,
	StatDash11          = 123,
	StatDash12          = 124,
	StatDash13          = 125,

	// ---- The Stealth region, 2026-09-15 (docs/SKILL_MAP.md), gate EGate::NightVision ----
	// Ambush (22) and Phantom (23) are above, at their frozen ids.
	SoftStep            = 126, // the entry: crouched and walking body noise halved again
	Nightfall           = 127, // darkness conceals twice as much
	SlipAway            = 128, // breaking line of sight drops Suspicion by a third
	CutTheHead          = 129, // killing a squad leader drops the squad's Suspicion
	SilentKill          = 130, // the major: a kill below Spotted is unseen and unheard

	// The region's roads: the Concealment Stat nodes.
	StatConceal01       = 131,
	StatConceal02       = 132,
	StatConceal03       = 133,
	StatConceal04       = 134,
	StatConceal05       = 135,
	StatConceal06       = 136,
	StatConceal07       = 137,
	StatConceal08       = 138,
	StatConceal09       = 139,
	StatConceal10       = 140,

	// ---- The Alien region, 2026-09-15 (docs/SKILL_MAP.md), gate EGate::AlienModule ----
	// Hive Capacity (20) and Hive Replenish (21) are above, at their frozen ids.
	HiveAttackSpeed     = 141, // the Hivehand fires faster
	Pack                = 142, // more ghosts out at once
	Tether              = 143, // ghost lifetime longer
	Recall              = 144, // summon cooldown shorter
	AlienMajor          = 145, // the major: the volley is energy damage.  Name provisional

	// The region's roads: the Hornet Replenish Stat nodes.
	StatHornet01        = 146,
	StatHornet02        = 147,
	StatHornet03        = 148,
	StatHornet04        = 149,
	StatHornet05        = 150,
	StatHornet06        = 151,
	StatHornet07        = 152,
	StatHornet08        = 153,
	StatHornet09        = 154,
	StatHornet10        = 155,
	StatHornet11        = 156,

	// ---- The Juggernaut's Matrix, 2026-09-15 (docs/SKILL_MAP.md), gate EGate::PulseModule ----
	// The Pulse Module is open by default (dlls/player_skills.cpp), so these
	// are shown and buyable today like the rest of the Pulse's nodes.
	DefenseMatrix       = 157, // hold the Pulse key: armour absorbs far more, slowed, on a cooldown
	MatrixOnKill        = 158, // a kill while the Matrix is up restores some armour
	JuggernautMajor     = 159, // the major: decaying armour on Matrix activation.  Name provisional

	_Count              = 160, // keep last
};

// ---------------------------------------------------------
// EStat
//
// What a Stat node grants.  A Skill has EStat::None; a Stat node
// has one of these and nothing else, and every node of the same
// stat adds the same amount (a cvar in game.cpp), so the effect
// is a count of unlocked nodes of that stat.  The full set, one
// per region plus the hub's two, is in docs/SKILL_TREE.md ("The
// road stats"); adding one is adding it here and to the place its
// effect is computed.
// ---------------------------------------------------------
enum class EStat : uint8_t
{
	None            = 0,
	MeleeDamage     = 1, // melee hits land harder, +skill_stat_melee_damage each
	BulletDamage    = 2, // bullets hit harder, +skill_stat_bullet_damage each
	Healing         = 3, // Infusions and medkits heal more, +skill_stat_healing each
	EnergyDamage    = 4, // energy hits harder, +skill_stat_energy_damage each
	MaxHealth       = 5, // the hub: max health +skill_stat_max_health each, as a fraction
	MaxArmour       = 6, // the hub and the Juggernaut: max armour +skill_stat_max_armor each, as a fraction
	DashRecovery    = 7, // Shinobi: the Dash comes back sooner, -skill_stat_dash_recovery of its recharge each
	Concealment     = 8, // Stealth: monsters learn about the player slower.  No effect yet
	HornetReplenish = 9, // Alien: hornets come back faster.  No effect yet
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

// The Status page's numbers message (SendSkillStatsToClient): twelve shorts
// -- max health, max armour, nine multipliers and shares in thousandths, the
// Dash recharge in milliseconds.  One constant so both DLLs agree on it.
inline constexpr int k_SkillStatsBytes = 12 * 2;

// The board is 15x15 (docs/SKILL_MAP.md).  Cells are checked against
// this so a typo cannot place a node off the board; the reachability
// flood fill below is sized by it too.
inline constexpr int k_BoardCols = 15;
inline constexpr int k_BoardRows = 15;

// ---------------------------------------------------------
// SkillDef
//
// Everything about a Skill that is the same for every player.
//
// No prerequisites: a node opens from any owned orthogonal
// neighbour (ADR-0012).  Where a node sits is therefore the whole
// of what gates it, which is why the cell is in this table and the
// table is the board.
// ---------------------------------------------------------
struct SkillDef
{
	ESkillId    id;
	const char* name;        // display name
	const char* description; // hover text
	const char* spriteName;  // HUD sprite name; nullptr renders without an icon
	int         gridCol;     // column on the board, 0..k_BoardCols-1
	int         gridRow;     // row    on the board, 0..k_BoardRows-1
	int         cost;        // Skill Points to unlock: 1, or 0 for the Suit alone
	ENodeTier   tier;        // visual weight; Suit marks the start
	EStat       stat;        // None for a Skill; what a Stat node grants
	EGate       gate;        // None, or the Module the node is hidden behind
};

// A reserved id: it exists, nothing on the board holds it, and it is not
// buyable.  Used for Skills cut pending other work and for ones held for a
// region that has not opened yet.  RebuildNodeList and SpentPoints both key
// off the null name, so a Skill moved to this state disappears from the tree
// and refunds itself on the next load.
#define SKILL_RESERVED(idName) \
	{ ESkillId::idName, nullptr, nullptr, nullptr, 0, 0, 0, ENodeTier::Minor, EStat::None, EGate::None }

// The Stat node macros: one per stat, so every node of a stat has one name,
// text and icon and only the cell differs.

#define STAT_MELEE(idName, col, row) \
	{ ESkillId::idName, "Melee Damage", "+5% melee damage", \
	  "d_crowbar", col, row, 1, ENodeTier::Stat, EStat::MeleeDamage, EGate::None }

#define STAT_BULLET(idName, col, row) \
	{ ESkillId::idName, "Bullet Damage", "+5% bullet damage", \
	  "d_9mmAR", col, row, 1, ENodeTier::Stat, EStat::BulletDamage, EGate::None }

// Healing scales what a medkit heals, so its road wears the medkit; the HUD's
// health cross belongs to Max Health below, the number it raises.
#define STAT_HEAL(idName, col, row) \
	{ ESkillId::idName, "Healing", "+10% healing", \
	  "item_healthkit", col, row, 1, ENodeTier::Stat, EStat::Healing, EGate::None }

#define STAT_ENERGY(idName, col, row) \
	{ ESkillId::idName, "Energy Damage", "+5% energy damage", \
	  "dmg_shock", col, row, 1, ENodeTier::Stat, EStat::EnergyDamage, EGate::None }

// The hub's two roads wear the HUD's own readout icons, 2026-09-16: the
// health cross for Max Health and the suit for Max Armour, the icons beside
// the two numbers these raise.
#define STAT_HEALTH(idName, col, row) \
	{ ESkillId::idName, "Max Health", "+5% max HP", \
	  "cross", col, row, 1, ENodeTier::Stat, EStat::MaxHealth, EGate::None }

#define STAT_ARMOUR(idName, col, row) \
	{ ESkillId::idName, "Max Armor", "+5% max AP", \
	  "suit_full", col, row, 1, ENodeTier::Stat, EStat::MaxArmour, EGate::None }

// The three hidden regions' Stat macros, 2026-09-15.  Gated like every other
// node in their region (docs/adr/0012): hidden until the player holds the
// Module, per "Hidden means impassable".
#define STAT_DASH(idName, col, row) \
	{ ESkillId::idName, "Dash Recovery", "-5% Dash recovery time", \
	  "item_longjump", col, row, 1, ENodeTier::Stat, EStat::DashRecovery, EGate::DashModule }

#define STAT_CONCEAL(idName, col, row) \
	{ ESkillId::idName, "Concealment", "-5% Suspicion gain", \
	  "dmg_gas", col, row, 1, ENodeTier::Stat, EStat::Concealment, EGate::NightVision }

#define STAT_HORNET(idName, col, row) \
	{ ESkillId::idName, "Hornet Replenish", "+5% hornet replenish rate", \
	  "d_hornet", col, row, 1, ENodeTier::Stat, EStat::HornetReplenish, EGate::AlienModule }

// Indexed by ESkillId, so entry [n] is always the Skill with id n.
//
// Every node costs ONE point except the Suit, which costs nothing and is
// never bought: under the board (docs/SKILL_TREE.md) the price of a Skill
// is the road of nodes to it, and a static_assert below holds every row
// to that.
//
// The board, from docs/SKILL_MAP.md.  Nine 5x5 regions: the hub in the
// centre, Melee W, Medical N, Weapon Specialist E, Juggernaut S on the
// edges, Shinobi NW, Stealth NE, Alien SE, Energy SW in the corners.
// Cells whose Skill is not built are empty.  The three hidden regions --
// Shinobi (EGate::DashModule), Stealth (EGate::NightVision), Alien
// (EGate::AlienModule) -- and the Juggernaut's three Matrix nodes
// (EGate::PulseModule) are placed and gated, 2026-09-15: a hidden node is a
// blank pad and cannot be bought until its Module is found (ADR-0012,
// "Hidden means impassable").  Melee's (1,5) was deliberately left empty
// until Glass Cannon filled (0,5) beside it; both are built now.
//
//       c0     c1     c2     c3     c4   c5     c6     c7     c8     c9   c10    c11    c12    c13    c14
//  r0  [AirD]  D      .      D      D    Overh  H    LastS    H    Leech  C      .    Night   C    [Silnt]
//  r1   D      .    Phase    .      D    H      .      .      .      H    C      C      C      .      C
//  r2   D      D      D    Reach    D    H      H    MedEx    H      H    Phntm   .      C    CutHd   C
//  r3  Reprs   .      D      .    SecW   .      .      H      .      .    .      .    SlipA    .      .
//  r4   D      D      .      D    Recov  .      .      H      .      .    SoftS   C    Ambsh    .      C
//  r5  {GlsC}  S      .      S      .    .      .    Fort     .      .    B      B    Headh    B    Demol
//  r6   S      .    Speed    S      S    Sure   H      H      H      .    .      .      B      .      B
//  r7   S    Bkstb    S    Reach    S    Force  M    (SUIT)   B    Marks  B    QDraw    B      B    Mstry
//  r8   S      .      .      .      .    A      A      A      A    ArmEx  .      .      B      .      B
//  r9  [Clev]  S      S      S      .    F-Up   .    Batt     .      .    B      B    FastR    B    [Swap]
//  r10  E    EnDmg    .      .      E    A      .      A      .      A    N      .    HiveC    .      N
//  r11  E      .      E      E      E    A      A      A      A    Ricoc  N      N      N    HiveR   N
//  r12 Insul   .      .      .      .    A      .      .      .    PWin   HiveA   .      N      .      .
//  r13  E      E      E      .      .    A      .    [JMaj]  Matrx  PRech  N     Pack    N    Tethr   N
//  r14  .      .    EnEff    E      .    A    PDsch  PRebd   MoK     .    .      .    Recal    N    [AMaj]
inline constexpr SkillDef k_SkillDefs[k_MaxSkills] =
{
	//  id                        name                description                                                    sprite           col row cost tier              stat          gate
	{ ESkillId::None,            "None",             "",                                                            nullptr,          0,  0,  0,  ENodeTier::Minor,  EStat::None,  EGate::None },

	// 1-2: Melee's entry on the road in from the hub, and the hub's west rim Minor
	{ ESkillId::MeleeReach,      "Melee Reach",      "+25% melee reach",            "d_tripmine",     3,  7,  1,  ENodeTier::Minor,  EStat::None,  EGate::None },
	{ ESkillId::MeleeForce,      "Melee Force",      "+50% melee damage",                                 "d_skull",        5,  7,  1,  ENodeTier::Minor,  EStat::None,  EGate::None },

	// 3-4: the Weapon Specialist's two older Skills.  Mastery keeps the
	// Major-sized frame by decision (docs/SKILL_MAP.md, Sizes).
	{ ESkillId::FastReload,      "Fast Reload",      "-20% reload time, shotgun included", "d_9mmhandgun", 12, 9,  1,  ENodeTier::Medium, EStat::None,  EGate::None },
	{ ESkillId::ExtraDamage,     "Weapon Mastery",   "+10% all damage",               "d_shotgun",      14, 7,  1,  ENodeTier::Major,  EStat::None,  EGate::None },

	// 5-6: cut (movement rules)
	SKILL_RESERVED(HighJump),
	SKILL_RESERVED(SprintSpeed),

	// 7-9: the hub's Skills.  Fortitude is the north rim Minor, Sure Footing
	// the west rim toward Shinobi, Armor Expert the east rim toward Juggernaut.
	{ ESkillId::FallResistance,  "Sure Footing",     "-50% fall damage",                             "item_longjump",  5,  6,  1,  ENodeTier::Minor,  EStat::None,  EGate::None },
	{ ESkillId::MoreHealth,      "Fortitude",        "+25 max HP",                                         "cross",          7,  5,  1,  ENodeTier::Minor,  EStat::None,  EGate::None },
	{ ESkillId::ArmorEfficiency, "Armor Expert",     "-10% damage through armor",                   "suit_full",      9,  8,  1,  ENodeTier::Medium, EStat::None,  EGate::None },

	// 10: cut (rewarded idling)
	SKILL_RESERVED(HealthRegen),

	// 11: Melee Speed, off the top road
	{ ESkillId::MeleeSpeed,      "Melee Speed",      "-30% melee swing time",                               "d_357",          2,  6,  1,  ENodeTier::Medium, EStat::None,  EGate::None },

	// 12: the Pulse, in the Juggernaut's far corner.  Gated on the Pulse
	// Module, which is open until that Module exists.
	{ ESkillId::PulseWindow,     "Pulse Window",     "+0.15 s Shield window",                             "autoaim_c",      9,  12, 1,  ENodeTier::Minor,  EStat::None,  EGate::PulseModule },

	// 13: the hub's south rim Minor
	{ ESkillId::BatteryCapacity, "Battery Capacity", "+50 max AP",                               "item_battery",   7,  9,  1,  ENodeTier::Minor,  EStat::None,  EGate::None },

	// 14: cut (rewarded idling)
	SKILL_RESERVED(BatteryRegen),

	// 15-17: the Pulse continued.  Discharge and Rebound keep the Major-sized
	// frame by decision.
	{ ESkillId::PulseRecharge,   "Pulse Recharge",   "-33% Pulse recharge",                 "flash_empty",    9,  13, 1,  ENodeTier::Medium, EStat::None,  EGate::PulseModule },
	{ ESkillId::PulseDischarge,  "Pulse Discharge",  "Negated hits vent as energy at the crosshair.",                 "d_egon",         6,  14, 1,  ENodeTier::Major,  EStat::None,  EGate::PulseModule },
	{ ESkillId::PulseRebound,    "Pulse Rebound",    "A deflect skips the Recharge, once per normal Recharge.", "flash_beam", 7, 14, 1, ENodeTier::Major, EStat::None,  EGate::PulseModule },

	// 18: the Melee x Juggernaut link, in the hub's south-west corner cell.
	// A melee payoff for a Pulse deflect; it does nothing for a player who
	// never deflects.  Major-sized by decision.
	{ ESkillId::FollowUp,        "Follow-Up",        "x3 melee damage on the hit after a deflect",       "d_gauss",        5,  9,  1,  ENodeTier::Major,  EStat::None,  EGate::None },

	// 19: the Medical Route's entry, on its spine
	{ ESkillId::MedExpert,       "Med Expert",       "+5 s Infusion",                          "item_syringe",   7,  2,  1,  ENodeTier::Medium, EStat::None,  EGate::None },

	// 20-21: the Alien region's root and its first road-mate, 2026-09-15
	// (docs/SKILL_MAP.md).  Gated on EGate::AlienModule with the rest of
	// the region, placed further down the table at ids 141-156.
	{ ESkillId::HiveCapacity,    "Hive Capacity",    "Bigger hornet capacity.",                            "d_hornet",       12, 10, 1,  ENodeTier::Minor,  EStat::None,  EGate::AlienModule },
	{ ESkillId::HiveRegrowth,    "Hive Replenish",   "+50% hornet replenish rate",                               "d_hornet",       13, 11, 1,  ENodeTier::Medium, EStat::None,  EGate::AlienModule },

	// 22-23: the Stealth region's unaware-damage pair, 2026-09-15
	// (docs/SKILL_MAP.md).  Ambush and Assassinate were one verb, merged the
	// day they were proposed (docs/SKILL_TREE.md, Stealth).  Gated on
	// EGate::NightVision with the rest of the region below.
	{ ESkillId::Ambush,          "Ambush",           "Hits on a monster that has not Spotted you x1.25, not Noticed you x1.5.", "d_skull", 12, 4, 1, ENodeTier::Medium, EStat::None, EGate::NightVision },
	{ ESkillId::Phantom,         "Phantom",          "A Backstab on an unaware monster: 2 s at x1.2 speed, all movement silent.", "flash_beam", 10, 2, 1, ENodeTier::Medium, EStat::None, EGate::NightVision },

	// 24-32: the Melee Route's roads (docs/SKILL_MAP.md, Melee)
	STAT_MELEE(StatMelee01, 4, 7),
	STAT_MELEE(StatMelee02, 2, 7),
	STAT_MELEE(StatMelee03, 0, 7),
	STAT_MELEE(StatMelee04, 0, 8),
	STAT_MELEE(StatMelee05, 3, 6),
	STAT_MELEE(StatMelee06, 4, 6),
	STAT_MELEE(StatMelee07, 0, 6),
	STAT_MELEE(StatMelee08, 3, 5),
	STAT_MELEE(StatMelee09, 1, 9),

	// 33: Backstab, mid-road
	{ ESkillId::Backstab,        "Backstab",         "x1.5 Backstab damage", "d_crossbow",    1,  7,  1,  ENodeTier::Medium, EStat::None,  EGate::None },

	// 34: Cleave, the Melee major, in the region's south-west corner
	{ ESkillId::Cleave,          "Cleave",           "When ready, a melee swing hits everything in front at x1.5; 4 s cooldown.", "d_grenade", 0, 9, 1, ENodeTier::Major, EStat::None, EGate::None },

	// 35: Marksman, the hub's east rim Minor
	{ ESkillId::Marksman,        "Marksman",         "+15% bullet damage",                                     "autoaim_c",      9,  7,  1,  ENodeTier::Minor,  EStat::None,  EGate::None },

	// 36: Quick Draw on the entry road.  Predicted: DefaultDeploy runs on
	// both sides, so the scale comes through skill_tuning.h.
	{ ESkillId::QuickDraw,       "Quick Draw",       "-40% weapon draw time",                                 "d_357",          11, 7,  1,  ENodeTier::Medium, EStat::None,  EGate::None },

	// 37-38: the typed pair on the region's top row.  Demolitions is dealt
	// and taken: the resistance covers the player's own grenades.
	// Headhunter is the head hitgroup multiplier, player hits only.
	{ ESkillId::Demolitions,     "Demolitions",      "+25% explosive damage dealt, -50% explosive damage taken", "d_rpg_rocket", 14, 5, 1, ENodeTier::Medium, EStat::None, EGate::None },
	{ ESkillId::Headhunter,      "Headhunter",       "x1.5 head hits",              "d_skull",        12, 5,  1,  ENodeTier::Medium, EStat::None,  EGate::None },

	// 39: Swap Surge, the Route's major, in the region's far bottom corner.
	// The window opens on every DefaultDeploy, on a cooldown, so the
	// specialist juggles weapons and every swap is a hit.
	{ ESkillId::SwapSurge,       "Swap Surge",       "After a weapon swap, x1.5 damage for 2 s; 6 s cooldown.", "d_bolt", 14, 9, 1, ENodeTier::Major, EStat::None, EGate::None },

	// 40-46: the Weapon Specialist's roads (docs/SKILL_MAP.md, Specialist)
	STAT_BULLET(StatBullet01, 10, 7),
	STAT_BULLET(StatBullet02, 12, 7),
	STAT_BULLET(StatBullet03, 13, 7),
	STAT_BULLET(StatBullet04, 12, 6),
	STAT_BULLET(StatBullet05, 12, 8),
	STAT_BULLET(StatBullet06, 14, 6),
	STAT_BULLET(StatBullet07, 14, 8),

	// 47: Overheal, Medical's north-west corner.  Server-side, in the
	// Infusion's tick (player_infusion.cpp); the excess drains after.
	{ ESkillId::Overheal,        "Overheal",         "Infusions heal up to 50 HP over max; the excess drains after.", "dmg_chem", 5, 0, 1, ENodeTier::Medium, EStat::None, EGate::None },

	// 48: Leech, Medical's north-east corner.  Melee hits on a living
	// monster heal; server-side, in CCrowbar::Swing and CleaveArc.
	{ ESkillId::Leech,           "Leech",            "Melee hits heal 10% of their damage.",         "dmg_bio",        9,  0,  1,  ENodeTier::Medium, EStat::None,  EGate::None },

	// 49: Last Stand, the Medical major, Medical's top cell.  Fires from
	// CBasePlayer::TakeDamage (dlls/player.cpp); the doubled healing below
	// 50 is CPlayerInfusion::Think (dlls/player_infusion.cpp).  Glass Cannon
	// (105) arms the same effect permanently; see docs/SKILL_TREE.md, "The keystone".
	{ ESkillId::LastStand,       "Last Stand",       "A lethal hit leaves 1 HP, 3 s invulnerable, and fires a Syringe; Infusions x2 below 50 HP; 60 s cooldown.", "dmg_heat", 7, 0, 1, ENodeTier::Major, EStat::None, EGate::None },

	// 50-53: the Medical Route's roads (docs/SKILL_MAP.md, Medical)
	STAT_HEAL(StatHeal01, 7, 4),
	STAT_HEAL(StatHeal02, 7, 3),
	STAT_HEAL(StatHeal03, 6, 2),
	STAT_HEAL(StatHeal04, 8, 2),

	// 54: Energy Damage, the Energy Route's entry, under Melee's bottom road.
	// DMG_ENERGYBEAM at the damage chokepoints, the Marksman shape.
	{ ESkillId::EnergyDamage,    "Energy Damage",    "+15% energy damage", "d_egon",       1,  10, 1,  ENodeTier::Minor,  EStat::None,  EGate::None },

	// 55: Egon Focus, between Energy Efficiency's and Insulation's Stat roads.
	// Right click unlocks the SDK's dormant narrow beam: single target,
	// cheaper on uranium, no splash.
	{ ESkillId::EgonFocus,       "Egon Focus",       "Right click: narrow egon beam, single target, cheaper uranium.", "flash_beam", 2, 12, 1, ENodeTier::Medium, EStat::None, EGate::None },

	// 56: Energy Efficiency, on the region's bottom row.  The interval between
	// the egon's ammo ticks (server-side) and the katana's wave's uranium cost
	// (both DLLs, the cost check being predicted).
	{ ESkillId::EnergyEfficiency, "Energy Efficiency", "-25% uranium use", "d_satchel",   2,  14, 1,  ENodeTier::Medium, EStat::None,  EGate::None },

	// 57: Quick Charge, cut 2026-09-15
	SKILL_RESERVED(QuickCharge),

	// 58: Insulation, on the region's west edge.  Server-side, in the player's
	// TakeDamage; shock included so it means something in Xen.
	{ ESkillId::Insulation,      "Insulation",       "-30% energy and shock taken",                         "dmg_rad",        0,  12, 1,  ENodeTier::Medium, EStat::None,  EGate::None },

	// 59: Overdraw, the Energy major, on the Juggernaut seam beside the
	// armour column's foot.  Energy attacks drain armour for bonus damage.
	{ ESkillId::EnergyMajor,     "Overdraw",         "Energy attacks drain 0.5 AP per uranium down to 20 AP; x1.5 energy damage above 20 AP.", "item_battery", 4, 14, 1, ENodeTier::Major, EStat::None, EGate::None },

	// 60-63: the Energy Route's roads (docs/SKILL_MAP.md, Energy)
	STAT_ENERGY(StatEnergy01, 0, 10),
	STAT_ENERGY(StatEnergy02, 0, 11),
	STAT_ENERGY(StatEnergy03, 0, 13),
	STAT_ENERGY(StatEnergy04, 1, 13),

	// 64: Ricochet, the Juggernaut's east side, one step from the Alien door.
	// Server-side, in the player's TakeDamage: bullets only, armour required.
	{ ESkillId::Ricochet,        "Ricochet",         "20% of bullets bounce back at the shooter; needs AP.", "d_tracktrain", 9, 11, 1, ENodeTier::Medium, EStat::None, EGate::None },

	// 65: the suit.  Cost 0, held from spawn, kept through a Reset, never
	// bought (ADR-0012).  The one row the cost-one assert exempts.  No icon
	// by decision (2026-09-16): the processor frame with its die is the
	// mark, and the suit icon belongs to the armour roads around it.
	{ ESkillId::Suit,            "HEV Suit",         "Every road starts here.",             nullptr,          7,  7,  0,  ENodeTier::Suit,   EStat::None,  EGate::None },

	// 66-67: Melee's bottom road past Cleave, toward the Energy doors
	STAT_MELEE(StatMelee10, 2, 9),
	STAT_MELEE(StatMelee11, 3, 9),

	// 68-73: the Weapon Specialist's top and bottom rows
	STAT_BULLET(StatBullet08, 10, 5),
	STAT_BULLET(StatBullet09, 11, 5),
	STAT_BULLET(StatBullet10, 13, 5),
	STAT_BULLET(StatBullet11, 10, 9),
	STAT_BULLET(StatBullet12, 11, 9),
	STAT_BULLET(StatBullet13, 13, 9),

	// 74-79: Medical's two side columns and its top row
	STAT_HEAL(StatHeal05, 5, 2),
	STAT_HEAL(StatHeal06, 9, 2),
	STAT_HEAL(StatHeal07, 5, 1),
	STAT_HEAL(StatHeal08, 9, 1),
	STAT_HEAL(StatHeal09, 6, 0),
	STAT_HEAL(StatHeal10, 8, 0),

	// 80-85: Energy's middle row and the Juggernaut-side door cells
	STAT_ENERGY(StatEnergy05, 2, 13),
	STAT_ENERGY(StatEnergy06, 2, 11),
	STAT_ENERGY(StatEnergy07, 3, 11),
	STAT_ENERGY(StatEnergy08, 4, 11),
	STAT_ENERGY(StatEnergy09, 4, 10),
	STAT_ENERGY(StatEnergy10, 3, 14),

	// 86-92: the hub.  Health on the north diagonal row, armour on the south.
	STAT_HEALTH(StatHealth01, 6, 6),
	STAT_HEALTH(StatHealth02, 7, 6),
	STAT_HEALTH(StatHealth03, 8, 6),
	STAT_ARMOUR(StatArmour01, 5, 8),
	STAT_ARMOUR(StatArmour02, 6, 8),
	STAT_ARMOUR(StatArmour03, 7, 8),
	STAT_ARMOUR(StatArmour04, 8, 8),

	// 93-94: the hub's one melee cell and one bullet cell, each facing its Route
	STAT_MELEE(StatMeleeHub, 6, 7),
	STAT_BULLET(StatBulletHub, 8, 7),

	// 95-104: the Juggernaut's armour column and its top rows (docs/SKILL_MAP.md, Juggernaut)
	STAT_ARMOUR(StatArmour05, 7, 10),
	STAT_ARMOUR(StatArmour06, 5, 10),
	STAT_ARMOUR(StatArmour07, 9, 10),
	STAT_ARMOUR(StatArmour08, 5, 11),
	STAT_ARMOUR(StatArmour09, 6, 11),
	STAT_ARMOUR(StatArmour10, 7, 11),
	STAT_ARMOUR(StatArmour11, 8, 11),
	STAT_ARMOUR(StatArmour12, 5, 12),
	STAT_ARMOUR(StatArmour13, 5, 13),
	STAT_ARMOUR(StatArmour14, 5, 14),

	// 105: Glass Cannon, the tree's one keystone (docs/SKILL_TREE.md, "The
	// keystone"), on the Melee-Dash seam.  Max health becomes 50 -- the
	// ceiling AFTER every other health bonus (ApplySkillHealthBonus,
	// dlls/player_skills.cpp) -- and Last Stand is permanently armed
	// (CBasePlayer::LastStandArmed).  No damage multiplier of any kind: the
	// cannon is whatever the roads around it provide.  Drawn Major-sized
	// with a RED frame by the client, the one keystone marker.
	{ ESkillId::GlassCannon,     "Glass Cannon",     "Max HP fixed at 50. Last Stand always armed.", "d_skull", 0, 5, 1, ENodeTier::Major, EStat::None, EGate::None },

	// 106: Melee's top-road Stat cell, deliberately left empty until Glass
	// Cannon existed to un-island it (docs/SKILL_MAP.md).
	STAT_MELEE(StatMelee12, 1, 5),

	// ---- 107-125: the Shinobi region, 2026-09-15 (docs/SKILL_MAP.md,
	// docs/SKILL_TREE.md "Shinobi -- the Dash Route").  Every row here is
	// EGate::DashModule: a blank pad until the Dash Module is found
	// (ADR-0012, "Hidden means impassable").  No effect is implemented yet --
	// a hidden node cannot be bought either way.
	{ ESkillId::DashReach,       "Dash Reach",       "Longer Dash.",                              "train_forward2", 3, 2,  1, ENodeTier::Minor,  EStat::None, EGate::DashModule },
	{ ESkillId::DashRecovery,    "Dash Recovery",    "-25% Dash recovery time",                                "train_back",     4, 4,  1, ENodeTier::Medium, EStat::None, EGate::DashModule },
	{ ESkillId::SecondWind,      "Second Wind",      "+1 Dash charge",  "train_forward1", 4, 3,  1, ENodeTier::Medium, EStat::None, EGate::DashModule },
	{ ESkillId::Reprisal,        "Reprisal",         "A one-hit melee kill refills a Dash.", "d_skull", 0, 3, 1, ENodeTier::Medium, EStat::None, EGate::DashModule },
	{ ESkillId::Phase,           "Phase",            "No damage while Dashing.",        "suit_empty",     2, 1,  1, ENodeTier::Medium, EStat::None, EGate::DashModule },
	{ ESkillId::AirDash,         "Air Dash",         "Dash in the air, along the aim, up included.", "train_forward3", 0, 0, 1, ENodeTier::Major, EStat::None, EGate::DashModule },

	// The region's roads: 13 Dash Recovery Stat nodes.
	STAT_DASH(StatDash01, 1, 0),
	STAT_DASH(StatDash02, 3, 0),
	STAT_DASH(StatDash03, 4, 0),
	STAT_DASH(StatDash04, 0, 1),
	STAT_DASH(StatDash05, 4, 1),
	STAT_DASH(StatDash06, 0, 2),
	STAT_DASH(StatDash07, 1, 2),
	STAT_DASH(StatDash08, 2, 2),
	STAT_DASH(StatDash09, 4, 2),
	STAT_DASH(StatDash10, 2, 3),
	STAT_DASH(StatDash11, 0, 4),
	STAT_DASH(StatDash12, 1, 4),
	STAT_DASH(StatDash13, 3, 4),

	// ---- 126-140: the Stealth region, 2026-09-15 (docs/SKILL_MAP.md,
	// docs/SKILL_TREE.md "Stealth").  Ambush (22) and Phantom (23) are
	// already placed above, at their frozen ids; every row here is
	// EGate::NightVision, same as they are.
	{ ESkillId::SoftStep,        "Soft Step",        "-50% body noise crouched or walking",     "flash_empty",    10, 4, 1, ENodeTier::Minor,  EStat::None, EGate::NightVision },
	{ ESkillId::Nightfall,       "Nightfall",        "x2 concealment from darkness",         "dmg_cold",       12, 0, 1, ENodeTier::Medium, EStat::None, EGate::NightVision },
	{ ESkillId::SlipAway,        "Slip Away",        "Breaking line of sight while Noticed drops Suspicion by a third.", "autoaim_c", 12, 3, 1, ENodeTier::Medium, EStat::None, EGate::NightVision },
	{ ESkillId::CutTheHead,      "Cut the Head",     "Killing a squad leader drops the squad's Suspicion to the notice floor.", "d_skull", 13, 2, 1, ENodeTier::Medium, EStat::None, EGate::NightVision },
	{ ESkillId::SilentKill,      "Silent Kill",      "A kill below Spotted leaves no witness, no Disturbance, no squad alert.", "d_crossbow", 14, 0, 1, ENodeTier::Major, EStat::None, EGate::NightVision },

	// The region's roads: 10 Concealment Stat nodes.
	STAT_CONCEAL(StatConceal01, 10, 0),
	STAT_CONCEAL(StatConceal02, 13, 0),
	STAT_CONCEAL(StatConceal03, 10, 1),
	STAT_CONCEAL(StatConceal04, 11, 1),
	STAT_CONCEAL(StatConceal05, 12, 1),
	STAT_CONCEAL(StatConceal06, 14, 1),
	STAT_CONCEAL(StatConceal07, 12, 2),
	STAT_CONCEAL(StatConceal08, 14, 2),
	STAT_CONCEAL(StatConceal09, 11, 4),
	STAT_CONCEAL(StatConceal10, 14, 4),

	// ---- 141-156: the Alien region, 2026-09-15 (docs/SKILL_MAP.md,
	// docs/SKILL_TREE.md "Alien").  Hive Capacity (20) and Hive Replenish
	// (21) are already placed above, at their frozen ids; every row here is
	// EGate::AlienModule, same as they are.
	{ ESkillId::HiveAttackSpeed, "Hive Attack Speed", "Faster Hivehand fire.",                                "d_hornet",       10, 12, 1, ENodeTier::Medium, EStat::None, EGate::AlienModule },
	{ ESkillId::Pack,            "Pack",              "More ghosts at once.",                 "d_snark",        11, 13, 1, ENodeTier::Medium, EStat::None, EGate::AlienModule },
	{ ESkillId::Tether,          "Tether",            "Ghosts last longer.",                  "dmg_bio",        13, 13, 1, ENodeTier::Medium, EStat::None, EGate::AlienModule },
	{ ESkillId::Recall,          "Recall",            "Shorter summon cooldown.",                   "train_back",     12, 14, 1, ENodeTier::Medium, EStat::None, EGate::AlienModule },
	// name provisional: SKILL_TREE.md's Alien major has none yet
	{ ESkillId::AlienMajor,      "Energy Volley",     "The Hivehand's volley is energy damage.", "dmg_shock", 14, 14, 1, ENodeTier::Major, EStat::None, EGate::AlienModule },

	// The region's roads: 11 Hornet Replenish Stat nodes.
	STAT_HORNET(StatHornet01, 10, 10),
	STAT_HORNET(StatHornet02, 14, 10),
	STAT_HORNET(StatHornet03, 10, 11),
	STAT_HORNET(StatHornet04, 11, 11),
	STAT_HORNET(StatHornet05, 12, 11),
	STAT_HORNET(StatHornet06, 14, 11),
	STAT_HORNET(StatHornet07, 12, 12),
	STAT_HORNET(StatHornet08, 10, 13),
	STAT_HORNET(StatHornet09, 12, 13),
	STAT_HORNET(StatHornet10, 14, 13),
	STAT_HORNET(StatHornet11, 13, 14),

	// ---- 157-159: the Juggernaut's Matrix, 2026-09-15 (docs/SKILL_MAP.md,
	// docs/SKILL_TREE.md "Juggernaut").  EGate::PulseModule, open by default
	// (dlls/player_skills.cpp), so these are shown and buyable today like
	// the rest of the Pulse's nodes -- the gate comes out entirely when the
	// Pulse becomes a Module.
	{ ESkillId::DefenseMatrix,   "Defense Matrix",   "Hold Pulse 1 s: armor takes far more of each hit, -20% speed; ends on release, 6 s or 0 AP; 10 s cooldown.", "suit_full", 8, 13, 1, ENodeTier::Medium, EStat::None, EGate::PulseModule },
	{ ESkillId::MatrixOnKill,    "Matrix on Kill",   "A kill during the Matrix restores AP.", "item_battery",   8, 14, 1, ENodeTier::Medium, EStat::None, EGate::PulseModule },
	// name provisional: SKILL_TREE.md's Juggernaut major has none yet
	{ ESkillId::JuggernautMajor, "Decaying Armor",   "Activating the Matrix grants +100 decaying AP.", "dmg_rad", 7, 13, 1, ENodeTier::Major, EStat::None, EGate::PulseModule },
};

#undef SKILL_RESERVED
#undef STAT_MELEE
#undef STAT_BULLET
#undef STAT_HEAL
#undef STAT_ENERGY
#undef STAT_HEALTH
#undef STAT_ARMOUR
#undef STAT_DASH
#undef STAT_CONCEAL
#undef STAT_HORNET

// A row with a name is a node on the board; a row without one is a reserved id.
constexpr bool SkillDefIsNode(const SkillDef& def)
{
	return def.name != nullptr && def.name[0] != '\0';
}

// Every node costs one, except the Suit, which costs nothing and is never
// bought.  The price of a Skill is the road to it, and a row that says
// otherwise is a row that would be drawn with no cost on it and charge
// something else -- so it is a compile error rather than a surprise.
constexpr bool SkillDefsCostOne()
{
	for (int i = 1; i < k_MaxSkills; ++i)
	{
		const SkillDef& def = k_SkillDefs[i];
		if (!SkillDefIsNode(def))
			continue;
		if (def.tier == ENodeTier::Suit ? def.cost != 0 : def.cost != 1)
			return false;
	}
	return true;
}

static_assert(SkillDefsCostOne(), "every node in the Skill Tree costs one point, and the Suit costs none; see docs/SKILL_TREE.md");

// Exactly one Suit, and it is ESkillId::Suit.
constexpr bool SkillDefsOneSuit()
{
	int suits = 0;
	for (int i = 1; i < k_MaxSkills; ++i)
	{
		if (SkillDefIsNode(k_SkillDefs[i]) && k_SkillDefs[i].tier == ENodeTier::Suit)
		{
			if (k_SkillDefs[i].id != ESkillId::Suit)
				return false;
			++suits;
		}
	}
	return suits == 1;
}

static_assert(SkillDefsOneSuit(), "the board has one start, ESkillId::Suit, and it is the only ENodeTier::Suit row");

// Every node is on the board.
constexpr bool SkillDefsOnBoard()
{
	for (int i = 1; i < k_MaxSkills; ++i)
	{
		const SkillDef& def = k_SkillDefs[i];
		if (!SkillDefIsNode(def))
			continue;
		if (def.gridCol < 0 || def.gridCol >= k_BoardCols || def.gridRow < 0 || def.gridRow >= k_BoardRows)
			return false;
	}
	return true;
}

static_assert(SkillDefsOnBoard(), "a Skill Tree row is placed off the 15x15 board");

// No two rows in one cell.  With 150 hand-placed rows this is the mistake
// that will be made, and it draws as one node hiding another.
constexpr bool SkillDefsOnePerCell()
{
	for (int i = 1; i < k_MaxSkills; ++i)
	{
		const SkillDef& a = k_SkillDefs[i];
		if (!SkillDefIsNode(a))
			continue;
		for (int j = i + 1; j < k_MaxSkills; ++j)
		{
			const SkillDef& b = k_SkillDefs[j];
			if (!SkillDefIsNode(b))
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

// ---------------------------------------------------------
// The board as cells
//
// SkillIdAtCell is the one lookup the rule below and the client's
// drawing both use, so "what is next to this" is defined once.
// ---------------------------------------------------------
constexpr int SkillIdAtCell(int col, int row)
{
	if (col < 0 || col >= k_BoardCols || row < 0 || row >= k_BoardRows)
		return 0;
	for (int i = 1; i < k_MaxSkills; ++i)
	{
		const SkillDef& def = k_SkillDefs[i];
		if (SkillDefIsNode(def) && def.gridCol == col && def.gridRow == row)
			return i;
	}
	return 0;
}

// The four orthogonal neighbours of a cell, in a fixed order.
inline constexpr int k_NeighbourDCol[4] = { -1, 1, 0, 0 };
inline constexpr int k_NeighbourDRow[4] = { 0, 0, -1, 1 };

// Every node has at least one orthogonal neighbour on the board.  With no
// edges, the layout mistake is an island: a cell nothing touches, which
// could never be bought.  Hidden nodes count as neighbours: this is about
// the layout, not the state.
constexpr bool SkillDefsNoIslands()
{
	for (int i = 1; i < k_MaxSkills; ++i)
	{
		const SkillDef& def = k_SkillDefs[i];
		if (!SkillDefIsNode(def))
			continue;
		bool touched = false;
		for (int d = 0; d < 4 && !touched; ++d)
			touched = SkillIdAtCell(def.gridCol + k_NeighbourDCol[d], def.gridRow + k_NeighbourDRow[d]) != 0;
		if (!touched)
			return false;
	}
	return true;
}

static_assert(SkillDefsNoIslands(), "a Skill Tree node has no orthogonal neighbour and could never be bought");

// Every node is reachable from the Suit by walking orthogonal neighbours: a
// flood fill over the board at compile time.  An island of two touching
// nodes passes the check above and fails this one.
constexpr bool SkillDefsAllReachableFromSuit()
{
	bool seen[k_BoardCols][k_BoardRows] = {};
	int  stackCol[k_MaxSkills] = {};
	int  stackRow[k_MaxSkills] = {};
	int  top = 0;

	const SkillDef& suit = k_SkillDefs[static_cast<int>(ESkillId::Suit)];
	seen[suit.gridCol][suit.gridRow] = true;
	stackCol[top] = suit.gridCol;
	stackRow[top] = suit.gridRow;
	++top;

	while (top > 0)
	{
		--top;
		const int col = stackCol[top];
		const int row = stackRow[top];
		for (int d = 0; d < 4; ++d)
		{
			const int nc = col + k_NeighbourDCol[d];
			const int nr = row + k_NeighbourDRow[d];
			if (nc < 0 || nc >= k_BoardCols || nr < 0 || nr >= k_BoardRows)
				continue;
			if (seen[nc][nr] || SkillIdAtCell(nc, nr) == 0)
				continue;
			seen[nc][nr] = true;
			stackCol[top] = nc;
			stackRow[top] = nr;
			++top;
		}
	}

	for (int i = 1; i < k_MaxSkills; ++i)
	{
		const SkillDef& def = k_SkillDefs[i];
		if (SkillDefIsNode(def) && !seen[def.gridCol][def.gridRow])
			return false;
	}
	return true;
}

static_assert(SkillDefsAllReachableFromSuit(), "a Skill Tree node cannot be reached from the Suit");

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
// SkillReachable
//
// The one implementation of the gating rule, shared by the server
// (which asks about its own unlocked array) and the client (which
// asks about the mask it was sent).  'has' is any callable taking
// an int id and returning bool.
//
// A node is reachable when any node in an orthogonally adjacent
// cell is held (ADR-0012).  The Suit is never reachable: it is held,
// not bought.  Whether a node is HIDDEN (its gate closed) is a
// separate question the caller asks; a hidden node is never sold.
// ---------------------------------------------------------
template <typename HasSkillFn>
inline bool SkillReachable(int id, HasSkillFn has)
{
	const SkillDef* def = GetSkillDef(id);
	if (!def || !SkillDefIsNode(*def) || def->tier == ENodeTier::Suit)
		return false;

	for (int d = 0; d < 4; ++d)
	{
		const int other = SkillIdAtCell(def->gridCol + k_NeighbourDCol[d], def->gridRow + k_NeighbourDRow[d]);
		if (other != 0 && has(other))
			return true;
	}
	return false;
}

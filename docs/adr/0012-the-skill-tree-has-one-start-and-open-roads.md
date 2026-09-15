# 12. The Skill Tree has one start and open roads

Date: 2026-09-15

## Status

Accepted, and **built the same day** (`SkillReachable`, `ESkillId::Suit`, the four board asserts, the
board of SKILL_MAP.md). Supersedes the *reachability* and *roots* bullets of [SKILL_TREE.md, "The matrix"](../SKILL_TREE.md#the-matrix--settled-2026-09-14)
(2026-09-14); everything else in that section stands.

## Context

The matrix of 2026-09-14 made the tree dense — Stat nodes as roads, every node one point — but kept two
rules from the small tree it replaced: every node opens through curated prerequisites (two at most, both
required), and each Route has its own root that is always available.

Under those two rules the phrase "pathed through efficiently", which the matrix section used, described
nothing. A node's prerequisites are fixed, so the set of nodes a player must own to reach a Skill is
fixed; the player chooses *whether* to reach Cleave and never *how*. Seven free roots made it worse: a
seam between two Routes' regions could never be the cheap way into the second Route, because that Route's
root already cost one point from nowhere. Seven ladders side by side is a menu. The model the tree is
built after, Path of Exile's, gets its interest from the opposite rules — one start per character, and any
node touching one you own is buyable — so that two builds reach the same notable by different roads and a
detour through a neighbouring cluster's stats is a real trade.

Decided in a grill on 2026-09-15, alongside the count, the keystone and the presentation, all recorded in
SKILL_TREE.md.

## Decision

**One start.** The centre of the tree is the HEV suit, a node the player holds from the start of the game
and keeps through a Reset. There are no other roots.

**Every node opens from any owned orthogonal neighbour.** No node has curated prerequisites; the grid is
the adjacency. Roads are terrain, not edges. *(A first cut earlier the same day kept curated AND gates on
Skills and Majors; it was dropped within the hour for one rule with no exceptions, Path of Exile's own.)*

**Empty cells limit pathing.** Where the design wants a long road to a Major, it leaves cells empty. Price
is position and nothing else.

**The layout is nine regions on a 15×15 grid.** The centre region is the hub: the suit, the generic suit
stats, the four old survivability Skills, and the cross-Route Skills in its corner cells. The four edge
regions are the Routes a player can use in the first hour; the four corners are the Routes gated on a
Module or a late weapon, reached only through their two edge neighbours. The full placement is in
SKILL_TREE.md.

## Consequences

**The gating rule changes and stays in one place.** `SkillPrereqMet` in `skill_defs.h` is the one
implementation the server validates against and the client draws from ([ADR-0008](0008-skill-definitions-are-shared-not-networked.md)).
It becomes: a node is reachable when any node in an orthogonally adjacent cell is held. Both DLLs compile
it, so they cannot disagree. `prereq` and `prereq2` leave `SkillDef`, and with them the connector edges
and the `skilltree_debug_edges` overlay; traces between neighbours replace both.

**Majors are cheaper than under the AND.** Cleave cost both roads, 14 points; it now costs the shortest
road the empty cells allow, six to eight. Against 100 findable points a thorough player reaches about six
Majors. Accepted with the rule.

**The suit node is a node with no price.** It has an id, a cell and a row in the table, is held on spawn
and after `TryReset`, and is the one row the cost-one `static_assert` must exempt. `SpentPoints` must not
count it, or a Reset would refund a point that was never spent.

**Every built node moves and nothing else about it changes.** The 52 nodes built on 2026-09-14 keep their
ids, effects, cvars and icons; only their `gridCol`/`gridRow` and, for Stat nodes, their now-meaningless
`prereq` change. `docs/skill_tree.csv`, the 16-column placement sheet, is superseded by the region map in
SKILL_TREE.md.

**A node with no neighbour is a bug the compiler can catch.** With edges gone, the layout mistake becomes
an island: a node whose four neighbours are all empty, or a cluster no road from the suit reaches. A
`static_assert` beside `SkillDefsOnePerCell` should reject the first and, if it can be written in
`constexpr`, the second.

**Price becomes position.** What a Skill costs is now the shortest road from what the player already owns,
so the same Skill is cheap for one build and dear for another. That is the point, and it is also why the
region map is a design document: moving a region moves every price in it.

**Save and wire are untouched.** The tree is about 140 nodes against a ceiling of 256.

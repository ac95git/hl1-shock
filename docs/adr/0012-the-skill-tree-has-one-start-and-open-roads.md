# 12. The Skill Tree has one start and open roads

Date: 2026-09-15

## Status

Accepted. Supersedes the *reachability* and *roots* bullets of [SKILL_TREE.md, "The matrix"](../SKILL_TREE.md#the-matrix--settled-2026-09-14)
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

**Stat nodes open from any owned orthogonal neighbour.** A Stat node has no curated prerequisites at all;
the grid is its adjacency. Roads are terrain, not edges.

**Skills and Majors keep curated gates.** A Skill opens when its prerequisites are held — two at most,
both required — exactly as today, and those prerequisites are nodes adjacent to it. A Major that wants two
roads to converge still gets its AND.

**The layout is nine regions on a 15×15 grid.** The centre region is the hub: the suit, the generic suit
stats, the four old survivability Skills, and the cross-Route Skills in its corner cells. The four edge
regions are the Routes a player can use in the first hour; the four corners are the Routes gated on a
Module or a late weapon, reached only through their two edge neighbours. The full placement is in
SKILL_TREE.md.

## Consequences

**The gating rule gains a second clause and stays in one place.** `SkillPrereqMet` in `skill_defs.h` is
the one implementation the server validates against and the client draws from ([ADR-0008](0008-skill-definitions-are-shared-not-networked.md)).
It becomes: a Stat-tier node is reachable when any node in an orthogonally adjacent cell is held; any other
node is reachable when its listed prerequisites are held. Both DLLs compile it, so they cannot disagree.

**The suit node is a node with no price.** It has an id, a cell and a row in the table, is held on spawn
and after `TryReset`, and is the one row the cost-one `static_assert` must exempt. `SpentPoints` must not
count it, or a Reset would refund a point that was never spent.

**Every built node moves and nothing else about it changes.** The 52 nodes built on 2026-09-14 keep their
ids, effects, cvars and icons; only their `gridCol`/`gridRow` and, for Stat nodes, their now-meaningless
`prereq` change. `docs/skill_tree.csv`, the 16-column placement sheet, is superseded by the region map in
SKILL_TREE.md.

**A Stat node with no neighbour is a bug the compiler can catch.** With curated edges gone from the roads,
the layout mistake becomes an island: a Stat node whose four neighbours are all empty. A `static_assert`
beside `SkillDefsOnePerCell` should reject it. The `skilltree_debug_edges` overlay keeps its job for the
Skills' curated edges, which must still join adjacent cells.

**Price becomes position.** What a Skill costs is now the shortest road from what the player already owns,
so the same Skill is cheap for one build and dear for another. That is the point, and it is also why the
region map is a design document: moving a region moves every price in it.

**Save and wire are untouched.** The tree is about 140 nodes against a ceiling of 256.

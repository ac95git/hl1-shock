# Map Brief — the proving map

What a map has to contain for this mod's finished systems to be *judged* rather than merely built, and
what has to be true before anyone opens an editor.

Six systems are complete code that no level exercises. [PILLARS.md](PILLARS.md) says *tune* under four of
its six pillars and cannot, because tuning needs a level to tune against. [ROADMAP.md](ROADMAP.md#maps)
files this as the mod's most-shared blocker. This document is the brief that entry asks for.

**Last updated:** 2026-08-31 (branch `hl-shock`) — the one code dependency is resolved; this brief is
now entirely map work.

## Correcting the record first

Both PILLARS.md and ROADMAP.md say no map places anything. That is no longer true, and the difference
matters because it moves this work from "start a map pipeline" to "already have one":

**`topmap` exists, and it is the mod's start map.** `liblist.gam` sets `startmap "topmap"` and
`trainmap "topmap"`. It ships compiled, with its source beside it, in the install directory:

| File | What it is |
| --- | --- |
| `topmap.bsp` | 182 KB, compiled and playable |
| `topmap.map` | 27 KB of Valve-220 source |
| `topmap.log` | The full compile log, which is where the toolchain below is recorded |

**It already places three `item_syringe`.** So the Health Syringe is reachable in normal play, not only
through `give item_syringe`. It places no `item_skillpoint` and no `item_resettoken` — those two really
are unreachable outside `skill_addpoints` / `skill_addtokens`.

**No level design has gone into it, and none was meant to.** It is a sandbox: a `player_weaponstrip`, one
of most monsters in the game, a `trigger_print` tour, and everything handed to the player at once. That
makes it a good harness and a useless test of the economy, for the reason given under
[Tier 2](#tier-2--the-proving-map). Both facts matter — the pipeline is solved, the level is not.

**The toolchain is proven end to end.** Nothing here needs choosing; it needs writing down.

| Part | What was used |
| --- | --- |
| Editor | J.A.C.K. 1.1.3773 Freeware, `_generator` in the map's worldspawn |
| Compilers | VHLT (Vluzacn's ZHLT) v3.4 VL34 64-bit, `D:/Apps/J.A.C.K./halflife/hlcsg.exe` and friends |
| Compile args | `-low -wadautodetect` |
| WADs | `halflife.wad`, `liquids.wad`, `xeno.wad`, `decals.wad`, all from `valve/` |
| Map format | Valve 220 |

## The FGD has to be synchronized on every change

`fgd/halflife.fgd` in the repo and `topmod/top_mod.fgd` in the install are the same file, and the repo's
is the source of truth. **Any change to one is copied to the other in the same sitting**, because the
editor reads only the install's copy and `git` sees only the repo's, so drift is invisible from both
sides until a mapper cannot find an entity that demonstrably exists.

They had already drifted, in **both** directions, which is the trap worth recording:

| Missing from the repo's copy | Missing from the install's copy |
| --- | --- |
| `trigger_print` — a real custom entity (`projects/vs2019/trigger_print.cpp`) that `topmap` places 15 of | `item_syringe`, `item_skillpoint`, `item_resettoken` |
| The `Angle` flag on `light_spot`, which gives J.A.C.K. its angle widget | The upstream `allow_item_dropping` keyvalue |

So a one-way copy in *either* direction would have silently deleted a working entity definition.
Reconciled 2026-08-02 by merging both ways into the repo's copy and then copying it out; the two are now
byte-identical, and a plain copy is correct from here on.

Note that `trigger_print.cpp` lives in `projects/vs2019/` rather than `dlls/`, which is why it is easy to
miss when looking for the mod's custom entities. It compiles into `hldll` regardless.

## The map source is not in version control

`topmap.map` exists only inside the Half-Life install. The only custom map in this project is one
directory wipe from gone, and its compile history is invisible to `git log`. It belongs in the repo —
`maps/` alongside `fgd/` — with the `.bsp` built from it the way the DLLs are, rather than the `.bsp`
being the artefact of record.

## One thing that looks like a problem and is not

The compile log's command line reads `D:\GameLibrary\steam\...\topmod\maps\topmap` while the DLLs are
described as deploying to `D:\Apps\steam\...\topmod`. These are **the same directory**: `D:\Apps\steam`
is a symbolic link to `D:\GameLibrary\steam`. There is one install, reached by two paths, and both are
correct. Recorded so nobody spends an afternoon hunting a second Half-Life installation.

## Tier 1 — exercise the two untested pickups

Cheap, immediate, and not a design test. Drop `item_skillpoint` and `item_resettoken` into `topmap`
wherever they fit and recompile. `topmap` is a sandbox — one of most monsters, a `player_weaponstrip`, a
`trigger_print` tour — so it can say nothing about whether the economy is *tuned*, but it can say whether
the pickups work at all, which has never been observed.

Worth deliberately placing one Skill Point where the player will walk over it **with a full Inventory
Grid**. PILLARS claims Skill Points and Reset Tokens occupy no Cells and a full Grid cannot refuse them,
on the grounds that a progression reward left on the floor reads as a bug. That claim has never been
tested against a full Grid.

**Done when** a Skill Point picked up in play increments the counter in the Skill Tree panel, a Reset
Token banks and spends, both survive a save/load, and a full Grid refuses neither.

## Tier 2 — the proving map

`topmap` cannot answer the real question, and the reason is structural rather than a matter of polish: a
sandbox has no critical path, so "off the critical path" has no meaning in it. Every number in PILLARS'
economy is a claim about the *ratio* between what a thorough player finds and what a direct one does, and
that ratio needs a level with a shape.

### What it must contain

- **A critical path that can be walked without exploring.** Without this nothing else on the list means
  anything. It is the control condition.
- **Three optional spaces at escalating discovery cost** — one visible but awkward to reach, one hinted
  (a vent, an airflow sound, a light left on), one genuinely concealed behind a `func_breakable`. Three,
  because one tells you nothing about which kind of hiding place players actually search.
- **Backtracking.** PILLARS commits to it — things left behind stay in the map they were left in, and
  "maps are designed with backtracking in mind" is load-bearing for the Inventory design.
- **An alien slave encounter.** PILLARS names the slave the best single Pulse test subject: it is the only
  one of the three intended targets with both a telegraphed melee and a hitscan attack, and both are on the
  Shield's damage list.
- **One dark area.** ROADMAP warns that Concealment may do nothing on maps lit for readability, so the
  first map that could test it should have somewhere dark on purpose. Nothing needs to read the light level
  yet — the geometry just has to exist before it does.
- **Inventory pressure.** Enough weapons and items on the critical path that the Grid genuinely fills,
  since almost every Inventory claim is about behaviour when it is full.

### The placement spec

PILLARS sets the campaign economy: a 35-point tree, findable points roughly equal to that total, a
critical-path player affording 60–70%, roughly one Skill Point per optional space, and 5–10 Reset Tokens
across the campaign with the first around 20% in.

One map cannot carry those totals. It should carry the **ratio**, deliberately over-sampled so a single
play produces a readable result:

| What | Count | Where | Question it asks |
| --- | --- | --- | --- |
| Skill Point | 2 | On the critical path | Is the floor — what a player who explores nothing gets — enough to feel like progress? |
| Skill Point | 3 | One per optional space | Is ~60% of a map's points being missable the right split? |
| Reset Token | 1 | In the second optional space | Does the first Token arriving after some commitment feel like a reprieve or an anticlimax? |
| Health Syringe | 1 | Critical path | Baseline. |
| Health Syringe | 1 | Optional space | Is a Syringe worth a detour, or does it read as a medkit and get skipped? |
| Row Grant | 1 | Deepest optional space | Is a permanent +1 Row worth the deepest detour on the map? |

A player who sweeps the map ends with 5 points; one who does not ends with 2. If those two players do not
feel meaningfully different at the tree, the tree's costs are wrong, and that is a finding no amount of
reading the table produces.

### What it must deliberately *not* contain

**No traversal that assumes a Module.** ROADMAP's first open question — are Modules limited or
unlimited — is explicitly a commitment across every map in the mod, and it is unanswered. A proving map
containing a gap only Dash crosses would either pre-commit the answer or need rebuilding once it is made.
Build the map so the question stays open, and it stays valid whichever way the answer goes.

**No encounter that requires stealth.** Same reasoning, for open question 5. One encounter that is
*avoidable* is a useful probe and costs nothing; one that is unwinnable head-on decides the question by
accident.

### Done when

A player who explores is measurably better off at the Skill Tree than one who does not — which is
pillar 1's own acceptance criterion, and would be the first time it has been evaluated rather than
asserted.

## The one code dependency — resolved 2026-08-31

**The Row Grant entity now exists.** `item_rowgrant` is a `CItem` subclass calling the existing grant
path, the same shape as `CItemSkillPoint`, with an FGD line beside the other two progression pickups.

It is **not** named `item_inventory_upgrade`, as this brief originally proposed. CONTEXT.md settles the
term as *Row Grant* and lists "upgrade" among the words to avoid for it; the glossary wins.

One behaviour a mapper has to know. It is the only progression pickup with a real ceiling —
`inv_rows_max` defaults to 9 against an `inv_rows_start` of 5 — so a campaign has room for exactly
**four** Grants. A fifth is refused and **left standing in the world** rather than consumed for nothing.
That is deliberate and visible: a Grant that disappeared for no effect would be indistinguishable from a
bug, and an over-placed map should be obvious to its author.

Everything in the spec above is now placeable.

## What this brief does not decide

Recorded so the omissions read as choices:

- **Whether `topmap` survives.** It is a sandbox and a useful one; the proving map is a second map, not a
  replacement.
- **Any of ROADMAP's eight open questions.** The map is designed to keep the two that touch level design
  (Modules, stealth) genuinely open, and the other six do not bear on it.
- **Art.** ART_DEBT's entries are unaffected by this and will not be improved by it — with one exception
  worth expecting: a map that places Skill Points beside a `models/w_longjump.mdl` placeholder is exactly
  the situation that register calls actively misleading.

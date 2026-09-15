# The proving map — level design and build plan

The level that answers [MAP_BRIEF.md](MAP_BRIEF.md#tier-2--the-proving-map). The brief says what the map
must *contain*; this document says what it *is* — the rooms, the order, what goes where — and the order to
build it in for a mapper who has not built a level before.

**Status: tested and closed, 2026-09-15.** It served its purpose as a showcase of coworking on a map:
the agent generated the greybox from a written spec, Andrei edited it in J.A.C.K., and the loop in
[MAP_WORKFLOW.md](MAP_WORKFLOW.md) was proven end to end. It was never going to be the economy test below:
the Skill Point economy is not an open question ([PILLARS pillar 4](PILLARS.md#the-economy)), so
[the test](#the-test-when-it-is-built) will not be run. **`topmap` is the default test map.** The design
below is kept as a record of the rooms and the mapping numbers, not as a plan. Files: `maps/proving.map`
in the repo, compiled to `topmod/maps/proving.bsp`.

**The greybox came from a script, once.** `utils/maptool/greybox_proving.py` wrote `maps/proving.map`
on 2026-09-13 from the room list below, as axis-aligned boxes: every interior volume gets six wall slabs,
every interior volume is subtracted from every slab, so volumes that touch are open to each other and
nothing else seals the map. It covers build-order steps 1, 2, 3, the geometry of 5, and the lights of 6 —
everything except monsters, nodes and textures. **From here the `.map` is edited in J.A.C.K. and the
script is not re-run**, or it would overwrite the edits. Open the `.map` in J.A.C.K. (File → Open reads
`.map` directly), save it as `proving.jmf` beside it, and export to `.map` before each commit.

## Where things stand on 2026-09-13

- **Tier 1 of the brief is placed.** `topmap.map` in the install now holds 20 `item_skillpoint`,
  3 `item_resettoken` and 3 `item_rowgrant` (recompiled 2026-09-13). Whether its *done-when* passed —
  counter increments, Token banks and spends, save/load, a full Grid refuses neither — is not recorded
  anywhere yet. PILLARS.md and ROADMAP.md still say no map places a Skill Point; that sentence is stale.
- **The map source is still not in the repo.** `maps/` does not exist. The brief asks for it; this map
  should be the first thing in it, and `topmap.map` should join it.
- **The Pulse comes with the suit.** ROADMAP's Modules entry will make it a found Module later, but today
  `item_suit` is what gives the player the Pulse, so the suit has to be placed before the slave encounter.

## The shape in one paragraph

One indoor facility on two floors, walked as a **loop**. The player starts in an airlock, enters a
two-storey **hub**, and leaves it north through a storage room, a holding pen with alien slaves, and stairs
down into a **dark maintenance deck**. From the deck a return gallery climbs back to the hub's east door,
which only opens from that side. Opening it also opens the hub's south exit. So the critical path is
A → B → C → D → E → F → B → H, the hub is crossed twice, and anything dropped in it is seen again. Three
optional spaces hang off the path at rising discovery cost: a ledge you can see, a vent you can hear, and a
wall you have to hit.

```
                 [C Storage] ---- [D Pen] 
                     |               |
   [A Airlock] -- [B Hub] ==door== [F Gallery]      stairs down from D, up into F
                     |               |         
                 [H Exit]        [E Dark deck]  (lower floor)
```

`==door==` is the one-way door: opened by a button in F. `[H Exit]` is behind a door the same button opens.

## The rooms

Sizes are in map units and are starting points, not law. Heights are floor to ceiling.

### A — Airlock (start)

256 × 256 × 128. Bright. `info_player_start`, `item_suit` (variant 0, Agility — this is what gives the
Pulse), `weapon_crowbar`, `weapon_9mmhandgun`, one `ammo_9mmclip`. A `func_door` east into B. Nothing
else. Its job is to hand over the suit and the crowbar so the Pulse and `func_breakable` both work from
the first room.

### B — Hub

512 × 512 × 192, two storeys: a floor and a walkway at 128 with stairs. Four doors: west (from A, open),
north (to C, open), east (from F, **closed**, targetname `door_hub_east`, `wait` −1 so it stays open once
opened), south (to H, **closed**, targetname `door_hub_exit`, `wait` −1). Lit normally. Two
`item_healthkit` on the walkway. This is the room the player crosses twice; it should look different the
second time only because of what they did (doors open, things dropped).

### C — Storage (first fight, optional space 1)

384 × 512 × 256, tall. Stacks of `func_breakable` crates (64-unit cubes, material Wood, one of them with
*Spawn On Break* set to a battery). Three `monster_headcrab` among the crates. On the floor, on the way
to the north door: `weapon_shotgun`, `ammo_buckshot`, and **critical-path Skill Point 1**, unmissable.

**Optional space 1 — the ledge.** A mezzanine along one wall at height 128, visible from the door, with
no stairs. It is reached by crouch-jumping up a crate stack (crates at 48 and 96, then the ledge at 128:
each step is a comfortable crouch-jump, none is a plain jump). On it: **Skill Point** and the **optional
Health Syringe**. Visible from the floor, so the question it asks is only "is it worth the climb".

### D — Holding pen (the Pulse test, optional space 2)

512 × 384 × 160. Two `monster_alien_slave`: one close, behind a pillar on the entry side, so its melee
lands early; one at the far end on a raised step, so it fires its beam across the room. Two or three
pillars as cover. `weapon_9mmAR` with `ammo_9mmAR`, `weapon_handgrenade`, two `item_battery`, and the
**critical-path Health Syringe** on a table past the fight. Stairs down to E in the far corner
(16 rise × 32 run).

**Optional space 2 — the vent.** In the corner opposite the stairs, behind a crate you have to push past
or break: a floor-level duct opening 48 × 48, closed by a `func_breakable` grille (material Metal,
strength 1, a `{`-prefixed grate texture, render mode Solid). The hint is sound and light: an
`ambient_generic` inside the duct playing an airflow loop (browse `ambience/` in the entity's sound
picker; `ambience/wind2.wav` and `ambience/steamjet1.wav` exist), and a `light` inside so the grille
glows. The duct is 48 × 48, two turns, dead-ending in a 128 × 128 × 96 nook holding a **Skill Point** and
**the Reset Token**. Dead end on purpose: a vent that came out somewhere useful would be a shortcut, and
the critical path must stay the control condition.

### E — Dark deck (the lower floor)

A ring of 128-wide, 112-high corridors around a central block, roughly 640 × 512 in plan, with two or
three side alcoves. **No `light` entities except**: one red emergency light (`_light 255 0 0 30`) at each
corner, and one flickering fluorescent (style *Fluorescent flicker*) at the bottom of the stairs from D.
Everything else black. This is the first geometry in the mod built for Concealment's light term, so
resist the urge to make it readable.

One `monster_human_grunt` walking a `path_corner` loop around the ring (three or four corners). Because
the ring has two sides, the player can wait and go round the other way, or fight. **Nothing requires
either.** One `monster_zombie` standing in the alcove that holds `weapon_357` and `ammo_357`, as a reason
to go into an alcove.

At the foot of the stairs up to F, in a lit alcove: **critical-path Skill Point 2**, and a
`trigger_autosave`.

### F — Return gallery (optional space 3)

A 128 × 128 corridor, 768 long, from the top of E's stairs back to the hub's east door, with a bend in the
middle. `weapon_crossbow` and `ammo_crossbow` halfway. At the hub end: a `func_button` beside the door,
targeting a `multi_manager` that opens `door_hub_east` and `door_hub_exit`. The player then walks back
across B to H.

**Optional space 3 — the cache.** On the inside of the bend, a wall panel that is a `func_breakable`
(material Metal, strength 50, so a crowbar swing or two) textured as a vent cover or access panel, flush
with the wall, no light, no sound. Behind it a 192 × 192 × 128 room with the **Row Grant**, a **Skill
Point**, and a crate. This is the deepest and best-hidden thing in the map, which is what the brief asks
the Row Grant to be worth.

### H — Exit

192 × 256 × 128 beyond the hub's south door. A `trigger_once` firing a `game_text` saying the map is over,
and a `trigger_print` for the console. No changelevel; the sandbox `topmap` stays the start map.

## The placement table, resolved

| Brief row | Count | Room | Note |
| --- | --- | --- | --- |
| Skill Point, critical path | 2 | C floor, E stair alcove | Unmissable, lit |
| Skill Point, optional | 3 | C ledge, D vent nook, F cache | One per optional space |
| Reset Token | 1 | D vent nook | The second optional space, as the brief asks |
| Health Syringe, critical path | 1 | D, after the slaves | |
| Health Syringe, optional | 1 | C ledge | |
| Row Grant | 1 | F cache | Deepest, most concealed |
| Alien slave encounter | 2 slaves | D | Melee and beam both land |
| Dark area | 1 | E | Built for Concealment, not readability |
| Backtracking | — | B crossed twice | Doors from F reveal it |
| Avoidable encounter | 1 grunt | E | A probe for stealth, never a requirement |

## Inventory pressure needs a smaller Grid for the test

The Grid is 12 Cells wide and starts at 5 Rows: **60 Cells**. A weapon takes 3, an item Stack takes 1, and
Stacks hold 5 medkits or batteries and 3 Syringes. Everything on this map's critical path — seven weapons
and four or five Stacks — is about 26 Cells, and the whole game only has fifteen weapons. A 60-Cell Grid
will not fill on any honest map.

So run the proving map with `inv_rows_start 2` (24 Cells). Then the critical path just overfills the Grid,
the choice of what to drop is real, and the Row Grant's +1 Row is a third more space rather than a fifth.
Whether 2 Rows is the right *shipping* start is a separate question this map can inform; the cvar exists
so it can be asked without rebuilding anything.

## What the map must not do

Straight from the brief, restated as building rules:

- **Nothing needs a Module.** The ledge is a crouch-jump; no gap is wider than a running jump with room to
  spare; no drop is deeper than 128. Long jump is not placed. Whether Modules are limited is still open,
  and this map must be valid either way.
- **Nothing needs stealth.** The grunt in E can be shot. Nothing is unwinnable head-on.
- **No traversal gated on a Skill.** Every Skill Point is found by walking, jumping or hitting something.

## Numbers a first-time GoldSrc mapper needs

| Thing | Units |
| --- | --- |
| Player, standing | 32 × 32 × 72, eyes at 64 |
| Player, crouched | 36 tall |
| Step the player walks up | 18 or less; use 16 |
| Ledge a plain jump reaches | 44; use 32 or 40 |
| Ledge a crouch-jump reaches | 60; use 48 or 56 |
| Door | 64 wide × 96 high (128 for a big one) |
| Corridor | 128 wide × 128 high; 96 × 96 is the tight end |
| Crawl duct | 48 × 48; 40 is the minimum |
| Stairs | 16 rise × 32 run |
| Safe drop | 192 or less; fall damage begins around 210 |
| Monster corridor | 64 wide minimum; grunts and slaves use the 32 × 32 × 72 hull |

Other rules that save an afternoon each:

- **Stay on the grid.** 64 for room shells, 16 for details, 8 at the very least. Off-grid vertices are
  where leaks and hairline cracks come from.
- **Seal the world.** Every room is a closed box of brushes; every entity is inside one. A leak fails the
  compile with a pointfile. In J.A.C.K., *Map → Load Pointfile* draws a line from the leak to the outside;
  follow it.
- **Build rooms from six slabs, or Block then Hollow (Ctrl+H, wall thickness 32).** Never Carve. Never
  let brushes overlap.
- **Monsters need `info_node`s.** Without a node graph a monster can only walk straight at what it can
  see. Drop one every 128–256 units along every floor a monster should be able to reach, including the
  ring in E. The engine builds `maps/graphs/proving.nod` on first load; delete it after changing nodes.
- **Textures last.** One plain texture from `halflife.wad` on everything until the layout plays. Mark
  faces that can never be seen `NULL` once it matters.
- **Indoors only.** No sky, no `light_environment`. One less way to leak.

## Build order

Each step ends with a compile and a walk. Do not go on to the next until the current one plays.

1. **Greybox the critical path.** Rooms A, B, C, D, E, F, H as boxes, corridors and stairs joining them,
   all doors as openings, one texture, `info_player_start`. Compile with CSG and BSP only (fullbright)
   and walk the loop. Fix leaks, fix any place the player snags.
2. **Doors and the loop.** The four hub doors, the button in F, the `multi_manager`. Walk the loop again
   and confirm B's east and south doors are shut until F is reached.
3. **Pickups.** Everything in the placement table plus the weapons and ammo. Play it once direct: the
   counter in the Skill Tree panel should read 2 at H.
4. **Monsters and nodes.** Headcrabs, slaves, grunt with its `path_corner` loop, zombie. `info_node`s
   everywhere they walk.
5. **The three optional spaces.** Ledge and crate steps; duct, grille, sound and glow; the breakable panel
   and cache. Play it once sweeping: the counter should read 5, one Token banked, the Grid one Row
   larger.
6. **Light.** `light` entities everywhere except E; E as specified. Run RAD. Stand in E with the Suspicion
   debug readout up and confirm the light term moves.
7. **VIS, textures, trim.** Only now.
8. **Export `maps/proving.map` and commit it** with the `.jmf` beside it, the same way `fgd/` is kept.

## Compiling

The toolchain is the one `topmap.log` records: J.A.C.K. 1.1.3773, VHLT v3.4 in `D:/Apps/J.A.C.K./halflife/`,
args `-low -wadautodetect`, WADs from `valve/`, output to `topmod/maps/`. Nothing here changes. For steps
1–5 untick `hlrad` and `hlvis` in the Run Map dialog (expert mode) — the map is fullbright and compiles in
seconds. Re-enable them at step 6.

The editor reads `topmod/top_mod.fgd`, the repo keeps `fgd/halflife.fgd`, and they must stay identical —
see the brief. All four progression pickups and `trigger_print` are already in both.

## The test, when it is built

Two full plays from a new game with `inv_rows_start 2`, by the same person, on different days if possible:

- **Direct:** critical path only, no jumping on crates, no hitting walls. Note the Skill Point count at H,
  what was dropped and where, whether the Grid ever refused a pickup.
- **Sweep:** everything. Note the same, plus which optional space was found first and what gave it away,
  and whether the Token felt like a reprieve or an anticlimax.
- In both: save in E, load, confirm the counts. Open the tree at H and spend. Write down whether the two
  players feel different at the tree. That sentence is the whole point of the map.

Findings go to PILLARS.md under the pillar they touch, in the same commit as any number they change.

# Map Workflow

How the mod's maps are made, compiled, checked and shipped, and how an agent takes part in that without
being able to see the game. Set up 2026-09-13 with the proving map ([PROVING_MAP.md](PROVING_MAP.md)) as
the first map through it. What a map must *contain* is in [MAP_BRIEF.md](MAP_BRIEF.md); this is the
mechanics.

The loop is: **`maps/<name>.map` in the repo is the source → copied to `topmod/maps/` → compiled there by
the four VHLT tools → played → edited in J.A.C.K., exported back to `.map` → committed.** The `.bsp` is a
build product, like the DLLs. The `.map` is what `git` sees, and it is plain text, which is what makes
everything below possible.

**`topmap` is deliberately outside that loop** (Andrei, 2026-09-18). It is the working test map, under
constant edit, and J.A.C.K. exports straight into `topmod/maps/` — so versioning it would add a
copy-back step to every single save for a map nobody ships. It lives in the mod directory only.
`maps/topmap.map` was brought into the repo that day and taken straight back out; **do not re-add it.**
A map that is finished, or that the agent is expected to write or read, goes in the repo as
`proving.map` does. (`proving.max` is J.A.C.K.'s rolling backup of the previous save, not a source
format — it is in the repo by accident and can go.)

**Never edit a `.map` under `topmod/maps/` while J.A.C.K. is open.** It saves over the file from its own
in-memory copy and the edit vanishes without a word — which is exactly how four entities were lost on
2026-09-18. Check `Get-Process jack` first.

## Facts that bind the work

- **The source is Valve 220 text.** A `.map` is a list of entities; the first is `worldspawn` and holds
  the world brushes; every brush is a list of planes, each given by three points, a texture name and two
  texture axes. Anything that can write text can write a map. J.A.C.K.'s own format is `.jmf` (binary);
  keep the `.jmf` beside the `.map` for the editor's sake, but the `.map` is what is versioned and
  compiled.
- **The compile is four programs run in order** from J.A.C.K.'s folder, `D:\Apps\J.A.C.K.\halflife\`:
  `hlcsg`, `hlbsp`, `hlvis`, `hlrad`, each taking the map path without extension, all with `-low`, CSG
  also with `-wadautodetect`. They write `<name>.log` beside the map, and that log is the record of what
  happened. J.A.C.K.'s Run Map dialog runs the same four; a shell can run them by hand — but **only with
  the current drive set to `D:`**, because `worldspawn`'s `wad` paths are drive-relative
  (`/apps/steam/...`) and CSG resolves them against the shell's drive, not the map's. From PowerShell,
  `Push-Location D:\` first. A clean `topmap` compile is a few seconds, RAD included.
- **CSG and BSP alone give a playable, fullbright map in seconds.** Skip VIS and RAD while the layout is
  still moving. RAD with no `light` entities gives a black map, not a fullbright one.
- **A leak fails the compile.** BSP reports `LEAK` and writes `<name>.lin`; J.A.C.K.'s *Map → Load
  Pointfile* draws it. The greybox generator avoids leaks by construction (below); hand edits are where
  they come from.
- **WADs are referenced by path in `worldspawn`'s `wad` key** and resolved from `valve/`. Texture names
  must exist in one of them or CSG stops. `utils/maptool` has no WAD lister; the one-off used to pick the
  greybox textures read the `WAD3` directory (16-byte names at the offset in the header) — trivial to
  redo.
- **Sounds are loose files** under `valve/sound/`, not in a `.pak`, so a sound's existence is a directory
  listing. `ambience/wind2.wav` and `ambience/steamjet1.wav` are confirmed.
- **The editor reads `topmod/top_mod.fgd`; the repo keeps `fgd/halflife.fgd`.** Same file, copied on every
  change, both ways once already — see the brief.
- **Monsters need `info_node`s.** The engine builds `maps/graphs/<name>.nod` on first load and does not
  rebuild it when the nodes change; delete it after editing nodes.
- **`D:\Apps\steam` and `D:\GameLibrary\steam` are the same install.** One is a symlink to the other.

## Tools

Under `utils/maptool/`. Python 3, Pillow for the plan.

| Tool | Does |
| --- | --- |
| `greybox_proving.py OUT.map` | Wrote the proving map's greybox from a room list. Every interior volume gets six wall slabs; every interior volume is subtracted from every slab; volumes that touch are therefore open to each other and nothing else seals the map. **One-shot**: once the `.map` has been edited in J.A.C.K., re-running it overwrites the edits. A new map gets a copy of the script with its own room list, not this one re-parameterised. |
| `greybox_minemap.py OUT.map` | Wrote `minemap` (2026-09-18), the test bed for mining and Stations, with the same construction and its own room list: a hall with the kit and the three Stations, a closet of forty keycards that fills the Grid, a tunnel of five stable deposits, and a chamber with two unstable veins, a wall of cover and two monsters to lure. Deposits and Stations are brush entities written straight into the text. One-shot in the same way. |
| `mapplan.py IN.map OUT.png [--cuts Z,Z]` | Prints a classname count and draws a floor plan: a horizontal section at each cut height (one per storey; `40,-216` for the proving map), so walls, pillars, crates and stairs show and floors and ceilings do not. Brush entities outlined and labelled, pickups and monsters as dots. This is how the agent checks a map. |

Outside the repo, the compilers above, and `ripent.exe` beside them: `ripent -export <name>` writes the
`.bsp`'s entities to `<name>.ent`, `ripent -import <name>` puts an edited `.ent` back **without a
recompile**. That is the fast path for changing keyvalues, moving a pickup, or adding a point entity to a
compiled map — but the `.map` has to get the same edit or the next compile reverts it. Not yet exercised
here; the flags are from its own usage text.

## Where the agent stands

Recorded so the division of labour is explicit rather than discovered mid-task. "Comfortable" means done
here and checked by a compile or a plan; "not" means either impossible from the terminal or unverified.

### Comfortable

- **Generating axis-aligned geometry from a written spec.** Rooms, corridors, stairs, ledges, ducts,
  pillars, doorways: anything that is a box or a stack of boxes. The proving map's 331 brushes came out
  sealed on the first compile.
- **Reading a `.map`.** Counting what it places, finding where, drawing the plan, checking the placement
  table in the brief against what is actually there. The same works on the sandbox `topmap.map`.
- **Editing entities in the text.** Keyvalues, origins, adding point entities, wiring `targetname` to
  `target`, `multi_manager` sequences, door and button keys. This is search-and-replace on a text file.
- **Running the compile chain and reading the log.** Leaks, missing textures, warnings, light and patch
  counts. The four tools are invoked from a shell exactly as J.A.C.K. invokes them.
- **The FGD.** Adding and correcting entity definitions, keeping the two copies identical.
- **Placement against the economy.** Which pickup goes where and why, from PILLARS and the brief.
- **GoldSrc scale.** The numbers in PROVING_MAP.md (hulls, steps, door and corridor sizes) are standard
  and the map is built to them. The jump heights are the customary values, not measured in this mod —
  see below.

### Not comfortable

- **Seeing the result.** The agent cannot launch the game or take a screenshot. Everything about how a
  map *plays* — whether the climb works, whether the vent reads as a hint, whether E is dark enough,
  whether the crate hides the grille — is Andrei's to observe, per the capture workflow in the memory
  notes. The plan shows layout; it shows nothing about height, light or texture.
- **Anything that is not a box.** Ramps, arches, angled walls, vertex-edited brushes. The generator does
  not make them; the plan draws them as their bounding box; a hand-written non-axis-aligned plane has not
  been tried and is easy to get wrong (the point order sets the normal, and a flipped normal is a leak).
- **Texturing and lighting as craft.** Alignment, scale, trims, which textures suit a room, light colour
  and falloff. The agent can place a `light` and choose a name that exists; it cannot judge the look.
- **J.A.C.K. itself.** Everything said about its dialogs, hotkeys and `.jmf` handling is from general
  knowledge of the editor, not from driving it. The `.jmf` format is binary and is not read or written
  here.
- **Editing a hand-edited map with the generator.** It would overwrite the edits. After J.A.C.K. has
  touched a map, the agent edits the `.map` text directly, or the `.ent` via ripent.
- **Monster behaviour in the space.** `info_node` density, `path_corner` loops, whether a grunt actually
  patrols a ring rather than sticking in a corner. Untested. Placing them is easy; knowing they work is a
  play.
- **VIS and performance.** Hint brushes, `func_detail`, leaf counts. Nothing in this map is large enough
  to need it, and none of it has been exercised.
- **Two numbers to check in play.** Jump heights: the map assumes a plain jump reaches 44 units and a
  crouch-jump 60, and the ledge climb in C is built on that (crates at 48 and 96, ledge at 128). And
  `func_door` direction: `angles "0 -1 0"` is up and a yaw slides sideways, from `SetMovedir` in the SDK;
  correct by reading, unconfirmed by a door opening.

## The loop for a change

1. Edit `maps/<name>.map` — in J.A.C.K. (then export), or in the text for entity work.
2. Copy it to `topmod/maps/`. Compile: CSG and BSP for layout, all four for anything the player will judge.
3. Read the log for `LEAK`, `Error`, `Warning`, and that RAD reports the expected number of direct lights.
4. Draw the plan if the layout moved; compare against the brief's placement table.
5. Andrei plays it. Findings go to PILLARS.md if they move a number, to PROVING_MAP.md if they change the
   map, to this file if they change how maps are made.
6. Commit the `.map` (and `.jmf`), never the `.bsp`.

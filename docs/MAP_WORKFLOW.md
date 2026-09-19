# Map Workflow

How the mod's maps are made, compiled, checked and shipped, and how an agent takes part in that without
being able to see the game. Set up 2026-09-13 with the proving map ([PROVING_MAP.md](PROVING_MAP.md)) as
the first map through it. What a map must *contain* is in [MAP_BRIEF.md](MAP_BRIEF.md); this is the
mechanics.

The loop is: **`maps/<name>.map` in the repo is the source → copied to `topmod/maps/` → compiled there by
the four VHLT tools → played → edited in J.A.C.K., exported back to `.map` → committed.** The `.bsp` is a
build product, like the DLLs. The `.map` is what `git` sees, and it is plain text, which is what makes
everything below possible. **A new map starts before that loop**, as a plan drawn from a room spec — see
[The loop for a new map](#the-loop-for-a-new-map), settled 2026-09-18.

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
  must exist in one of them or CSG stops. `wadpack.py --list` reads any WAD's directory, valve's included,
  and `--dump` writes its textures out as PNGs to look at. The mod's own WAD is under
  [The mod's WAD](#the-mods-wad).
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
| `greybox.py SPEC.rooms.txt [OUT.map] [--plan OUT.png] [--cuts Z,...]` | **The generator for every map from 2026-09-18 on.** The same construction as the two one-shot scripts below, with the room list read from a text spec (the grammar is in [The loop for a new map](#the-loop-for-a-new-map) and at the top of the script). With `--plan` and no `OUT.map` it draws the plan from the map it built in memory, so the layout is judged before any `.map` exists. Checked against `greybox_minemap.py` on minemap's room list: the same 49 brushes, 114 entities and 366 textured faces. Not re-run on a map J.A.C.K. has saved. |
| `wadpack.py OUT.wad PNG_DIR [--install]` / `--list X.wad [--dump DIR] [--limit N]` | Packs a folder of PNGs into a WAD3, one miptex per file: each texture quantised to its own 256-colour palette, the three smaller mips box-filtered and mapped back onto it, a `{` name treated as masked with alpha 0 at index 255 in the engine's blue. `--install` copies to `topmod/`. `--list` reads a WAD back; run on `halflife.wad` it reports the layout the engine expects (mips, a `256`, the 768-byte palette, two bytes of padding), and ours matches it. Needs Pillow. |
| `greybox_proving.py OUT.map` | Wrote the proving map's greybox from a room list. Every interior volume gets six wall slabs; every interior volume is subtracted from every slab; volumes that touch are therefore open to each other and nothing else seals the map. **One-shot**: once the `.map` has been edited in J.A.C.K., re-running it overwrites the edits. A new map gets a copy of the script with its own room list, not this one re-parameterised. |
| `greybox_minemap.py OUT.map` | Wrote `minemap` (2026-09-18), the test bed for mining and Stations, with the same construction and its own room list: a hall with the kit and the three Stations, a closet of forty keycards that fills the Grid, a tunnel of five stable deposits, a chamber with two unstable veins, a wall of cover and two monsters to lure, and an arena behind a door with a mixed squad of melee and hornet alien grunts. Deposits and Stations are brush entities written straight into the text. One-shot in the same way — and **spent**: J.A.C.K. has since saved `minemap.map`, and the Panthereye's den west of the hall was added to the `.map` text by a one-off script that cut the doorway out of the wall brush in its way, so the `.map` is the source now. |
| `mapsemdiff.py OLD.map NEW.map` | **The reader-back for a J.A.C.K. save.** Compares two maps by meaning, not by text: every brush as its computed vertices and textures, every entity as its classname and keyvalues, and prints only what was removed, added or moved between entities. J.A.C.K. rewrites the whole file on save, so `git diff` shows thousands of lines for one pillar; this shows the pillar. Run against `git show HEAD:maps/<name>.map` after every hand edit, before compiling. |
| `brushes_near.py MAP x0 y0 z0 x1 y1 z1` | Lists every brush with a vertex inside the box: its owner entity and brush number (the number `Ctrl+Shift+G` jumps to in J.A.C.K.), faces, computed vertices, extents, which axis a prism or point runs along, and how many vertices are off the 16 grid. This is how a wrong-way cylinder was diagnosed from the terminal. |
| `mapplan.py IN.map OUT.png [--cuts Z,Z]` | Prints a classname count and draws a floor plan: a horizontal section at each cut height (one per storey; `40,-216` for the proving map), so walls, pillars, crates and stairs show and floors and ceilings do not. Brush entities outlined and labelled, pickups and monsters as dots. This is how the agent checks a map. `greybox.py` calls its `parse_text` and `draw` on a map it has not written yet. |

Outside the repo, the compilers above, `Wally`-class WAD editors if one is ever wanted (none is installed
and none is needed), and `ripent.exe` beside the compilers: `ripent -export <name>` writes the
`.bsp`'s entities to `<name>.ent`, `ripent -import <name>` puts an edited `.ent` back **without a
recompile**. That is the fast path for changing keyvalues, moving a pickup, or adding a point entity to a
compiled map — but the `.map` has to get the same edit or the next compile reverts it. Not yet exercised
here; the flags are from its own usage text.

## The loop for a new map

Settled 2026-09-18, in the grill on the art workflow, and the reason is one fact from the section below:
the generator is one-shot. Once J.A.C.K. has saved a map the agent can only edit its text, never
regenerate it, so a layout change across many rooms is cheap before the editor opens and dear after. The
loop puts the layout work where it is cheap and the craft where the eyes are:

1. **The spec first.** `maps/<name>.rooms.txt`, in the grammar below. Its header is the map's brief:
   what the map is for, where the player enters and leaves, **every gate as one line** — `hard`, or
   `soft: intended X, alternative Y`, the rule ROADMAP.md's *The shape of the game* sets — and the
   pickups and encounters, which the generator counts back. The rooms and the rest follow.
2. **The plan before the map.** `greybox.py SPEC --plan OUT.png` draws the plan from the spec with no
   `.map` written. Andrei reads the plan and says what moves; the spec changes; the plan is redrawn. As
   many rounds as it takes. Nothing here costs more than editing a text line.
3. **The map once the plan is agreed.** `greybox.py SPEC maps/<name>.map`, copy to `topmod/maps/`, CSG
   and BSP, walk it fullbright. Layout faults found on foot go back to the spec, and the map is
   regenerated: the editor has not opened yet.
4. **J.A.C.K. opens once**, for what the generator cannot do and the agent cannot judge: texture, light,
   detail, and the shapes that are not boxes. From here the `.map` is edited in J.A.C.K. or in the text,
   and the generator is not run on it again.
5. **Both are committed.** The `.map` (and `.jmf`) as the source, and the `.rooms.txt` beside it as the
   record of what the layout was meant to be — the intent the hand edits departed from, and the thing to
   read before asking why a room is where it is.

`topmap` stays outside this loop as it is outside the other one. `minemap` and `proving` predate it;
their specs were never written and their `.map` files are the source. **`shaft1` is the first map through
it** (2026-09-19): `maps/shaft1.rooms.txt` and its plan `maps/shaft1.plan.png`, the plan accepted the same
day and the `.map` generated, installed and compiled; three cuts walked and confirmed the same day, and
**at step 4 since the evening of 2026-09-19: the layout is frozen, the `.map` is edited in J.A.C.K. or in
the text, and the generator is not run on it again.** The spec stays beside it as the intent. Two facts
from it: a `trigger_once`'s `message` is a titles.txt key, so
raw text on screen is a `game_text` it targets; and `func_door` with a negative `lip` travels further than
its own thickness, which is how a 16-unit cage floor rises 448.

### The grammar

One item per line, `#` comments, `key=value` options after the numbers, quotes for a value with spaces.
Coordinates are map units, x east, y north, z up, absolute — or `@ROOM dx dy dz`, offsets from that
room's minimum corner. A doorway is a small room touching two big ones; no line opens it, touching is
enough. The header's text lines (`brief`, `gate`, `message` and the rest) are prose and are taken raw, so
an apostrophe in them is not an open quote; `message` may be quoted or not. The full text is at the top
of `greybox.py`.

| Line | Makes |
| --- | --- |
| `map NAME`, `message TEXT`, `wad A;B;C` | The header; `wad` defaults to valve's four |
| `wall TEX`, `floor TEX`, `ceil TEX` | The default texture of every wall, floor and ceiling face |
| `brief TEXT`, `entrance TEXT`, `exit TEXT` | The map's brief; continuation lines start with whitespace |
| `gate NAME hard` / `gate NAME soft: intended X, alternative Y` | One line per gate, counted and echoed |
| `room NAME x0 y0 z0 x1 y1 z1 [tex=T]` | An interior volume; `tex=` is one texture all round, for a duct |
| `solid x0 y0 z0 x1 y1 z1 [tex=T]` | A world brush inside a room, never subtracted: a ledge, a pillar, cover, a crate that must not break |
| `stairs x0 y0 z0 x1 y1 z1 dir=+x [tread=32] [rise=16] [tex=T]` | Stepped solids climbing along `dir` from `z0` to `z1`; the proving map's stairs exactly. `tread=32 rise=8` reads as a ramp |
| `sets x0 y0 z0 x1 y1 z1 along=x [every=128] [post=16] [tex=T]` | Support sets down a drift: two posts and a cap beam every `every` units, the box being the drift's interior. Steel or timber is the texture |
| `brushent CLASS x0 y0 z0 x1 y1 z1 tex=T [key=value ...]` | Any brush entity: `func_door`, `func_button`, `func_breakable`, `func_deposit`, `func_station`, `trigger_*` |
| `door x0 y0 z0 x1 y1 z1 [tex=T] [key=value ...]` | A `func_door` with the generators' defaults: rises, speed 100, lip 8, wait 4 (or -1 with a `targetname`) |
| `point CLASS x y z [key=value ...]` | Any point entity |
| `pickup CLASS x y z ...`, `encounter CLASS x y z ...` | The same, counted for the brief |
| `light x y z [R G B BRIGHT] [style=N]` | A `light`; default `255 255 255 200` |
| `node x y z` | An `info_node`; monsters need them |

## Decompiled Half-Life maps

Since 2026-09-19 Andrei keeps decompiles of the stock campaign under `E:\CustomAssets\maps\decompiled\`,
made with the Half-Life Unified SDK's map decompiler (the log says "Decompiling map using Tree strategy",
which is that tool's name for its brush-rebuilding mode). Each map comes as `<name>.map` in Valve 220, a
`<name>_generated.wad` holding the textures the tool pulled out of the BSP, and a `<name>.log`. Present so
far: the tram ride, `c0a0` to `c0a0e`, and Anomalous Materials, `c1a0` to `c1a0e` — the locker room, the
lab corridors, the test chamber.

**Not yet used, and whether to use them is an open grill** (ROADMAP.md, "Open questions"). The two
readings: a reference for proportions and construction, to be read with `mapplan.py` and in J.A.C.K. and
learned from (how wide a Black Mesa corridor is, how a tram track is built from `func_tracktrain` and
`path_track`, how the locker room is furnished); or brushwork to lift into this mod's maps, which is
Valve's copyrighted level geometry and a licensing question before it is a design one. A decompile is
never a source of truth here: the `.map` it writes is a reconstruction, with the tool's own warnings about
tiny volumes and unsplit brushes in the log.

## The mod's WAD

Settled 2026-09-18. The mod has used valve's textures alone; the first it needs of its own is the
crystal, which no stock WAD carries and which ART_DEBT.md wants to agree in hue with the Shard, the
arcs and the Heart, so the vein and the model are painted from one source.

- **The source tier is `E:\CustomAssets\textures\wad\<name>.png`**, painted by Andrei at final size:
  sides multiples of 16, 256 or under to sit beside valve's, the file name the texture name (15
  characters or fewer), and valve's name conventions for the special ones (`{` masked, with alpha 0 as
  the clear colour; `!` water; `sky`, `aaatrigger`, `null`, `clip`). **No script writes into that
  folder**, the rule every hand-made tier shares (CLAUDE.md, *Hand-made art*).
- **`wadpack.py` builds `topmod.wad` from it**, quantising each texture to its own 256-colour palette
  and building the mips. The result ships like a compiled model: a copy in the repo under `wads/`, and
  `topmod/topmod.wad` in the mod directory (`--install`), by hand, in the same sitting.
- **A map names it** in `worldspawn`'s `wad` key beside valve's, on the same drive-relative form
  (`/apps/steam/steamapps/common/Half-Life/topmod/topmod.wad`); `greybox.py`'s `wad` header line is
  where. Until a map does, CSG never looks for it.
- **Judging a texture is Andrei's**, in J.A.C.K.'s browser and in the game. The agent can read the WAD
  back (`--list`, `--dump`) and say whether the quantise banded a gradient; it cannot say whether a
  wall looks right.

## Where the agent stands

Recorded so the division of labour is explicit rather than discovered mid-task. "Comfortable" means done
here and checked by a compile or a plan; "not" means either impossible from the terminal or unverified.

### Comfortable

- **Generating axis-aligned geometry from a written spec.** Rooms, corridors, stairs, ledges, ducts,
  pillars, doorways: anything that is a box or a stack of boxes. The proving map's 331 brushes came out
  sealed on the first compile. Since 2026-09-18 the spec is a text file, `maps/<name>.rooms.txt`, and
  the plan is drawn from it before the map exists.
- **Packing a WAD** from painted PNGs, and reading any WAD back to check it.
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
  **The division that works, from 2026-09-20:** Andrei builds the shape in J.A.C.K. from steps the agent
  writes, and the agent reads the save back with `mapsemdiff.py` and `brushes_near.py`, then compiles.
  The record of those sessions is [CRAFT_LOG.md](CRAFT_LOG.md).
- **Texturing and lighting as craft.** Alignment, scale, trims, which textures suit a room, light colour
  and falloff. The agent can place a `light` and choose a name that exists; it cannot judge the look.
  Painting a texture is Andrei's too; the agent packs it.
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

For a map J.A.C.K. has already saved. A new map starts with [the loop above](#the-loop-for-a-new-map).

1. Edit `maps/<name>.map` — in J.A.C.K. (then export), or in the text for entity work.
   After a J.A.C.K. save, `mapsemdiff.py` against `HEAD` says what actually changed.
2. Copy it to `topmod/maps/`. Compile: CSG and BSP for layout, all four for anything the player will judge.
3. Read the log for `LEAK`, `Error`, `Warning`, and that RAD reports the expected number of direct lights.
4. Draw the plan if the layout moved; compare against the brief's placement table.
5. Andrei plays it. Findings go to PILLARS.md if they move a number, to PROVING_MAP.md if they change the
   map, to this file if they change how maps are made.
6. Commit the `.map` (and `.jmf`), never the `.bsp`.

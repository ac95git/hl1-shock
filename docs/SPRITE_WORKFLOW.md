# 2D Sprite Workflow

How the mod's HUD sprites are designed, built, reviewed and shipped. Set up 2026-09-12 with the Health
Syringe icon as the first piece through it; that icon's script is the worked example for everything below.

The loop is: **a Python script draws the icon → PNG → the agent looks at a preview sheet and iterates →
`.spr` → `hud.txt` → copied to the install → checked in game.** The art is code. An icon is changed by
editing its script and re-running it, never by editing the `.spr`. Since 2026-09-18 there is a second
way in: **a PNG drawn by hand**, which takes the same loop from the PNG on — see
[Hand-drawn icons](#hand-drawn-icons).

## Facts that bind the art

These are engine and SDK constraints, not preferences. Every one of them bit at least once.

- **`.spr` is 8-bit paletted.** 256 colours, one palette per file. For HUD work the source is greyscale
  and the palette is whatever the quantiser makes of it.
- **HUD sprites are drawn additively and tinted by code.** `SPR_Set(r,g,b)` then `SPR_DrawAdditive`.
  The sprite supplies brightness; the code supplies colour. A coloured source comes out wrong, and
  **dark pixels vanish** — over a lit background (the Grid's tinted cell, a bright wall) black is
  invisible. Edges must be carried by highlights, not outlines.
- **`SPR_DrawAdditive` draws at native size; `SPR_DrawGeneric` is the one scaled draw.** It takes a
  width, height and a GL blend pair. Two traps: the width and height are the size for the **whole sprite
  frame**, and the rect is cut out at that scale — a 340×90 icon on a 512×128 sheet asked for at 340×90
  comes out at two thirds, so scale the request by frame-over-rect. And the blend factors are raw GL enums
  (`SPR_BLEND_*` in `cl_util.h`). Both are wrapped once, in `SPR_DrawFitted` (`cl_dll/spr_fit.h`), which
  fits a rect into a box keeping its aspect; the Inventory Grid's Footprints and the Skill Tree's nodes
  both draw through it. Everything else on the HUD still draws native, so a sprite is still made at the
  size it will be seen, per resolution bucket, unless its caller fits it.
- **The scaled draw shrinks but does not magnify.** Asked for a size larger than the sprite's frame, the
  engine draws the frame's worth and clips the rest — measured 2026-09-14 on the Skill Tree, where the
  gauss and egon icons (64×32 at 1280, on an 80×56 box) were the only two magnified and lost their right
  edge. `SPR_DrawFitted` caps the fit at 1:1 for that reason, so a sprite smaller than its box sits centred
  at native size. **Art for a fitted box is made at least as large as the box will ever be**, at the
  largest bucket it is expected to be seen at; shrinking is free, growing is impossible.
- **`hud.txt` is resolution-bucketed.** Every name is defined at 320, 640, 1280 and 2560, and the engine
  picks the bucket for the current screen width. A name missing at one bucket makes `GetSpriteIndex`
  return -1, and the callers do not check. **Every icon ships at all four buckets.**
- **The engine takes the first `hud.txt` it finds.** So the mod's file must contain every valve entry too;
  a mod-side file with only the new lines would delete the whole stock HUD. Ours is generated, never
  hand-edited (see below).
- **Sizes follow the family the icon sits in.** Item icons (`item_healthkit`, `item_battery`) are
  20/44/88/132. Suit readouts (`suit_full`) are 20/40/80/120; the small readouts (`cross`, `flash_full`)
  16/32/64/96. The `dmg_*` tiles are 64 at 640 and 192 at 2560. The Skill Tree is the exception: it needs
  **one fixed size at every bucket**, under about 64px — see the register entry in `ART_DEBT.md`.
- **File paths in `hud.txt` are relative to `sprites/`, without extension**, and subdirectories work
  (valve uses `1280/`, `2560/`; ours use `top/`).
- Valve's `hud.txt` header says 257 entries and the file holds 247. The engine tolerates the overcount;
  our generated file carries the exact count.
- Valve's 2560 sprites are padded to 256×256 with the icon in the top-left rect. Decoding one gives the
  padded image; crop to the `hud.txt` rect for a like-for-like reference.

## Tools

All under `utils/sprtool/`. Python 3 with Pillow (`pip install pillow`).

| Tool | Does |
| --- | --- |
| `sprtool.py decode IN.spr OUT.png` | `.spr` → RGBA PNG. Additive sprites come out on transparent black; alphatest treats index 255 as clear. |
| `sprtool.py encode IN.png OUT.spr --format additive` | RGBA PNG → `.spr`. Flattens on black and quantises to 256 colours. `alphatest` reserves index 255 as the mask. |
| `sprtool.py sheet HUD.txt SPRITES_DIR OUT.png --res 640 --scale 4 --only a,b,c` | Tiles every `hud.txt` entry of one bucket into a labelled contact sheet. This is how the stock visual language gets studied. |
| `make_hud_txt.py VALVE_HUD.txt sprites/hud_additions.txt sprites/hud.txt` | Builds the shipped `hud.txt`: all of valve's entries plus ours, exact count. Refuses an addition that lacks any of the four buckets. |
| `icons/<name>.py OUT_DIR [--preview SHEET.png]` | One script per icon. Draws it supersampled, downsamples to each bucket size, writes PNG and `.spr`, and optionally a review sheet. |
| `from_png.py IN.png NAME --family item\|readout\|small\|skill\|weapon [--out sprites/top] [--additions sprites/hud_additions.txt] [--weapons-dir sprites] [--preview SHEET.png] [--selected] [--hud VALVE_HUD.txt]` | The hand-drawn path: one greyscale-on-black PNG → the four buckets of its family, sharpened where small, encoded additive, the `hud_additions.txt` lines set (idempotent), and the review sheet in every tint. For the weapon family it writes the `weapon` lines of `sprites/weapon_<name>.txt` instead and `--selected` derives the `_s` sprite. `--hud` regenerates `hud.txt` afterwards. Warns if the source carried colour or alpha. |

Outside the repo: **spr_explorer** (`D:\CreativeTools\GameAssets\spr_explorer`) opens a `.spr` for a human
look without launching the game. The agent's own reviewing is done by reading the preview PNGs directly.

## Layout

```
sprites/
  hud_additions.txt    our hud.txt lines, all four buckets per name  (hand-edited)
  hud.txt              generated: valve's entries + ours              (never hand-edited)
  top/*.spr            our HUD icons, one file per icon per bucket    (generated)
  inv/*.spr            Inventory Icons, one file per classname        (generated, see below)
utils/sprtool/
  sprtool.py           decode / encode / sheet
  make_hud_txt.py      hud.txt generator
  icons/*.py           the HUD icons' sources
```

`sprites/` in the repo is the source of truth. The install copy is `topmod/sprites/` in the Half-Life
directory, and **nothing in the build copies it** — copy `hud.txt`, `top/*.spr` and `inv/*.spr` by hand
after every change, in the same sitting, the same discipline as the FGD (`docs/MAP_BRIEF.md`).

## Inventory Icons

The Grid's Icons are a different kind of sprite from everything above, and the facts that bind HUD icons
mostly do not bind them:

- **Full colour, untinted, alpha-blended.** `SPR_Set(255,255,255)` and `SPR_DrawGeneric` with
  `GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA`. The source is a colour render; dark pixels stay.
- **`alphatest` format**, so index 255 is the mask and the other 255 colours are the item's own. One
  palette per file is plenty for a single object.
- **One size, no buckets, no `hud.txt`.** The Grid fits the sprite to the Footprint, so the size only
  sets detail: weapons 384×128 (3:1, three Cells), items 128×128. Not power-of-two, and the engine does
  not mind.
- **Found by classname.** `sprites/inv/<classname>.spr` — `weapon_357`, `item_healthkit` — loaded by
  `CInventoryGridView::IconFor` with no table entry anywhere. Weapons use `WEAPON::szName`, items the
  `classname` column of `k_ItemTypes`. A missing file is not an error; the Entry keeps its HUD sprite.

The source of every Icon is the thing's **world model**, rendered by `E:\CustomAssets\scripts\render_icon.py`
(Blender headless; see [MODEL_WORKFLOW.md](MODEL_WORKFLOW.md) for the working directory and the decompile
rule — the SMD has to have been decompiled by hand first):

```
blender.exe --background --python E:\CustomAssets\scripts\render_icon.py -- IN.smd OUT.png TEXDIR
    [--size 384x128] [--view top|side|front] [--roll DEG] [--margin 0.015] [--light flat|studio]
python utils\sprtool\sprtool.py encode OUT.png sprites\inv\<classname>.spr --format alphatest
```

Orthographic, on transparent, the long axis laid horizontal, fitted with the margin. `flat` lighting is
the set's choice: these textures carry their shading baked in, and `studio` on top of it went dark.
`top` suits a model that lies on the floor (most guns show their profile from above); `side` and `front`
are for the ones that do not (the RPG, the egon face-on, the battery, the grenade, the snark, the gauss,
the satchel). `--roll` turns the image; a label reading upside down is the usual reason. The tripmine has
no world model — its pickup is the viewmodel's `tripmine_boned_world` submodel, and that SMD is the input.

Review by contact sheet (`E:\CustomAssets\render\icons\sheet.png`, a few lines of Pillow) and then in
the Grid. The only judgement that matters is whether the thing is recognisable at Footprint size over the
bare lattice, since there is no box behind it.

Adding one: decompile its world model, render, encode, copy `sprites/inv/*.spr` to `topmod/sprites/inv/`,
open the Inventory. Nothing to rebuild.

## Style

Two families in the stock HUD, and a new icon should join one of them:

- **Item icons** (`item_*`): shaded greyscale objects in three-quarter view, visible thickness, no
  outline, bright edges over dark mass. The Syringe, the keycard (`icons/security.py`) and the Crystal
  Shard (`icons/shard.py`, flat-shaded facets with highlighted edges, and a second smaller point at the
  foot so it reads as crystal rather than as a pencil) are these.
- **Suit readouts** (`suit_full`, `flash_full`, `cross`): flat glyph silhouettes with a soft one-to-two
  pixel glow, drawn as light. The carried arrow (`inv_carried`, `icons/carried.py`) is one of these,
  drawn at the item family's sizes because it sits beside an item icon; the Concealment eye will be
  another.

Study them before drawing with the `sheet` command on valve's `hud.txt` at 640 and 2560.

Drawing technique that worked for the Syringe: draw axis-aligned at 8× to 16× the largest target size,
shade cylinders per scanline with a lambert curve plus a specular band and a rim light, draw discs as
ellipses with a lit near rim and a dark far rim so they read as tilted, rotate the whole canvas once, crop
to content, then LANCZOS-downsample to each bucket with a 6% margin. Below about 24px add an unsharp mask
or the silhouette turns to mush. Make the part of the silhouette that names the object (the needle) heavier
than it would be in life; it is the first thing to disappear.

## Review

The icon script's `--preview` writes a sheet with, per bucket: the raw icon on black, the icon as the
Grid draws it (additive over the cell's tint fill), and the icon as a status icon (additive, tinted). The
last column can hold a stock reference at the same size. `additive_preview` in `sprtool.py` is the
simulation (lifted out of `icons/syringe.py` on 2026-09-18, when `from_png.py` became its second user).

The check is honest only if the review is: look at the smallest size first, on the tinted background,
and ask whether it is still the object. If the 20px version needs the 132px version beside it to be read,
it is not done.

## Hand-drawn icons

Settled 2026-09-18, in the grill on the art workflow (the rule for every hand-made asset is in
[CLAUDE.md](../CLAUDE.md#hand-made-art-the-rule)). An icon can be drawn instead of scripted, and the
pipeline takes it from the PNG on.

**The source.** One PNG per icon, drawn in paint.net, **greyscale on black**, at about sixteen times the
largest size it ships at — 512 pixels across for an item icon whose largest bucket is 132, wider than tall
for a weapon selection sprite (its frame is 510×135). It lives at `E:\CustomAssets\sprites\src\<name>.png`,
named as its `hud.txt` name (`item_shard.png`) or, for a weapon selection sprite, the weapon's classname
(`weapon_katana.png`). **That folder is never written by a script or an agent**: it is Andrei's, the way a
decompile folder is Crowbar's, and a re-run of anything cannot eat a drawing.

**Why greyscale.** Every tintable sprite in the mod is one family: the HUD icons, the Skill Tree's node
icons and the weapon selection sprites are all additive greyscale images that the code colours at draw
time — the HUD colour, the tree's state tint over its region wash, the HUD colour again for the weapon
list with a brighter `_s` when selected. The sprite carries brightness only and the colour is code's. So
draw in greyscale from the start (paint.net: Adjustments → Black and White before saving). The script
takes the luminance anyway and warns if the source carried colour, but a drawing made in colour and
desaturated after tends to lose the contrast that carried its edges.

**The rules of the drawing**, which are the engine's:

- **Draw the highlights, not the outlines.** Black is the transparency: over a lit background — the Grid's
  tinted cell, a bright wall — a dark outline vanishes, so the edge of a shape has to be its bright side.
- **No alpha.** Additive sprites have none. A transparent PNG is flattened on black with a warning.
- **Judge the 20-pixel version first**, on the preview sheet, before the big one. If it needs the big one
  beside it to be read, it is not done. Make the part of the silhouette that names the thing heavier than
  it would be in life.

**What the script derives** (`from_png.py`, in the tools table): the four buckets of the icon's family,
fitted with the 6% margin, LANCZOS down, an unsharp mask below 24 pixels; a `.spr` per bucket, additive;
the four `hud_additions.txt` lines, replaced in place if the name was already there; for a weapon, the
`weapon` and `weapon_s` lines of `sprites/weapon_<name>.txt` with the ammo and crosshair lines kept; the
selected variant, which is the same drawing with every pixel doubled and clipped — measured on valve's
own pairs, where the silhouettes are identical and the median per-pixel ratio is 1.94 to 2.12; and the
preview sheet, per bucket: raw, as the Grid draws it, as a status icon in the HUD colour, the selected
state beside it for a weapon, and for a Skill Tree icon its three state tints (white unlocked, gold
available, grey locked) on the hub's wash and then unlocked on every region's wash. No PNGs are left in
`sprites/top/` to delete.

**The Skill Tree's icons are a set problem.** One fixed size at every bucket (64, fitted by the tree),
and the constraint that matters is that every icon be distinguishable from every other at 20 pixels
(ART_DEBT, the node-icons entry). Review them together on one sheet, not one at a time; a skill icon's
files keep the whole name (`skill_backstab_320.spr`) so they never share a file with an item's.

**What stays scripted.** Code-drawn icons remain the path for what nobody wants to draw by hand, and
the Inventory Icons are not part of this at all — they are renders of the world models (below) and come
free with the modelling work.

## Adding an icon

1. Pick the family and therefore the sizes. Add the name to the `spriteName` column of the shared
   table (`inventory_defs.h`, `skill_defs.h`) or the constant that names it, in the same change.
2. Either write `utils/sprtool/icons/<name>.py`, modelled on `syringe.py`, and iterate on the preview
   sheet — or draw `E:\CustomAssets\sprites\src\<name>.png` (above) and run
   `python utils/sprtool/from_png.py E:\CustomAssets\sprites\src\<name>.png <name> --family <family> --preview <sheet>.png`,
   which does steps 3 and 4 itself (`--hud <valve>/sprites/hud.txt` for the regeneration; `--selected`
   for a weapon).
3. Scripted path: run the icon script with `OUT_DIR = sprites/top`, delete the PNGs it leaves there
   (only `.spr` is shipped).
4. Scripted path: add four lines to `sprites/hud_additions.txt`; regenerate `sprites/hud.txt`.
5. Copy `sprites/hud.txt` and `sprites/top/*.spr` to `topmod/sprites/` (and `sprites/weapon_<name>.txt`
   for a weapon). Rebuild both DLLs if a shared header changed; the build installs itself.
6. Verify in game before committing. Update the icon's entry in `ART_DEBT.md` in the same commit.

## Not yet covered

Models have their own pipeline now — [MODEL_WORKFLOW.md](MODEL_WORKFLOW.md) — and sounds have theirs since
2026-09-18, [SOUND_WORKFLOW.md](SOUND_WORKFLOW.md). Map textures are under
[MAP_WORKFLOW.md](MAP_WORKFLOW.md#the-mods-wad), since a WAD is a map input and not a sprite.

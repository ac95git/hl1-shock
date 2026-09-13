# 2D Sprite Workflow

How the mod's HUD sprites are designed, built, reviewed and shipped. Set up 2026-09-12 with the Health
Syringe icon as the first piece through it; that icon's script is the worked example for everything below.

The loop is: **a Python script draws the icon → PNG → the agent looks at a preview sheet and iterates →
`.spr` → `hud.txt` → copied to the install → checked in game.** The art is code. An icon is changed by
editing its script and re-running it, never by editing the `.spr`.

## Facts that bind the art

These are engine and SDK constraints, not preferences. Every one of them bit at least once.

- **`.spr` is 8-bit paletted.** 256 colours, one palette per file. For HUD work the source is greyscale
  and the palette is whatever the quantiser makes of it.
- **HUD sprites are drawn additively and tinted by code.** `SPR_Set(r,g,b)` then `SPR_DrawAdditive`.
  The sprite supplies brightness; the code supplies colour. A coloured source comes out wrong, and
  **dark pixels vanish** — over a lit background (the Grid's tinted cell, a bright wall) black is
  invisible. Edges must be carried by highlights, not outlines.
- **`SPR_DrawAdditive` draws at native size; `SPR_DrawGeneric` is the one scaled draw.** It takes a
  width, height and a GL blend pair, and the Inventory Grid fits its tile art through it (`DrawTileSprite`,
  `cl_dll/vgui_inventory_grid.cpp`). Two traps: the width and height are the size for the **whole sprite
  frame**, and the rect is cut out at that scale — a 340×90 icon on a 512×128 sheet asked for at 340×90
  comes out at two thirds, so scale the request by frame-over-rect. And the blend factors are raw GL enums
  (`SPR_BLEND_*` in `cl_util.h`). Everything else on the HUD still draws native, so a sprite is still made
  at the size it will be seen, per resolution bucket, unless its caller fits it.
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
  outline, bright edges over dark mass. The Syringe and the keycard (`icons/security.py`) are these.
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
last column can hold a stock reference at the same size. `additive_preview` in `icons/syringe.py` is the
simulation; lift it into `sprtool.py` when the second icon needs it.

The check is honest only if the review is: look at the smallest size first, on the tinted background,
and ask whether it is still the object. If the 20px version needs the 132px version beside it to be read,
it is not done.

## Adding an icon

1. Pick the family and therefore the sizes. Add the name to the `spriteName` column of the shared
   table (`inventory_defs.h`, `skill_defs.h`) or the constant that names it, in the same change.
2. Write `utils/sprtool/icons/<name>.py`, modelled on `syringe.py`. Iterate on the preview sheet.
3. Run it with `OUT_DIR = sprites/top`, delete the PNGs it leaves there (only `.spr` is shipped).
4. Add four lines to `sprites/hud_additions.txt`; regenerate `sprites/hud.txt`.
5. Copy `sprites/hud.txt` and `sprites/top/*.spr` to `topmod/sprites/`. Rebuild both DLLs if a shared
   header changed; the build installs itself.
6. Verify in game before committing. Update the icon's entry in `ART_DEBT.md` in the same commit.

## Not yet covered

Models have their own pipeline now — [MODEL_WORKFLOW.md](MODEL_WORKFLOW.md). Sounds have nothing.

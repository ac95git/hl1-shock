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
- **There is no scaled draw.** `SPR_DrawAdditive` draws at native size. A sprite is made at the size it
  will be seen, per resolution bucket.
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
  top/*.spr            our icons, one file per icon per bucket        (generated)
utils/sprtool/
  sprtool.py           decode / encode / sheet
  make_hud_txt.py      hud.txt generator
  icons/*.py           the icons' sources
```

`sprites/` in the repo is the source of truth. The install copy is `topmod/sprites/` in the Half-Life
directory, and **nothing in the build copies it** — copy `hud.txt` and `top/*.spr` by hand after every
change, in the same sitting, the same discipline as the FGD (`docs/MAP_BRIEF.md`).

## Style

Two families in the stock HUD, and a new icon should join one of them:

- **Item icons** (`item_*`): shaded greyscale objects in three-quarter view, visible thickness, no
  outline, bright edges over dark mass. The Syringe is one of these.
- **Suit readouts** (`suit_full`, `flash_full`, `cross`): flat glyph silhouettes with a soft one-to-two
  pixel glow, drawn as light. The Concealment eye will be one of these.

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

World models, weapon models and sounds have no equivalent pipeline yet. The plan for models is Crowbar
to decompile, text surgery on SMD/QC in the repo, and Blender driven headless for new geometry, with the
compile done by hand; nothing of that is built or proven. Sounds have nothing.

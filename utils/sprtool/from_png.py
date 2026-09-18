"""A HUD sprite from a hand-drawn PNG -- the hand-drawn path of docs/SPRITE_WORKFLOW.md.

The source is one greyscale-on-black PNG per icon, drawn at roughly sixteen times the largest size it
ships at, under E:\\CustomAssets\\sprites\\src\\<name>.png. That file is never written by this script.
Everything else is derived from it here: the four resolution buckets, the sharpening the small ones
need, the additive .spr per bucket, the hud.txt lines, the selected variant of a weapon icon, and a
preview sheet that shows the icon in every tint the code draws it in.

    python utils/sprtool/from_png.py IN.png NAME --family item|readout|small|skill|weapon
        [--out sprites/top] [--additions sprites/hud_additions.txt] [--weapons-dir sprites]
        [--preview SHEET.png] [--selected] [--hud VALVE_HUD.txt]

NAME is the hud.txt name (item_shard, inv_carried, skill_backstab) or, for the weapon family, the
weapon's classname (weapon_katana). The sprite files take the name after its first underscore, the way
the code-drawn icons do: item_shard -> top/shard_320.spr ... top/shard_2560.spr. A skill icon keeps its
whole name (skill_backstab -> top/skill_backstab_320.spr) so it cannot collide with an item's.

Families and their sizes per bucket (320/640/1280/2560), from the stock hud.txt:
    item     20 / 44 / 88 / 132       item_* pickups, and what sits beside them
    readout  20 / 40 / 80 / 120       suit readouts (suit_full)
    small    16 / 32 / 64 / 96        the small readouts (cross, flash_full)
    skill    64 at every bucket       Skill Tree node icons: one fixed size, fitted by the tree
    weapon   80x20 / 170x45 / 340x90 / 510x135   weapon selection, written to sprites/weapon_<name>.txt

--selected (weapon family only) derives the "_s" sprite the selection list draws for the current weapon.
Measured on valve's 1280 pairs (crowbar, 9mmhandgun, shotgun, 357): the selected image is the same
drawing with every pixel doubled and clipped -- median per-pixel ratio 1.94..2.12, identical silhouettes.

--hud VALVE_HUD.txt regenerates sprites/hud.txt afterwards through make_hud_txt.py (the same thing as
running it by hand; the additions file is the input either way). Without it, run make_hud_txt.py yourself.

Only .spr files are written to --out; the PNGs live on the preview sheet, so there is nothing to delete.
"""

import argparse
import math
import os
import re
import subprocess
import sys

from PIL import Image, ImageFilter

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import sprtool  # noqa: E402

RESOLUTIONS = (320, 640, 1280, 2560)

# (width, height) per bucket.
FAMILIES = {
    "item":    {320: (20, 20),  640: (44, 44),   1280: (88, 88),   2560: (132, 132)},
    "readout": {320: (20, 20),  640: (40, 40),   1280: (80, 80),   2560: (120, 120)},
    "small":   {320: (16, 16),  640: (32, 32),   1280: (64, 64),   2560: (96, 96)},
    "skill":   {320: (64, 64),  640: (64, 64),   1280: (64, 64),   2560: (64, 64)},
    "weapon":  {320: (80, 20),  640: (170, 45),  1280: (340, 90),  2560: (510, 135)},
}

MARGIN = 0.06            # share of the frame left clear on each side, as the code-drawn icons do
SHARPEN_BELOW = 24       # px: the unsharp mask the Syringe needed under this size
COLOUR_SPREAD_WARN = 24  # max(r,g,b)-min(r,g,b) above this means the source was not greyscale
SELECTED_GAIN = 2.0      # valve's _s: pixels doubled, clipped

HUD_COLOUR = (255, 160, 0)          # RGB_YELLOWISH, what status icons and the weapon list draw in
GRID_CELL_BG = (10 + 180 * 100 // 255,) * 3   # the Grid's tinted cell, from icons/syringe.py
GRID_ICON_TINT = (180, 180, 180)
SCENE_BG = (35, 35, 35)

# Skill Tree: the icon is tinted by state and sits on a region wash over the substrate
# (cl_dll/vgui_skilltree.cpp k_Regions, the wash alpha 26 of 255; the hub 18).
SKILL_STATE_TINTS = {"unlocked": (255, 255, 255), "available": (255, 200, 60), "locked": (100, 80, 80)}
SKILL_SUBSTRATE = (28, 20, 12)
SKILL_REGIONS = [
    ("Shinobi", (150, 80, 220), 26), ("Medical", (210, 235, 210), 26), ("Stealth", (70, 55, 150), 26),
    ("Melee", (183, 65, 14), 26), ("Hub", (184, 115, 51), 18), ("Specialist", (95, 115, 135), 26),
    ("Energy", (40, 210, 225), 26), ("Juggernaut", (255, 180, 40), 26), ("Alien", (120, 220, 70), 26),
]


def sprite_base(name, family):
    """item_shard -> shard; weapon_katana -> katana_weapon (the selection list's own sprite).

    A Skill Tree icon keeps its whole name (skill_backstab -> skill_backstab_320), so it can never share
    a file with an item icon of the same word.
    """
    if family == "skill":
        return name
    base = name.split("_", 1)[1] if "_" in name else name
    return base + "_weapon" if family == "weapon" else base


def load_greyscale(path):
    """The PNG as L, flattened on black, with a warning if it carried colour."""
    img = Image.open(path).convert("RGBA")
    flat = Image.new("RGB", img.size, (0, 0, 0))
    flat.paste(img.convert("RGB"), mask=img.getchannel("A"))
    bands = flat.split()
    spread = 0
    for r, g, b in zip(*(band.tobytes() for band in bands)):
        s = max(r, g, b) - min(r, g, b)
        if s > spread:
            spread = s
    if spread > COLOUR_SPREAD_WARN:
        print(f"warning: {path} carried colour (channel spread {spread}); luminance used. "
              f"Draw greyscale on black -- the code supplies the tint.", file=sys.stderr)
    if img.getchannel("A").getextrema()[0] < 255:
        print(f"warning: {path} has transparency; flattened on black (additive sprites have no alpha).",
              file=sys.stderr)
    return flat.convert("L")


def fit(icon, size):
    """Crop to content and fit into a (w, h) frame with the margin, keeping the aspect; sharpen if small."""
    w, h = size
    bbox = icon.getbbox()
    if bbox is None:
        sys.exit("the source is entirely black")
    src = icon.crop(bbox)
    inner_w, inner_h = w * (1 - 2 * MARGIN), h * (1 - 2 * MARGIN)
    scale = min(inner_w / src.size[0], inner_h / src.size[1])
    tw = max(1, round(src.size[0] * scale))
    th = max(1, round(src.size[1] * scale))
    small = src.resize((tw, th), Image.LANCZOS)
    if min(w, h) <= SHARPEN_BELOW:
        small = small.filter(ImageFilter.UnsharpMask(radius=1, percent=120, threshold=0))
    out = Image.new("L", (w, h), 0)
    out.paste(small, ((w - tw) // 2, (h - th) // 2))
    return out


def selected_variant(icon):
    return icon.point(lambda v: min(255, int(v * SELECTED_GAIN)))


def write_additive(icon, path):
    rgba = Image.merge("RGBA", (icon, icon, icon, icon))
    sprtool.write_spr(path, sprtool.image_to_sprite(rgba, sprtool.SPR_ADDITIVE))


# ---- hud_additions.txt and weapon_<name>.txt -------------------------------------------------------

def hud_line(name, res, file_, w, h):
    return f"{name:<17} {res:<5} {file_:<19} 0  0  {w:<4} {h}"


def replace_lines(path, names, new_lines, header=None):
    """Replace every entry line for a name in `names` with new_lines (appended if none was there)."""
    old = []
    if os.path.exists(path):
        with open(path) as f:
            old = [l.rstrip("\n") for l in f]
    kept = []
    for l in old:
        parts = l.split("//")[0].split()
        if len(parts) == 7 and parts[0] in names:
            continue
        kept.append(l)
    if not kept and header:
        kept = list(header)
    while kept and kept[-1] == "":
        kept.pop()
    with open(path, "w", newline="\n") as f:
        f.write("\n".join(kept + new_lines) + "\n")


def write_weapon_txt(path, base, out_rel, sizes, selected):
    """sprites/weapon_<name>.txt: the weapon and weapon_s lines replaced, ammo/crosshair lines kept, count fixed."""
    old = []
    if os.path.exists(path):
        with open(path) as f:
            old = [l.rstrip("\n") for l in f]
    kept = []
    for l in old[1:] if old else []:
        parts = l.split()
        if len(parts) == 7 and parts[0] in ("weapon", "weapon_s"):
            continue
        if l.strip():
            kept.append(l)
    new = []
    for res in sorted(sizes, reverse=True):
        w, h = sizes[res]
        new.append(f"weapon\t\t\t{res} {out_rel}/{base}_{res}\t0\t0\t{w}\t{h}")
        if selected:
            new.append(f"weapon_s\t\t{res} {out_rel}/{base}_s_{res}\t0\t0\t{w}\t{h}")
    lines = new + kept
    count = sum(1 for l in lines if len(l.split()) == 7)
    with open(path, "w", newline="\n") as f:
        f.write(f"{count}\n" + "\n".join(lines) + "\n")


# ---- the preview sheet ---------------------------------------------------------------------------

def wash_bg(colour, alpha):
    return tuple(int(SKILL_SUBSTRATE[i] + (colour[i] - SKILL_SUBSTRATE[i]) * alpha / 255) for i in range(3))


def preview_sheet(icons, family, selected, path):
    """Per bucket: raw on black, as the Grid draws it, as a status icon in the HUD colour; a weapon's
    selected state; a Skill Tree icon in its three state tints on the hub and unlocked on every wash."""
    rows, labels = [], []
    for res in RESOLUTIONS:
        icon = icons[res]
        w, h = icon.size
        scale = max(1, math.ceil(132 / max(w, h)))
        row = [
            sprtool.additive_preview(icon, (0, 0, 0), (255, 255, 255), scale),
            sprtool.additive_preview(icon, GRID_CELL_BG, GRID_ICON_TINT, scale),
            sprtool.additive_preview(icon, SCENE_BG, HUD_COLOUR, scale),
        ]
        if selected is not None:
            row.append(sprtool.additive_preview(selected[res], SCENE_BG, HUD_COLOUR, scale))
        if family == "skill":
            hub = next(r for r in SKILL_REGIONS if r[0] == "Hub")
            for tint in SKILL_STATE_TINTS.values():
                row.append(sprtool.additive_preview(icon, wash_bg(hub[1], hub[2]), tint, scale))
            for _name, colour, alpha in SKILL_REGIONS:
                row.append(sprtool.additive_preview(icon, wash_bg(colour, alpha), SKILL_STATE_TINTS["unlocked"], scale))
        rows.append(row)
        labels.append(f"{res}: {w}x{h}")
    cw = max(im.size[0] for r in rows for im in r) + 8
    ch = max(im.size[1] for r in rows for im in r) + 8
    cols = max(len(r) for r in rows)
    sheet = Image.new("RGB", (cw * cols + 90, ch * len(rows)), (24, 24, 24))
    from PIL import ImageDraw
    draw = ImageDraw.Draw(sheet)
    for ri, row in enumerate(rows):
        draw.text((4, ri * ch + 4), labels[ri], fill=(200, 200, 200))
        for ci, im in enumerate(row):
            sheet.paste(im, (90 + ci * cw + 4, ri * ch + 4))
    sheet.save(path)
    print("preview", path)


# ---- main ----------------------------------------------------------------------------------------

def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("src", help="the hand-drawn PNG (greyscale on black)")
    ap.add_argument("name", help="hud.txt name, or the weapon classname for --family weapon")
    ap.add_argument("--family", choices=FAMILIES, required=True)
    ap.add_argument("--out", default=os.path.join("sprites", "top"), help="where the .spr files go")
    ap.add_argument("--additions", default=os.path.join("sprites", "hud_additions.txt"))
    ap.add_argument("--weapons-dir", default="sprites", help="where weapon_<name>.txt lives (weapon family)")
    ap.add_argument("--preview", help="write the review sheet here")
    ap.add_argument("--selected", action="store_true", help="weapon family: also derive the _s sprite")
    ap.add_argument("--hud", help="valve's hud.txt: regenerate sprites/hud.txt afterwards")
    a = ap.parse_args()

    if a.selected and a.family != "weapon":
        sys.exit("--selected is for the weapon family only")
    if a.family == "weapon" and not a.name.startswith("weapon_"):
        sys.exit("a weapon family NAME is the weapon's classname, weapon_<name>")

    sizes = FAMILIES[a.family]
    base = sprite_base(a.name, a.family)
    out_rel = os.path.basename(os.path.normpath(a.out))  # "top": hud.txt paths are relative to sprites/
    os.makedirs(a.out, exist_ok=True)

    source = load_greyscale(a.src)
    icons, selected = {}, ({} if a.selected else None)
    for res in RESOLUTIONS:
        icon = fit(source, sizes[res])
        icons[res] = icon
        write_additive(icon, os.path.join(a.out, f"{base}_{res}.spr"))
        if a.selected:
            selected[res] = selected_variant(icon)
            write_additive(selected[res], os.path.join(a.out, f"{base}_s_{res}.spr"))
    print(f"wrote {base}_{{{','.join(map(str, RESOLUTIONS))}}}.spr" + (" and _s" if a.selected else "") + f" to {a.out}")

    if a.family == "weapon":
        txt = os.path.join(a.weapons_dir, f"{a.name}.txt")
        write_weapon_txt(txt, base, out_rel, sizes, a.selected)
        print(f"{txt}: weapon{' and weapon_s' if a.selected else ''} lines set; other lines kept")
    else:
        lines = [hud_line(a.name, res, f"{out_rel}/{base}_{res}", *sizes[res]) for res in RESOLUTIONS]
        replace_lines(a.additions, {a.name}, lines)
        print(f"{a.additions}: {a.name} lines set")
        if a.hud:
            hud_out = os.path.join(os.path.dirname(a.additions), "hud.txt")
            subprocess.check_call([sys.executable, os.path.join(HERE, "make_hud_txt.py"), a.hud, a.additions, hud_out])

    if a.preview:
        preview_sheet(icons, a.family, selected, a.preview)


if __name__ == "__main__":
    main()

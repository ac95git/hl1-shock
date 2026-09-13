"""Carried arrow -- the inv_carried HUD sprite at the four resolution buckets.

Drawn beside an item's icon on the pickup history when the pickup went into the Inventory
instead of being used on the spot (cl_dll/ammohistory.cpp). A flat glyph with a soft glow,
in the style of the suit readouts: a down arrow going into an open tray, sitting at the right
of the item family's square, centred vertically. The history draws the square immediately left
of the icon, so the glyph lands beside the icon at its mid-height and the square's empty left
side is the gap.

    python utils/sprtool/icons/carried.py OUT_DIR [--preview PREVIEW.png] [--item ITEM_PNG]

Writes OUT_DIR/carried_{320,640,1280,2560}.png and .spr (additive).
"""

import argparse
import math
import os
import subprocess
import sys

from PIL import Image, ImageDraw, ImageFilter, ImageChops

HERE = os.path.dirname(os.path.abspath(__file__))
SPRTOOL = os.path.join(HERE, "..", "sprtool.py")

# The item_* family's sizes (item_healthkit is 20/44/88/132); the overlay is drawn at the
# same size as the icon under it, corners aligned.
SIZES = {320: 20, 640: 44, 1280: 88, 2560: 132}

# Fraction of the square the glyph occupies, anchored bottom-right. 0.58 was
# the first cut and it walked over the item; a corner badge, not a stamp.
GLYPH = 0.40


def draw_glyph(canvas, box):
    """Down arrow into a tray, inside the box (x0, y0, x1, y1)."""
    d = ImageDraw.Draw(canvas)
    x0, y0, x1, y1 = box
    S = x1 - x0
    cx = x0 + S / 2
    lw = max(1, int(S * 0.15))

    # tray: an open-topped U across the bottom
    ty = y0 + S * 0.56
    d.line((x0 + lw / 2, ty, x0 + lw / 2, y1 - lw / 2), fill=255, width=lw)
    d.line((x1 - lw / 2, ty, x1 - lw / 2, y1 - lw / 2), fill=255, width=lw)
    d.line((x0, y1 - lw / 2, x1, y1 - lw / 2), fill=255, width=lw)

    # arrow: shaft from the top, head ending inside the tray
    sw = max(1, int(S * 0.17))
    head_top = y0 + S * 0.42
    tip = y0 + S * 0.80
    d.line((cx, y0, cx, head_top + sw), fill=255, width=sw)
    hw = S * 0.30
    d.polygon([(cx - hw, head_top), (cx + hw, head_top), (cx, tip)], fill=255)


def render(size, ss=8):
    W = size * ss
    canvas = Image.new("L", (W, W), 0)
    # Anchored right, centred vertically: the history lines the square up with
    # the icon, so the glyph lands at the icon's mid-height.
    margin = W * 0.04
    g = W * GLYPH
    top = (W - g) / 2
    box = (W - margin - g, top, W - margin, top + g)
    draw_glyph(canvas, box)

    # soft glow under the glyph, then the glyph itself on top
    glow = canvas.filter(ImageFilter.GaussianBlur(radius=W * 0.035)).point(lambda v: v * 0.55)
    out = ImageChops.add(glow, canvas)

    small = out.resize((size, size), Image.LANCZOS)
    if size <= 24:
        small = small.filter(ImageFilter.UnsharpMask(radius=1, percent=100, threshold=0))
    return small


def additive_preview(icon, bg, tint, scale):
    w, h = icon.size
    base = Image.new("RGB", (w, h), bg)
    tinted = Image.merge("RGB", [icon.point(lambda v, c=c: v * c // 255) for c in tint])
    out = ImageChops.add(base, tinted)
    return out.resize((w * scale, h * scale), Image.NEAREST)


def main():
    global GLYPH
    ap = argparse.ArgumentParser()
    ap.add_argument("out_dir")
    ap.add_argument("--preview")
    ap.add_argument("--item", help="greyscale PNG of an item icon at 2560 (132px) to composite under the arrow")
    ap.add_argument("--glyph", type=float, default=None, help="fraction of the square the arrow occupies")
    a = ap.parse_args()
    if a.glyph is not None:
        GLYPH = a.glyph
    os.makedirs(a.out_dir, exist_ok=True)

    icons = {}
    for res, size in SIZES.items():
        icon = render(size)
        icons[res] = icon
        png = os.path.join(a.out_dir, f"carried_{res}.png")
        Image.merge("RGBA", (icon, icon, icon, icon)).save(png)
        subprocess.check_call([sys.executable, SPRTOOL, "encode", png, os.path.join(a.out_dir, f"carried_{res}.spr"), "--format", "additive"])

    if a.preview:
        suit = (60, 220, 255)
        item = Image.open(a.item).convert("L") if a.item else None
        rows = []
        for res, size in SIZES.items():
            scale = max(1, math.ceil(132 / size))
            row = [
                additive_preview(icons[res], (0, 0, 0), (255, 255, 255), scale),
                additive_preview(icons[res], (35, 35, 35), suit, scale),
            ]
            if item is not None:
                it = item.resize((size, size), Image.LANCZOS)
                both = ImageChops.add(it, icons[res])
                row.append(additive_preview(it, (35, 35, 35), suit, scale))
                row.append(additive_preview(both, (35, 35, 35), suit, scale))
            rows.append(row)
        cw = max(im.size[0] for r in rows for im in r) + 8
        ch = max(im.size[1] for r in rows for im in r) + 8
        sheet = Image.new("RGB", (cw * max(len(r) for r in rows), ch * len(rows)), (24, 24, 24))
        for ri, row in enumerate(rows):
            for ci, im in enumerate(row):
                sheet.paste(im, (ci * cw + 4, ri * ch + 4))
        sheet.save(a.preview)
        print("preview", a.preview)


if __name__ == "__main__":
    main()

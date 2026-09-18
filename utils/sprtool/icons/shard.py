"""Crystal Shard icon -- the item_shard HUD sprite at the four resolution buckets.

Drawn in the style of valve's item_* icons and the Syringe: a shaded greyscale object in three-quarter
view, no outline, edges carried by highlights because the Grid draws it additively over a tinted cell.
A single hexagonal crystal point, three faces visible, with a second smaller point at its foot so the
silhouette says "crystal" rather than "pencil" at 20px.

    python utils/sprtool/icons/shard.py OUT_DIR [--preview PREVIEW.png]

Writes OUT_DIR/shard_{320,640,1280,2560}.png and .spr (additive).
"""

import argparse
import math
import os
import subprocess
import sys

from PIL import Image, ImageDraw, ImageFilter

HERE = os.path.dirname(os.path.abspath(__file__))
SPRTOOL = os.path.join(HERE, "..", "sprtool.py")
sys.path.insert(0, HERE)
from syringe import additive_preview  # noqa: E402  (the shared review simulation)

SIZES = {320: 20, 640: 44, 1280: 88, 2560: 132}


def crystal(d, cx, base_y, height, width, tip, shade):
    """One hexagonal crystal point standing on base_y, seen three-quarter: three faces and three tip facets.

    shade is (left, middle, right) face greys; the tip facets are brighter than their faces, and every
    face edge gets a highlight line, which is what carries the form once drawn additively.
    """
    hw = width / 2
    # The three visible vertical faces, left to right, as x positions of their edges.
    xs = [cx - hw, cx - hw * 0.35, cx + hw * 0.45, cx + hw]
    top = base_y - height
    apex = (cx + hw * 0.05, top - tip)
    faces = []
    for i in range(3):
        x0, x1 = xs[i], xs[i + 1]
        # The base of each face bows slightly so the prism reads as round-ish, not flat.
        faces.append([(x0, top), (x1, top), (x1, base_y - (1 - i) * width * 0.04), (x0, base_y - (1 - i) * width * 0.04)])
    for poly, g in zip(faces, shade):
        d.polygon(poly, fill=g)
    # Tip facets: each face's top edge rises to the apex.
    for i, g in enumerate(shade):
        d.polygon([(xs[i], top), (xs[i + 1], top), apex], fill=min(255, int(g * 1.35 + 20)))
    # Edge highlights.
    ew = max(1, int(width * 0.05))
    for x in xs[1:3]:
        d.line((x, top, x, base_y), fill=255, width=ew)
        d.line((x, top, apex[0], apex[1]), fill=255, width=ew)
    d.line((xs[0], top, apex[0], apex[1]), fill=200, width=ew)
    d.line((xs[3], top, apex[0], apex[1]), fill=230, width=ew)


def draw_shard(canvas):
    W = canvas.size[0]
    d = ImageDraw.Draw(canvas)
    base = W * 0.78
    # The small point first, behind and to the right, leaning out.
    small = Image.new("L", canvas.size, 0)
    crystal(ImageDraw.Draw(small), W * 0.5, base, W * 0.22, W * 0.14, W * 0.09, (70, 120, 170))
    small = small.rotate(-24, resample=Image.BICUBIC, center=(W * 0.5, base))
    canvas.paste(small, (int(W * 0.035), 0), small)
    # The main point.
    crystal(d, W * 0.47, base, W * 0.38, W * 0.20, W * 0.14, (90, 150, 215))


def render(size, angle=-18.0, ss=8):
    W = size * ss * 2
    canvas = Image.new("L", (W, W), 0)
    draw_shard(canvas)
    rot = canvas.rotate(angle, resample=Image.BICUBIC, expand=False)
    rot = rot.crop(rot.getbbox())
    margin = 0.06
    scale = (size * (1 - 2 * margin)) / max(rot.size)
    small = rot.resize((max(1, round(rot.size[0] * scale)), max(1, round(rot.size[1] * scale))), Image.LANCZOS)
    if size <= 24:
        small = small.filter(ImageFilter.UnsharpMask(radius=1, percent=120, threshold=0))
    out = Image.new("L", (size, size), 0)
    out.paste(small, ((size - small.size[0]) // 2, (size - small.size[1]) // 2))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out_dir")
    ap.add_argument("--preview")
    a = ap.parse_args()
    os.makedirs(a.out_dir, exist_ok=True)

    icons = {}
    for res, size in SIZES.items():
        icon = render(size)
        icons[res] = icon
        png = os.path.join(a.out_dir, f"shard_{res}.png")
        Image.merge("RGBA", (icon, icon, icon, icon)).save(png)
        subprocess.check_call([sys.executable, SPRTOOL, "encode", png, os.path.join(a.out_dir, f"shard_{res}.spr"), "--format", "additive"])

    if a.preview:
        cell_bg = (10 + 180 * 100 // 255,) * 3
        rows = []
        for res, size in SIZES.items():
            scale = max(1, math.ceil(132 / size))
            rows.append([
                additive_preview(icons[res], (0, 0, 0), (255, 255, 255), scale),
                additive_preview(icons[res], cell_bg, (180, 180, 180), scale),
                additive_preview(icons[res], (35, 35, 35), (0, 255, 0), scale),
            ])
        cw = max(im.size[0] for r in rows for im in r) + 8
        ch = max(im.size[1] for r in rows for im in r) + 8
        sheet = Image.new("RGB", (cw * 3, ch * len(rows)), (24, 24, 24))
        for ri, row in enumerate(rows):
            for ci, im in enumerate(row):
                sheet.paste(im, (ci * cw + 4, ri * ch + 4))
        sheet.save(a.preview)
        print("preview", a.preview)


if __name__ == "__main__":
    main()

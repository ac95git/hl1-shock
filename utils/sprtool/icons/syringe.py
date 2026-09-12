"""Health Syringe icon -- the item_syringe HUD sprite at the four resolution buckets.

Drawn in the style of valve's item_* icons: a shaded greyscale object in three-quarter view,
no outline, edges carried by highlights because the Grid draws it additively over a tinted cell.

    python utils/sprtool/icons/syringe.py OUT_DIR [--preview PREVIEW.png]

Writes OUT_DIR/syringe_{320,640,1280,2560}.png and .spr (additive).
"""

import argparse
import math
import os
import subprocess
import sys

from PIL import Image, ImageDraw, ImageFilter, ImageChops

HERE = os.path.dirname(os.path.abspath(__file__))
SPRTOOL = os.path.join(HERE, "..", "sprtool.py")

# Size of the item_* family at each bucket (item_healthkit is 20/44/88/132).
SIZES = {320: 20, 640: 44, 1280: 88, 2560: 132}


def shade_cylinder(draw, x0, x1, y0, y1, base, spec=1.0, rim=0.35):
    """Horizontal cylinder lit from above: bright band near the top, dark belly, faint rim at the bottom."""
    h = y1 - y0
    for row in range(int(h)):
        t = row / max(1, h - 1)                     # 0 top .. 1 bottom
        lambert = math.cos((t - 0.28) * math.pi) * 0.5 + 0.5
        v = base * (0.18 + 0.82 * lambert)
        # specular streak
        v += base * spec * 0.55 * math.exp(-((t - 0.22) / 0.07) ** 2)
        # rim light along the bottom edge
        v += base * rim * math.exp(-((t - 0.95) / 0.05) ** 2)
        g = int(max(0, min(255, v)))
        draw.line((x0, y0 + row, x1, y0 + row), fill=g)


def draw_syringe(canvas):
    """Draw a horizontal syringe, needle to the left, into a square 'L' canvas."""
    W = canvas.size[0]
    d = ImageDraw.Draw(canvas)
    L = W * 0.94
    ox = (W - L) / 2
    cy = W / 2

    def X(f):
        return ox + f * L

    # --- needle ---  heavy enough to survive 20px; it is the half of the silhouette that says "syringe"
    nw = W * 0.032
    d.line((X(0.0), cy, X(0.30), cy), fill=190, width=int(nw))
    d.line((X(0.0), cy - nw * 0.25, X(0.30), cy - nw * 0.25), fill=255, width=max(1, int(nw * 0.4)))
    # bevel tip
    d.polygon([(X(0.0), cy), (X(0.045), cy - nw * 0.6), (X(0.045), cy + nw * 0.6)], fill=255)

    # --- hub (luer cone) ---
    hub_h = W * 0.09
    d.polygon([(X(0.29), cy - nw), (X(0.36), cy - hub_h / 2), (X(0.36), cy + hub_h / 2), (X(0.29), cy + nw)], fill=140)
    shade_cylinder(d, X(0.30), X(0.36), cy - hub_h / 2, cy + hub_h / 2, base=170, spec=0.6)
    # keep the cone taper: cut the corners back with black
    d.polygon([(X(0.29), cy - hub_h), (X(0.29), cy - nw), (X(0.36), cy - hub_h / 2), (X(0.36), cy - hub_h)], fill=0)
    d.polygon([(X(0.29), cy + hub_h), (X(0.29), cy + nw), (X(0.36), cy + hub_h / 2), (X(0.36), cy + hub_h)], fill=0)

    # --- barrel (glass) ---  dark glass so the liquid level reads against it
    bh = W * 0.21
    bx0, bx1 = X(0.36), X(0.80)
    by0, by1 = cy - bh / 2, cy + bh / 2
    shade_cylinder(d, bx0, bx1, by0, by1, base=105, spec=1.2, rim=0.6)

    # liquid inside, from the needle end up to the plunger seal
    fill_to = 0.64
    lh = bh * 0.78
    shade_cylinder(d, X(0.37), X(fill_to), cy - lh / 2, cy + lh / 2, base=225, spec=0.5, rim=0.25)
    # meniscus edge
    d.line((X(fill_to), cy - lh / 2, X(fill_to), cy + lh / 2), fill=90, width=max(1, int(W * 0.012)))

    # plunger seal (dark rubber) just behind the liquid
    d.rectangle((X(fill_to), by0 + bh * 0.08, X(fill_to + 0.035), by1 - bh * 0.08), fill=40)
    d.line((X(fill_to + 0.035), by0 + bh * 0.08, X(fill_to + 0.035), by1 - bh * 0.08), fill=120, width=max(1, int(W * 0.006)))

    # graduation ticks along the top of the barrel
    for i in range(7):
        tx = X(0.40 + i * 0.055)
        d.line((tx, by0 + bh * 0.16, tx, by0 + bh * (0.30 if i % 2 else 0.40)), fill=255, width=max(1, int(W * 0.008)))

    # glass highlight strip re-applied on top of contents
    d.line((bx0, by0 + bh * 0.22, bx1, by0 + bh * 0.22), fill=255, width=max(1, int(W * 0.012)))

    # --- flange at the back of the barrel: a disc seen at an angle, so it has thickness ---
    fw = W * 0.06
    fh = W * 0.36
    disc(d, X(0.80), cy, fw, fh)

    # --- plunger rod and thumb rest ---
    rh = W * 0.075
    shade_cylinder(d, X(0.80) + fw * 0.6, X(0.955), cy - rh / 2, cy + rh / 2, base=160, spec=0.8, rim=0.3)
    th = W * 0.24
    tw = W * 0.055
    disc(d, X(0.955), cy, tw, th)


def disc(d, x, cy, w, h):
    """A flat disc whose face is turned toward the viewer: lit face, bright near rim, dark far rim."""
    rx, ry = w / 2, h / 2
    cx = x + rx
    for row in range(int(h)):
        t = row / max(1, h - 1)
        half = rx * math.sqrt(max(0.0, 1 - (2 * t - 1) ** 2))
        v = 120 + 70 * (1 - abs(2 * t - 1))
        d.line((cx - half, cy - ry + row, cx + half, cy - ry + row), fill=int(v))
    d.ellipse((cx - rx, cy - ry, cx + rx, cy + ry), outline=245, width=max(1, int(w * 0.18)))
    # far rim darker so the ellipse reads as a tilted disc, not a ring
    d.arc((cx - rx, cy - ry, cx + rx, cy + ry), start=100, end=260, fill=70, width=max(1, int(w * 0.18)))


def render(size, angle=34.0, ss=8):
    W = size * ss * 2  # room for the rotation
    canvas = Image.new("L", (W, W), 0)
    draw_syringe(canvas)
    rot = canvas.rotate(angle, resample=Image.BICUBIC, expand=False)
    bbox = rot.getbbox()
    rot = rot.crop(bbox)
    # fit the longest side to the icon with a small margin
    margin = 0.06
    scale = (size * (1 - 2 * margin)) / max(rot.size)
    small = rot.resize((max(1, round(rot.size[0] * scale)), max(1, round(rot.size[1] * scale))), Image.LANCZOS)
    if size <= 24:
        small = small.filter(ImageFilter.UnsharpMask(radius=1, percent=120, threshold=0))
    out = Image.new("L", (size, size), 0)
    out.paste(small, ((size - small.size[0]) // 2, (size - small.size[1]) // 2))
    return out


def additive_preview(icon, bg, tint, scale):
    """Simulate SPR_DrawAdditive of a greyscale icon with a tint over a background colour."""
    w, h = icon.size
    base = Image.new("RGB", (w, h), bg)
    tinted = Image.merge("RGB", [icon.point(lambda v, c=c: v * c // 255) for c in tint])
    out = ImageChops.add(base, tinted)
    return out.resize((w * scale, h * scale), Image.NEAREST)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out_dir")
    ap.add_argument("--preview")
    ap.add_argument("--angle", type=float, default=34.0)
    ap.add_argument("--reference", help="PNG of item_healthkit at 2560 for side-by-side")
    a = ap.parse_args()
    os.makedirs(a.out_dir, exist_ok=True)

    icons = {}
    for res, size in SIZES.items():
        icon = render(size, a.angle)
        icons[res] = icon
        png = os.path.join(a.out_dir, f"syringe_{res}.png")
        Image.merge("RGBA", (icon, icon, icon, icon)).save(png)
        subprocess.check_call([sys.executable, SPRTOOL, "encode", png, os.path.join(a.out_dir, f"syringe_{res}.spr"), "--format", "additive"])

    if a.preview:
        # Grid cell: dark panel + tint fill at alpha 100, icon tinted grey 180.  Status icon: green over a dark scene.
        cell_bg = (10 + 180 * 100 // 255,) * 3
        rows = []
        for res, size in SIZES.items():
            scale = max(1, math.ceil(132 / size))
            row = [
                additive_preview(icons[res], (0, 0, 0), (255, 255, 255), scale),
                additive_preview(icons[res], cell_bg, (180, 180, 180), scale),
                additive_preview(icons[res], (35, 35, 35), (0, 255, 0), scale),
            ]
            if a.reference and res == 2560:
                ref = Image.open(a.reference).convert("L")
                row.append(additive_preview(ref, cell_bg, (50, 200, 80), 1))
            rows.append(row)
        cw = max(im.size[0] for r in rows for im in r) + 8
        ch = max(im.size[1] for r in rows for im in r) + 8
        sheet = Image.new("RGB", (cw * 4, ch * len(rows)), (24, 24, 24))
        for ri, row in enumerate(rows):
            for ci, im in enumerate(row):
                sheet.paste(im, (ci * cw + 4, ri * ch + 4))
        sheet.save(a.preview)
        print("preview", a.preview)


if __name__ == "__main__":
    main()

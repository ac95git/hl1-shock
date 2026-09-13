"""Keycard icon -- the item_security HUD sprite at the four resolution buckets.

Drawn in the style of valve's item_* icons: a shaded greyscale object in three-quarter view,
no outline, edges carried by highlights because it is drawn additively. A card with a photo
at the left, text lines at the right and a magnetic stripe along the bottom, tilted so it
reads as a card lying on something rather than a flat rectangle.

    python utils/sprtool/icons/security.py OUT_DIR [--preview PREVIEW.png]

Writes OUT_DIR/security_{320,640,1280,2560}.png and .spr (additive).
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


def draw_card(canvas):
    """A landscape card, drawn axis-aligned into a square 'L' canvas."""
    W = canvas.size[0]
    d = ImageDraw.Draw(canvas)
    cw, ch = W * 0.80, W * 0.50
    x0, y0 = (W - cw) / 2, (W - ch) / 2
    x1, y1 = x0 + cw, y0 + ch
    r = W * 0.04

    # thickness: the card's underside offset down-right, darker
    t = W * 0.022
    d.rounded_rectangle((x0 + t, y0 + t, x1 + t, y1 + t), radius=r, fill=70)

    # face: lit from the top-left, so a gradient down the card
    face = Image.new("L", (int(cw), int(ch)), 0)
    fd = ImageDraw.Draw(face)
    for row in range(int(ch)):
        tt = row / max(1, ch - 1)
        v = 165 - 55 * tt
        fd.line((0, row, cw, row), fill=int(v))
    mask = Image.new("L", face.size, 0)
    ImageDraw.Draw(mask).rounded_rectangle((0, 0, cw - 1, ch - 1), radius=r, fill=255)
    canvas.paste(face, (int(x0), int(y0)), mask)

    # magnetic stripe along the bottom
    sy0, sy1 = y1 - ch * 0.30, y1 - ch * 0.12
    d.rectangle((x0, sy0, x1, sy1), fill=40)
    d.line((x0, sy0, x1, sy0), fill=210, width=max(1, int(W * 0.008)))

    # photo at the left: a frame, and a head-and-shoulders inside it
    px0, py0 = x0 + cw * 0.07, y0 + ch * 0.14
    px1, py1 = px0 + cw * 0.26, sy0 - ch * 0.10
    d.rectangle((px0, py0, px1, py1), fill=95, outline=235, width=max(1, int(W * 0.012)))
    pcx = (px0 + px1) / 2
    hr = (px1 - px0) * 0.20
    d.ellipse((pcx - hr, py0 + (py1 - py0) * 0.18, pcx + hr, py0 + (py1 - py0) * 0.18 + 2 * hr), fill=200)
    d.pieslice((pcx - hr * 2.0, py0 + (py1 - py0) * 0.56, pcx + hr * 2.0, py1 + (py1 - py0) * 0.6), start=180, end=360, fill=200)
    d.rectangle((px0 + max(1, int(W * 0.012)), py1 - (py1 - py0) * 0.0, px1, py1), fill=95)

    # text lines to the right of the photo
    lx0, lx1 = px1 + cw * 0.09, x1 - cw * 0.07
    lw = max(1, int(W * 0.022))
    ly = py0 + ch * 0.06
    for i, frac in enumerate((1.0, 0.72, 0.88, 0.55)):
        d.line((lx0, ly, lx0 + (lx1 - lx0) * frac, ly), fill=240 if i == 0 else 205, width=lw)
        ly += ch * 0.135

    # rim light along the top and left edges, the brightest thing on the card
    d.line((x0 + r, y0, x1 - r, y0), fill=255, width=max(1, int(W * 0.014)))
    d.line((x0, y0 + r, x0, y1 - r), fill=255, width=max(1, int(W * 0.012)))
    d.arc((x0, y0, x0 + 2 * r, y0 + 2 * r), start=180, end=270, fill=255, width=max(1, int(W * 0.014)))


def render(size, angle=22.0, ss=8):
    W = size * ss * 2
    canvas = Image.new("L", (W, W), 0)
    draw_card(canvas)
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


def additive_preview(icon, bg, tint, scale):
    w, h = icon.size
    base = Image.new("RGB", (w, h), bg)
    tinted = Image.merge("RGB", [icon.point(lambda v, c=c: v * c // 255) for c in tint])
    out = ImageChops.add(base, tinted)
    return out.resize((w * scale, h * scale), Image.NEAREST)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out_dir")
    ap.add_argument("--preview")
    ap.add_argument("--angle", type=float, default=22.0)
    ap.add_argument("--reference", help="PNG of item_battery at 2560 for side-by-side")
    a = ap.parse_args()
    os.makedirs(a.out_dir, exist_ok=True)

    icons = {}
    for res, size in SIZES.items():
        icon = render(size, a.angle)
        icons[res] = icon
        png = os.path.join(a.out_dir, f"security_{res}.png")
        Image.merge("RGBA", (icon, icon, icon, icon)).save(png)
        subprocess.check_call([sys.executable, SPRTOOL, "encode", png, os.path.join(a.out_dir, f"security_{res}.spr"), "--format", "additive"])

    if a.preview:
        suit = (60, 220, 255)
        rows = []
        for res, size in SIZES.items():
            scale = max(1, math.ceil(132 / size))
            row = [
                additive_preview(icons[res], (0, 0, 0), (255, 255, 255), scale),
                additive_preview(icons[res], (35, 35, 35), suit, scale),
            ]
            if a.reference and res == 2560:
                ref = Image.open(a.reference).convert("L")
                row.append(additive_preview(ref, (35, 35, 35), suit, 1))
            rows.append(row)
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

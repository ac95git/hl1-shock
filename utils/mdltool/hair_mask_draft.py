"""Draft hair masks for Ivan's three head textures, for Andrei to correct in Paint.NET.

Hair is the saturated dark brown (s >= ~0.7), skin the paler one (s 0.3-0.45); shadowed skin sits
between, so the ramp is soft and the draft is expected to be wrong there.  Writes, per texture:
  <name>.png            the texture at its real size (the Paint.NET guide layer)
  <name>_hair.png       the draft mask, greyscale, real size
  <name>_check_x6.png   texture with the mask in magenta, UV-used area outlined, x6
"""
import colorsys
import os
import sys

from PIL import Image, ImageDraw

IVAN = r"E:\CustomAssets\models\decompiled\npcs\ivan"
OUT = sys.argv[1]
TEX = {
    "remap5_128_191_000.bmp": "face",
    "side_of_head1.bmp": "side",
    "back_of_head1.bmp": "back",
}


def ramp(x, lo, hi):
    return max(0.0, min(1.0, (x - lo) / (hi - lo)))


def hairness(rgb):
    r, g, b = (c / 255 for c in rgb)
    h, s, v = colorsys.rgb_to_hsv(r, g, b)
    hue = h * 360
    # The side's hair highlights are the same colour as skin, so no colour test finds them: that area
    # is left to the hand.  (A grey term was tried; it caught the whites of the eyes.)
    if not (10 <= hue <= 50):  # eyes, anything not brown
        return 0.0
    return ramp(s, 0.55, 0.72) * (1.0 - ramp(v, 0.60, 0.75))


def uv_triangles():
    lines = open(os.path.join(IVAN, "ivan.smd")).read().splitlines()
    i = lines.index("triangles") + 1
    tris = {}
    while lines[i] != "end":
        tex = lines[i].strip().lower()
        uv = [tuple(float(x) for x in lines[i + 1 + k].split()[7:9]) for k in range(3)]
        tris.setdefault(tex, []).append(uv)
        i += 4
    return tris


def main():
    tris = uv_triangles()
    files = {n.lower(): n for n in os.listdir(IVAN)}
    for key, name in TEX.items():
        tex = Image.open(os.path.join(IVAN, files[key])).convert("RGB")
        w, h = tex.size
        tex.save(os.path.join(OUT, name + ".png"))

        mask = Image.new("L", tex.size)
        mask.putdata([int(round(255 * hairness(p))) for p in tex.getdata()])
        mask.save(os.path.join(OUT, name + "_hair.png"))

        k = 6
        check = tex.resize((w * k, h * k), Image.NEAREST)
        tint = Image.new("RGB", check.size, (255, 0, 255))
        check = Image.composite(tint, check, mask.resize(check.size, Image.NEAREST).point(lambda m: m * 0.7))
        draw = ImageDraw.Draw(check)
        for uv in tris.get(key, []):
            # SMD v runs up from the bottom of the texture
            draw.polygon([(u * w * k, (1 - v) * h * k) for u, v in uv], outline=(0, 255, 255))
        check.save(os.path.join(OUT, name + "_check_x6.png"))
        print(name, tex.size)


main()

"""Draft skin and eye masks for Ivan's head textures, for Andrei to correct in Paint.NET.

    python face_mask_draft.py OUTDIR

Built on Andrei's hair masks (E:\\CustomAssets\\textures\\ivan\\*_hair.png): skin is what the mesh uses
and is not hair, not iris and not the white of an eye.  The irises are green (hue 60-160) and two or
three pixels across; the whites are pale and grey-yellow, so the draft is expected to be wrong at the
edges of the eyes.  The back of the head is all hair and gets neither mask.  Writes, per texture:
  <name>_skin.png         the draft skin mask, greyscale, real size
  <name>_eyes.png         the draft eye mask (face only)
  <name>_check_x6.png     texture with skin in magenta, eyes in cyan, UV-used area outlined, x6
"""
import colorsys
import os
import sys

from PIL import Image, ImageChops, ImageDraw

IVAN = r"E:\CustomAssets\models\decompiled\npcs\ivan"
MASKS = r"E:\CustomAssets\textures\ivan"
OUT = sys.argv[1]
TEX = {
    "remap5_128_191_000.bmp": "face",
    "side_of_head1.bmp": "side",
}


EYE_ROWS = (28, 37)  # the face texture's rows holding the eyes, inclusive


def hsv(rgb):
    h, s, v = colorsys.rgb_to_hsv(*(c / 255 for c in rgb))
    return h * 360, s, v


def is_iris(rgb):
    h, s, v = hsv(rgb)
    return 60 <= h <= 160 and s >= 0.25


def is_white(rgb):
    h, s, v = hsv(rgb)
    return 40 <= h <= 70 and s <= 0.25


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
        hair = Image.open(os.path.join(MASKS, name + "_hair.png")).convert("L")

        # The area the mesh samples, a pixel wider so bilinear filtering at the seams is covered.
        used = Image.new("L", tex.size)
        du = ImageDraw.Draw(used)
        for uv in tris.get(key, []):
            du.polygon([(u * w, (1 - v) * h) for u, v in uv], fill=255, outline=255)

        # The beard and nostrils have green specks too, so the iris test only runs on the eyes' rows.
        eyes = Image.new("L", tex.size)
        eyes.putdata([255 if name == "face" and EYE_ROWS[0] <= i // w <= EYE_ROWS[1] and is_iris(p) else 0
                      for i, p in enumerate(tex.getdata())])
        white = Image.new("L", tex.size)
        white.putdata([255 if name == "face" and is_white(p) else 0 for p in tex.getdata()])
        skin = ImageChops.subtract(ImageChops.subtract(ImageChops.subtract(used, hair), eyes), white)
        skin.save(os.path.join(OUT, name + "_skin.png"))
        if name == "face":
            eyes.save(os.path.join(OUT, name + "_eyes.png"))

        k = 6
        check = tex.resize((w * k, h * k), Image.NEAREST)
        for mask, colour in ((skin, (255, 0, 255)), (eyes, (0, 255, 255))):
            tint = Image.new("RGB", check.size, colour)
            check = Image.composite(tint, check, mask.resize(check.size, Image.NEAREST).point(lambda m: m * 0.6))
        draw = ImageDraw.Draw(check)
        for uv in tris.get(key, []):
            draw.polygon([(u * w * k, (1 - v) * h * k) for u, v in uv], outline=(255, 255, 0))
        check.save(os.path.join(OUT, name + "_skin_check_x6.png"))
        print(name, tex.size)


main()

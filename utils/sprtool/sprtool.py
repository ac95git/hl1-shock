"""GoldSrc .spr reader/writer and HUD contact-sheet tool.

Format (utils/sprgen/spritegn.h, plus the Half-Life palette block that follows the header):

    dsprite_t          40 bytes: "IDSP", version 2, type, texFormat, boundingradius,
                       width, height, numframes, beamlength, synctype
    short              palette size (256)
    byte[768]          RGB palette
    per frame:
        int            frame type (0 = single, 1 = group)
        dspriteframe_t origin[2], width, height
        byte[w*h]      palette indices

Group frames are not needed for HUD work and are rejected on read.

Usage:
    sprtool.py decode  IN.spr OUT.png            8-bit .spr -> RGBA PNG (index 255 = transparent for ALPHTEST)
    sprtool.py encode  IN.png OUT.spr [--format additive|alphatest|indexalpha|normal]
    sprtool.py sheet   HUD.txt SPRITES_DIR OUT.png [--res 640] [--scale 4] [--only name,name]
                                                   tile every hud.txt entry for one resolution bucket
"""

import argparse
import os
import struct
import sys

from PIL import Image, ImageDraw

SPR_NORMAL, SPR_ADDITIVE, SPR_INDEXALPHA, SPR_ALPHTEST = 0, 1, 2, 3
TEX_NAMES = {"normal": SPR_NORMAL, "additive": SPR_ADDITIVE, "indexalpha": SPR_INDEXALPHA, "alphatest": SPR_ALPHTEST}
SPR_VP_PARALLEL = 2

HEADER = struct.Struct("<4siiifiiifi")
FRAME = struct.Struct("<iiiii")  # type, origin x, origin y, width, height


class Sprite:
    def __init__(self, width, height, tex_format, palette, frames, sprite_type=SPR_VP_PARALLEL):
        self.width, self.height = width, height
        self.tex_format = tex_format
        self.palette = palette  # 768 bytes
        self.frames = frames    # list of (origin_x, origin_y, w, h, bytes)
        self.sprite_type = sprite_type


def read_spr(path):
    with open(path, "rb") as f:
        data = f.read()
    ident, version, stype, texfmt, _radius, width, height, numframes, _beam, _sync = HEADER.unpack_from(data, 0)
    if ident != b"IDSP" or version != 2:
        raise ValueError(f"{path}: not a v2 GoldSrc sprite")
    off = HEADER.size
    (palsize,) = struct.unpack_from("<H", data, off)
    off += 2
    palette = data[off:off + palsize * 3]
    off += palsize * 3
    frames = []
    for _ in range(numframes):
        ftype, ox, oy, w, h = FRAME.unpack_from(data, off)
        if ftype != 0:
            raise ValueError(f"{path}: group frames unsupported")
        off += FRAME.size
        frames.append((ox, oy, w, h, data[off:off + w * h]))
        off += w * h
    return Sprite(width, height, texfmt, palette, frames, stype)


def write_spr(path, spr):
    out = bytearray()
    radius = ((spr.width / 2) ** 2 + (spr.height / 2) ** 2) ** 0.5
    out += HEADER.pack(b"IDSP", 2, spr.sprite_type, spr.tex_format, radius,
                       spr.width, spr.height, len(spr.frames), 0.0, 0)
    out += struct.pack("<H", 256)
    out += spr.palette
    for ox, oy, w, h, px in spr.frames:
        out += FRAME.pack(0, ox, oy, w, h)
        out += px
    with open(path, "wb") as f:
        f.write(out)


def frame_to_image(spr, index=0):
    """Frame as RGBA. Additive sprites come out on transparent black; alphatest uses index 255 as clear."""
    ox, oy, w, h, px = spr.frames[index]
    img = Image.new("RGBA", (w, h))
    pal = spr.palette
    put = img.putdata
    rows = []
    for i in px:
        r, g, b = pal[i * 3], pal[i * 3 + 1], pal[i * 3 + 2]
        if spr.tex_format == SPR_ALPHTEST:
            a = 0 if i == 255 else 255
        elif spr.tex_format == SPR_ADDITIVE:
            a = max(r, g, b)  # approximates additive-over-black for previewing
        elif spr.tex_format == SPR_INDEXALPHA:
            a = i
            r, g, b = pal[765], pal[766], pal[767]
        else:
            a = 255
        rows.append((r, g, b, a))
    put(rows)
    return img


def image_to_sprite(img, tex_format):
    """Quantise an RGBA image into a 256-colour sprite frame."""
    w, h = img.size
    if tex_format == SPR_ADDITIVE:
        # Additive sprites are drawn as light: colour is the palette entry, alpha is implicit in brightness.
        flat = Image.new("RGB", (w, h), (0, 0, 0))
        flat.paste(img.convert("RGB"), mask=img.getchannel("A"))
        q = flat.quantize(colors=256, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
        pal_list = q.getpalette()[:768]
        pal_list += [0] * (768 - len(pal_list))
        pal = bytes(pal_list)
        px = q.tobytes()
    elif tex_format == SPR_ALPHTEST:
        rgb = img.convert("RGB")
        q = rgb.quantize(colors=255, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
        pal_list = q.getpalette()[:765]
        pal_list += [0] * (765 - len(pal_list))
        pal = bytes(pal_list) + bytes((0, 0, 255))
        alpha = img.getchannel("A").tobytes()
        px = bytes(255 if a < 128 else i for i, a in zip(q.tobytes(), alpha))
    else:
        raise ValueError("encode supports additive and alphatest")
    return Sprite(w, h, tex_format, pal, [(0, 0, w, h, px)])


def parse_hud_txt(path):
    """Yield (name, res, file, x, y, w, h) for every entry in a hud.txt."""
    with open(path) as f:
        lines = [l.split("//")[0].strip() for l in f]
    entries = []
    for line in lines[1:]:
        parts = line.split()
        if len(parts) != 7:
            continue
        name, res, file_, x, y, w, h = parts
        entries.append((name, int(res), file_, int(x), int(y), int(w), int(h)))
    return entries


def cmd_decode(a):
    spr = read_spr(a.src)
    print(f"{a.src}: {spr.width}x{spr.height} frames={len(spr.frames)} texFormat={spr.tex_format} type={spr.sprite_type}")
    frame_to_image(spr, a.frame).save(a.dst)


def cmd_encode(a):
    img = Image.open(a.src).convert("RGBA")
    write_spr(a.dst, image_to_sprite(img, TEX_NAMES[a.format]))
    print(f"wrote {a.dst} {img.size[0]}x{img.size[1]} {a.format}")


def cmd_sheet(a):
    entries = [e for e in parse_hud_txt(a.hud) if e[1] == a.res]
    if a.only:
        wanted = set(a.only.split(","))
        entries = [e for e in entries if e[0] in wanted]
    cache = {}
    tiles = []
    for name, _res, file_, x, y, w, h in entries:
        if file_ not in cache:
            cache[file_] = frame_to_image(read_spr(os.path.join(a.sprites, file_ + ".spr")))
        tiles.append((name, cache[file_].crop((x, y, x + w, y + h))))
    if not tiles:
        sys.exit("no entries matched")
    cell_w = max(t[1].size[0] for t in tiles) * a.scale + 8
    cell_h = max(t[1].size[1] for t in tiles) * a.scale + 22
    cols = a.cols
    rows = (len(tiles) + cols - 1) // cols
    sheet = Image.new("RGBA", (cols * cell_w, rows * cell_h), (24, 24, 24, 255))
    draw = ImageDraw.Draw(sheet)
    for i, (name, img) in enumerate(tiles):
        cx, cy = (i % cols) * cell_w, (i // cols) * cell_h
        big = img.resize((img.size[0] * a.scale, img.size[1] * a.scale), Image.NEAREST)
        sheet.alpha_composite(big, (cx + 4, cy + 4))
        draw.text((cx + 4, cy + cell_h - 16), f"{name} {img.size[0]}x{img.size[1]}", fill=(200, 200, 200, 255))
    sheet.save(a.dst)
    print(f"wrote {a.dst}: {len(tiles)} tiles")


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd", required=True)
    d = sub.add_parser("decode"); d.add_argument("src"); d.add_argument("dst"); d.add_argument("--frame", type=int, default=0); d.set_defaults(fn=cmd_decode)
    e = sub.add_parser("encode"); e.add_argument("src"); e.add_argument("dst"); e.add_argument("--format", choices=TEX_NAMES, default="additive"); e.set_defaults(fn=cmd_encode)
    s = sub.add_parser("sheet"); s.add_argument("hud"); s.add_argument("sprites"); s.add_argument("dst")
    s.add_argument("--res", type=int, default=640); s.add_argument("--scale", type=int, default=4)
    s.add_argument("--cols", type=int, default=8); s.add_argument("--only"); s.set_defaults(fn=cmd_sheet)
    a = p.parse_args()
    a.fn(a)


if __name__ == "__main__":
    main()

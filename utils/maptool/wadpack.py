"""Packs a folder of PNGs into a WAD3, the texture archive GoldSrc maps read.

    python wadpack.py OUT.wad PNG_DIR [--install]
    python wadpack.py --list SOME.wad [--dump DIR]

Every PNG in PNG_DIR becomes one miptex lump named after the file (its stem, 15 characters or
fewer). The source tier is E:\\CustomAssets\\textures\\wad\\ (docs/MAP_WORKFLOW.md, "The mod's
WAD"); this script only ever reads it. Sides must be multiples of 16, since the fourth mip is a
sixteenth of the first. Each texture is quantised to its own 256-colour palette; the three
smaller mips are box-filtered from the full-size image and mapped back onto that palette, so a
mip never introduces a colour the palette lacks. A name starting with `{` is a masked texture:
its pixels with alpha 0 go to index 255, which is set to (0, 0, 255), the engine's clear colour,
and the mips are reduced premultiplied so the clear colour does not bleed into the edge.

`--install` copies the finished WAD to the mod directory. `--list` reads a WAD back and prints
each lump's name, size, mip offsets and palette size; `--dump DIR` writes each texture's mip 0
as a PNG. That reader is the check on the writer: run it on valve's halflife.wad and the numbers
come out as the engine expects, run it on ours and they must match.

The layout, from the format: header `WAD3`, int32 lump count, int32 directory offset; each
directory entry 32 bytes (int32 file position, int32 disk size, int32 size, byte type 0x43,
byte compression 0, two bytes padding, 16-byte null-padded name); each miptex 16-byte name,
uint32 width, uint32 height, four uint32 mip offsets relative to the miptex, the four mips at
1, 1/2, 1/4 and 1/8, a uint16 256, then the 768-byte palette, then two bytes of padding.
"""

import os
import shutil
import struct
import sys

from PIL import Image

INSTALL_DIR = r"D:\GameLibrary\steam\steamapps\common\Half-Life\topmod"
LUMP_MIPTEX = 0x43
CLEAR = (0, 0, 255)


def die(msg):
    sys.exit("wadpack: " + msg)


def quantise(im, masked):
    """RGBA -> (P image, 768-byte palette). Masked textures keep index 255 for the clear colour."""
    rgb = im.convert("RGB")
    colours = 255 if masked else 256
    p = rgb.quantize(colors=colours, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    pal = p.getpalette()[:colours * 3]
    pal += [0] * (768 - len(pal))
    if masked:
        pal[765:768] = list(CLEAR)
        p.putpalette(pal)
    return p, bytes(pal)


def mip_indices(im, p_full, pal, masked, level):
    """Index bytes for mip `level` (0..3), mapped onto the texture's own palette."""
    if level == 0:
        idx = bytearray(p_full.tobytes())
        if masked:
            alpha = im.getchannel("A").tobytes()
            for i, a in enumerate(alpha):
                if a == 0:
                    idx[i] = 255
        return bytes(idx)
    factor = 2 ** level
    if masked:
        small = im.convert("RGBa").reduce(factor).convert("RGBA")
    else:
        small = im.convert("RGB").reduce(factor)
    palimg = Image.new("P", (1, 1))
    palimg.putpalette(list(pal))
    mapped = small.convert("RGB").quantize(palette=palimg, dither=Image.Dither.NONE)
    idx = bytearray(mapped.tobytes())
    if masked:
        alpha = small.getchannel("A").tobytes()
        for i, a in enumerate(alpha):
            if a < 128:
                idx[i] = 255
    return bytes(idx)


def miptex(name, im):
    w, h = im.size
    if w % 16 or h % 16:
        die("%s is %dx%d; sides must be multiples of 16" % (name, w, h))
    masked = name.startswith("{")
    p, pal = quantise(im, masked)
    mips = [mip_indices(im, p, pal, masked, lv) for lv in range(4)]
    off = 40
    offsets = []
    for m in mips:
        offsets.append(off)
        off += len(m)
    head = struct.pack("<16sIIIIII", name.encode("ascii"), w, h, *offsets)
    return head + b"".join(mips) + struct.pack("<H", 256) + pal + b"\0\0"


def pack(out, src_dir, install=False):
    files = sorted(f for f in os.listdir(src_dir) if f.lower().endswith(".png"))
    if not files:
        die("no PNGs in " + src_dir)
    lumps = []
    for f in files:
        name = os.path.splitext(f)[0]
        if len(name) > 15:
            die("%s: a texture name is 15 characters or fewer" % name)
        im = Image.open(os.path.join(src_dir, f)).convert("RGBA")
        lumps.append((name, miptex(name, im), im.size))
    body = b""
    directory = b""
    pos = 12
    for name, data, (w, h) in lumps:
        directory += struct.pack("<iiiBBxx16s", pos, len(data), len(data), LUMP_MIPTEX, 0,
                                 name.encode("ascii"))
        body += data
        pos += len(data)
    with open(out, "wb") as fh:
        fh.write(b"WAD3" + struct.pack("<ii", len(lumps), 12 + len(body)))
        fh.write(body)
        fh.write(directory)
    for name, data, (w, h) in lumps:
        print("  %-16s %4dx%-4d %7d bytes" % (name, w, h, len(data)))
    print("%s: %d textures, %d bytes" % (out, len(lumps), 12 + len(body) + len(directory)))
    if install:
        dst = os.path.join(INSTALL_DIR, os.path.basename(out))
        shutil.copyfile(out, dst)
        print("installed " + dst)


def read(path):
    """Yields (name, width, height, mip offsets, disksize, palette count, lump bytes) per miptex."""
    with open(path, "rb") as fh:
        data = fh.read()
    magic, count, diroff = struct.unpack_from("<4sii", data, 0)
    if magic != b"WAD3":
        die("%s is not a WAD3 (%r)" % (path, magic))
    for i in range(count):
        filepos, disksize, size, typ, comp = struct.unpack_from("<iiiBB", data, diroff + 32 * i)
        name = data[diroff + 32 * i + 16:diroff + 32 * i + 32].split(b"\0")[0].decode("latin1")
        lump = data[filepos:filepos + disksize]
        if typ != LUMP_MIPTEX:
            yield name, None, None, None, disksize, None, lump
            continue
        mname, w, h, o0, o1, o2, o3 = struct.unpack_from("<16sIIIIII", lump, 0)
        palpos = o3 + (w // 8) * (h // 8)
        palcount = struct.unpack_from("<H", lump, palpos)[0]
        yield name, w, h, (o0, o1, o2, o3), disksize, palcount, lump


def list_wad(path, dump=None, limit=None):
    n = 0
    for name, w, h, offs, disksize, palcount, lump in read(path):
        if w is None:
            print("  %-16s (not a miptex, %d bytes)" % (name, disksize))
            continue
        expect = 40 + w * h * 85 // 64 + 2 + 768
        note = "" if disksize in (expect, expect + 2) else "  <- size off by %d" % (disksize - expect)
        print("  %-16s %4dx%-4d offsets %s palette %d disksize %d (mips+palette %d)%s"
              % (name, w, h, offs, palcount, disksize, expect, note))
        if dump:
            palpos = offs[3] + (w // 8) * (h // 8)
            pal = lump[palpos + 2:palpos + 2 + 768]
            im = Image.frombytes("P", (w, h), lump[offs[0]:offs[0] + w * h])
            im.putpalette(list(pal))
            os.makedirs(dump, exist_ok=True)
            im.convert("RGB").save(os.path.join(dump, name.replace("{", "_").replace("!", "_") + ".png"))
        n += 1
        if limit and n >= limit:
            break
    print("%s: %d textures listed" % (path, n))


def main(argv):
    if "--list" in argv:
        i = argv.index("--list")
        path = argv[i + 1]
        dump = argv[argv.index("--dump") + 1] if "--dump" in argv else None
        limit = int(argv[argv.index("--limit") + 1]) if "--limit" in argv else None
        list_wad(path, dump, limit)
        return
    install = "--install" in argv
    args = [a for a in argv if not a.startswith("--")]
    if len(args) != 2:
        die("usage: wadpack.py OUT.wad PNG_DIR [--install] | --list X.wad [--dump DIR] [--limit N]")
    pack(args[0], args[1], install)


if __name__ == "__main__":
    main(sys.argv[1:])

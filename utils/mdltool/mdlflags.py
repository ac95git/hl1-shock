"""Read or patch the per-texture render flags in a compiled GoldSrc .mdl.

studiomdl sets CHROME from a texture's name and (this repo's build) ADDITIVE / MASKED from
$texrendermode, and nothing can set FULLBRIGHT.  The flag is one int in the texture record
(mstudiotexture_t, engine/studio.h), so it is patched here after the compile.

    python utils/mdltool/mdlflags.py MODEL.mdl                       list textures and flags
    python utils/mdltool/mdlflags.py MODEL.mdl TEXTURE +fullbright   set a flag on one texture
    python utils/mdltool/mdlflags.py MODEL.mdl TEXTURE -chrome       clear a flag

Flags: flatshade chrome fullbright nomips alpha additive masked.  TEXTURE matches the stored name
case-insensitively, with or without .bmp.  Several +/- operations may be given.
"""

import struct
import sys

FLAGS = {"flatshade": 0x1, "chrome": 0x2, "fullbright": 0x4, "nomips": 0x8, "alpha": 0x10, "additive": 0x20, "masked": 0x40}


def names(flags):
    return ",".join(n for n, v in FLAGS.items() if flags & v) or "-"


def textures(d):
    numtextures, textureindex = struct.unpack_from("<2i", d, 180)
    for i in range(numtextures):
        off = textureindex + i * 80
        name = d[off:off + 64].split(b"\0", 1)[0].decode("latin-1")
        (flags,) = struct.unpack_from("<i", d, off + 64)
        yield off, name, flags


def main():
    path = sys.argv[1]
    d = bytearray(open(path, "rb").read())
    if d[:4] != b"IDST":
        sys.exit(f"{path}: not a studio model")
    if len(sys.argv) == 2:
        for _, name, flags in textures(d):
            print(f"  {name:<32} 0x{flags:02x} {names(flags)}")
        return
    target = sys.argv[2].lower().removesuffix(".bmp")
    ops = sys.argv[3:]
    hit = False
    for off, name, flags in textures(d):
        if name.lower().removesuffix(".bmp") != target:
            continue
        hit = True
        new = flags
        for op in ops:
            bit = FLAGS[op[1:].lower()]
            new = new | bit if op[0] == "+" else new & ~bit
        struct.pack_into("<i", d, off + 64, new)
        print(f"{name}: 0x{flags:02x} ({names(flags)}) -> 0x{new:02x} ({names(new)})")
    if not hit:
        sys.exit(f"{path}: no texture named {sys.argv[2]}")
    open(path, "wb").write(d)


if __name__ == "__main__":
    main()

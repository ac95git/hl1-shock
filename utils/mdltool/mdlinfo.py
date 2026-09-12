"""Read a GoldSrc .mdl header and print what matters for model surgery: bones, textures, bodyparts,
sequences.  Struct layouts from engine/studio.h.  No decompile needed.

    python utils/mdltool/mdlinfo.py MODEL.mdl [MODEL2.mdl ...] [--bones-only]
"""

import struct
import sys


def cstr(b):
    return b.split(b"\0", 1)[0].decode("latin-1")


def read_mdl(path):
    with open(path, "rb") as f:
        d = f.read()
    if d[:4] != b"IDST":
        raise ValueError(f"{path}: not a studio model (ident {d[:4]!r})")
    # studiohdr_t offsets
    name = cstr(d[8:72])
    (numbones, boneindex, numbonectl, _bci, numhitboxes, _hbi, numseq, seqindex,
     numseqgroups, _sgi, numtextures, textureindex, _tdi, numskinref, numskinfamilies, _ski,
     numbodyparts, bodypartindex, numattachments, attachmentindex) = struct.unpack_from("<20i", d, 140)

    bones = []
    for i in range(numbones):
        off = boneindex + i * 112
        bname = cstr(d[off:off + 32])
        (parent,) = struct.unpack_from("<i", d, off + 32)
        bones.append((bname, parent))

    textures = []
    for i in range(numtextures):
        off = textureindex + i * 80
        tname = cstr(d[off:off + 64])
        flags, w, h = struct.unpack_from("<3i", d, off + 64)
        textures.append((tname, w, h, flags))

    bodyparts = []
    for i in range(numbodyparts):
        off = bodypartindex + i * 76
        bpname = cstr(d[off:off + 64])
        nummodels, _base, modelindex = struct.unpack_from("<3i", d, off + 64)
        models = []
        for j in range(nummodels):
            moff = modelindex + j * 112  # mstudiomodel_t
            mname = cstr(d[moff:moff + 64])
            _type, _rad, nummesh, _mi, numverts = struct.unpack_from("<ifiii", d, moff + 64)
            models.append((mname, nummesh, numverts))
        bodyparts.append((bpname, models))

    seqs = []
    for i in range(numseq):
        off = seqindex + i * 176
        seqs.append(cstr(d[off:off + 32]))

    attachments = []
    for i in range(numattachments):
        off = attachmentindex + i * 88
        (bone,) = struct.unpack_from("<i", d, off + 68)
        attachments.append(bone)

    return dict(path=path, name=name, bones=bones, textures=textures, bodyparts=bodyparts,
                seqs=seqs, attachments=attachments, numskinfamilies=numskinfamilies)


def extract_textures(path, out_dir, as_bmp=False):
    """Write every embedded texture (8-bit indices + 768-byte palette follow each texture's index).

    PNG (prefixed with the model name) for looking at; with as_bmp, 8-bit BMPs under the exact stored
    texture names, which is what a QC compiled with $cdtexture expects to find.
    """
    import os
    from PIL import Image
    with open(path, "rb") as f:
        d = f.read()
    numtextures, textureindex = struct.unpack_from("<2i", d, 180)
    os.makedirs(out_dir, exist_ok=True)
    base = os.path.splitext(os.path.basename(path))[0]
    for i in range(numtextures):
        off = textureindex + i * 80
        tname = cstr(d[off:off + 64])
        _flags, w, h, idx = struct.unpack_from("<4i", d, off + 64)
        img = Image.frombytes("P", (w, h), d[idx:idx + w * h])
        img.putpalette(d[idx + w * h:idx + w * h + 768])
        if as_bmp:
            out = os.path.join(out_dir, tname)
            img.save(out, "BMP")
        else:
            out = os.path.join(out_dir, f"{base}__{os.path.splitext(tname)[0]}.png")
            img.convert("RGB").save(out)
        print(f"  wrote {out} {w}x{h}")


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    bones_only = "--bones-only" in sys.argv
    extract_to = None
    as_bmp = False
    for a in sys.argv[1:]:
        if a.startswith("--extract="):
            extract_to = a.split("=", 1)[1]
        elif a.startswith("--extract-bmp="):
            extract_to = a.split("=", 1)[1]
            as_bmp = True
    for path in args:
        m = read_mdl(path)
        if extract_to:
            extract_textures(path, extract_to, as_bmp)
        print(f"=== {m['name']}  bones={len(m['bones'])} textures={len(m['textures'])} "
              f"bodyparts={len(m['bodyparts'])} seqs={len(m['seqs'])} attachments={len(m['attachments'])}")
        for i, (b, p) in enumerate(m["bones"]):
            print(f"  bone {i:2d} {b:<24} parent={p}")
        if bones_only:
            continue
        for t, w, h, fl in m["textures"]:
            print(f"  tex  {t:<32} {w}x{h} flags=0x{fl:x}")
        for bp, models in m["bodyparts"]:
            print(f"  bodypart {bp}")
            for mn, nm, nv in models:
                print(f"     model {mn:<24} meshes={nm} verts={nv}")
        print(f"  seqs {', '.join(m['seqs'])}")


if __name__ == "__main__":
    main()

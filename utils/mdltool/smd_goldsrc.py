"""Convert an SMD exported by Blender Source Tools into the form GoldSrc's studiomdl reads.

Source Tools writes each vertex as
    <parent> x y z nx ny nz u v <nlinks> <bone> <weight> [<bone> <weight> ...]
and GoldSrc studiomdl reads only the leading <parent> column, ignoring the weights.  Source Tools
puts 0 there, so every vertex lands on the root bone and studiomdl prunes the rest of the skeleton
("cannot find bone ... for bbox").  This rewrites <parent> to the heaviest linked bone and drops the
link list.  Vertices with no links keep their parent.

Also, with --wrap-uv, shifts each triangle's UVs by whole tiles so the triangle's centre lies in 0..1.
Source models routinely carry UVs whole tiles outside 0..1; studiomdl scales them by the texture size as
they are and the game samples off the texture (the surface comes out black).  A whole-number shift
changes nothing on a repeating texture.

    python utils/mdltool/smd_goldsrc.py IN.smd OUT.smd [--wrap-uv]
"""

import math
import sys


def convert(src, dst, wrap_uv=False):
    out = []
    in_tris = False
    fixed = 0
    wrapped = 0
    tri = []  # pending vertex lines of the current triangle (as part lists)

    def flush():
        nonlocal wrapped
        if not tri:
            return
        if wrap_uv:
            du = math.floor(sum(float(p[7]) for p in tri) / len(tri))
            dv = math.floor(sum(float(p[8]) for p in tri) / len(tri))
            if du or dv:
                wrapped += 1
            for p in tri:
                p[7] = f"{float(p[7]) - du:.6f}"
                p[8] = f"{float(p[8]) - dv:.6f}"
        for p in tri:
            out.append(" ".join(p[:9]) + "\n")
        tri.clear()

    with open(src) as f:
        for line in f:
            s = line.strip()
            if s == "triangles":
                in_tris = True
                out.append(line)
                continue
            if in_tris and s == "end":
                flush()
                in_tris = False
            if in_tris:
                parts = s.split()
                if len(parts) >= 9 and parts[0].lstrip("-").isdigit():
                    if len(parts) >= 10:
                        nlinks = int(parts[9])
                        if nlinks > 0:
                            links = parts[10:10 + 2 * nlinks]
                            best = max(range(nlinks), key=lambda i: float(links[2 * i + 1]))
                            parts[0] = links[2 * best]
                            fixed += 1
                    tri.append(parts[:9])
                    if len(tri) == 3:
                        flush()
                    continue
                flush()  # a material line
            out.append(line)
    with open(dst, "w", newline="\n") as f:
        f.writelines(out)
    return fixed, wrapped


if __name__ == "__main__":
    n, w = convert(sys.argv[1], sys.argv[2], "--wrap-uv" in sys.argv)
    print(f"{sys.argv[2]}: {n} vertices reassigned, {w} triangles UV-wrapped")

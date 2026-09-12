"""Convert an SMD exported by Blender Source Tools into the form GoldSrc's studiomdl reads.

Source Tools writes each vertex as
    <parent> x y z nx ny nz u v <nlinks> <bone> <weight> [<bone> <weight> ...]
and GoldSrc studiomdl reads only the leading <parent> column, ignoring the weights.  Source Tools
puts 0 there, so every vertex lands on the root bone and studiomdl prunes the rest of the skeleton
("cannot find bone ... for bbox").  This rewrites <parent> to the heaviest linked bone and drops the
link list.  Vertices with no links keep their parent.

    python utils/mdltool/smd_goldsrc.py IN.smd OUT.smd
"""

import sys


def convert(src, dst):
    out = []
    in_tris = False
    fixed = 0
    with open(src) as f:
        for line in f:
            s = line.strip()
            if s == "triangles":
                in_tris = True
                out.append(line)
                continue
            if in_tris and s == "end":
                in_tris = False
            if in_tris:
                parts = s.split()
                if len(parts) >= 10 and parts[0].lstrip("-").isdigit():
                    nlinks = int(parts[9])
                    if nlinks > 0:
                        links = parts[10:10 + 2 * nlinks]
                        best = max(range(nlinks), key=lambda i: float(links[2 * i + 1]))
                        parts[0] = links[2 * best]
                        fixed += 1
                    line = " ".join(parts[:9]) + "\n"
            out.append(line)
    with open(dst, "w", newline="\n") as f:
        f.writelines(out)
    return fixed


if __name__ == "__main__":
    n = convert(sys.argv[1], sys.argv[2])
    print(f"{sys.argv[2]}: {n} vertices reassigned")

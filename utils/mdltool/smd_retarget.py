"""Move an animation SMD onto another skeleton that shares its bones under other names.

    python utils/mdltool/smd_retarget.py SRC_ANIM.smd TARGET_REF.smd OUT.smd [--map "Src Bone=Dst Bone" ...]
                                         [--rest SRC_REST_ANIM.smd] [--tolerance 0.01]

Bones are matched by index by default (source bone i -> target bone i for i < the target's bone
count); --map overrides by name.  Source bones with no target -- fingers, a second arm -- are dropped;
a target bone with no source keeps its rest pose from TARGET_REF.  Before writing, the source's rest
pose (--rest, frame 0; default the source animation's own frame 0) is compared with the target's
rest for every matched bone and the largest position/rotation difference is printed: if the rests
differ, this is the wrong tool, since the numbers are copied, not solved.

Written 2026-09-15 for HL Extended's crowbar animations, whose 45-bone Bip01 rig carries Valve's 11
bones (Clavicle/UpperArm/Forearm for R Arm/R Arm1/R Arm2) with identical rest values.
"""

import argparse
import math
import sys

import smd_pose as sp


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("src")
    ap.add_argument("target_ref")
    ap.add_argument("out")
    ap.add_argument("--map", action="append", default=[], help='"Source Bone=Target Bone"')
    ap.add_argument("--rest", default=None, help="a source animation whose frame 0 is the rig's rest pose")
    ap.add_argument("--tolerance", type=float, default=0.01, help="max rest difference to accept, units and radians")
    a = ap.parse_args()

    snodes, sframes, _ = sp.parse_smd(a.src)
    tnodes, tframes, _ = sp.parse_smd(a.target_ref)
    rest_src = sp.parse_smd(a.rest)[1][0] if a.rest else sframes[0]
    trest = tframes[0]

    sname = {n: i for i, n, _ in snodes}
    tname = {n: i for i, n, _ in tnodes}
    # target index -> source index
    link = {ti: ti for ti, _, _ in tnodes if ti < len(snodes)}
    for m in a.map:
        s, t = m.split("=", 1)
        if s not in sname or t not in tname:
            sys.exit(f"--map {m!r}: unknown bone")
        link[tname[t]] = sname[s]

    worst = 0.0
    for ti, tn, tp in tnodes:
        if ti not in link:
            print(f"target bone {ti} {tn!r}: no source, keeps rest")
            continue
        si = link[ti]
        sp_, sr = rest_src[si]
        tp_, tr = trest[ti]
        d = max(max(abs(sp_[k] - tp_[k]) for k in range(3)), max(abs(sr[k] - tr[k]) for k in range(3)))
        worst = max(worst, d)
        print(f"target {ti:2d} {tn:18s} <- source {si:2d} {snodes[si][1]:22s} rest diff {d:.4f}")
    print(f"largest rest difference {worst:.4f}" + ("" if worst <= a.tolerance else "  ** rests differ: the numbers cannot simply be copied **"))
    if worst > a.tolerance:
        sys.exit(1)

    out = ["version 1", "nodes"]
    out += [f'{i} "{n}" {p}' for i, n, p in tnodes]
    out += ["end", "skeleton"]
    for f, frame in enumerate(sframes):
        out.append(f"time {f}")
        for ti, _, _ in tnodes:
            pos, rot = frame[link[ti]] if ti in link else trest[ti]
            out.append(f"  {ti} " + " ".join(f"{v:.6f}" for v in list(pos) + list(rot)))
    out += ["end"]
    with open(a.out, "w", newline="\n") as fh:
        fh.write("\n".join(out) + "\n")
    print(f"wrote {a.out}: {len(sframes)} frames, {len(tnodes)} bones")


if __name__ == "__main__":
    main()

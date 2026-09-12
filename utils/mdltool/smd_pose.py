"""Pose a GoldSrc reference SMD with one frame of an animation SMD, writing a new (static) SMD.

Does what the engine does at draw time: each vertex belongs to one bone; it is moved by
    M_anim[bone] * inverse(M_rest[bone])
where M_* are the bones' world matrices built by chaining  T(pos) * Rz(rz) * Ry(ry) * Rx(rx)  down the
parent tree.  The output's skeleton is the animation frame, so a viewer (or Blender) shows the model
the way the game shows it for that frame -- viewmodel space, camera at the origin looking down +X.

    python utils/mdltool/smd_pose.py REF.smd ANIM.smd FRAME OUT.smd
"""

import math
import sys


def mat_mul(a, b):
    return [[sum(a[i][k] * b[k][j] for k in range(4)) for j in range(4)] for i in range(4)]


def mat_vec(m, v):
    return [sum(m[i][k] * v[k] for k in range(3)) + m[i][3] for i in range(3)]


def mat_dir(m, v):
    return [sum(m[i][k] * v[k] for k in range(3)) for i in range(3)]


def bone_matrix(pos, rot):
    rx, ry, rz = rot
    cx, sx = math.cos(rx), math.sin(rx)
    cy, sy = math.cos(ry), math.sin(ry)
    cz, sz = math.cos(rz), math.sin(rz)
    Rx = [[1, 0, 0], [0, cx, -sx], [0, sx, cx]]
    Ry = [[cy, 0, sy], [0, 1, 0], [-sy, 0, cy]]
    Rz = [[cz, -sz, 0], [sz, cz, 0], [0, 0, 1]]
    R = [[sum(Rz[i][k] * Ry[k][j] for k in range(3)) for j in range(3)] for i in range(3)]
    R = [[sum(R[i][k] * Rx[k][j] for k in range(3)) for j in range(3)] for i in range(3)]
    return [[R[0][0], R[0][1], R[0][2], pos[0]],
            [R[1][0], R[1][1], R[1][2], pos[1]],
            [R[2][0], R[2][1], R[2][2], pos[2]],
            [0, 0, 0, 1]]


def invert_rigid(m):
    R = [[m[j][i] for j in range(3)] for i in range(3)]  # transpose
    t = [m[i][3] for i in range(3)]
    it = [-sum(R[i][k] * t[k] for k in range(3)) for i in range(3)]
    return [[R[0][0], R[0][1], R[0][2], it[0]],
            [R[1][0], R[1][1], R[1][2], it[1]],
            [R[2][0], R[2][1], R[2][2], it[2]],
            [0, 0, 0, 1]]


def parse_smd(path):
    nodes, frames, tri_lines = [], [], []
    section = None
    with open(path) as f:
        for raw in f:
            line = raw.strip()
            if not line or line.startswith("//"):
                continue
            if line in ("nodes", "skeleton", "triangles"):
                section = line
                continue
            if line == "end":
                section = None
                continue
            if section == "nodes":
                idx, rest = line.split(None, 1)
                name, parent = rest.rsplit(None, 1)
                nodes.append((int(idx), name.strip('"'), int(parent)))
            elif section == "skeleton":
                if line.startswith("time"):
                    frames.append({})
                else:
                    p = line.split()
                    frames[-1][int(p[0])] = ([float(x) for x in p[1:4]], [float(x) for x in p[4:7]])
            elif section == "triangles":
                tri_lines.append(raw.rstrip("\n"))
    return nodes, frames, tri_lines


def world_matrices(nodes, frame):
    mats = {}
    for idx, _name, parent in nodes:
        pos, rot = frame[idx]
        local = bone_matrix(pos, rot)
        mats[idx] = mat_mul(mats[parent], local) if parent >= 0 else local
    return mats


def pose(ref_path, anim_path, frame_no, out_path):
    nodes, rest_frames, tris = parse_smd(ref_path)
    anodes, aframes, _ = parse_smd(anim_path)
    if [n[1] for n in nodes] != [n[1] for n in anodes]:
        sys.exit("bone lists differ between reference and animation")
    frame = aframes[min(frame_no, len(aframes) - 1)]
    rest = world_matrices(nodes, rest_frames[0])
    posed = world_matrices(nodes, frame)
    xf = {i: mat_mul(posed[i], invert_rigid(rest[i])) for i in posed}

    out = ["version 1", "nodes"]
    out += [f'{i} "{n}" {p}' for i, n, p in nodes]
    out += ["end", "skeleton", "time 0"]
    out += [f"{i} " + " ".join(f"{v:.6f}" for v in frame[i][0] + frame[i][1]) for i, _, _ in nodes]
    out += ["end", "triangles"]
    for line in tris:
        p = line.split()
        if len(p) >= 9 and p[0].lstrip("-").isdigit():
            b = int(p[0])
            v = mat_vec(xf[b], [float(x) for x in p[1:4]])
            n = mat_dir(xf[b], [float(x) for x in p[4:7]])
            out.append(f"{b} " + " ".join(f"{x:.6f}" for x in v + n) + " " + " ".join(p[7:9]))
        else:
            out.append(line.strip())
    out.append("end")
    with open(out_path, "w", newline="\n") as f:
        f.write("\n".join(out) + "\n")
    # report where the model went
    pts = [[float(x) for x in l.split()[1:4]] for l in out if l and l[0].isdigit() and len(l.split()) == 9]
    lo = [min(p[i] for p in pts) for i in range(3)]
    hi = [max(p[i] for p in pts) for i in range(3)]
    print(f"{out_path}: frame {frame_no}/{len(aframes)} bounds "
          f"({lo[0]:.1f},{lo[1]:.1f},{lo[2]:.1f})..({hi[0]:.1f},{hi[1]:.1f},{hi[2]:.1f})")


if __name__ == "__main__":
    pose(sys.argv[1], sys.argv[2], int(sys.argv[3]), sys.argv[4])

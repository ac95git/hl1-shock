"""List every brush of a Valve 220 .map whose vertices fall inside a box, with its
computed vertices, extents and an orientation guess. Usage:
    brushes_near.py MAP x0 y0 z0 x1 y1 z1
"""
import itertools
import re
import sys

PLANE = re.compile(r'\(\s*([-\d.e]+)\s+([-\d.e]+)\s+([-\d.e]+)\s*\)\s*'
                   r'\(\s*([-\d.e]+)\s+([-\d.e]+)\s+([-\d.e]+)\s*\)\s*'
                   r'\(\s*([-\d.e]+)\s+([-\d.e]+)\s+([-\d.e]+)\s*\)\s*(\S+)')


def sub(a, b): return tuple(a[i] - b[i] for i in range(3))
def dot(a, b): return sum(a[i] * b[i] for i in range(3))
def cross(a, b): return (a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0])


def plane_of(p1, p2, p3):
    # Valve 220: points are clockwise seen from outside; normal = (p3-p1) x (p2-p1)
    n = cross(sub(p3, p1), sub(p2, p1))
    return n, dot(n, p1)


def solve3(planes):
    (a, da), (b, db), (c, dc) = planes
    det = dot(a, cross(b, c))
    if abs(det) < 1e-9:
        return None
    x = tuple((da * cross(b, c)[i] + db * cross(c, a)[i] + dc * cross(a, b)[i]) / det for i in range(3))
    return x


def vertices(planes):
    verts = []
    for trio in itertools.combinations(planes, 3):
        p = solve3(trio)
        if p is None:
            continue
        if all(dot(n, p) - d <= 1e-3 * max(1.0, abs(d)) for n, d in planes):
            if not any(all(abs(p[i]-q[i]) < 0.01 for i in range(3)) for q in verts):
                verts.append(p)
    return verts


def main():
    path = sys.argv[1]
    x0, y0, z0, x1, y1, z1 = map(float, sys.argv[2:8])
    lines = open(path, encoding='utf-8', errors='replace').read().splitlines()
    depth = 0; ent = -1; brush = -1; cls = None; cur = []
    for line in lines:
        s = line.strip()
        if s == '{':
            depth += 1
            if depth == 1: ent += 1; brush = -1; cls = '?'
            elif depth == 2: brush += 1; cur = []
        elif s == '}':
            if depth == 2:
                planes = []; texs = []
                for pl in cur:
                    m = PLANE.match(pl)
                    if not m: continue
                    v = list(map(float, m.groups()[:9]))
                    planes.append(plane_of(tuple(v[0:3]), tuple(v[3:6]), tuple(v[6:9])))
                    texs.append(m.group(10))
                vs = vertices(planes)
                if vs and any(x0 <= x <= x1 and y0 <= y <= y1 and z0 <= z <= z1 for x, y, z in vs):
                    xs = [v[0] for v in vs]; ys = [v[1] for v in vs]; zs = [v[2] for v in vs]
                    print(f'--- entity {ent} ({cls}) brush {brush}: {len(planes)} faces, {len(vs)} vertices, textures {sorted(set(texs))}')
                    print(f'    extents x {min(xs):g}..{max(xs):g}  y {min(ys):g}..{max(ys):g}  z {min(zs):g}..{max(zs):g}')
                    # which axis do the plane normals avoid? (a prism's side normals are all perpendicular to its axis)
                    for name, i in (('x', 0), ('y', 1), ('z', 2)):
                        sides = [n for n, d in planes if abs(n[i]) < 1e-6 * max(1, max(abs(c) for c in n))]
                        if len(sides) >= 3:
                            print(f'    {len(sides)} faces are parallel to the {name} axis (a prism or point along {name})')
                    offgrid = [v for v in vs if any(abs(c / 16 - round(c / 16)) > 1e-3 for c in v)]
                    print(f'    vertices off the 16 grid: {len(offgrid)}')
                    for v in sorted(vs):
                        print('      (%8.2f %8.2f %8.2f)' % v)
            depth -= 1
        elif depth == 1 and s.startswith('"classname"'):
            cls = s.split('"')[3]
        elif depth == 2 and s.startswith('('):
            cur.append(s)


if __name__ == '__main__':
    main()

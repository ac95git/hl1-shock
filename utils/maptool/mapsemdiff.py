"""Semantic diff of two Valve 220 .map files: brushes compared by their computed
vertices and textures, entities by classname and keyvalues. Formatting is ignored.
Usage: mapsemdiff.py OLD.map NEW.map
"""
import os
import sys
from collections import Counter

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from brushes_near import PLANE, plane_of, vertices  # noqa: E402


def parse(path):
    lines = open(path, encoding='utf-8', errors='replace').read().splitlines()
    ents = []
    depth = 0; cur = None; brush = None
    for line in lines:
        s = line.strip()
        if s == '{':
            depth += 1
            if depth == 1: cur = {'kv': {}, 'brushes': []}
            elif depth == 2: brush = []
        elif s == '}':
            if depth == 2:
                planes = []; texs = []
                for pl in brush:
                    m = PLANE.match(pl)
                    if not m: continue
                    v = list(map(float, m.groups()[:9]))
                    planes.append(plane_of(tuple(v[0:3]), tuple(v[3:6]), tuple(v[6:9])))
                    texs.append(m.group(10))
                vs = tuple(sorted(tuple(round(c, 1) for c in p) for p in vertices(planes)))
                cur['brushes'].append((vs, tuple(sorted(texs))))
            elif depth == 1:
                ents.append(cur)
            depth -= 1
        elif depth == 1 and s.startswith('"'):
            parts = s.split('"')
            if len(parts) >= 5: cur['kv'][parts[1]] = parts[3]
        elif depth == 2 and s.startswith('('):
            brush.append(s)
    return ents


def key(e):
    kv = dict(e['kv'])
    return (kv.get('classname', '?'), tuple(sorted(kv.items())))


def describe(vs):
    xs = [v[0] for v in vs]; ys = [v[1] for v in vs]; zs = [v[2] for v in vs]
    return f'{len(vs)} verts x {min(xs):g}..{max(xs):g} y {min(ys):g}..{max(ys):g} z {min(zs):g}..{max(zs):g}'


def main():
    old, new = parse(sys.argv[1]), parse(sys.argv[2])
    print(f'entities: {len(old)} -> {len(new)}')
    ok, on = Counter(key(e) for e in old), Counter(key(e) for e in new)
    for k in sorted(set(ok) | set(on)):
        if ok[k] != on[k]:
            print(f'  entity {k[0]} {dict(k[1])}: {ok[k]} -> {on[k]}')
    ob = Counter(b for e in old for b in e['brushes'])
    nb = Counter(b for e in new for b in e['brushes'])
    print(f'brushes: {sum(ob.values())} -> {sum(nb.values())}')
    for b in ob:
        if nb[b] < ob[b]:
            print(f'  removed x{ob[b]-nb[b]}: {describe(b[0])} {sorted(set(b[1]))}')
    for b in nb:
        if nb[b] > ob[b]:
            print(f'  added   x{nb[b]-ob[b]}: {describe(b[0])} {sorted(set(b[1]))}')
    # brushes that changed entity ownership
    oo = Counter((e['kv'].get('classname'), b) for e in old for b in e['brushes'])
    no = Counter((e['kv'].get('classname'), b) for e in new for b in e['brushes'])
    moved = [(k, oo[k], no[k]) for k in set(oo) | set(no) if oo[k] != no[k] and ob[k[1]] == nb[k[1]]]
    for (cls, b), a, c in moved:
        print(f'  brush {describe(b[0])} owner {cls}: {a} -> {c}')


if __name__ == '__main__':
    main()

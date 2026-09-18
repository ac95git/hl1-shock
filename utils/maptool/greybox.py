"""Generates a greybox .map from a room spec, and draws its plan before any .map exists.

    python greybox.py SPEC.rooms.txt [OUT.map] [--plan OUT.png] [--cuts Z[,Z...]]

The construction is greybox_proving.py's and greybox_minemap.py's, lifted out of the code so
the layout is a document (docs/MAP_WORKFLOW.md, "The loop for a new map"): every interior volume
gets six wall slabs, every volume is subtracted from every slab, so two volumes that touch are
open to each other and nothing else seals the map. A doorway is a small room that touches two
big ones. Everything is axis-aligned boxes. With --plan and no OUT.map only the plan is drawn,
which is the loop's first step; the .map is written once the plan is agreed. Once J.A.C.K. has
saved the .map the spec is the record of the intent and this script is not re-run on it.

The spec, one item per line, `#` comments, key=value options after the numbers (quote a value
with spaces: prompt_title="Arena door"). Coordinates are map units, x east, y north, z up, and
absolute -- or `@ROOM dx dy dz`, offsets from that room's minimum corner, for anything placed
inside a room. The header:

    map NAME                      the map's name (worldspawn message unless `message` is given)
    message TEXT
    wad A;B;C                     WAD paths for worldspawn; default valve's four
    wall TEX / floor TEX / ceil TEX   the default textures of every wall, floor and ceiling face
    brief TEXT                    the map's purpose; continuation lines start with whitespace
    entrance TEXT / exit TEXT
    gate NAME hard                every gate gets a line, per the roadmap's rule
    gate NAME soft: intended X, alternative Y

The geometry and the entities:

    room NAME x0 y0 z0 x1 y1 z1 [tex=T]        an interior volume; tex= is one texture all round (a duct)
    solid x0 y0 z0 x1 y1 z1 [tex=T]            a world brush inside a room, never subtracted:
                                               a ledge, a pillar, a wall of cover, a crate that must not break
    stairs x0 y0 z0 x1 y1 z1 dir=+x [tread=32] [rise=16] [tex=T]
                                               stepped solids climbing along dir (+x -x +y -y), the first
                                               tread `rise` above z0, the last reaching z1
    brushent CLASS x0 y0 z0 x1 y1 z1 tex=T [key=value ...]
                                               any brush entity: func_door, func_button, func_breakable,
                                               func_deposit, func_station, trigger_once, trigger_autosave
    door x0 y0 z0 x1 y1 z1 [tex=T] [key=value ...]
                                               a func_door with the generators' defaults: rises (angles
                                               "0 -1 0"), speed 100, lip 8, movesnd 2, stopsnd 1, wait 4,
                                               or wait -1 when a targetname is given
    point CLASS x y z [key=value ...]          any point entity
    pickup CLASS x y z [key=value ...]         a point entity, and counted as a pickup in the summary
    encounter CLASS x y z [key=value ...]      a point entity, and counted as an encounter
    light x y z [R G B BRIGHT] [style=N]       default 255 255 255 200
    node x y z                                 an info_node; monsters need them

Wall thickness is 32. Textures are names in the WADs listed; CSG stops on one it cannot find.
"""

import io
import os
import shlex
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import mapplan  # noqa: E402

T = 32  # wall thickness

DEFAULT_WADS = ";".join(
    "/apps/steam/steamapps/common/Half-Life/valve/" + w
    for w in ("halflife.wad", "liquids.wad", "xeno.wad", "decals.wad")
)
DEFAULT_TEX = {"wall": "-0CRETE4_WALL01", "floor": "LAB1_FLOOR1", "ceil": "LAB1_FLOOR3"}
DOOR_DEFAULTS = [("angles", "0 -1 0"), ("speed", "100"), ("lip", "8"), ("movesnd", "2"), ("stopsnd", "1")]
HEADER_TEXT = ("map", "message", "wad", "wall", "floor", "ceil", "brief", "entrance", "exit", "gate")


class Box:
    __slots__ = ("x0", "y0", "z0", "x1", "y1", "z1", "tex")

    def __init__(self, x0, y0, z0, x1, y1, z1, tex=None):
        self.x0, self.y0, self.z0 = x0, y0, z0
        self.x1, self.y1, self.z1 = x1, y1, z1
        self.tex = tex  # None = wall/floor/ceiling by face, str = one texture all round

    def intersects(self, o):
        return (self.x0 < o.x1 and o.x0 < self.x1 and
                self.y0 < o.y1 and o.y0 < self.y1 and
                self.z0 < o.z1 and o.z0 < self.z1)

    def minus(self, o):
        """Axis-aligned subtraction: the parts of self not inside o, as up to six boxes."""
        if not self.intersects(o):
            return [self]
        out = []
        ix0, ix1 = max(self.x0, o.x0), min(self.x1, o.x1)
        iy0, iy1 = max(self.y0, o.y0), min(self.y1, o.y1)
        iz0, iz1 = max(self.z0, o.z0), min(self.z1, o.z1)
        if self.x0 < ix0:
            out.append(Box(self.x0, self.y0, self.z0, ix0, self.y1, self.z1, self.tex))
        if ix1 < self.x1:
            out.append(Box(ix1, self.y0, self.z0, self.x1, self.y1, self.z1, self.tex))
        if self.y0 < iy0:
            out.append(Box(ix0, self.y0, self.z0, ix1, iy0, self.z1, self.tex))
        if iy1 < self.y1:
            out.append(Box(ix0, iy1, self.z0, ix1, self.y1, self.z1, self.tex))
        if self.z0 < iz0:
            out.append(Box(ix0, iy0, self.z0, ix1, iy1, iz0, self.tex))
        if iz1 < self.z1:
            out.append(Box(ix0, iy0, iz1, ix1, iy1, self.z1, self.tex))
        return out


def face(p1, p2, p3, tex, u, v):
    return "( %d %d %d ) ( %d %d %d ) ( %d %d %d ) %s [ %d %d %d 0 ] [ %d %d %d 0 ] 0 1 1" % (
        *p1, *p2, *p3, tex, *u, *v)


def brush(b, tex):
    x0, y0, z0, x1, y1, z1 = b.x0, b.y0, b.z0, b.x1, b.y1, b.z1
    if b.tex:
        wall = floor = ceil = b.tex
    else:
        wall, floor, ceil = tex["wall"], tex["floor"], tex["ceil"]
    X = ((0, 1, 0), (0, 0, -1))
    Y = ((1, 0, 0), (0, 0, -1))
    Z = ((1, 0, 0), (0, -1, 0))
    faces = [
        face((x0, y0, z1), (x0, y0, z0), (x0, y1, z1), wall, *X),   # -x
        face((x1, y1, z1), (x1, y1, z0), (x1, y0, z1), wall, *X),   # +x
        face((x0, y1, z1), (x0, y1, z0), (x1, y1, z1), wall, *Y),   # +y
        face((x1, y0, z1), (x1, y0, z0), (x0, y0, z1), wall, *Y),   # -y
        face((x1, y0, z1), (x0, y0, z1), (x1, y1, z1), floor, *Z),  # +z (top of a solid: a floor)
        face((x0, y1, z0), (x0, y0, z0), (x1, y1, z0), ceil, *Z),   # -z (underside: a ceiling)
    ]
    return "{\n" + "\n".join(faces) + "\n}\n"


def entity(keys, brushes, tex):
    s = "{\n"
    for k, v in keys:
        s += '"%s" "%s"\n' % (k, v)
    for b in brushes:
        s += brush(b, tex)
    return s + "}\n"


def shell(v):
    t = v.tex
    return [
        Box(v.x0 - T, v.y0 - T, v.z0 - T, v.x1 + T, v.y1 + T, v.z0, t),   # floor
        Box(v.x0 - T, v.y0 - T, v.z1, v.x1 + T, v.y1 + T, v.z1 + T, t),   # ceiling
        Box(v.x0 - T, v.y0 - T, v.z0, v.x0, v.y1 + T, v.z1, t),           # west
        Box(v.x1, v.y0 - T, v.z0, v.x1 + T, v.y1 + T, v.z1, t),           # east
        Box(v.x0 - T, v.y0 - T, v.z0, v.x1 + T, v.y0, v.z1, t),           # south
        Box(v.x0 - T, v.y1, v.z0, v.x1 + T, v.y1 + T, v.z1, t),           # north
    ]


def stairs(b, direction, tread, rise, tex):
    """Steps climbing along `direction` from z0 to z1 across the box's length in that axis."""
    steps = []
    along_x = direction[1] == "x"
    length = (b.x1 - b.x0) if along_x else (b.y1 - b.y0)
    n = max(1, int(length // tread))
    height = b.z1 - b.z0
    for i in range(n):
        top = min(b.z1, b.z0 + rise * (i + 1)) if rise * n >= height else b.z0 + height * (i + 1) / n
        lo, hi = i * tread, (i + 1) * tread
        if direction[0] == "-":
            lo, hi = length - hi, length - lo
        if along_x:
            steps.append(Box(b.x0 + lo, b.y0, b.z0 - T, b.x0 + hi, b.y1, int(top), tex))
        else:
            steps.append(Box(b.x0, b.y0 + lo, b.z0 - T, b.x1, b.y0 + hi, int(top), tex))
    return steps


class Spec:
    def __init__(self):
        self.header = {"map": "greybox", "wad": DEFAULT_WADS}
        self.tex = dict(DEFAULT_TEX)
        self.brief = []
        self.gates = []
        self.rooms = {}
        self.solids = []
        self.ents = []           # (classname, keys, [Box]) with keys a list of pairs, brushes maybe empty
        self.pickups = 0
        self.encounters = 0


def parse_spec(text):
    spec = Spec()
    last_text_key = None
    for lineno, raw in enumerate(text.splitlines(), 1):
        if raw.strip() == "" or raw.lstrip().startswith("#"):
            continue
        if raw[0].isspace() and last_text_key:
            spec.brief.append((last_text_key, raw.strip()))
            continue
        last_text_key = None
        try:
            toks = shlex.split(raw, comments=True)
        except ValueError as e:
            sys.exit("%d: %s" % (lineno, e))
        if not toks:
            continue
        kw, rest = toks[0].lower(), toks[1:]
        try:
            if kw in HEADER_TEXT:
                _header(spec, kw, raw, rest)
                if kw in ("brief", "entrance", "exit"):
                    last_text_key = kw
            elif kw == "room":
                name = rest[0]
                b, opts = _box(spec, rest[1:])
                b.tex = opts.pop("tex", None)
                _no_extra(opts, lineno)
                spec.rooms[name] = b
            elif kw == "solid":
                b, opts = _box(spec, rest)
                b.tex = opts.pop("tex", spec.tex["wall"])
                _no_extra(opts, lineno)
                spec.solids.append(b)
            elif kw == "stairs":
                b, opts = _box(spec, rest)
                d = opts.pop("dir", "+x")
                if d not in ("+x", "-x", "+y", "-y"):
                    raise ValueError("dir must be one of +x -x +y -y")
                spec.solids.extend(stairs(b, d, int(opts.pop("tread", 32)), int(opts.pop("rise", 16)),
                                         opts.pop("tex", spec.tex["wall"])))
                _no_extra(opts, lineno)
            elif kw in ("brushent", "door"):
                if kw == "brushent":
                    cls, rest = rest[0], rest[1:]
                else:
                    cls = "func_door"
                b, opts = _box(spec, rest)
                b.tex = opts.pop("tex", spec.tex["wall"])
                keys = [("classname", cls)]
                if kw == "door":
                    keys += DOOR_DEFAULTS
                    keys.append(("wait", "-1" if "targetname" in opts else "4"))
                keys += _keys(opts, keys)
                spec.ents.append((cls, keys, [b]))
            elif kw in ("point", "pickup", "encounter"):
                cls, rest = rest[0], rest[1:]
                (x, y, z), opts = _xyz(spec, rest)
                keys = [("classname", cls), ("origin", "%d %d %d" % (x, y, z))] + _keys(opts, [])
                spec.ents.append((cls, keys, []))
                spec.pickups += kw == "pickup"
                spec.encounters += kw == "encounter"
            elif kw == "light":
                (x, y, z), opts = _xyz(spec, rest)
                colour = " ".join(str(int(float(o))) for o in opts.pop("_pos", [])) or "255 255 255 200"
                keys = [("classname", "light"), ("origin", "%d %d %d" % (x, y, z)), ("_light", colour)]
                keys += _keys(opts, keys)
                spec.ents.append(("light", keys, []))
            elif kw == "node":
                (x, y, z), opts = _xyz(spec, rest)
                _no_extra(opts, lineno)
                spec.ents.append(("info_node", [("classname", "info_node"),
                                                ("origin", "%d %d %d" % (x, y, z))], []))
            else:
                raise ValueError("unknown keyword %r" % kw)
        except (ValueError, IndexError, KeyError) as e:
            sys.exit("%d: %s\n    %s" % (lineno, e, raw.strip()))
    return spec


def _header(spec, kw, raw, rest):
    value = raw.split(None, 1)[1].strip() if len(raw.split(None, 1)) > 1 else ""
    if kw in ("wall", "floor", "ceil"):
        spec.tex[kw] = value
    elif kw == "gate":
        spec.gates.append(value)
    elif kw in ("brief", "entrance", "exit"):
        spec.brief.append((kw, value))
    else:
        spec.header[kw] = value


def _split(spec, rest):
    """(numbers, options) where options is a dict of key=value tokens and `_pos` the positional
    tokens after the coordinates; an @ROOM prefix moves the origin to that room's min corner."""
    origin = (0, 0, 0)
    if rest and rest[0].startswith("@"):
        room = spec.rooms[rest[0][1:]]
        origin = (room.x0, room.y0, room.z0)
        rest = rest[1:]
    nums, opts, pos = [], {}, []
    for t in rest:
        if "=" in t and not t.lstrip("-").replace(".", "").isdigit():
            k, v = t.split("=", 1)
            opts[k] = v
        elif not opts and not pos:
            try:
                nums.append(float(t))
            except ValueError:
                pos.append(t)
        else:
            pos.append(t)
    return nums, opts, pos, origin


def _box(spec, rest):
    nums, opts, pos, o = _split(spec, rest)
    if len(nums) < 6:
        raise ValueError("a box needs six numbers")
    if pos:
        raise ValueError("unexpected %r" % pos)
    n = [int(v) for v in nums[:6]]
    b = Box(n[0] + o[0], n[1] + o[1], n[2] + o[2], n[3] + o[0], n[4] + o[1], n[5] + o[2])
    if b.x0 >= b.x1 or b.y0 >= b.y1 or b.z0 >= b.z1:
        raise ValueError("a box's second corner must be greater on every axis")
    return b, opts


def _xyz(spec, rest):
    nums, opts, pos, o = _split(spec, rest)
    if len(nums) < 3:
        raise ValueError("a position needs three numbers")
    if len(nums) > 3:
        opts["_pos"] = nums[3:]
    if pos:
        raise ValueError("unexpected %r" % pos)
    return (int(nums[0]) + o[0], int(nums[1]) + o[1], int(nums[2]) + o[2]), opts


def _keys(opts, existing):
    have = {k for k, _ in existing}
    out = []
    for k, v in opts.items():
        if k.startswith("_"):
            continue
        if k in have:
            out[:] = [(kk, vv) for kk, vv in out if kk != k]
            existing[:] = [(kk, vv) for kk, vv in existing if kk != k]
        out.append((k, v))
    return out


def _no_extra(opts, lineno):
    extra = [k for k in opts if not k.startswith("_")]
    if extra:
        raise ValueError("unknown option(s) %s" % ", ".join(extra))


def build(spec):
    interiors = list(spec.rooms.values())
    world = []
    for v in interiors:
        for slab in shell(v):
            pieces = [slab]
            for cut in interiors:
                if cut is v:
                    continue
                pieces = [p for piece in pieces for p in piece.minus(cut)]
            world.extend(pieces)
    world.extend(spec.solids)

    out = io.StringIO()
    message = spec.header.get("message", spec.header["map"])
    out.write(entity([("classname", "worldspawn"), ("mapversion", "220"), ("wad", spec.header["wad"]),
                      ("MaxRange", "4096"), ("message", message)], world, spec.tex))
    for cls, keys, brushes in spec.ents:
        out.write(entity(keys, brushes, spec.tex))
    return out.getvalue(), len(world)


def main(argv):
    cuts = [40.0]
    if "--cuts" in argv:
        i = argv.index("--cuts")
        cuts = [float(c) for c in argv[i + 1].split(",")]
        del argv[i:i + 2]
    plan = None
    if "--plan" in argv:
        i = argv.index("--plan")
        plan = argv[i + 1]
        del argv[i:i + 2]
    if not argv or len(argv) > 2:
        sys.exit(__doc__.split("\n\n")[0] + "\n" + __doc__.split("\n")[2])
    spec = parse_spec(open(argv[0], encoding="utf-8").read())
    text, nbrush = build(spec)
    print("%s: %d rooms, %d world brushes, %d entities, %d pickups, %d encounters, %d gates" % (
        spec.header["map"], len(spec.rooms), nbrush, len(spec.ents), spec.pickups, spec.encounters,
        len(spec.gates)))
    for g in spec.gates:
        print("  gate " + g)
    if len(argv) == 2:
        with open(argv[1], "w", newline="\n") as f:
            f.write(text)
        print("wrote " + argv[1])
    if plan:
        mapplan.draw(mapplan.parse_text(text), plan, cuts)


if __name__ == "__main__":
    main(sys.argv[1:])

"""Generates minemap, the test bed for mining and Stations, as a Valve 220 .map.

Built overnight on 2026-09-18 so the morning test is `map minemap` with no editor work
(docs/OVERNIGHT_BRIEF.md, docs/MORNING_CHECKLIST.md). A copy of greybox_proving.py's
construction with its own room list, per MAP_WORKFLOW.md: every interior volume gets six wall
slabs, every volume is subtracted from every slab, so volumes that touch are open to each other
and nothing else seals the map.

One-shot like its parent: once minemap.map is edited in J.A.C.K., edit the .map, not this. That has
happened: J.A.C.K. saved it on 2026-09-18 with a Pulse Module added, and the Panthereye's den west of
the hall was added the same day to the .map text directly. Re-running this would lose both.

  A  the hall: start, the weapons, loose Shards, the three Stations on the north wall
  K  the closet off A: forty keycards, to fill the Grid (the refusal rows)
  B  the tunnel east of A: five stable deposits -- in the walls, one high, one free-standing,
     one fat at the end
  C  the chamber south of B: two unstable veins, a wall of cover, a zombie and a headcrab to lure
  D  the arena north of B, behind a door a button in the tunnel opens: two melee alien grunts and one
     hornet grunt, placed in sight of each other so they form one mixed squad (added the same night)

Usage:  python greybox_minemap.py [out.map]
"""

import sys

T = 32  # wall thickness

TEX_WALL = "-0TNNL_RCK1"
TEX_FLOOR = "-0OUT_GRVL1"
TEX_CEIL = "-0OUT_RK3"
TEX_VEIN = "CRYS_1A"
TEX_STATION = "+0~GENERIC85"
TEX_COVER = "-0OUT_RK4"
TEX_DOOR = "LAB1_DOOR2B"
TEX_BUTTON = "+0BUTTON1"

WADS = ";".join(
    "/apps/steam/steamapps/common/Half-Life/valve/" + w
    for w in ("halflife.wad", "liquids.wad", "xeno.wad", "decals.wad")
)


class Box:
    __slots__ = ("x0", "y0", "z0", "x1", "y1", "z1", "tex")

    def __init__(self, x0, y0, z0, x1, y1, z1, tex=None):
        self.x0, self.y0, self.z0 = x0, y0, z0
        self.x1, self.y1, self.z1 = x1, y1, z1
        self.tex = tex

    def intersects(self, o):
        return (self.x0 < o.x1 and o.x0 < self.x1 and
                self.y0 < o.y1 and o.y0 < self.y1 and
                self.z0 < o.z1 and o.z0 < self.z1)

    def minus(self, o):
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


def brush(b):
    x0, y0, z0, x1, y1, z1 = b.x0, b.y0, b.z0, b.x1, b.y1, b.z1
    if b.tex:
        wall = floor = ceil = b.tex
    else:
        wall, floor, ceil = TEX_WALL, TEX_FLOOR, TEX_CEIL
    X = ((0, 1, 0), (0, 0, -1))
    Y = ((1, 0, 0), (0, 0, -1))
    Z = ((1, 0, 0), (0, -1, 0))
    faces = [
        face((x0, y0, z1), (x0, y0, z0), (x0, y1, z1), wall, *X),
        face((x1, y1, z1), (x1, y1, z0), (x1, y0, z1), wall, *X),
        face((x0, y1, z1), (x0, y1, z0), (x1, y1, z1), wall, *Y),
        face((x1, y0, z1), (x1, y0, z0), (x0, y0, z1), wall, *Y),
        face((x1, y0, z1), (x0, y0, z1), (x1, y1, z1), floor, *Z),
        face((x0, y1, z0), (x0, y0, z0), (x1, y1, z0), ceil, *Z),
    ]
    return "{\n" + "\n".join(faces) + "\n}\n"


def entity(keys, brushes=()):
    s = "{\n"
    for k, v in keys:
        s += '"%s" "%s"\n' % (k, v)
    for b in brushes:
        s += brush(b)
    return s + "}\n"


# ---------------------------------------------------------------------------
# Interior volumes. x east, y north, z up; the floor is z 0 everywhere.
# ---------------------------------------------------------------------------
V = {}


def vol(name, x0, y0, z0, x1, y1, z1, tex=None):
    V[name] = Box(x0, y0, z0, x1, y1, z1, tex)


vol("A", 0, 0, 0, 768, 512, 192)            # the hall
vol("K", 64, -192, 0, 320, 0, 128)          # the closet, open along A's south wall
vol("B", 768, 128, 0, 1600, 384, 160)       # the tunnel, open along A's east wall
vol("C", 1024, -512, 0, 1536, 0, 224)       # the chamber
vol("CB", 1216, 0, 0, 1344, 128, 112)       # its doorway up into the tunnel
vol("D", 1024, 448, 0, 1536, 960, 192)      # the arena
vol("DB", 1344, 384, 0, 1440, 448, 112)     # its doorway down into the tunnel, closed by a door

interiors = list(V.values())


def shell(v):
    t = v.tex
    return [
        Box(v.x0 - T, v.y0 - T, v.z0 - T, v.x1 + T, v.y1 + T, v.z0, t),
        Box(v.x0 - T, v.y0 - T, v.z1, v.x1 + T, v.y1 + T, v.z1 + T, t),
        Box(v.x0 - T, v.y0 - T, v.z0, v.x0, v.y1 + T, v.z1, t),
        Box(v.x1, v.y0 - T, v.z0, v.x1 + T, v.y1 + T, v.z1, t),
        Box(v.x0 - T, v.y0 - T, v.z0, v.x1 + T, v.y0, v.z1, t),
        Box(v.x0 - T, v.y1, v.z0, v.x1 + T, v.y1 + T, v.z1, t),
    ]


world = []
for v in interiors:
    for slab in shell(v):
        pieces = [slab]
        for cut in interiors:
            if cut is v:
                continue
            pieces = [p for piece in pieces for p in piece.minus(cut)]
        world.extend(pieces)

# C: a wall of cover between the doorway and the free-standing unstable vein. Standing behind it
# is inside the vein's reach and out of its sight (checklist U4).
world.append(Box(1216, -176, 0, 1344, -160, 112, TEX_COVER))

# ---------------------------------------------------------------------------
# Entities
# ---------------------------------------------------------------------------
ents = []


def point(classname, x, y, z, *extra):
    ents.append(entity([("classname", classname), ("origin", "%d %d %d" % (x, y, z))] + list(extra)))


def light(x, y, z, colour="255 255 255 200"):
    point("light", x, y, z, ("_light", colour))


def deposit(b, *extra):
    ents.append(entity([("classname", "func_deposit")] + list(extra), [Box(*b, TEX_VEIN)]))


def station(b, stationtype, *extra):
    ents.append(entity([("classname", "func_station"), ("stationtype", str(stationtype))] + list(extra),
                       [Box(*b, TEX_STATION)]))


# A -- the start and the kit
point("info_player_start", 96, 256, 36, ("angles", "0 0 0"))
point("item_suit", 96, 160, 4)
point("weapon_pickaxe", 192, 192, 4)
point("weapon_crowbar", 192, 256, 4)
point("weapon_katana", 192, 320, 4)
point("weapon_9mmhandgun", 256, 192, 4)
point("ammo_9mmclip", 256, 224, 4)
point("weapon_handgrenade", 256, 288, 4)
point("weapon_gauss", 256, 352, 4)            # shows the uranium count the station pays into
point("weapon_summon", 320, 352, 4)           # and the Cores count

# A -- seven loose Shards under the Stations: with one deposit's three, a Fuel processor's ten
for i in range(7):
    point("item_shard", 272 + 32 * i, 432, 4)

# A -- the Stations, set into the north wall at y 512
station((96, 496, 32, 160, 520, 96), 0)       # Fuel processor
station((288, 496, 32, 352, 520, 96), 1)      # Ammunition station: uranium, three trades
station((480, 496, 32, 544, 520, 96), 2)      # Ammunition station: Cores, three trades

# K -- forty keycards, one Cell each and unique, so walking through the closet fills the Grid
for j in range(5):
    for i in range(8):
        point("item_security", 88 + 28 * i, -176 + 32 * j, 4)

# B -- five stable deposits. The first four are embedded in the walls; the boulder stands free.
deposit((880, 352, 24, 944, 400, 88))                                   # north wall, waist height
deposit((1040, 112, 40, 1104, 160, 104))                                # south wall
deposit((1200, 352, 104, 1264, 400, 152))                               # north wall, high (Shards still land)
deposit((1400, 224, 0, 1464, 288, 56), ("material", "8"))               # a boulder, rock sounds
deposit((1568, 208, 16, 1616, 304, 112), ("yield", "6"), ("health", "200"))  # the fat one at the end

# C -- two unstable veins, out of step with each other
deposit((1248, -288, 0, 1312, -224, 128), ("spawnflags", "8"), ("yield", "5"))   # free-standing
deposit((1512, -440, 40, 1560, -360, 120), ("spawnflags", "8"))                  # in the east wall

# C -- something to lure past them
point("monster_zombie", 1440, -448, 4, ("angles", "0 180 0"))
point("monster_headcrab", 1088, -448, 4, ("angles", "0 0 0"))
for (nx, ny) in ((1088, -64), (1280, -64), (1440, -96), (1088, -320), (1440, -320), (1088, -464),
                 (1280, -464), (1440, -464), (1280, 64), (1280, 256), (1000, 256), (700, 256)):
    point("info_node", nx, ny, 16)

# D -- the arena. A door fills the doorway until the button beside it in the tunnel is pressed, so
# nothing in here joins the mining tests uninvited. Three grunts in sight of each other at spawn: two
# melee, one with hornets. Alien military recruit across classnames, so this is one squad.
ents.append(entity([("classname", "func_door"), ("targetname", "arena_door"), ("angles", "0 -1 0"),
                    ("speed", "100"), ("lip", "8"), ("wait", "-1"), ("movesnd", "2"), ("stopsnd", "1")],
                   [Box(1344, 400, 0, 1440, 432, 112, TEX_DOOR)]))
ents.append(entity([("classname", "func_button"), ("target", "arena_door"), ("spawnflags", "1"),
                    ("wait", "-1"), ("sounds", "1"), ("prompt_title", "Arena door")],
                   [Box(1456, 376, 40, 1488, 384, 72, TEX_BUTTON)]))
point("monster_alien_grunt_melee", 1216, 800, 4, ("angles", "0 270 0"))
point("monster_alien_grunt_melee", 1344, 800, 4, ("angles", "0 270 0"))
point("monster_alien_grunt", 1280, 896, 4, ("angles", "0 270 0"))
for (nx, ny) in ((1088, 512), (1280, 512), (1472, 512), (1088, 704), (1280, 704), (1472, 704),
                 (1088, 896), (1472, 896), (1392, 416)):
    point("info_node", nx, ny, 16)

# lights
for (lx, ly) in ((1152, 576), (1408, 576), (1152, 832), (1408, 832)):
    light(lx, ly, 176, "220 255 220 160")
for (lx, ly) in ((128, 128), (384, 128), (640, 128), (128, 384), (384, 384), (640, 384)):
    light(lx, ly, 176)
light(192, -96, 112, "255 255 255 120")
for lx in (864, 1088, 1312, 1536):
    light(lx, 256, 144, "255 230 200 160")
for (lx, ly) in ((1120, -64), (1440, -64), (1120, -448), (1440, -448)):
    light(lx, ly, 208, "255 220 180 110")

# ---------------------------------------------------------------------------
out = sys.argv[1] if len(sys.argv) > 1 else "minemap.map"
with open(out, "w", newline="\n") as f:
    f.write(entity([("classname", "worldspawn"), ("mapversion", "220"), ("wad", WADS),
                    ("MaxRange", "4096"), ("message", "minemap: mining and Stations")], world))
    for e in ents:
        f.write(e)
print("%s: %d world brushes, %d entities" % (out, len(world), len(ents)))

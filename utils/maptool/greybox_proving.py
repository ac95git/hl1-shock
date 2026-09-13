"""Generates the step-1 greybox of the proving map (docs/PROVING_MAP.md) as a Valve 220 .map.

Run once, open the result in J.A.C.K., and edit there from then on -- this script is the
starting point, not the source of truth. Everything is axis-aligned boxes:

  * an interior VOLUME is empty space (a room, a corridor, a doorway, a duct);
  * every volume gets six wall slabs around it, and every interior volume is subtracted
    from every slab, so two volumes that touch are open to each other and a volume
    that crosses a wall cuts a hole in it. Nothing else seals the map.

Usage:  python greybox_proving.py [out.map]
"""

import sys

T = 32  # wall thickness

TEX_WALL = "-0CRETE4_WALL01"
TEX_FLOOR = "LAB1_FLOOR1"
TEX_CEIL = "LAB1_FLOOR3"
TEX_STEP = "LAB1_STAIR1A"
TEX_CRATE = "BCRATE02"
TEX_GRATE = "{GRATE1"
TEX_DUCT = "DUCT_WALL01"
TEX_PANEL = "DUCT_VNT"
TEX_DOOR = "-0C2A4D_PANEL"
TEX_BUTTON = "+0~GENERIC85"
TEX_TRIGGER = "AAATRIGGER"

WADS = ";".join(
    "/apps/steam/steamapps/common/Half-Life/valve/" + w
    for w in ("halflife.wad", "liquids.wad", "xeno.wad", "decals.wad")
)


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
        face((x0, y0, z1), (x0, y0, z0), (x0, y1, z1), wall, *X),   # -x
        face((x1, y1, z1), (x1, y1, z0), (x1, y0, z1), wall, *X),   # +x
        face((x0, y1, z1), (x0, y1, z0), (x1, y1, z1), wall, *Y),   # +y
        face((x1, y0, z1), (x1, y0, z0), (x0, y0, z1), wall, *Y),   # -y
        face((x1, y0, z1), (x0, y0, z1), (x1, y1, z1), floor, *Z),  # +z (top of a solid: a floor)
        face((x0, y1, z0), (x0, y0, z0), (x1, y1, z0), ceil, *Z),   # -z (underside: a ceiling)
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
# Interior volumes. Coordinates are map units, x east, y north, z up.
# Upper floor is z 0; the dark deck is z -256.
# ---------------------------------------------------------------------------
V = {}


def vol(name, x0, y0, z0, x1, y1, z1, tex=None):
    V[name] = Box(x0, y0, z0, x1, y1, z1, tex)


# A -- airlock, and the short corridor into B
vol("A", -352, 128, 0, -96, 384, 128)
vol("A_B", -96, 224, 0, 0, 288, 96)

# B -- hub, two storeys
vol("B", 0, 0, 0, 512, 512, 192)
vol("B_C", 224, 512, 0, 288, 544, 96)       # north doorway
vol("B_F", 512, 432, 0, 576, 496, 96)       # east doorway, through B's wall and F's end cap
vol("B_H", 224, -64, 0, 288, 0, 96)         # south doorway

# C -- storage
vol("C", 64, 544, 0, 448, 1056, 256)
vol("C_D", 448, 928, 0, 480, 992, 96)

# D -- holding pen
vol("D", 480, 672, 0, 992, 1056, 160)
vol("D_S", 992, 800, 0, 1024, 928, 128)     # opening onto the down stairs

# stairs down, eastward, four segments with a stepping ceiling
for k in range(4):
    vol("SD%d" % k, 1024 + 128 * k, 800, -256, 1152 + 128 * k, 928, 112 - 64 * k)

# E -- the dark deck: a ring around a solid block, plus alcoves
vol("E_N", 1536, 800, -256, 2176, 928, -144)
vol("E_S", 1536, 416, -256, 2176, 544, -144)
vol("E_W", 1536, 416, -256, 1664, 928, -144)
vol("E_E", 2048, 416, -256, 2176, 928, -144)
vol("E_AS", 1728, 288, -256, 1856, 416, -144)   # south alcove
vol("E_AE", 2176, 608, -256, 2304, 736, -144)   # east alcove (357, zombie)

# stairs up, westward, into F
for k in range(4):
    vol("SU%d" % k, 1408 - 128 * k, 416, -256, 1536 - 128 * k, 544, -80 + 64 * k)

# F -- return gallery, and the cache behind its south wall
vol("F", 576, 416, 0, 1024, 544, 128)
vol("F_X", 800, 368, 0, 864, 416, 96)       # the hole the panel fills
vol("X", 736, 176, 0, 928, 368, 128)

# H -- exit
vol("H", 160, -320, 0, 352, -64, 128)

# the vent off D: grille in the north wall, two turns, a nook
vol("VD1", 528, 1056, 0, 576, 1216, 48, TEX_DUCT)
vol("VD2", 528, 1168, 0, 768, 1216, 48, TEX_DUCT)
vol("VN", 768, 1152, 0, 896, 1280, 96, TEX_DUCT)

interiors = list(V.values())


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


world = []
for v in interiors:
    for slab in shell(v):
        pieces = [slab]
        for cut in interiors:
            if cut is v:
                continue
            pieces = [p for piece in pieces for p in piece.minus(cut)]
        world.extend(pieces)

# Solid detail inside rooms (never subtracted -- these are things, not walls)
detail = []

# B: walkway along the east wall and the stair up to it
detail.append(Box(384, 256, 112, 512, 512, 128, TEX_STEP))
for i in range(8):
    detail.append(Box(384, 32 * i, -T, 512, 32 * (i + 1), 16 * (i + 1), TEX_STEP))

# C: the ledge along the west wall, and the two crate steps at its north end
detail.append(Box(64, 544, 96, 192, 1056, 128, TEX_STEP))
detail.append(Box(192, 992, 0, 256, 1056, 48, TEX_CRATE))   # world brushes on purpose:
detail.append(Box(192, 928, 0, 256, 992, 96, TEX_CRATE))    # a breakable step strands the ledge

# D: pillars
for (px, py) in ((600, 780), (760, 900), (760, 720)):
    detail.append(Box(px, py, 0, px + 64, py + 64, 160, TEX_WALL))

# stairs down (east) and up (west): 16 steps of 16 x 32
for i in range(15):
    detail.append(Box(1024 + 32 * i, 800, -288, 1056 + 32 * i, 928, -16 * (i + 1), TEX_STEP))
for j in range(16):
    detail.append(Box(1504 - 32 * j, 416, -288, 1536 - 32 * j, 544, -256 + 16 * (j + 1), TEX_STEP))

world.extend(detail)

# ---------------------------------------------------------------------------
# Entities
# ---------------------------------------------------------------------------
ents = []


def point(classname, x, y, z, *extra):
    ents.append(entity([("classname", classname), ("origin", "%d %d %d" % (x, y, z))] + list(extra)))


def light(x, y, z, colour="255 255 255 200", style=None):
    keys = [("_light", colour)]
    if style is not None:
        keys.append(("style", str(style)))
    point("light", x, y, z, *keys)


def door(name, b, targetname=None, angles="0 -1 0"):
    # "0 -1 0" is the engine's "up"; a yaw slides sideways into the wall instead
    keys = [("classname", "func_door"), ("angles", angles), ("speed", "100"), ("lip", "8"),
            ("movesnd", "2"), ("stopsnd", "1")]
    if targetname:
        keys += [("targetname", targetname), ("wait", "-1")]
    else:
        keys += [("wait", "4")]
    ents.append(entity(keys, [Box(*b, TEX_DOOR)]))


# start
point("info_player_start", -224, 256, 40, ("angles", "0 0 0"))
point("item_suit", -224, 336, 4, ("variant", "0"))
point("weapon_crowbar", -160, 256, 4)
point("weapon_9mmhandgun", -288, 256, 4)
point("ammo_9mmclip", -288, 208, 4)

# the four hub doors
door("A", (-48, 224, 0, -16, 288, 96))
door("N", (224, 512, 0, 288, 544, 96))
door("E", (512, 432, 0, 544, 496, 96), "door_hub_east", angles="0 90 0")  # sideways: the walkway is above it
door("S", (224, -32, 0, 288, 0, 96), "door_hub_exit")

# the button in F that opens them, via a multi_manager
ents.append(entity([("classname", "func_button"), ("target", "mm_unlock"), ("spawnflags", "1"),
                    ("wait", "-1"), ("sounds", "1")],
                   [Box(592, 536, 40, 624, 544, 72, TEX_BUTTON)]))
point("multi_manager", 600, 500, 100, ("targetname", "mm_unlock"),
      ("door_hub_east", "0"), ("door_hub_exit", "0.5"))

# hub
point("item_healthkit", 448, 400, 132)
point("item_healthkit", 448, 480, 132)

# storage
point("weapon_shotgun", 300, 800, 4)
point("ammo_buckshot", 336, 800, 4)
point("item_skillpoint", 400, 960, 4)          # critical path 1
point("item_skillpoint", 128, 700, 132)        # ledge
point("item_syringe", 128, 760, 132)           # ledge

# pen
point("weapon_9mmAR", 600, 900, 4)
point("ammo_9mmAR", 636, 900, 4)
point("weapon_handgrenade", 700, 720, 4)
point("item_battery", 900, 1000, 4)
point("item_battery", 936, 1000, 4)
point("item_syringe", 950, 720, 4)             # critical path

# the vent: a crate over the grille, the grille, a sound and a glow inside, the nook
ents.append(entity([("classname", "func_breakable"), ("material", "1"), ("health", "20")],
                   [Box(512, 992, 0, 576, 1056, 64, TEX_CRATE)]))
ents.append(entity([("classname", "func_breakable"), ("material", "2"), ("health", "1")],
                   [Box(528, 1056, 0, 576, 1064, 48, TEX_GRATE)]))
point("ambient_generic", 552, 1140, 24, ("message", "ambience/wind2.wav"), ("health", "6"),
      ("spawnflags", "34"))
light(552, 1100, 24, "255 255 200 60")
light(832, 1216, 80, "255 255 255 120")
point("item_skillpoint", 832, 1200, 4)         # vent nook
point("item_resettoken", 832, 1240, 4)         # the Reset Token

# dark deck
light(1600, 864, -160, "255 255 255 150", style=10)   # fluorescent flicker at the stair foot
for (lx, ly) in ((2112, 864), (2112, 480), (1600, 480)):
    light(lx, ly, -160, "255 0 0 30")
light(1568, 480, -160, "255 255 255 100")               # the lit alcove at the up stairs
point("item_skillpoint", 1568, 480, -252)      # critical path 2
ents.append(entity([("classname", "trigger_autosave")],
                   [Box(1536, 416, -256, 1600, 544, -160, TEX_TRIGGER)]))
point("weapon_357", 2240, 672, -252)
point("ammo_357", 2272, 672, -252)

# gallery and cache
point("weapon_crossbow", 800, 480, 4)
point("ammo_crossbow", 836, 480, 4)
ents.append(entity([("classname", "func_breakable"), ("material", "2"), ("health", "50")],
                   [Box(800, 400, 0, 864, 416, 96, TEX_PANEL)]))
point("item_rowgrant", 832, 240, 4)
point("item_skillpoint", 832, 300, 4)          # cache
ents.append(entity([("classname", "func_breakable"), ("material", "1"), ("health", "20"),
                    ("spawnobject", "1")],                     # 1 = Battery in the FGD's Spawn On Break list
                   [Box(752, 192, 0, 816, 256, 64, TEX_CRATE)]))

# exit
ents.append(entity([("classname", "trigger_once"), ("target", "mm_end")],
                   [Box(160, -320, 0, 352, -224, 96, TEX_TRIGGER)]))
point("multi_manager", 256, -280, 100, ("targetname", "mm_end"), ("text_end", "0"), ("print_end", "0"))
point("game_text", 256, -280, 80, ("targetname", "text_end"), ("message", "The proving map ends here."),
      ("x", "-1"), ("y", "0.6"), ("effect", "0"), ("color", "255 255 255"), ("color2", "0 0 0"),
      ("fadein", "0.5"), ("fadeout", "1"), ("holdtime", "5"), ("fxtime", "0"), ("channel", "2"),
      ("spawnflags", "1"))
point("trigger_print", 256, -280, 60, ("targetname", "print_end"), ("message", "proving: end reached"))

# lights everywhere the dark deck is not
light(-224, 256, 112)
for (lx, ly) in ((128, 128), (384, 128), (128, 384), (256, 384)):
    light(lx, ly, 176)
light(256, 700, 240)
light(256, 900, 240)
light(620, 820, 144)
light(880, 800, 144)
light(1088, 864, 96)
light(1472, 864, -96)
light(1088, 480, -96)
light(1472, 480, -224)
light(700, 480, 112)
light(900, 480, 112)
light(832, 272, 112)
light(256, -192, 112)
light(-48, 256, 80, "255 255 255 100")

# ---------------------------------------------------------------------------
out = sys.argv[1] if len(sys.argv) > 1 else "proving.map"
with open(out, "w", newline="\n") as f:
    f.write(entity([("classname", "worldspawn"), ("mapversion", "220"), ("wad", WADS),
                    ("MaxRange", "4096"), ("message", "The proving map (greybox)")], world))
    for e in ents:
        f.write(e)
print("%s: %d world brushes, %d entities" % (out, len(world), len(ents)))

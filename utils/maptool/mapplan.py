"""Draws a top-down plan of a Valve 220 .map and prints what it places.

    python mapplan.py IN.map OUT.png [--cuts Z[,Z...]]

An architect's plan: a horizontal section at each cut height. A world brush is drawn (as its
bounding rectangle) only where a cut passes through it, so floors and ceilings vanish and walls,
pillars, crates and stairs remain; the first cut is orange, the second blue, and so on. Default
is one cut at 40, eye height of a crouched player on a floor at 0; a two-storey map wants one
cut per storey, e.g. --cuts 40,-216. Brush entities are outlined green and labelled at any
height; point entities are red dots with their classname (lights and logic entities are skipped
so the plan stays readable). Brushes that are not axis-aligned boxes are drawn by the bounds of
their face points, which is right for the plan even if it is not the shape. Needs Pillow.

This is how the agent looks at a map without launching the game. It shows layout and placement;
it cannot show height, light or texture.
"""

import re
import sys
from collections import Counter

from PIL import Image, ImageDraw

SKIP_POINT = {"light", "light_spot", "multi_manager", "game_text", "trigger_print", "ambient_generic",
              "env_sound", "info_node", "path_corner", "multisource", "trigger_relay"}


def parse(path):
    return parse_text(open(path, encoding="latin1").read())


def parse_text(text):
    """Entities of a Valve 220 .map given as text: a list of (keys, [brush bounds])."""
    ents = []
    i = 0
    n = len(text)
    while i < n:
        if text[i] != "{":
            i += 1
            continue
        # entity
        i += 1
        keys = {}
        brushes = []
        while i < n:
            c = text[i]
            if c == "}":
                i += 1
                break
            if c == '"':
                m = re.compile(r'"([^"]*)"\s+"([^"]*)"').match(text, i)
                keys[m.group(1)] = m.group(2)
                i = m.end()
            elif c == "{":
                j = text.index("}", i)
                pts = re.findall(r"\(\s*(-?[\d.]+)\s+(-?[\d.]+)\s+(-?[\d.]+)\s*\)", text[i:j])
                xs = [float(p[0]) for p in pts]
                ys = [float(p[1]) for p in pts]
                zs = [float(p[2]) for p in pts]
                brushes.append((min(xs), min(ys), min(zs), max(xs), max(ys), max(zs)))
                i = j + 1
            else:
                i += 1
        ents.append((keys, brushes))
    return ents


def main():
    cuts = [40.0]
    if "--cuts" in sys.argv:
        i = sys.argv.index("--cuts")
        cuts = [float(c) for c in sys.argv[i + 1].split(",")]
        del sys.argv[i:i + 2]
    src, out = sys.argv[1], sys.argv[2]
    draw(parse(src), out, cuts)


def draw(ents, out, cuts=(40.0,)):
    """Prints the classname count and writes the plan PNG. greybox.py calls this on a map it has
    only generated in memory, so a plan exists before any .map is on disk."""
    counts = Counter(k.get("classname", "?") for k, _ in ents)
    for name, c in sorted(counts.items(), key=lambda t: (-t[1], t[0])):
        print("%4d  %s" % (c, name))

    allb = [b for _, bs in ents for b in bs]
    x0 = min(b[0] for b in allb) - 64
    x1 = max(b[3] for b in allb) + 64
    y0 = min(b[1] for b in allb) - 64
    y1 = max(b[4] for b in allb) + 64
    scale = min(1.0, 1600.0 / max(x1 - x0, y1 - y0))
    W, H = int((x1 - x0) * scale), int((y1 - y0) * scale)
    im = Image.new("RGB", (W, H), "white")
    d = ImageDraw.Draw(im)

    def P(x, y):
        return (int((x - x0) * scale), int((y1 - y) * scale))

    colours = [(253, 208, 162), (158, 202, 225), (199, 233, 192), (218, 218, 235)]
    world = ents[0][1]
    for ci, cut in reversed(list(enumerate(cuts))):
        col = colours[ci % len(colours)]
        for b in world:
            if b[2] <= cut < b[5]:
                d.rectangle([P(b[0], b[4]), P(b[3], b[1])], fill=col, outline=(90, 90, 90))
    for keys, bs in ents[1:]:
        cls = keys.get("classname", "?")
        if bs:
            bx0 = min(b[0] for b in bs); by0 = min(b[1] for b in bs)
            bx1 = max(b[3] for b in bs); by1 = max(b[4] for b in bs)
            d.rectangle([P(bx0, by1), P(bx1, by0)], outline=(0, 120, 0), width=2)
            d.text(P(bx0, by1 + 14), cls + (" " + keys["targetname"] if "targetname" in keys else ""),
                   fill=(0, 100, 0))
        elif "origin" in keys and cls not in SKIP_POINT:
            x, y = [float(v) for v in keys["origin"].split()[:2]]
            px, py = P(x, y)
            d.ellipse([px - 3, py - 3, px + 3, py + 3], fill="red")
            d.text((px + 4, py - 6), cls.replace("item_", "").replace("weapon_", ""), fill=(120, 0, 0))
    im.save(out)
    print(out)


if __name__ == "__main__":
    main()

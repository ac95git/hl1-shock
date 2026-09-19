# Craft log

Andrei's hand-made work, one entry per session, newest first. Started 2026-09-20 when the split was
reset: the agent had written nearly every line of code, layout and doc for ten days, and Andrei's part
had shrunk to deciding and judging. The remedy chosen that day: sessions open with his sentence, not a
menu from the roadmap; he writes the intent (a rooms spec, the "what I want" half of a model brief); he
tunes in the console; the next map is blocked out by him in J.A.C.K. with no generator; and he writes a
code feature now and then with the agent as reviewer. Mapping he ranks first ("the single most crucial
point"), modelling second (the three weapons, then improving existing models; no monster from scratch).

The two ladders, each rung one technique on one room, checked by a read-back and a compile:

**Mapping, on `shaft1`** — 1 texture fit and alignment on leg A; 2 trim and timber in the old workings
(clipping tool, bevels, the 16 grid); ~~3 a shape that is not a box: the chamber's pillars~~ done below;
4 lighting (texture lights through `lights.rad`, lamp brushes, colour and falloff); 5 detail from brushes
(the cars, the cage, the ladders; when a brush is a `func_wall` or `func_detail`); 6 scripting in the
editor (the crew's `scripted_sequence` and sentence triggers wired in the entity dialog); 7 the autonomy
milestone: map two greyboxed by Andrei from a plan he drew, no generator.

**Modelling** — 1 the pickaxe's second cut (thin it, fix the head's form; steps in its brief); 2 the alien
grunt's melee weapon from nothing (mirror modifier, loop cuts, seams, a 256-colour texture); 3 the energy
rifle (several parts, weights to the hand bone, world and player versions); 4 improving existing models,
textures first, then a mesh edit on a decompile — the freed slave without his collar and bracelets is the
first real customer.

How the agent takes part: it writes the steps (tool, dialog, number), reads the saved file back
(`utils/maptool/mapsemdiff.py` and `brushes_near.py` for a map; the Blender MCP for a scene), compiles,
and installs. It does not build the thing.

---

## 2026-09-20 — the chamber's four crystal pillars (mapping rung 3)

**Built by Andrei in J.A.C.K.:** the four 64×64×288 boxes at `shaft1`'s chamber corners replaced with
crystals, each a six-sided prism (z -192 to about 123) under a six-sided point (to 224), 64 across, in
`CRYS_3A`, each pair tied to its own `func_detail`. One built by hand, three cloned with Shift-drag on
the 64 grid. Compiled clean, installed, judged in game, committed.

**What was learned:**

- **A cylinder or spike is built in the plane of the 2D view that is active when Enter is pressed.**
  Drag the footprint in the top view and press Enter there; set the height afterwards by resizing the
  finished brush in the front view. The first attempt had the box dragged in two views, and came out as
  a hexagonal bar lying on its side with the spike pointing east.
- **Resizing scales, dragging an edge moves it.** The prism's floor came out at -171 and, fixed by a
  resize, its top moved to 122.9. Harmless where two brushes meet face to face (the compiler removes
  both faces), but a seam meant to sit on the grid wants the edge dragged.
- **A six-sided prism in a 64 box lands every vertex on the 16 grid**: points at ±32 on one axis,
  ±16/±32 on the other. Nothing to fix.
- **`func_detail` is already in the editor**: J.A.C.K. loads the compiler's own `zhlt.fgd` beside
  `top_mod.fgd` (`VDKGameCfg.ini`), so the mod's FGD did not need it. A slanted world brush chops every
  face it touches; as detail it does not, and a detail brush inside a sealed room cannot leak.
- **The chamber's crystal was purple.** Every pillar and deposit carried `CRYS_1A`, and in `xeno.wad`
  that family is violet. Decision 7 says amber; Valve's amber set is `CRYS_3A`/`CRYS_3B` (sides, 80×128)
  with `CRYS_3TOP` (80×80). The pillars are amber now; the four deposits still carry `CRYS_1A` and are
  the texture rung's first job.
- **J.A.C.K.'s manual is at `D:\Apps\J.A.C.K\VDKManual.pdf`** (89 pages, text extractable with `pypdf`).
  Its hotkey list is on pages 81–83. The agent's editor steps come from it now, not from memory.

**Next rung, Andrei's pick:** the deposits' texture (rung 1, on the chamber rather than leg A), or the
timber in the old workings (rung 2).

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

**Modelling** — ~~1 the pickaxe: second cut (thin it, fix the head's form), third cut (form and
chrome groups)~~ both done below, only taste left; 2 the alien
grunt's melee weapon from nothing (mirror modifier, loop cuts, seams, a 256-colour texture); 3 the energy
rifle (several parts, weights to the hand bone, world and player versions); 4 improving existing models,
textures first, then a mesh edit on a decompile — the freed slave without his collar and bracelets is the
first real customer.

**Animation, in Blender** — ~~1 keyframes on an object; 2 the Graph Editor and the three interpolations;
3 an armature: build bones in Edit Mode, bind with automatic weights, key in Pose Mode; 4 Actions, one
per sequence, swapped in the Action Editor~~ all four done below on a block; 5 an Action through Source
Tools to `anims/*.smd`, the converter, a QC `$sequence` and HLMV (the gap MODEL_WORKFLOW.md names as
unexplored); 6 weights by hand, one bone per vertex; 7 a sequence on a real rig, a stock weapon's hands
or the miner's swing.

How the agent takes part: it writes the steps (tool, dialog, number), reads the saved file back
(`utils/maptool/mapsemdiff.py` and `brushes_near.py` for a map; the Blender MCP for a scene), compiles,
and installs. It does not build the thing.

---

## 2026-09-21 — the pickaxe's third cut: form where the screen can see it (modelling rung 1, closed)

**Built by Andrei** across a long day in `v_pickaxe.blend`, opened on his sentence "pickaxe round 3".
503 → **827 triangles**, 11 bones, 12 sequences, the gloves untouched. His verdict at the end: "the
pickaxe look really good… now it is up to taste."

**The shaft.** The second cut's three soft swells each got a plain-radius flank ring 0.45 either
side, turning a shoulder spread over 3.4 units into one spread over 0.9, and the crests went up to
0.80 and 0.78. Then the measurement that changed the plan: the fist covers z −7.79 to −4.05, but the
*rendered frame* cuts the shaft at about **z 1.1** at rest, so three of those four features were
built where no one would ever see them. Three grooves went on the open run instead — z 5.166, 1.749,
−1.669, on the shaft's own 3.42 rhythm — and the rib inside the fist was flattened back to the
taper, which also removed a poke-through against the glove's fingers. Later, the ring under the head
became a proper **ferrule**: two more loops at r 0.800 making a 0.37-tall cylinder with 46° shoulders
instead of a 32° cone, and its 32 vertical edges marked sharp so the collar reads as an eight-sided
fitting on a round haft.

**The head.** The two 22-gons were triangulated by hand so both cheeks export the same fan. The body
got a **fuller**, which Andrei had identified as the offender before any measurement agreed with him;
later he reshaped it from the outline-following pentagon into a parallelogram, which halved its area
to match the blade's panels. He modelled **the back of the head himself** — two insets and an
extrusion — and it came back with every face mirrored exactly. The blade's cutting edge was halved in
thickness and two recessed panels cut into the inner sweep, both his placements. 153 of the head's
211 interior edges are creased at a **13°** threshold taken from a measured gap in the distribution.

**The textures.** `chrome_map.py` is new: a matcap generator with a hot spot, a horizon, a rim light,
a `--step` and a `--cast`. The head became four chrome groups — edge, blade, body, haft — and the
ferrule a fifth, carrying the Suit Variant accent as a new column in the QC's `$texturegroup`.

**What was learned:**

- **A recess's wall angle is `sink ÷ inset width`, not depth.** A chrome map is sampled by the
  normal, so it reads *angle* and is blind to how deep a cut is. That single fact decided every
  recess on the model: the body's fuller works at 0.07 deep because its ring is 0.12 wide (30°),
  and a first attempt on the blade at 0.015 deep with a 0.35 ring gave **2.4°** — flatter than the
  4.1° cheek it was cut into, and would have vanished. On thin geometry, **narrow the ring; never
  deepen the cut.**
- **Measure what the screen sees, not what the mesh has.** Two different limits, and the tighter one
  was not the obvious one: the glove hides z −7.79…−4.05, but the frame's bottom edge at rest sits at
  **z ≈ 1.1**. Three shaft features and the butt band were invisible before that was checked. Read
  the pose render against the mesh coordinates before siting anything.
- **A chrome map with no range throws geometry away.** `pickaxe_black.py`'s palette curve (floor 10,
  ceil 175, gamma 1.7) put the shaft's map at **mean 23/255, max 132**, so two opposite walls of a
  groove sampled tones a few levels apart. Andrei spotted it by eye — "the black chrome doesn't do it
  honor in reading the shapes" — before it was measured. And a *gradient* horizon is not enough: the
  first map had only a 6-level break across it. `--step` puts a real discontinuity there, which is
  the only thing that lets the blade's 4.1–4.5° cheek flip tone through a swing.
- **`Select All by Trait` extends the selection, it does not replace it.** Press `A` first and Mark
  Sharp creases the whole mesh. This cost two full passes in one day. The sequence is always
  `Alt+A` → trait select → Mark Sharp; the only `A` belongs to Reset Vectors at the end.
- **The smoothing threshold is a measured gap.** The head's edge angles clustered at ≤11.3° and
  ≥14.3° with nothing between, so 13° split form from sweep with **zero** edges misclassified either
  way. Same discipline as the second cut's 60°, different number, because it is a different mesh.
- **In Material Properties, `New` replaces the highlighted slot.** It does not add one — `+` does. Hit
  in the wrong order it put the new material into the *shaft's* slot (173 faces on the edge texture),
  orphaned `pickshaftChrome` where a save would have purged it, and renamed the **object** to match,
  which would have broken the compile outright because Source Tools names the exported SMD after the
  object. Recovery: rename the object back, re-link slot 0 through the material browse dropdown, then
  `+` and fill the new slot.
- **Never run `Alt+N` Reset Vectors with the gloves visible.** 45 glove edges carry sharp marks that
  are inert only because Valve's imported custom normals override them. A reset would switch all 45
  on at once. Verified after the fact by comparing corner normals against a recompute: the pickaxe
  deviates by a median 1.7°, the gloves by 13.1° — still untouched.
- **Materials live on faces, sharpness lives on edges.** Nothing carries a material but a polygon,
  and there is no unassigned state — "removing" a texture is always assigning another. Removing a
  *slot* that still has faces silently shifts every index above it and re-textures parts of the model.
- **The shaft reads round because it was made round.** The second cut smoothed its octagon into a
  cylinder at 60°, so everything mounted on it inherits that while the head is faceted and hard.
  Andrei saw the seam — "the head reads very sharp while that ring reads curvy" — and the fix was to
  facet the collar, not to soften the head.
- **A part joins the Suit Variant through the QC, not through code.** Give it its own material, one
  map per family with the accents from `game_shared/suit_defs.h`, and a column in the
  `$texturegroup` beside the four glove textures. Family 0 is **cyan**, not orange. A saturated tint
  is a multiply and costs luminance — red lost a third of it — so the three maps were balanced by
  their printed means to within 5%. Confirmed by parsing the compiled `.mdl` header: 19 textures,
  9 skin refs, 3 families, all five pickaxe textures flagged `0x3` flatshade,chrome.
- **What the measurement killed.** A planned "grind the blade into a wedge" was retracted: quoted at
  12°, it survives honest arithmetic at about 5°, and at 23 px per unit on screen *any* feature the
  blade's 0.08–0.5 thickness can afford is 1 to 6 pixels wide. The blade is flat because it is a
  blade. Its answer was the map, not the mesh.

**Next:** rung 2, the alien grunt's melee weapon from nothing. The pickaxe's remaining work is taste
(Andrei's own pass), then `w_pickaxe` — still a blackened crowbar from 2026-09-18 — and the Icon,
which is what actually closes the ART_DEBT entry.

## 2026-09-21, small hours — the pickaxe's second cut (modelling rung 1)

**Built by Andrei** in the saved `v_pickaxe.blend`, opened on his own sentence, "lets improve the
pickaxe model". The head came from 19.45 units end to end to **11.9**, from 5.89 tall to 2.87, and from
a uniform 1.41 slab to a profile that runs 0.99 at the eye to 0.09 out along the blade; the square poll
became a short **upturned back spike** — his design, not the brief's shrink-and-bevel — merged from its
three corners and given a ridge and a notch of its own, so the head reads as a miner's pick with a long
sweeping blade one way and a spike the other, at no cost in width. The shaft was ovalled to 1.0 × 0.81,
cut into nine rings, and flared into a ferrule under the head and a swell where the glove closes. Then
the whole addition was smooth-shaded and switched from flat grey placeholders to chrome. 403 → **503
triangles**. Installed to the repo and the mod, uncommitted, not yet seen in game. Every number was
measured off the live mesh over the MCP before the step was written; the full record, including the
recovery procedures, is in `E:\CustomAssets\models\blender\v_pickaxe\BRIEF.md`.

**What was learned:**

- **Loop Cut destroys geometry, silently, when a modifier displays on the edit cage.** The Armature
  modifier's *On Cage* / *Display in Edit Mode* toggles make `Ctrl+R` solve its slide against the
  deformed mesh. Blender says only "Loop cut does not work well on deformed edit mesh display" and then
  puts every cut at the ends of the piece, where they are invisible: six cuts left the shaft carrying
  **74 vertices and 99 faces** where 16 and 10 belonged, three coincident copies of each ring, plus a
  zero-area triangle spanning the whole shaft and three non-manifold edges. Turn both toggles off before
  any loop cut on a rigged mesh. The recovery, with only the piece selected: `M` → By Distance 0.01,
  Clean Up → Degenerate Dissolve, Clean Up → Delete Loose, Clean Up → Limited Dissolve at 1°.
- **An SMD import carries custom split normals, and they override face smooth flags.** `Shade Smooth`
  on imported-then-edited geometry does nothing at all until `Alt+N` → **Reset Vectors** rewrites them.
  Nor can the object-level *Shade Auto Smooth* be used: it adds a modifier instead of writing mesh data,
  and it reaches the stock gloves, 225 of whose 400 edges exceed 35°. The safe shape of it is Edit Mode
  with the gloves hidden — material-select, `Shift+H`, Face ▸ Shade Smooth, Select All by Trait ▸ Sharp
  Edges, Edge ▸ Mark Sharp, `A`, `Alt+N` ▸ Reset Vectors, `Alt+H`.
- **The smoothing angle is a measurement, not a habit.** The head's edges are either ≤45° (the blade's
  length runs) or ≥75° (the rim, the spine ridge, the collar corners) with nothing in between; the
  shaft's octagon facets are 45°, up to 54° once ovalled. 60° smooths the blade and rounds the shaft
  into a cylinder while every rim stays sharp — and the habitual 35° would have left the shaft fully
  faceted. Read the distribution first.
- **Smooth Vertices has per-axis checkboxes.** Unticking X and Y flattens a kink in a silhouette without
  bunching the vertices along the piece or undoing a thickness profile. Its *Repeat* resets to 1 unless
  the operator panel is opened, which is why the first pass moved a fifth of what was asked.
- **A chrome texture is an environment map sampled by the surface normal, so it ignores the UVs
  entirely.** That means the unwrap can be deferred while a model is on chrome, and that smooth shading
  matters twice as much, because a flat-shaded chrome facet is one flat tone. It also means
  `render_smd.py` cannot show chrome: it applies the BMP through the UVs as diffuse, so a sphere map
  renders near-black. Silhouette and shading are trustworthy in those renders; the material is not.
  Chrome is judged in HLMV. The two maps used were not painted — they are `pickaxe_black.py`'s own
  `chrome.bmp` and `chrome_red.bmp`, already tuned dark for this weapon.
- **studiomdl matches a material to a texture file by name, extension included, and its chrome test is
  case-insensitive.** The PNG in `textures/` must be named exactly as the Blender material or the
  compile dies with `./NAME.bmp not found`; `pickheadChrome` came out `0x03 flatshade,chrome`, the same
  flags as Valve's `rubbergloveCHROME.bmp`.

**Andrei's judgment afterwards, and what it sets up:** "definitely a big improvement from the first
model", with three things for the texturing pass. The **shaft's swells do not read** — soft flares were
the wrong choice and he wants tactical rings or some other crisp detail instead, which matches the
measurement that the ferrule's ring breaks at only 5.1° and the grip's at 1.6°. **Chrome will not
carry the head**: unlike the crowbar's thin bar, the pick head is a large piece and one uniform
environment map on it looks poor. And **the head looks asymmetrical in game**, the visible side being
the worse one. That last one was checked against the mesh: the geometry is symmetric to within
**0.005** (the whole head is centred on y = −0.005 rather than 0, a leftover from the first cut, which
is half a percent of its thickness), so what he is seeing is not the shape. It is the surface — either
the two Smart-UV islands, whose texel density already spans 4.5× across the head, or the fact that the
two big faces are 22-gons that triangulate differently on export while studiomdl's `flatshade` on a
chrome texture lights per triangle. Both are the texturing pass's to settle, and the second argues for
triangulating the head by hand before the export that carries paint.

**Next rung:** 2, the alien grunt's melee weapon from nothing — but the pickaxe's texturing pass comes
first, and its steps are in the model's brief.

## 2026-09-20, evening — animation from nothing (animation rungs 1 to 4)

**Built by Andrei in Blender 5.2**, on a fresh file, nothing kept: the default cube keyed at frame 1
and at frame 24 three metres up; the same rise seen as a curve in the Graph Editor and switched between
Bezier, Linear and Constant; the cube made a 4 m block (scale applied, six subdivisions), an armature of
two bones (`Bone` floor to 1 m, `Bone.001` to the top) built in Edit Mode, bound with automatic weights,
and the upper bone keyed in Pose Mode to a 45° lean, which bent the block like a finger; then that
Action named `bend`, a second Action `twist` (90° about Z) keyed from a blank, both shielded, and the two
swapped in the Action Editor. Every rung read back over the MCP: keys, bone hierarchy, modifier, weight
groups, the two Actions with their fake users. The file was not saved; it was practice.

**What was learned:**

- **"The cube moves but there is no loop."** The keys were right; the playback range still ended at 250,
  so one second of motion sat in ten of waiting. Blender plays the range it is given and nothing tells
  it where an animation ends. The Timeline's `End` field is the first thing to set.
- **Ease is real and it ships.** At frame 12 of a 1-to-24 rise, Z read 1.40 where a straight line gives
  1.435. Bezier eases both ends, Linear is one speed with a hard stop, Constant is a snap. Studiomdl
  reads no curves: the exporter writes every bone on every frame, so the interpolation chosen in the
  Graph Editor is baked into the SMD. Bezier for a body, Linear for a mechanism, Constant for a switch.
- **Three modes, three jobs.** Object Mode places things, Edit Mode on an armature builds bones
  (extrude from a tip with `E`), Pose Mode moves them and is the only place to key them. `Ctrl+Tab`
  toggles Pose Mode on an armature.
- **Before `Ctrl+P` with automatic weights:** `Ctrl+A` All Transforms on the mesh, or the exporter reads
  the wrong size; tick `In Front` on the armature's viewport display, or the bones are invisible inside
  the mesh; click the mesh first and the armature last, the active object becomes the parent.
- **Bones rotate as quaternions by default**, which is why a bone's Rotation shows W X Y Z. Fine for
  SMD; a read-back of a 45° lean about X is W 0.92, X 0.38.
- **An Action with no user is deleted on save.** The shield (Fake User) beside the name in the Action
  Editor keeps it. Unlink (`X`) before `New`, or `New` copies the assigned Action instead of starting
  blank. The browse dropdown swaps Actions on the rig; Action names are for the author, the QC's
  `$sequence` order is what the code reads.
- **Automatic weights blend a vertex between bones.** GoldSrc gives a vertex one bone; `smd_goldsrc.py`
  collapses the blend on every export, so it works, and a real model wants its weights painted single.
- **For the agent's read-back in Blender 5:** an Action has no `fcurves`. Curves live under
  `action.layers[].strips[].channelbags[].fcurves`.

**Next rung:** 5, the block's `bend` through Source Tools to an SMD, converted, compiled with a two-line
QC and opened in HLMV. That is the mod's own unexplored step, and the block is the cheapest thing to
try it on.

## 2026-09-20, later — the deposits as crystal masses, and the glow

**Decided first: brush, not model.** Andrei asked whether a deposit that reads as a mass of crystals
should be a model or a brush, and how GoldSrc shows a model with no function at all. Brush won on three
counts: a brush face can emit light and a model cannot (a model is lit from one lightmap sample under its
origin); `func_deposit` already traces its real shape for the arc's surface and the pick's chips, and a
model form would collide as a box; and Valve's own crystals are brushes, which is what the amber set's
cap texture is for. The model form keeps one use for later, small wall outcrops, once he can model an
organic mass. The scan of Valve's maps that answered the second question: 126 decorative model
placements in the whole campaign, 82 `monster_generic` (the hologram, bones, forklifts), 39 `cycler`
(hair, fungus, bubbles), 3 `cycler_sprite`, 2 `monster_furniture`; everything else is brushwork. So: a
model only where a brush cannot (organic, animated) or a script needs a body.

**Built by Andrei in J.A.C.K.:** a **deposit unit**, seven crystals in a 64×64×64 box, each a six-sided
prism with a six-sided point, leaning outward, tips pushed off-centre, from a table of local coordinates
(origin at the unit's south-west floor corner; he took liberties from there). Sizes were argued down
from a first table that reached 240 tall (his call: "enormous"), tried at 32 (too small, faces 2 units
wide) and settled at 64, which is also just over the crouch-jump, so a ring of units still seals the
Heart. The unit is world brushes on the clipboard, pasted eight times around the Heart and tied to
`func_deposit` per unit, so each is its own deposit; the four slabs are gone. Two units, north and
south, are Unstable (the spec had one). A `light_surface` entity at the chamber's ceiling names
`CRYS_3A` at brightness 150, so every crystal face in the map emits: RAD's direct lights went from 57 to
1748. His verdict in play: solid pathing, and "each crystal has its own hitbox", which the arc and the
chips also benefit from.

**What was learned:**

- **A unit's base stays in the world.** The first plan had a rock block inside the entity so the ring
  would seal; Andrei's correction: the base does not break with the crystals, so it is world brushwork
  and the crystals alone are the entity.
- **J.A.C.K. writes every FGD default into an entity** once its properties are opened (`health 120`,
  `yield 3`, `material 0`...). They match the code's defaults, so it is noise, but the read-back shows
  them as differences.
- **"Ambiguous leafnode content" warnings**, 71 of them, all in the deposits' clipping hulls: the
  widened collision copies of crystals that pass through each other disagree about what is solid. The
  compiler resolves each; tested by walking against every side and jumping at the ring, nothing snags.
  Noise, kept.
- **Texture lights are one entity, no side file:** `light_surface` with `_tex` and `_light`, from the
  compiler's FGD; RAD reports the faces in its direct-light count.
- Still purple: the two deposits at the abandoned face and the one at the vein, old slabs in `CRYS_1A`,
  the next place to paste the unit. Prefab libraries are not in J.A.C.K.'s manual; the clipboard did.
- **The unit's source is `maps/prefabs.map`**, Andrei's own: a lit 576-unit test box with a player
  start and the unit standing in it as a `func_deposit`. Open it beside a map, copy the unit from there.
  Every hand-made unit from now on lives in that file.

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

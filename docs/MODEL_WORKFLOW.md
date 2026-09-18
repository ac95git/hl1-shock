# 3D Model Workflow

How the mod's models are taken apart, changed, previewed and compiled. Set up 2026-09-12 with the Gauss
Katana viewmodel as the first piece through it; the scripts that made it are the worked example. Companion
to [SPRITE_WORKFLOW.md](SPRITE_WORKFLOW.md), which covers 2D.

The loop is: **Crowbar decompiles a `.mdl` → SMD/QC/BMP sources → a Blender script (headless) does the
mesh work and exports SMD → two small converters put the SMD in the form GoldSrc's studiomdl and a
viewer expect → studiomdl compiles → renders from the in-game viewmodel camera are looked at → HLMV or
the game.** Like the sprites, the model is produced by a script from sources, so a change is a change to
the script and a re-run, not a hand edit of an exported file.

## Where things are

| | Path |
| --- | --- |
| Working directory (sources, not in this repo) | `E:\CustomAssets` |
| Crowbar-decompiled stock and imported models | `E:\CustomAssets\models\decompiled\<name>\` |
| Sources we compile from (QC, SMDs, BMPs) | `E:\CustomAssets\models\src\<name>\` |
| Blender scripts | `E:\CustomAssets\scripts\` |
| Renders | `E:\CustomAssets\render\<name>\` |
| Repo tools | `utils/mdltool/` |
| Blender (use **5.2**, with Blender Source Tools) | `D:\CreativeTools\Visual\Blender\blender.exe` |
| studiomdl (the SDK's; Crowbar uses the same one) | `D:\GameLibrary\steam\steamapps\common\Half-Life SDK\Model Tools\studiomdl.exe` |
| Crowbar, HLMV | `D:\CreativeTools\GameAssets\` |

Compiled `.mdl` files that ship go in the repo under `models/` and are copied to `topmod/models/` by
hand, the same arrangement as `sprites/`. Sources stay in `E:\CustomAssets`; they run to megabytes.

## The one rule

**Decompiling is done by hand, with Crowbar, by the person running the loop.** If a task needs a model
whose sources are not under `models/decompiled/<name>/` (reference SMD, anims folder, QC), the task stops
and asks for that decompile by name. No decompiler gets written, no mesh gets pulled out of a `.mdl`
another way, no work proceeds on a partial set. The only exception is textures, which Crowbar sometimes
leaves out and `mdlinfo.py --extract-bmp` can supply.

**Check the SDK before asking.** Valve's own sources for the cut monsters, the grunts, the player model,
every stock viewmodel and every `w_` pickup are in the Half-Life SDK folder, listed in
[HL_SDK.md](HL_SDK.md). Where a model is there, copy its folder into `E:\CustomAssets\models\src\<name>\`
and work from it; no decompile is needed. Its QC may not be the exact file that built the shipped model,
so compare it with `mdlinfo.py` on the `.mdl` first.

## Facts that bind the work

- **studiomdl rotates every SMD +90° about Z when it compiles.** SMD **-Y** is the game's **+X**, forward
  (Z up, Y left): the stock crowbar viewmodel's decompile runs y -0.9..-15.3 away from the hands, and a
  world model built with its business end along -Y points the way the entity faces (`syringe_world.py`,
  measured 2026-09-13; an earlier version of this line said +Y, which is backwards). Anything that
  measures "forward" on an SMD, or renders one from the player's eye, has to apply that rotation first.
  `render_smd.py` does.
- **A viewmodel's origin is the player's eye.** Camera at the origin looking down +X. Half-Life's `fov 90`
  is the horizontal angle at 4:3; at widescreen the engine keeps the vertical angle (73.74°) and widens.
- **A vertex belongs to exactly one bone.** GoldSrc has no weights. Blender Source Tools exports
  Source-style vertex lines with the bone in a trailing weight list and `0` in the leading column, which
  is the only column studiomdl reads, so every vertex lands on the root and the rest of the skeleton is
  pruned (`cannot find bone X for bbox`). `smd_goldsrc.py` rewrites the leading column from the heaviest
  link. **Always run it on an exported SMD before compiling.**
- **Source Tools binds an import to whatever armature is already in the scene.** Importing two models
  into one Blender session puts the second mesh on the first skeleton and creates no second armature.
  Import the one whose skeleton you need only for measurement first, unlink its armature from the scene,
  then import the other.
- **Sequence order is a contract with the code.** Weapons address sequences by index (`CROWBAR_DRAW` is
  the second sequence, and so on). A model that replaces or reuses a weapon's animations keeps its QC's
  sequence list in the same order, with nothing inserted. A sequence of the mod's own goes **after** the
  stock ones: the Cleave's swipe is index 11 (`CROWBAR_CLEAVE`) in `v_crowbar.qc` and `v_katana.qc`,
  behind the eleven the crowbar shipped with.
- **An animation SMD is the skeleton alone, one `time` block per frame, every bone every frame**, with
  the rest pose the anims share as frame 0 (the stock attacks all start and end on the same pose; take
  it from any of them, not from the reference SMD, whose root sits elsewhere). Bone rotations are the
  SMD's local Euler triples, composed as `smd_pose.py` does; the engine interpolates between frames, so
  30 fps keyframes are plenty. `crowbar_cleave.py` is the worked example: it never poses a bone by hand
  but designs the swing as a path in game space — where the hand goes, which way the crowbar points —
  and solves the three arm bones to follow it each frame, then eases back to rest in bone space so the
  last frame is the rest pose exactly.
- **A swing has three things to say, and a still shows two of them.** Where the hand is, which way
  the bar points, and which way the bar is *rolled* about its own shaft. The Cleave swipe's five cuts
  (2026-09-15) each got one of them wrong in turn; the section *What the Cleave swipe taught*, below,
  is the record, and it is where to start before authoring or judging any viewmodel swing.
- **UVs must lie in 0..1.** Source models routinely carry UVs whole tiles outside it (the Dystopia
  katana's v ran -2.4..-2.1). Blender repeats the texture so the preview looks right; studiomdl multiplies
  u,v by the texture size as they are, the game samples off the texture, and the surface comes out black.
  Shift each face by whole tiles before export; `katana_graft.py` does. A whole-number shift changes
  nothing on a repeating texture.
- **Textures are 8-bit BMP**, one palette each, up to 512×512. A 24- or 32-bit BMP has to be quantised
  first (Pillow: `convert("RGB").quantize(256).save(..., "BMP")`). Names containing `CHROME` get the chrome
  flag from studiomdl; the SMD material name is the BMP file name, exactly, including a double `.bmp.bmp`
  where valve has one.
- **Texture render flags: the engine ignores FULLBRIGHT and honours ADDITIVE — but additive does not
  glow in the dark.** Tested on the katana's hot blade, 2026-09-12, and corrected on the progression
  pickups, 2026-09-14: the studio renderer still multiplies an additive texture by the room's lighting,
  so in a dark room it fades with the rest of the model. The katana's blade looked lit in the dark
  because its own dynamic light was lighting it. Additive is a *look* — drawn as light over what is
  behind it, dark pixels vanish, the surface a little transparent — not a light source. A model that
  must be seen in the dark needs a dynamic light; the pickups get one per visible entity on the client
  (`cl_dll/entity.cpp`, `ProgressionLight`), keyed by entity index, no network cost. The flag is
  patched after the compile with `mdlflags.py` or `smdprims.set_flags` (studiomdl cannot set it; this
  repo's studiomdl knows `$texrendermode additive` but the SDK's does not). Give an additive surface a
  material of its own and keep the rest solid.
- **Skin families are the switch for texture states.** The viewmodel's skin is never sent by the
  server; `cl_dll/view.cpp` sets it every frame from the model's family count: three families are the
  suit colours, six are suit × cold/hot in glove-major order. `qc_skins.py --state COLD=HOT` lays the
  six out. A **world** model's skin is different — it is the entity's own `pev->skin`, which the server
  sets and the engine networks, as `item_suit` does from its `variant` keyvalue.
- **`$externaltextures` puts the skin families in the T file.** A model that keeps it (the stock
  `w_suit` does) compiles to `NAME.mdl` plus `NAMET.mdl`, and the texture header — textures, flags and
  `numskinfamilies` — is entirely in the T file. `mdlinfo.py` on the `.mdl` alone correctly reports
  **zero** textures and zero families, which reads like a failed compile and is not; read the T file.
  Both files have to reach `models/` and `topmod/models/`: ship only the first and the engine falls back
  to valve's textures with no error.
- **`$attachment` coordinates are bone-local.** studiomdl copies the three numbers into the model as
  given and the engine places the attachment at the bone's matrix times them every frame; every stock
  QC's are small hand-relative offsets. Found 2026-09-15 when the katana's swing trail rode the hand
  at a fixed displacement: the graft had written model-space points, assuming studiomdl would
  transform them like vertices. `katana_bend.py` converts through the hand bone's rest matrix from the
  SMD's own skeleton (`hand_bone_local`); any script that writes an attachment has to do the same.
- Crowbar may decompile a model without writing its textures. `mdlinfo.py --extract-bmp=DIR` pulls them
  straight out of the `.mdl` under their stored names.
- **A Source decompile's SMDs crash the SDK's studiomdl until their first line goes.** Crowbar 0.74
  heads every SMD it writes from a *Source* model with `// Created by Crowbar 0.74`; its GoldSrc
  decompiles carry no such line. studiomdl dies on it with an access violation and no message
  (found 2026-09-16 on the Age of Chivalry longsword, bisected down to that one line). Strip comment
  lines from every reference and animation SMD before compiling; `katana_aoc.py` does. Otherwise a
  Source rig compiles as it is: 44 bones, dotted `ValveBiped.` names, Source-style vertex lines
  through `smd_goldsrc.py`, animation SMDs verbatim. GoldSrc's limits that bite first are 2048
  vertices per submodel (the AoC knight hands are 4,200, the footman 1,018) and 128 bones.
- The `'Scene' object has no attribute 'vs'` traceback Source Tools prints under factory settings is
  harmless noise.

## Tools

Repo, under `utils/mdltool/` (Python 3 with Pillow):

| Tool | Does |
| --- | --- |
| `mdlinfo.py MODEL.mdl [...] [--bones-only] [--extract=DIR] [--extract-bmp=DIR]` | Reads a `.mdl` header: bones with parents, textures with sizes and flags, bodyparts and submodels with vertex counts, sequence names. Extracts textures as PNG (to look at) or 8-bit BMP (to compile with). No decompile needed; this is how the stock viewmodels were surveyed. |
| `smd_goldsrc.py IN.smd OUT.smd [--wrap-uv]` | Source-style vertex lines → GoldSrc single-bone lines. Run on every Blender export. `--wrap-uv` shifts each triangle's UVs by whole tiles into 0..1, for Source-derived meshes (see the UV trap below). |
| `mdlflags.py MODEL.mdl [TEXTURE +flag -flag ...]` | Lists or patches per-texture render flags in a compiled `.mdl`: flatshade, chrome, fullbright, nomips, alpha, additive, masked. The only way to set fullbright or additive on a model this pipeline compiles. |
| `smd_retarget.py SRC_ANIM.smd TARGET_REF.smd OUT.smd [--map "Src=Dst" ...] [--rest SRC_IDLE.smd]` | Moves an animation onto a skeleton that carries the same bones under other names: matched by index (or `--map` by name), unmatched source bones dropped, unmatched target bones held at rest. Checks first that the two rests agree for every matched bone and refuses if they do not, since the numbers are copied, not solved. Written for HL Extended's crowbar animations, whose 45-bone rig has Valve's 11 bones first, with `Clavicle`/`UpperArm`/`Forearm` for `R Arm`/`R Arm1`/`R Arm2` and identical rest values. |
| `smd_pose.py REF.smd ANIM.smd FRAME OUT.smd` | Applies one animation frame to a reference SMD, bone for bone as the engine does, and writes a static SMD. The way to preview a model in a pose without trusting an addon's animation import. |

Working directory, `E:\CustomAssets\scripts\`:

| Script | Does |
| --- | --- |
| `render_smd.py IN.smd OUT_PREFIX TEXDIR...` | Blender headless: imports an SMD, applies the +90° compile rotation, loads its BMPs, renders from the viewmodel camera plus orbit, side and top views. |
| `render_icon.py IN.smd OUT.png TEXDIR... [--size WxH] [--view top\|side\|front] [--roll DEG] [--margin F] [--light flat\|studio]` | An Inventory Icon from a world model: orthographic, on transparent, long axis horizontal, fitted. The `.spr` step and the conventions are in [SPRITE_WORKFLOW.md](SPRITE_WORKFLOW.md), *Inventory Icons*. |
| `katana_graft.py --out DIR [--roll --pitch --yaw --slide --shift] [--blade-tex NAME]` | The worked example: vanilla crowbar hands + Dystopia katana blade → one reference SMD, blade straight. Measures the crowbar's grip axis and the blade's axis by principal component, aligns them, and exposes the residual corrections as numbers (the shipped model uses none). Writes `attachments.qc`: 0 the grip, 1 the blade's point. |
| `katana_bend.py IN.smd OUT.smd [--sori 0.04] [--cuts 8] [--anchor root\|chord] [--metal MAT] [--handle MAT] [--attachments QC]` | The katana's sori, as a stage on SMD triangles between the graft's export and `smd_goldsrc.py`; `katana_world.py` imports the same function, so hand, floor and Icon carry one curve. Finds the blade metal by texture row (`METAL_V`), tells the cutting edge from the spine by the rows their long edges map to (measured 2026-09-15: that side leads every crowbar attack), fits one uniform arc — root at the guard, deepest bow `--sori` of the blade's length as a real katana's sori is measured (a real one is 2–3%; 4% was chosen in HLMV over 2.5%, which read as a hint), convex on the edge's side, arc length kept — and, because the Dystopia blade is four long quads with no vertex between guard and point, slices every metal triangle across the blade `--cuts` times first; without that a bend only turns the end tangents. `--anchor root` (the default) has the blade leave the guard along the handle's line and the point swing toward the spine; `chord` keeps root and point on the straight line and tilts the root by half the arc, which kinked at the guard in HLMV. `--attachments` moves `$attachment 1` to the bent point. `--sori 0` reproduces the straight model. |
| `hev_gloves.py model DECOMPILED_DIR SRC_DIR` / `sheet DECOMPILED_DIR OUT.png` | The mod's own HEV glove textures, generated per model from that model's own glove BMPs (the shared `GLOVE*`/`rubbergloveCHROME` set, the crossbow family's `xbow_sleeve`, the MP5's `PLAYER_ForeArm`/`Cuff`, the shotgun's `HAND_ForeArm`): luminance kept, orange plates recoloured, thin grooves and the hand-back screen turned into an accent light, chrome map tinted. Three variants, cyan/red/purple. Writes cyan under the stock names, red and purple with suffixes, a `skins.qc` fragment, and `preview_<variant>/` folders for `render_smd.py`. |
| `qc_skins.py MODEL.qc SKINS.qc [--state COLD.bmp=HOT.bmp]` | Inserts (or replaces) the generated `$texturegroup` into a QC, before the first `$sequence`. Three skin families, cyan first, so skin 0 is what a model shows with no code at all; with `--state`, six, each glove family cold then hot. |
| `katana_aoc.py [--scale F] [--slide U] [--slim F] [--slim-below T] [--keep-upper-arms] [--zrot DEG] [--no-compile] [--no-render] [--install] [--frames anim:frame,...]` | The longsword probe (ROADMAP.md, *The longsword moveset*): Age of Chivalry's `v_longsword` rig and its eight animations compiled for GoldSrc as decompiled, nothing retargeted, with AoC's footman hands and a `blade` bodygroup of the decompiled longsword and the katana blade. Pure Python on SMD text, no Blender except for the renders. Places the blade by mapping the katana's straight axis and cutting-edge direction (from `katana_bend`'s texture rows) onto the longsword's long axis and the flat that leads its swings — measured over swing1 and swing2 the way `katana_swing_lead.py` measures the crowbar's — grip on grip, each grip the right hand bone's origin dropped onto its blade's line; then slides it out (`--slide`, 2 by default: the guard's underside past the right thumb's tip) and slims the handle's width below the guard (`--slim` 0.45: the Dystopia handle is a flat card 4–6 wide, the hands close on 2). Drops the clavicle and upper-arm triangles by majority bone. Paints the footman hands as an HEV glove in `hev_gloves.py`'s palette and three variants by texture bands (`paint_hands`), which works because the footman's texture is one island stacked by v — fingers and hand at the top, forearm, upper arm. Strips Crowbar's comment lines (the crash above), writes the QC with the crowbar's twelve slots then block and deflect, the katana first in its bodygroup (body 0), the trail's three attachments bone-local on the sword bone, and gloves × blade cold/hot as the six glove-major skin families `V_SetViewModelSkin` reads; compiles, patches the hot blade additive, and poses and renders the requested frames from the eye for both blades. `--install` copies it over the install's `v_katana.mdl` for a session (the repo's `models/v_katana.mdl` restores it). Output `models/src/v_katana_aoc/`; ships nowhere. |
| `katana_hot.py` | The katana's hot blade texture: the gold metal of `katana_02.bmp` turned gauss orange grading to white-hot along the metal's own shading; everything else untouched. |
| `crowbar_cleave.py [--frames 20] [--return-from 9] [--render [--katana] [--render-frames 0,1,...]] [--fork-weight W] [--fork-facing x,y,z] [--rest-weights 9 numbers] [--force]` | The Cleave's forehand swipe as a path solved onto the arm — the script's cut, superseded by a hand-made SMD but kept for its measurements. Keys in game space — hand azimuth, radius and height about the shoulder, bar azimuth and elevation — through Catmull-Rom per frame; the shoulder, elbow and wrist solved by Levenberg–Marquardt to put the hand and the crowbar's tip on target and the fork nudged toward the sweep, each frame from the last and pulled softly toward rest; the return eased in bone space to land on rest exactly. Prints the tip's path with an edge check at 16:9 and 4:3 and the fork's on-screen facing per frame; with `--render` poses the reference at the key frames through `smd_pose.py` and renders them from the viewmodel camera. Writes `cleave.smd` into both models' anim folders **only if the file there is its own** (`--force` overrides). Recompile both QCs after it. Its `load()`, `fk()` and `ik()` are what `bar_lean.py` and the strain and orientation surveys import. |
| `bar_lean.py [ANIM ...]` | The swipe-or-thrust readout for a crowbar-hands animation, stock or new: per frame, the bar's on-screen length as a share of its true length (how broadside it is), its screen angle, and the tip's direction of travel. A bar along its own travel is a thrust; attack2's sweep, ~95% and ~90° against a tip moving at ~140°, is the reference. |
| `katana_swing_lead.py [ANIM ...]` | The measurement behind the sori's direction: poses the katana reference at every frame of the crowbar's attack animations with `smd_pose.py`'s math, tracks the point and the two long edges of the blade, and prints which edge leads the point's motion, frame by frame and summed. Run it before trusting a guess about which side of any borrowed blade is the edge. |
| `katana_world.py [--scale] [--tex] [--sori]` | `w_katana.mdl` from the Dystopia world prop without Blender: one bone at the origin, the katana rotated to lie on its flat, centred, floor at z 0, scaled 1.2 (0.82 matched the viewmodel blade and read too small on the floor), the blade bowed by `katana_bend.py` in the floor plane, UVs wrapped, one-frame idle, QC, studiomdl, render. The pattern for any single-bone world model from a Source prop. |
| `syringe_world.py [--scale] [--no-icon]` | `w_syringe.mdl` with **no source mesh at all**: the geometry is tubes, cones and discs emitted straight into the SMD (284 triangles, winding checked per triangle against its normals), the one 128×128 texture is painted by Pillow with the liquid, stopper and graduations on the barrel, then QC, studiomdl, the orbit render, and the Inventory Icon rendered and encoded to `sprites/inv/item_syringe.spr`. The first prop authored from numbers; its primitives were then factored into `smdprims.py`. |
| `smdprims.py` | The shared kit for models authored from numbers: a `Mesh` that winds every triangle counter-clockwise about its own normals, `tube` / `cap` / `annulus` / `box` primitives (tapered, elliptical, inward-facing, cone tips, planar or per-facet UVs), the `Y_TO_Z` and `TURN_180` transforms, region-based texture painting with cylinder shading, and `build()` — reference SMD from one Mesh or several (one material each), idle, QC, studiomdl, an `additive=` list of textures patched through `set_flags()`, `mdlinfo` check — plus `render()`. Import it; do not copy it. |
| `progression_world.py [--only NAME] [--scale]` | The three progression pickups on `smdprims.py`: `w_skillpoint.mdl` (a hex bipyramid stood up with `Y_TO_Z`, per-facet UVs so the edges are painted bright), `w_resettoken.mdl` (a puck with a rim, an inward-facing recess wall and two planar-mapped painted faces at 128px, since 64px made the tick ring a zigzag) and `w_rowgrant.mdl` (boxes only: bars, a divider panel, a handle). Each has a second `<name>_glow.bmp` material patched additive — the shard whole, the Token's marks and the Row Grant's grid as thin overlays 0.15 above solid faces — which is how they emit light in place of the old glow shell. About 100–330 triangles each. |
| `gloves_rollout.py [model ...]` | The whole thing for every stock viewmodel: copy the decompile to `models/src/`, gloves, QC, studiomdl, verify three skin families in the `.mdl`, orbit render, contact sheet. Stops and names the model if a decompile is missing. |
| `suit_world.py [--no-compile]` | The `w_suit` pickup in three Suit Variants: a colour wash over the stock front/back textures (hue from the variant, luminance from the suit, a 22% wash on the grey panels), skins.qc, QC, studiomdl, a three-up preview sheet. Looser thresholds than the gloves on purpose — see below. Produces `w_suit.mdl` **and** `w_suitT.mdl`; both ship. |
| `pickaxe_black.py [--install]` | The Carbon Pickaxe's stand-ins (2026-09-18): the mod's `v_crowbar` and the vanilla `w_crowbar` decompiles copied to `models/src/{v,w}_pickaxe`, the metal textures' **palettes** remapped from luminance onto a dark blued-steel curve (indices untouched, so UVs and studiomdl's chrome flags carry over), renamed, compiled. Drops `$externaltextures` so one `w_pickaxe.mdl` ships. Writes each recoloured texture as a new file from the decompile: overwriting a file `shutil` has just copied failed with `EINVAL` here. `--install` copies both into `models/` and `topmod/models/`. |

Blender is always run as `blender.exe --background --python SCRIPT -- ARGS`. Renders use Workbench, so no
GPU is needed and a run takes seconds.

## The katana, as a worked example

The Dystopia katana came as a Source model on a 45-bone rig with 512px Source hand textures; it matched
nothing else in the game. v1 puts its blade on Half-Life's own crowbar hands and the crowbar's eleven
animations:

1. `mdlinfo.py` on the stock `v_crowbar.mdl`: 11 bones, crowbar geometry on `Bip01 R Hand`, six textures.
2. Crowbar decompiles the stock crowbar (hands mesh, anims); `mdlinfo.py --extract-bmp` supplies the
   textures Crowbar left out.
3. `katana_graft.py`: import the katana first, unlink its armature; import the crowbar; delete the
   crowbar's `chrome` faces from the hands and everything but `katana_01` from the katana; align the
   blade's long axis to the crowbar shaft's, put the katana's grip bone where the hand bone projects onto
   the shaft; join, weight every blade vertex to `Bip01 R Hand`; export.
4. `katana_bend.py` on the export, with `--attachments` pointed at the QC, for the sori (added
   2026-09-15; the blade came straight from Dystopia); `smd_goldsrc.py` on that; a QC copied from
   the crowbar's with the model name and reference changed and the sequence list untouched;
   studiomdl; `mdlflags.py v_katana.mdl katana_02_hot.bmp +fullbright +additive`, since the compile
   resets the hot blade's flags.
5. `smd_pose.py` with `idle1` frame 0 and `attack1` frame 5, rendered by `render_smd.py`, next to the
   vanilla crowbar through the same two steps as the calibration.

Result: `E:\CustomAssets\models\src\v_katana\v_katana.mdl`, 11 bones, 689 triangles (577 before the
sori's slicing), 5 textures before the glove skins, the crowbar's 11 sequences in order.

## Review

Render the stock model the same way first. If the vanilla crowbar does not look like the crowbar in
game, the camera is wrong, not the model. Then judge the new model at the same frames: rest, mid-swing,
draw. Silhouette and grip read from the viewmodel camera; the orbit view is for checking the mesh is
where the numbers say it is.

## What the Cleave swipe taught

The mod's first animation of its own, 2026-09-15: a forehand horizontal swipe for the crowbar hands
(`CROWBAR_CLEAVE`, sequence 11 on `v_crowbar.mdl` and `v_katana.mdl`), authored by
`crowbar_cleave.py` — a designed path in game space solved onto the shoulder, elbow and wrist — and
judged by Andrei in HLMV from the player's eye over five cuts. The script's cut is a stand-in: it got
the positioning right by the second cut and never quite got the bar's attitude right, and a hand-made
SMD is replacing it (see *Dropping in a hand-made animation*, below). What it taught, in the order it
was learned:

- **Design in game space and render from the eye.** A viewmodel swing is a path — where the hand
  goes, which way the weapon points, frame by frame, camera at the origin looking down +X — and the
  bones are solved to follow it. The tip-path table is not enough to judge it; render the key frames
  from the viewmodel camera (`render_smd.py`) next to the stock swings through the same pipeline. If
  the stock crowbar does not look like the game, the camera is wrong, not the animation.
- **The rest pose is any attack's frame 0.** The stock attacks all start and end on one pose; take it
  from an animation SMD, not the reference SMD, whose root sits elsewhere. An animation SMD is the
  skeleton alone, one `time` block per frame, every bone every frame; the engine interpolates, so 30 fps
  keys are plenty. The stock rig facts: shoulder at game (−3.7, −6.7, −4.6), arm 22.7 long, crowbar tip
  20.6 from the hand bone.
- **A "level" swing is not a horizontal one on screen** (cut 1, "a punt"). The arm, from attack3's
  peak: nearly straight (hand 20–21 units from the shoulder), the hand about 11 below the eye; a hand
  more than 0.75× its forward distance below the eye is under the bottom edge. A bar pointing down the
  view with a bent elbow foreshortens into a thrust. `bar_lean.py` prints the two numbers that tell a
  swipe from a thrust: the bar's on-screen length as a share of its true length, and its screen angle
  against the tip's direction of travel. attack2's sweep is the reference: ~95% and ~90° against a tip
  moving at ~140°.
- **The bar's azimuth stays on the hand's line of sight from the eye** (cut 2). That is what makes it
  vertical on screen and broadside to the sweep, and it is what attack2 does through its whole sweep.
  Swinging it round to "lean" into the follow-through was the invention that broke the next thing.
- **The roll about the shaft is the third thing, and it says which way the swing goes** (cut 2's
  fault, found by Andrei's arrows: "the crowbar starts to point down"). The crowbar's fork has to face
  the direction of travel through the sweep. Position alone — hand and tip on target — leaves the roll
  to fall where the rest pull puts it. *Raising the bar's world elevation in reply to "pointing down"
  was a guess and made it worse* (cut 3): when a HLMV complaint is a direction word, ask which camera
  and get screenshots with arrows before changing a number.
- **The roll is not the wrist's to add** (cut 4, "the tilt is way too crazy"). Forcing the fork left
  with a strong solver term twisted the elbow and wrist past 90°; the stock swings never pass 81° of
  true rotation from rest. Freeing the forearm's twist axis and clamping the wrist only moved the
  same 100° from one joint to the other: the twist was demanded by the keys, not chosen by the
  solver. Judge strain by a joint's geodesic rotation from rest, not by its Euler angles, which hide
  a twist across two axes.
- **The fork's facing is set by the bar's direction and how far the arm has swung** (cut 5). The
  fork is perpendicular to the shaft and faces forward at rest, so it turns with the arm. Surveyed
  with no roll term at all, solving from rest: a bar standing at 75–80° of elevation on the line of
  sight has the fork facing screen-left at the centre and the exit with the elbow and wrist inside
  45° of rest; at 45° of elevation (attack2's) it faces the floor by the exit whatever the azimuth; at
  the entry no orientation faces it left, so it trails there, as attack3's does in its wind-up, and
  comes round by the centre. Survey the design space before adding a solver term.
- **Measure the fork off the geometry.** The shaft is a 26-unit cylinder with vertices only at its
  two ends, so an axis fitted to "the straight part" sees six vertices at one end, comes out 13° off,
  and flips which side the fork is on; the first roll term held the fork the wrong way for a whole
  cut because of it. Fit the axis to every chrome vertex and take the fork from the vertex furthest
  off it (4.7 units, on the hand's +x).
- **Reading the stock swings as numbers is the calibration for all of the above.** `stock_params` and
  the strain table (both throwaway, easy to recreate from `smd_pose.py`'s math) gave attack2's and
  attack3's hand azimuth, radius and height, bar azimuth and elevation, fork facing, and joint rotation
  per frame; every rule here is a stock number the cut violated.

### Dropping in a hand-made or borrowed animation

A sequence made outside the script goes in the same slot. Done 2026-09-15 with HL Extended's
`attack_swing_miss3`, which is what ships as the Cleave swing now:

1. Same rig: the 11 bones of `crowbar_reference.smd`, same names and order, every bone every frame.
   An animation from another mod's crowbar is usually on Valve's bones under other names —
   `smd_retarget.py` renames and prunes it, and refuses if the rests differ. Frame 0 need not be the
   rest pose: the viewmodel blends into a new sequence over 0.2 s, and HL Extended's starts mid-swing.
   The last frame should be rest. Any frame count; 30 fps is the QC's.
2. Save it as `cleave.smd` in **both** `models/src/v_crowbar/v_crowbar_anims/` and
   `models/src/v_katana/v_crowbar_anims/` — one file, the katana rides the same hands.
   `crowbar_cleave.py` refuses to overwrite a `cleave.smd` that is not its own (it stamps a
   `// generated by crowbar_cleave.py` line); `--force` if the script's cut is wanted back.
3. `studiomdl` both QCs; `mdlflags.py v_katana.mdl katana_02_hot.bmp +fullbright +additive`; copy both
   `.mdl`s to the repo's `models/` and to `topmod/models/`.
4. Set `cleave_swing_time` (dlls/game.cpp; 1.2 s for the 36-frame HL Extended swing) to the new
   length so the swing is seen whole, and rebuild both DLLs; the katana's trail and the sequence
   index need nothing.
5. `bar_lean.py cleave` and a strain check against attack2/attack3 are the numbers to look at before
   HLMV, if the animation was made by hand and not measured.

## Adding a viewmodel on stock hands

1. Decide which stock weapon's hands and animations it borrows. That fixes the rig and the sequence
   contract with the code.
2. Decompile that weapon (Crowbar) into `models/decompiled/`; extract textures with `mdlinfo.py` if
   Crowbar did not.
3. Get the new weapon geometry as an SMD (Crowbar from a Source model, or Blender-authored).
4. Copy `katana_graft.py` and change the material names, the grip bone, and the guide axis if the
   borrowed weapon is not a straight shaft.
5. Run, convert, compile in `models/src/<name>/`; pose and render; iterate on the correction numbers.
6. HLMV, then the game once code references the model. Record the stand-ins it replaces in `ART_DEBT.md`.

## Not yet covered

- **Animations of its own: one attempted, from a path, and superseded by hand.** The Cleave's swipe
  (`crowbar_cleave.py`) was the first: a designed path solved onto the arm, five cuts judged in HLMV,
  each right about one more thing (*What the Cleave swipe taught*, above). It got the positioning and
  never quite the bar's attitude, and a hand-made SMD is taking its slot. What the path method is
  good for is measuring — the stock swings as numbers, the strain of a pose, where the fork faces —
  and checking a hand-made animation against them; authoring the feel of a swing by numbers was the
  slow way. **Retargeting between rigs that share Valve's bones is done** (`smd_retarget.py`, HL
  Extended's crowbar swing onto ours, a rename). Retargeting onto a rig with *different* rest values
  — the Dystopia 45-bone katana rig — blending two stock sequences, and anything organic are still
  unexplored; so is exporting an animation from Blender through Source Tools. **A whole foreign rig
  compiled as it is, no retarget, is done** (`katana_aoc.py`, 2026-09-16): the Age of Chivalry
  longsword's 44 bones and eight animations drive the katana blade in HLMV. If that moveset is kept,
  the next thing is either painting AoC's footman hands as an HEV glove or an orientation-matching
  retarget onto the gauss's two-armed stock rig, decided after it is judged (ROADMAP.md).
- **Uniform hands across the vanilla set: done, and selected per player.** Fourteen stock viewmodels (all
  but the hivehand, which has no glove, and the chumtoad, which the game never uses) plus the katana
  compile with three glove skins from `gloves_rollout.py`; the compiled files are in the repo's `models/`
  and the install's `topmod/models/`. Which one shows is the player's Suit Variant — see PILLARS.md. The
  **suit model itself** is still the stock one, colour-washed into three variants by `suit_world.py` as a
  stand-in, and only for the pickup: the chargers and the player model are untouched.
- **Accent lights do not glow in the dark.** Studiomdl from the SDK cannot flag part of a texture
  fullbright; the seams dim with the map lighting like the rest of the glove. Making them emissive means
  a separate accent texture with `STUDIO_NF_FULLBRIGHT` set in the compiled `.mdl`, which is a mesh change
  plus a flag patch, not a texture change.
- **Rigged world and player models**, and a Python decompiler to drop the Crowbar step; `mdlinfo.py` has
  the header parsing that one would start from. Static one-bone world models are covered twice over:
  from a Source prop (`katana_world.py`) and from nothing (`syringe_world.py`, `progression_world.py`).
- **Meshes authored in Blender by hand or script.** Every authored model so far was emitted as SMD text
  through `smdprims.py`, which suits primitives and nothing with an organic surface.

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

## Facts that bind the work

- **studiomdl rotates every SMD +90° about Z when it compiles.** Decompiled SMDs therefore face +Y while
  the game's viewmodel space faces +X (Z up, Y left). Anything that measures "forward" on an SMD, or renders
  one from the player's eye, has to apply that rotation first. `render_smd.py` does.
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
  sequence list in the same order, with nothing inserted.
- **Textures are 8-bit BMP**, one palette each, up to 512×512. Names containing `CHROME` get the chrome
  flag from studiomdl; the SMD material name is the BMP file name, exactly, including a double `.bmp.bmp`
  where valve has one.
- Crowbar may decompile a model without writing its textures. `mdlinfo.py --extract-bmp=DIR` pulls them
  straight out of the `.mdl` under their stored names.
- The `'Scene' object has no attribute 'vs'` traceback Source Tools prints under factory settings is
  harmless noise.

## Tools

Repo, under `utils/mdltool/` (Python 3 with Pillow):

| Tool | Does |
| --- | --- |
| `mdlinfo.py MODEL.mdl [...] [--bones-only] [--extract=DIR] [--extract-bmp=DIR]` | Reads a `.mdl` header: bones with parents, textures with sizes and flags, bodyparts and submodels with vertex counts, sequence names. Extracts textures as PNG (to look at) or 8-bit BMP (to compile with). No decompile needed; this is how the stock viewmodels were surveyed. |
| `smd_goldsrc.py IN.smd OUT.smd` | Source-style vertex lines → GoldSrc single-bone lines. Run on every Blender export. |
| `smd_pose.py REF.smd ANIM.smd FRAME OUT.smd` | Applies one animation frame to a reference SMD, bone for bone as the engine does, and writes a static SMD. The way to preview a model in a pose without trusting an addon's animation import. |

Working directory, `E:\CustomAssets\scripts\`:

| Script | Does |
| --- | --- |
| `render_smd.py IN.smd OUT_PREFIX TEXDIR...` | Blender headless: imports an SMD, applies the +90° compile rotation, loads its BMPs, renders from the viewmodel camera plus orbit, side and top views. |
| `katana_graft.py --out DIR [--roll --pitch --yaw --slide --shift]` | The worked example: vanilla crowbar hands + Dystopia katana blade → one reference SMD. Measures the crowbar's grip axis and the blade's axis by principal component, aligns them, and exposes the residual corrections as numbers. |

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
4. `smd_goldsrc.py` on the export; a QC copied from the crowbar's with the model name and reference
   changed and the sequence list untouched; studiomdl.
5. `smd_pose.py` with `idle1` frame 0 and `attack1` frame 5, rendered by `render_smd.py`, next to the
   vanilla crowbar through the same two steps as the calibration.

Result: `E:\CustomAssets\models\src\v_katana\v_katana.mdl`, 11 bones, 577 triangles, 5 textures, the
crowbar's 11 sequences in order.

## Review

Render the stock model the same way first. If the vanilla crowbar does not look like the crowbar in
game, the camera is wrong, not the model. Then judge the new model at the same frames: rest, mid-swing,
draw. Silhouette and grip read from the viewmodel camera; the orbit view is for checking the mesh is
where the numbers say it is.

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

- **Animations of its own.** Everything so far borrows a stock sequence set. Retargeting the Dystopia
  animations onto the 11-bone rig, or authoring new ones, is unexplored.
- **Uniform hands across the vanilla set.** The survey says texture-level is a same-size swap for the
  hand backplate, knuckle and chrome across 12 models and a per-model sleeve; nothing is built.
- **World and player models**, and a Python decompiler to drop the Crowbar step; `mdlinfo.py` has the
  header parsing that one would start from.

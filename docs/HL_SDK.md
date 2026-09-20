# The Half-Life SDK resources

Valve's official Half-Life SDK 2.3 content, installed through Steam at:

```
D:\GameLibrary\steam\steamapps\common\Half-Life SDK
```

It is the mod's **reference library for tools and cut content**: Valve's own model sources (the QC files, the
SMDs as exported from 3ds Max, and the textures) for monsters that were cut from Half-Life and for some that
shipped, plus the compilers, Hammer, and the design docs. It is read-only. Nothing in it is changed; what the
mod takes from it is copied to `E:\CustomAssets` or into the repo first. Recorded 2026-09-18.

**Look here before asking for a decompile.** If a model's source is in this folder, it is Valve's original
source, with the original sequence names, events and controllers. A Crowbar decompile only reconstructs those
([MODEL_WORKFLOW.md](MODEL_WORKFLOW.md#the-one-rule)). One warning: these are the sources as Valve left
them, not necessarily the exact files that built the shipped `.mdl`. Compare the QC with `mdlinfo.py` on the
shipped model before relying on a detail. For example, both `Hgrunt/` and `RGrunt/` have a QC that writes
`hgrunt.mdl`.

**Worth a reading pass at some point.** Besides the assets, the folder has Valve's own documents on mod
work: events, networked entities, predicted player physics, the view, the triangle API, input, and
modelling for Half-Life. None of them has been read for this mod yet. They are listed in the last row of
the table below. Some of them bear directly on open work: `PlayerPhysics.doc` on
[the prediction problem](ROADMAP.md#the-prediction-problem), `NetworkEntity.doc` on sending new state
to the client, and "Modeling for Half-Life.doc" on the model loop. Anything useful found in them belongs
in the doc for its area, with a pointer back here.

## What is in it

| Folder | What it holds | Why the mod cares |
| --- | --- | --- |
| `Monster Models/` | Sources for every monster Valve cut, plus Barney, the human grunt, the heavy-weapons grunt and the tentacle | The [cut monsters](ROADMAP.md#xen-hell-and-the-cut-monsters), the [Panthereye](ROADMAP.md#panthereye) and the [soldiers](ROADMAP.md#soldiers) |
| `Monster Models/cinematics/` | The SMDs for scripted-sequence animations (`panther_scientist_fall`, `c1a3a_zombie_vent`, `cpr`, …) | `panther_scientist_fall` is the Panthereye's cinematic, the likely origin of `monster_cine_panther` |
| `Player Models/player/` | The player model's QC and animations, and every `p_` weapon model | The one shipped model that is built for split upper and lower body animation (see the soldiers entry) |
| `Player Models/DMatch/` | The multiplayer player models | |
| `Weapon Models/` | Viewmodel sources for every stock weapon; `world_models/` has sources for every `w_` pickup, `w_battery` included | The glove textures and the katana already work on viewmodels; the [green Core](ART_DEBT.md#the-alien-module--stand-in-models-and-the-summon-weapons-borrowed-everything) starts from `wrld_battery` |
| `Prop Models/` | 262 compiled prop `.mdl` files | Candidates for [interactable props](ROADMAP.md#interactable-props) |
| `Model Tools/` | `studiomdl.exe`, the 3ds Max and Maya exporters | `studiomdl.exe` is the compiler in [MODEL_WORKFLOW.md](MODEL_WORKFLOW.md) |
| `Sprite Tools/` | `sprgen.exe`, the sprite tutorial | [SPRITE_WORKFLOW.md](SPRITE_WORKFLOW.md) uses the repo's own `utils/sprtool/` instead |
| `Texture Wad Files/`, `Texture Wad Tools/` | `halflife.wad`, `xeno.wad`, `decals.wad`, `liquids.wad`; `qlumpy`, `makels`, `makefont` | |
| `Map Tools/`, `Hammer Editor/` | The original `qcsg`/`qbsp2`/`vis`/`qrad` and Hammer 3.x with its FGDs | Superseded. Maps are built with VHLT in J.A.C.K. ([MAP_WORKFLOW.md](MAP_WORKFLOW.md)) |
| `Map Files/` | `c1a0.rmf`, `c1a0d.rmf`, `env_beam.rmf` with their `.rad` | Valve's own entity scripting (`multi_manager`, `scripted_sequence`, `scripted_sentence`) as a worked example |
| `*.doc` at the root | Events, NetworkEntity, PlayerPhysics, View, Triangle, Input, HLTV, HLVoice, "Modeling for Half-Life", "How to make a mod" | `PlayerPhysics.doc` is about predicted movement, the [prediction problem](ROADMAP.md#the-prediction-problem); `Events.doc` covers client events |

## The monster sources, by QC

| Folder | Builds | Status in Half-Life |
| --- | --- | --- |
| `Diablo` | `panther.mdl` | **The Panthereye.** Cut. The mod's copy comes from HL: Extended |
| `Archer` | `archer.mdl` | Cut |
| `Bigrat` | `bigrat.mdl` | Cut |
| `Floater` | `floater.mdl` | Cut |
| `gasbag` | `gasbag.mdl` | Cut |
| `kingpin` | `kingpin.mdl` | Cut. [Xen hell's boss](ROADMAP.md#xen-hell-and-the-cut-monsters) |
| `Mr_Friendly` | `friendly.mdl`, `bigfriendly.mdl` | Cut |
| `Snapbug` | `snapbug.mdl` | Cut |
| `Stukabat` | `stukabat.mdl` | Cut. A flyer, see [aerial melee](ROADMAP.md#settled-2026-09-17--the-fuel-the-processors-the-air-dash-gate) |
| `Chubtoad` | `chumtoad.mdl` | Cut |
| `Aflock`, `Bird` | `Aflock.mdl`, `boid.mdl` | Flocking creatures |
| `Skeleton`, `Sphere`, `Turret`, `charger` | `skeleton.mdl`, `Sphere.mdl`, `turret.mdl`; `charger` has no QC | Not checked |
| `Hgrunt`, `RGrunt` | `hgrunt.mdl`, `g_hgrunt.mdl` | Shipped. `hgrunt_shared.qc` holds the sequences |
| `Hvyweapons` | `hassault.mdl`, `g_heavy.mdl` | Shipped as the heavy-weapons grunt |
| `Barney`, `Tentacle2`, `bonegibs`, `GlassGibs` | | Shipped |

## Facts already read from it

- **The Panthereye's controllers** (`Diablo/diablo.qc`): `$controller 0 "Bip01 Spine" YR 90 -90` and
  `$controller 1 "Bip01 Spine" ZR 0 50`. Both are on the spine. Its sequences carry the claw events
  (`attack_primary` has events 1 at frames 8 and 11).
- **The grunt's strafes are firing animations** (`Hgrunt/hgrunt_shared.qc`): `strafeleft` and `straferight`
  are made from `strafefire_l2` and `strafefire_r2`, tagged `ACT_STRAFE_LEFT`/`_RIGHT`, and have **no fire
  events**. The standing and crouching MP5 sequences fire at frames 10, 12 and 14 (`event 4/5/6`, with a
  `5001` muzzle flash on each). The grunt's head controller is `$controller 0 "bip01 head" XR -70 70`.

## Half-Life: Decay

A **second** source, and not part of the SDK. The 2007 community PC port of the PS2 expansion is installed
beside Half-Life:

```
D:\GameLibrary\steam\steamapps\common\Half-Life\decay
```

Unlike the SDK, **it holds no sources** — compiled `.mdl` files, a `decay.dll`, `.bsp` maps. What it is good
for is content Half-Life itself does not have, and for reading the port team's entity design out of
`decay.fgd` and out of the map entity blocks, which are plain text inside a `.bsp` and grep cleanly.

Taking a model from here means a Crowbar decompile before it can gain sequences
([MODEL_WORKFLOW.md](MODEL_WORKFLOW.md)), and the AI is written new either way — the port's code ships only
as a DLL.

| What | Where | Why the mod cares |
| --- | --- | --- |
| `flyer.mdl`, `flyer_gibs.mdl` | `models/` | [The alien flyer](ROADMAP.md#the-alien-flyer). A large organic craft with one `idle1` sequence, seven hitboxes, `STUDIO_TRACE_HITBOX` already set, and four gib pieces |
| `decay.fgd` | root | The port's own entities. `monster_alienflyer` rides `path_corner`s that carry two added spawnflags, 8 "Alienflyer laser" and 16 "Alienflyer attack" |
| `maps/*.bsp` | `maps/` | Entity blocks are greppable. `dy_fubar.bsp` is the only map with a flyer in it |
| `sound/ambience/alienflyby2.wav` | `sound/` | Candidate flyer sound, unverified |

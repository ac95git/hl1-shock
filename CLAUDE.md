# CLAUDE.md

Index of project documentation. Read the linked file when a task touches its area — this file is only a map.

## What this is

A Half-Life GoldSrc mod based on [TWHL's Half-Life Updated](https://github.com/twhl-community/halflife-updated), with custom gameplay systems added on top (player skill tree, VGUI inventory). C++17, VGUI1, Visual Studio 2019/2022 on Windows, Makefiles on Linux.

The mod is built around six gameplay pillars — exploration, enhanced combat, custom items, skill trees, inventory management, stealth. [docs/PILLARS.md](docs/PILLARS.md) tracks what each one actually does today; read it before proposing feature work, and [docs/ROADMAP.md](docs/ROADMAP.md) for what is intended and unbuilt.

## Two agents, one repo

Work happens from two surfaces, and either may pick up where the other left off — the docs here are the handoff, so both read and write them the same way.

- **The CLI agent** (Claude Code in a terminal on this machine) — the default for code, builds, commits and the scripted pipelines.
- **The desktop agent** (the Claude desktop app, Cowork) — used when a session needs **connectors**. Sessions there are also attached to the claude.ai project *hl-shock*.

**Both talk to the Blender MCP** since 2026-09-18 (the CLI too, once `/mcp` shows `blender` connected): the live Blender 5.2 on this machine, for inspecting a scene, running `bpy` against it and taking screenshots of it. See [docs/MODEL_WORKFLOW.md](docs/MODEL_WORKFLOW.md#the-blender-mcp-desktop-agent) for what the MCP is for and what it is not. Blender tutoring sessions run from either surface.

The desktop agent reaches this folder through a bridge: a shell on this machine when it starts, otherwise by copying files up to its cloud workspace and writing them back. When that shell is unavailable it cannot build, run scripts or commit — it says so and leaves those to the CLI agent.

## Hand-made art: the rule

Settled 2026-09-18, in a grill on the art workflow. **What Andrei makes by hand is a source file under `E:\CustomAssets`, in a tier of its own that no agent or script ever writes into.** A script turns it into what ships; the repo holds the shipped file and the spec. [docs/ART_DEBT.md](docs/ART_DEBT.md) is the request queue for all of it.

| He makes | Where it lives | The script | Ships as |
| --- | --- | --- | --- |
| A model: `.blend`, his own SMD exports, `textures/*.png` | `E:\CustomAssets\models\blender\<name>\`, opened by its `BRIEF.md` | `E:\CustomAssets\scripts\blender_build.py` | `models/*.mdl` |
| A map's layout, iterated as a plan before any `.map` | `maps/<name>.rooms.txt` (in the repo: it is the intent) | `utils/maptool/greybox.py` | `maps/*.map`, then J.A.C.K. once |
| An icon, greyscale on black | `E:\CustomAssets\sprites\src\<name>.png` | `utils/sprtool/from_png.py` | `sprites/top/*.spr` and its `hud_additions.txt` lines; a weapon's selection pair goes to `sprites/weapon_<name>.txt` |
| A sound, anything FL Studio exports | `E:\CustomAssets\sounds\src\<engine path>.wav` | `utils/sndtool/convert.py` | `sound/<path>.wav` |
| A map texture, at final size | `E:\CustomAssets\textures\wad\<name>.png` | `utils/maptool/wadpack.py` | `wads/topmod.wad` |

Each workflow doc below has the rules for its tier.

## Agent instructions

| File | Read it when |
| --- | --- |
| [instructions/00-PROJECT-OVERVIEW.md](instructions/00-PROJECT-OVERVIEW.md) | Deciding whether a change is in scope, or orienting for the first time |
| [instructions/01-ARCHITECTURE.md](instructions/01-ARCHITECTURE.md) | Working out where code belongs — client/server split, directory layout, entity hierarchy |
| [instructions/02-BUILDING-AND-SETUP.md](instructions/02-BUILDING-AND-SETUP.md) | Compiling the two DLLs, build flags that must not change, or where a build gets installed |
| [instructions/04-CUSTOM-FEATURES.md](instructions/04-CUSTOM-FEATURES.md) | Adding or editing a skill, or linking a skill to a HUD sprite |

Referenced by `00-PROJECT-OVERVIEW.md` but **not yet written**: `10-COMMON-TASKS.md`. Ignore that link until the file exists.

## Docs

| File | Read it when |
| --- | --- |
| [CONTEXT.md](CONTEXT.md) | Naming anything in the custom gameplay layer. It is the glossary — match its vocabulary in code, comments, and commits, and add to it when a new term is settled |
| [docs/PILLARS.md](docs/PILLARS.md) | Orienting on what the mod is for, or checking what a gameplay system actually does today. Update it in the same commit as a change that moves a pillar |
| [docs/ROADMAP.md](docs/ROADMAP.md) | Picking what to build next, or proposing a feature — it records everything intended and unbuilt, the two blockers most of it shares, and the open questions. When an entry is built, its content moves to PILLARS.md and the entry is deleted |
| [docs/SKILL_TREE.md](docs/SKILL_TREE.md) | Adding, pricing or placing a Skill — the agreed tree as one reference: seven Routes, every node, the prerequisite structure, the cross-Route links and the reveal gates. The reasoning behind it is in ROADMAP.md's Routes section |
| [docs/SKILL_MAP.md](docs/SKILL_MAP.md) | Placing any node — the 15×15 board cell by cell, every door between regions, the prices from the suit, and the placement rules. `docs/skill_map.csv` is the same grid as data. Settled 2026-09-15, not built |
| [docs/SKILL_PANEL.md](docs/SKILL_PANEL.md) | Drawing or changing the Skill Tree's page — the circuit theme, the header, the gauge strip, the Reset switch, the 1:1 pan, the palette, the frames, the tooltip. Settled 2026-09-15, not built |
| [docs/STATUS_PANEL.md](docs/STATUS_PANEL.md) | Drawing or changing the Inventory Panel's Status tab — the Modules as fixed Slots on a doll of the suit, which stats get a line and how each number is written, and the server message the numbers come from. Settled and built 2026-09-16 |
| [docs/MINES1.md](docs/MINES1.md) | Touching `mines1`, the Pit Bottom — what it is, the sequence and gates, every room by tier, and the work list from Andrei's walks. One such file per map from 2026-09-22 |
| [docs/MAP_BRIEF.md](docs/MAP_BRIEF.md) | Building or editing a map — the toolchain that is already in use and the FGD sync rule. Closed 2026-09-15: the Skill Point economy is a non-issue, and `topmap` is the default test map |
| [docs/CRAFT_LOG.md](docs/CRAFT_LOG.md) | Starting any session with Andrei making something by hand — the two skill ladders (mapping on `shaft1`, modelling the weapons), how the agent takes part (steps, read-back, compile; never the building), and one entry per session with what was learned. Started 2026-09-20 |
| [docs/MAP_WORKFLOW.md](docs/MAP_WORKFLOW.md) | Making or changing a map — the `.map`-to-install-to-compile loop, the facts that bind it, the tools under `utils/maptool/`, and what the agent can and cannot do in map work |
| [docs/JACK.md](docs/JACK.md) | Telling Andrei which keys to press in J.A.C.K. — the editor's own techniques, one per question asked: face selection, vertex manipulation, cutting an opening into a wall that already exists |
| [docs/PROVING_MAP.md](docs/PROVING_MAP.md) | Looking up GoldSrc unit sizes and editor rules for a first-time mapper. The proving map itself is tested and closed (2026-09-15), a showcase of coworking in mapping; `topmap` is the default test map |
| [docs/MADDENED.md](docs/MADDENED.md) | Touching `monster_maddened`, the maddened miner — what he is today (code, model, numbers, placement) and the open questions for his grill. Built and verified 2026-09-19 |
| [docs/PERCEPTION.md](docs/PERCEPTION.md) | Touching `Look`, `Listen`, the sound list, the monster state machine, squads, or anything deciding whether the player has been noticed. Part 1 documents the base SDK's perception exactly as it is; part 2 is the model this mod adds |
| [docs/STEALTH_CHECKLIST.md](docs/STEALTH_CHECKLIST.md) | Retesting 5f — the cost of a kill, the Search, the silencer. Closed 2026-09-18 with its results at the end; the block on stealth work is lifted |
| [docs/MORNING_CHECKLIST.md](docs/MORNING_CHECKLIST.md) | **Before building on anything from the overnight session of 2026-09-18** — mining, Stations, the Pulse's tail, the Dash in the air, the melee alien grunt. The test rows Andrei reports against, every decision made unattended, and what was left undone. [docs/OVERNIGHT_BRIEF.md](docs/OVERNIGHT_BRIEF.md) is the grill that set it up |
| [docs/HOUR_CHECKLIST.md](docs/HOUR_CHECKLIST.md) | **Before building on anything from the one-hour session of 2026-09-20** — 5g, Phase's effect, the deflected-melee root cause, 5e, the VGUI collector. The rows Andrei reports against. [docs/HOUR_BRIEF.md](docs/HOUR_BRIEF.md) is the brief: the decisions made in advance and on the fly, each with its rejected alternative |
| [docs/adr/](docs/adr/) | Before changing something that looks arbitrary — the decisions recorded there were deliberate and the reasoning is not visible in the code. ADR-0015 is the working split itself: Andrei builds the craft, the agent writes steps and reads back, and a session opens with his sentence, not a menu |
| [docs/TECH_DEBT.md](docs/TECH_DEBT.md) | Touching the skill tree tooltip or the inventory grid — both have known-issue entries with acceptance criteria |
| [docs/ART_DEBT.md](docs/ART_DEBT.md) | Replacing a placeholder sprite or sound. Records what each stand-in is, why it's wrong, and what the replacement has to achieve |
| [docs/SPRITE_WORKFLOW.md](docs/SPRITE_WORKFLOW.md) | Making or changing a HUD sprite — the script-to-`.spr`-to-`hud.txt` loop, the hand-drawn path, the engine constraints that shape the art, and the tools under `utils/sprtool/` |
| [docs/MODEL_WORKFLOW.md](docs/MODEL_WORKFLOW.md) | Making or changing a model — the decompile-Blender-studiomdl loop, the hand-made `blender/<name>/` tier and its brief, the Blender MCP, where sources live outside the repo, the compile-time rotation and other traps, and the tools under `utils/mdltool/` |
| [docs/SOUND_WORKFLOW.md](docs/SOUND_WORKFLOW.md) | Making or changing a sound — FL Studio to `sounds/src/` to the converter to `sound/`, the engine's format, loop cues, and the rule that a sound is judged in the sequence it plays in |
| [docs/HL_SDK.md](docs/HL_SDK.md) | Looking for a tool, a stock model's source or cut content: the official Half-Life SDK folder on `D:` holds Valve's QC/SMD sources for the cut monsters, the grunts, the player, and every stock weapon and pickup. Check it before asking for a decompile. Half-Life: Decay's folder is the second source, compiled models only |
| [BUILDING.md](BUILDING.md) | Setting up a build or packaging the mod |
| [INSTALL.md](INSTALL.md) | Installing the built mod into a Half-Life instance |
| [README.md](README.md) | Upstream context, contributors, licensing |
| [CHANGELOG.md](CHANGELOG.md) / [FULL_UPDATED_CHANGELOG.md](FULL_UPDATED_CHANGELOG.md) | Tracing when an upstream fix landed |
| [docs/tutorials/](docs/tutorials/) | Setting up the .NET SDK / `dotnet script` used by the packaging script (upstream tooling) |

## Scope

Allowed: bug fixes, refactoring, new gameplay features, quality-of-life work, asset fixes.
Out of scope: engine-level changes — graphics upgrades, physics, and anything else below the SDK. Also avoid gameplay overhauls that break compatibility. See [instructions/00-PROJECT-OVERVIEW.md](instructions/00-PROJECT-OVERVIEW.md).

## Layout

- `dlls/` — server: game logic, entities, AI, weapons, save/restore. Authoritative.
- `cl_dll/` — client: HUD, view, input, prediction, `vgui_*` interfaces. Presentation only.
- `common/`, `engine/`, `public/` — SDK and engine headers. Treat as read-only.
- `game_shared/`, `pm_shared/` — code compiled into both DLLs; a change here hits client and server.
- `utils/` — map/model compilers. `utils/sprtool/` — .spr tooling and the scripts that generate the mod's icons. `utils/mdltool/` — .mdl inspection and SMD converters; model sources live in `E:\CustomAssets`, not here. `utils/sndtool/` — the sound converter. `projects/vs2019/projects.sln` — main solution.
- `maps/` — the mod's map sources (`.map`, Valve 220), source of truth for `topmod/maps/`; the `.bsp` is built from them with the VHLT tools in J.A.C.K.'s folder, see [docs/PROVING_MAP.md](docs/PROVING_MAP.md). A new map starts as `maps/<name>.rooms.txt`, the spec `utils/maptool/greybox.py` builds from and the plan is drawn from; the spec stays beside the `.map` as the intent. `utils/maptool/` also holds the two one-shot generators that started `proving.map` and `minemap.map`; no generator is re-run once a map is hand-edited.
- `wads/` — the mod's own WAD, `topmod.wad`, packed by `utils/maptool/wadpack.py` from painted PNGs in `E:\CustomAssets\textures\wad\`; source of truth for `topmod/topmod.wad`, copied by hand like `models/`. See [docs/MAP_WORKFLOW.md](docs/MAP_WORKFLOW.md).
- `models/` — the mod's compiled `.mdl` files, source of truth for `topmod/models/`; copied by hand like `sprites/`. Their sources are in `E:\CustomAssets`, see [docs/MODEL_WORKFLOW.md](docs/MODEL_WORKFLOW.md).
- `sound/` — the mod's sounds, source of truth for `topmod/sound/`; copied by hand like `sprites/`. Loose `.wav`, mono 16-bit PCM, 22050 Hz, made by `utils/sndtool/convert.py` from `E:\CustomAssets\sounds\src\`, see [docs/SOUND_WORKFLOW.md](docs/SOUND_WORKFLOW.md). Imported sounds say where they came from in [docs/ART_DEBT.md](docs/ART_DEBT.md) — `player/recharged.wav` is from Team Fortress 2.
- `events/` — the mod's own event scripts (empty by design; the engine only needs the file to exist), source of truth for `topmod/events/`.
- `sprites/` — the mod's HUD sprites and its `hud.txt`, source of truth for `topmod/sprites/`. `hud.txt` is generated from `hud_additions.txt` by `utils/sprtool/make_hud_txt.py`; copy both to the install after every change, like the FGD.

## Custom systems

- Skill tree — definitions `game_shared/skill_defs.h` (both DLLs), server state `dlls/player_skills.cpp` / `.h`, client `cl_dll/vgui_skilltree.cpp` / `.h`
- Inventory — `cl_dll/vgui_inventory.cpp`, `cl_dll/vgui_inventory_grid.cpp` and headers
- Status page — Module table `game_shared/module_defs.h` (both DLLs), server `SendSkillStatsToClient` in
  `dlls/player_skills.cpp`, client `cl_dll/vgui_status.cpp` / `.h`
- Suit Variant — definitions `game_shared/suit_defs.h` (both DLLs), server `CItemSuit` in `dlls/items.cpp`,
  client `CHud::UpdateSuitVariant` / `SuitColour*` in `cl_dll/hud.cpp`. The value is the player's
  `pev->skin`; there is no save field and no user message for it

Skill ids are saved and are bit positions in the sync message: keep them stable once added, and never reuse a removed one. All static Skill data is one shared table — see [instructions/04-CUSTOM-FEATURES.md](instructions/04-CUSTOM-FEATURES.md).

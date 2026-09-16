# CLAUDE.md

Index of project documentation. Read the linked file when a task touches its area — this file is only a map.

## What this is

A Half-Life GoldSrc mod based on [TWHL's Half-Life Updated](https://github.com/twhl-community/halflife-updated), with custom gameplay systems added on top (player skill tree, VGUI inventory). C++17, VGUI1, Visual Studio 2019/2022 on Windows, Makefiles on Linux.

The mod is built around six gameplay pillars — exploration, enhanced combat, custom items, skill trees, inventory management, stealth. [docs/PILLARS.md](docs/PILLARS.md) tracks what each one actually does today; read it before proposing feature work, and [docs/ROADMAP.md](docs/ROADMAP.md) for what is intended and unbuilt.

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
| [docs/MAP_BRIEF.md](docs/MAP_BRIEF.md) | Building or editing a map — the toolchain that is already in use and the FGD sync rule. Closed 2026-09-15: the Skill Point economy is a non-issue, and `topmap` is the default test map |
| [docs/MAP_WORKFLOW.md](docs/MAP_WORKFLOW.md) | Making or changing a map — the `.map`-to-install-to-compile loop, the facts that bind it, the tools under `utils/maptool/`, and what the agent can and cannot do in map work |
| [docs/PROVING_MAP.md](docs/PROVING_MAP.md) | Looking up GoldSrc unit sizes and editor rules for a first-time mapper. The proving map itself is tested and closed (2026-09-15), a showcase of coworking in mapping; `topmap` is the default test map |
| [docs/PERCEPTION.md](docs/PERCEPTION.md) | Touching `Look`, `Listen`, the sound list, the monster state machine, squads, or anything deciding whether the player has been noticed. Part 1 documents the base SDK's perception exactly as it is; part 2 is the model this mod adds |
| [docs/adr/](docs/adr/) | Before changing something that looks arbitrary — the decisions recorded there were deliberate and the reasoning is not visible in the code |
| [docs/TECH_DEBT.md](docs/TECH_DEBT.md) | Touching the skill tree tooltip or the inventory grid — both have known-issue entries with acceptance criteria |
| [docs/ART_DEBT.md](docs/ART_DEBT.md) | Replacing a placeholder sprite or sound. Records what each stand-in is, why it's wrong, and what the replacement has to achieve |
| [docs/SPRITE_WORKFLOW.md](docs/SPRITE_WORKFLOW.md) | Making or changing a HUD sprite — the script-to-`.spr`-to-`hud.txt` loop, the engine constraints that shape the art, and the tools under `utils/sprtool/` |
| [docs/MODEL_WORKFLOW.md](docs/MODEL_WORKFLOW.md) | Making or changing a model — the decompile-Blender-studiomdl loop, where sources live outside the repo, the compile-time rotation and other traps, and the tools under `utils/mdltool/` |
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
- `utils/` — map/model compilers. `utils/sprtool/` — .spr tooling and the scripts that generate the mod's icons. `utils/mdltool/` — .mdl inspection and SMD converters; model sources live in `E:\CustomAssets`, not here. `projects/vs2019/projects.sln` — main solution.
- `maps/` — the mod's map sources (`.map`, Valve 220), source of truth for `topmod/maps/`; the `.bsp` is built from them with the VHLT tools in J.A.C.K.'s folder, see [docs/PROVING_MAP.md](docs/PROVING_MAP.md). `utils/maptool/` — the one-shot greybox generator that started `proving.map`; not re-run once the map is hand-edited.
- `models/` — the mod's compiled `.mdl` files, source of truth for `topmod/models/`; copied by hand like `sprites/`. Their sources are in `E:\CustomAssets`, see [docs/MODEL_WORKFLOW.md](docs/MODEL_WORKFLOW.md).
- `sound/` — the mod's sounds, source of truth for `topmod/sound/`; copied by hand like `sprites/`. Loose `.wav`, mono 16-bit PCM. Imported sounds say where they came from in [docs/ART_DEBT.md](docs/ART_DEBT.md) — `player/recharged.wav` is from Team Fortress 2.
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

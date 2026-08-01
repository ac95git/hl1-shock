# CLAUDE.md

Index of project documentation. Read the linked file when a task touches its area — this file is only a map.

## What this is

A Half-Life GoldSrc mod based on [TWHL's Half-Life Updated](https://github.com/twhl-community/halflife-updated), with custom gameplay systems added on top (player skill tree, VGUI inventory). C++17, VGUI1, Visual Studio 2019/2022 on Windows, Makefiles on Linux.

The mod is built around five gameplay pillars — exploration, enhanced combat, custom items, skill trees, inventory management. [docs/PILLARS.md](docs/PILLARS.md) tracks what each one actually does today; read it before proposing feature work.

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
| [docs/PILLARS.md](docs/PILLARS.md) | Orienting on what the mod is for, checking what a gameplay system actually does today, or picking what to build next. Update it in the same commit as a change that moves a pillar |
| [docs/adr/](docs/adr/) | Before changing something that looks arbitrary — the decisions recorded there were deliberate and the reasoning is not visible in the code |
| [docs/TECH_DEBT.md](docs/TECH_DEBT.md) | Touching the skill tree tooltip or the inventory grid — both have known-issue entries with acceptance criteria |
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
- `utils/` — map/model compilers. `projects/vs2019/projects.sln` — main solution.

## Custom systems

- Skill tree — server `dlls/player_skills.cpp` / `.h`, client `cl_dll/vgui_skilltree.cpp` / `.h`
- Inventory — `cl_dll/vgui_inventory.cpp`, `cl_dll/vgui_inventory_grid.cpp` and headers

Skill ids are networked and saved: keep them stable once added. Server `SkillDef` data and the client's local metadata table are edited together — see [instructions/04-CUSTOM-FEATURES.md](instructions/04-CUSTOM-FEATURES.md).

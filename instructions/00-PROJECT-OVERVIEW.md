# Half-Life Updated - Project Overview

## What is This Project?

This is a **Half-Life GoldSrc modification** based on the [Half-Life Updated](https://github.com/twhl-community/halflife-updated) project by the TWHL community. It provides an updated version of the Half-Life SDK with modern tooling, bug fixes, and custom enhancements.

## Project Purpose

The primary goals of this project are:

1. **Bug Fixes** - Fix bugs present in the original Half-Life SDK
2. **Modern Tooling** - Support for Visual Studio 2019/2022
3. **Custom Features** - Add new gameplay systems (skill trees, inventory, etc.)
4. **Maintainability** - Clean, well-organized code following modern C++ practices
5. **Modding Base** - Provide a solid foundation for Half-Life mods

## What's In Scope

✅ **Allowed Changes:**
- Bug fixes for game code
- Code refactoring and improvements
- New gameplay features (skills, inventory, etc.)
- Quality of life improvements
- Fixing game-breaking bugs in assets

❌ **Out of Scope:**
- Graphical upgrades (engine-level)
- Physics engine changes
- Other engine modifications
- Major gameplay overhauls that break compatibility

## Key Features

### From Half-Life Updated Base
- **HL25 Integration** - Backported changes from Half-Life 25th Anniversary update
- **Modern Build System** - Visual Studio 2019/2022 project files
- **Bug Fixes** - Hundreds of bug fixes from the community
- **C++17 Support** - Modern C++ standard
- **Cross-platform** - Windows and Linux support

### Custom Additions (This Fork)
- **Player Skill System** - RPG-style skill tree with unlockable abilities
- **Custom Inventory** - VGUI-based inventory grid system
- **Enhanced HUD** - Additional HUD elements and improvements

## Project Structure

```
halflife-updated/
├── cl_dll/          # Client-side DLL (HUD, rendering, input)
├── dlls/            # Server-side DLL (game logic, entities, AI)
├── common/          # Shared headers between client and server
├── game_shared/     # Shared code between client and server
├── pm_shared/       # Player movement code (shared)
├── engine/          # Engine interface headers
├── public/          # Public SDK headers
├── utils/           # Map compilation tools
├── projects/        # Visual Studio project files
├── docs/            # Documentation
└── instructions/    # AI agent instructions (this folder)
```

## Technology Stack

- **Language:** C++17
- **Engine:** GoldSrc (Half-Life 1 engine)
- **Build System:** Visual Studio 2019/2022, Linux Makefiles
- **Platform:** Windows (primary), Linux (supported)
- **UI Framework:** VGUI1 (Valve's GUI system)

## Version Information

- **Base SDK:** Half-Life Updated v1.1.1 (in development)
- **Engine:** GoldSrc (Half-Life 1)
- **Supported Game:** Half-Life (Steam version)
- **C++ Standard:** C++17
- **Visual Studio:** 2019 or 2022

## Important Links

- **TWHL Half-Life Updated:** https://github.com/twhl-community/halflife-updated
- **TWHL Community:** https://twhl.info/
- **TWHL Discord:** https://discord.gg/jEw8EqD
- **Valve Half-Life Issues:** https://github.com/ValveSoftware/halflife/issues
- **TWHL Wiki:** https://twhl.info/wiki/page/Half-Life_Programming_-_Getting_Started

## License

Half Life 1 SDK Copyright © Valve Corp.

This project is licensed under the Half-Life 1 SDK License. You may:
- ✅ Develop and distribute mods for free
- ✅ Modify and distribute the SDK for free
- ❌ Use for commercial purposes without Valve's permission

See LICENSE file for full details.

## Getting Help

1. **TWHL Website:** https://twhl.info/ - General modding help
2. **TWHL Discord:** Channels for modding assistance (not #unified-sdk for general help)
3. **GitHub Issues:** Report bugs specific to Half-Life Updated
4. **This Documentation:** Check the instructions/ folder for detailed guides

## Quick Start

1. Read `01-ARCHITECTURE.md` to understand the codebase structure
2. Read `02-BUILDING-AND-SETUP.md` to set up your development environment
3. Read `04-CUSTOM-FEATURES.md` to understand custom additions
4. Read `10-COMMON-TASKS.md` for common development workflows

## Contributors

This project builds on the work of many contributors to Half-Life Updated. See the main README.md for a full list of contributors.

## Next Steps

- **For Building:** See `02-BUILDING-AND-SETUP.md`
- **For Architecture:** See `01-ARCHITECTURE.md`
- **For Custom Features:** See `04-CUSTOM-FEATURES.md`
- **For Development:** See `10-COMMON-TASKS.md`

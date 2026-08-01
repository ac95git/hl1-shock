# Architecture Overview

## High-Level Architecture

Half-Life uses a **client-server architecture** even in single-player mode. The game is split into two main DLLs:

```
┌─────────────────────────────────────────────────────────┐
│                    Half-Life Engine                      │
│                      (hl.exe)                           │
└────────────┬─────────────────────────┬──────────────────┘
             │                         │
    ┌────────▼────────┐       ┌───────▼────────┐
    │   Client DLL    │       │   Server DLL   │
    │   (client.dll)  │       │   (hl.dll)     │
    │                 │       │                │
    │ • HUD           │       │ • Game Logic   │
    │ • Rendering     │       │ • Entities     │
    │ • Input         │       │ • AI           │
    │ • Prediction    │       │ • Physics      │
    └─────────────────┘       └────────────────┘
```

## Directory Structure

### Core Directories

```
halflife-updated/
│
├── dlls/                    # SERVER-SIDE CODE
│   ├── *.cpp/h             # Game entities, AI, weapons (server)
│   ├── player.cpp/h        # Player entity (server-side)
│   ├── monsters/           # Monster AI implementations
│   ├── weapons.cpp/h       # Weapon base classes
│   └── gamerules.cpp/h     # Game mode rules
│
├── cl_dll/                  # CLIENT-SIDE CODE
│   ├── *.cpp/h             # HUD, rendering, client prediction
│   ├── hud.cpp/h           # HUD system
│   ├── view.cpp            # View/camera code
│   ├── input.cpp           # Input handling
│   ├── vgui_*.cpp/h        # VGUI interface elements
│   └── hl/                 # Half-Life specific client code
│
├── common/                  # SHARED HEADERS (read-only)
│   ├── *.h                 # Engine interfaces, constants
│   ├── const.h             # Game constants
│   ├── entity_state.h      # Network entity state
│   └── weaponinfo.h        # Weapon info structures
│
├── game_shared/             # SHARED CODE (client & server)
│   ├── *.cpp/h             # Code used by both DLLs
│   └── voice_*.cpp/h       # Voice communication
│
├── pm_shared/               # PLAYER MOVEMENT (shared)
│   ├── pm_shared.cpp/h     # Movement physics
│   ├── pm_math.cpp         # Movement math
│   └── pm_debug.cpp        # Movement debugging
│
├── engine/                  # ENGINE HEADERS (read-only)
│   └── *.h                 # Engine API definitions
│
├── public/                  # PUBLIC SDK HEADERS
│   └── *.h                 # Public interfaces
│
├── utils/                   # MAP TOOLS
│   ├── qcsg/               # CSG compiler
│   ├── qbsp2/              # BSP compiler
│   ├── vis/                # Visibility calculator
│   ├── qrad/               # Lighting compiler
│   └── studiomdl/          # Model compiler
│
└── projects/                # BUILD PROJECTS
    └── vs2019/             # Visual Studio 2019/2022 projects
        ├── hldll.vcxproj   # Server DLL project
        ├── hl_cdll.vcxproj # Client DLL project
        └── projects.sln    # Main solution
```

## Client vs Server Responsibilities

### Server DLL (dlls/)

**Authoritative** - The server is the source of truth for game state.

**Responsibilities:**
- ✅ Game logic and rules
- ✅ Entity management and spawning
- ✅ AI and pathfinding
- ✅ Physics simulation
- ✅ Damage calculation
- ✅ Weapon firing (authoritative)
- ✅ Player state management
- ✅ Save/restore system
- ✅ Multiplayer networking (server-side)

**Key Files:**
- `player.cpp/h` - Player entity
- `weapons.cpp/h` - Weapon base classes
- `monsters.cpp/h` - Monster base classes
- `gamerules.cpp/h` - Game mode rules
- `client.cpp` - Client connection handling
- `world.cpp` - World entity and initialization

### Client DLL (cl_dll/)

**Presentation** - The client displays game state and predicts player actions.

**Responsibilities:**
- ✅ HUD rendering
- ✅ View/camera control
- ✅ Input handling
- ✅ Client-side prediction
- ✅ Particle effects
- ✅ Sound playback (client-side)
- ✅ VGUI interfaces
- ✅ Temporary entities (visual only)
- ✅ Interpolation and smoothing

**Key Files:**
- `hud.cpp/h` - HUD system
- `view.cpp` - View and camera
- `input.cpp` - Input handling
- `in_camera.cpp` - Camera control
- `vgui_TeamFortressViewport.cpp` - Main VGUI viewport
- `ev_hldm.cpp` - Event playback (weapon effects)

## Entity System

### Entity Hierarchy

```
CBaseEntity (base class for all entities)
├── CBaseDelay (entities with delayed actions)
│   ├── CBaseAnimating (entities with animations)
│   │   ├── CBaseToggle (doors, buttons, etc.)
│   │   │   ├── CBaseDoor
│   │   │   ├── CBaseButton
│   │   │   └── CBasePlatTrain
│   │   └── CBaseMonster (all NPCs and monsters)
│   │       ├── CBasePlayer (player entity)
│   │       ├── CTalkMonster (talking NPCs)
│   │       │   ├── CBarney
│   │       │   └── CScientist
│   │       ├── CSquadMonster (squad-based AI)
│   │       │   ├── CHGrunt
│   │       │   └── CAlienGrunt
│   │       └── CFlyingMonster (flying creatures)
│   └── CBasePlayerItem (items player can carry)
│       ├── CBasePlayerWeapon (weapons)
│       │   ├── CCrowbar
│       │   ├── CGlock
│       │   ├── CMP5
│       │   └── ... (other weapons)
│       └── CBasePlayerAmmo (ammo pickups)
└── CPointEntity (point entities, no model)
    ├── CLight
    ├── CEnvSound
    └── ... (other point entities)
```

### Entity Creation Flow

```
1. Map loads → Engine parses .bsp file
2. Engine calls DispatchSpawn() for each entity
3. DispatchSpawn() looks up entity classname
4. Creates C++ object via entity factory
5. Calls KeyValue() for each property
6. Calls Spawn() to initialize entity
7. Calls Activate() after all entities spawned
```

## Network Architecture

### Message Flow

```
Server                          Client
  │                               │
  │  Entity State Updates         │
  ├──────────────────────────────>│
  │                               │
  │  User Commands (movement)     │
  │<──────────────────────────────┤
  │                               │
  │  Reliable Messages (HUD)      │
  ├──────────────────────────────>│
  │                               │
  │  Events (weapon fire)         │
  ├──────────────────────────────>│
  │                               │
```

### Client Prediction

The client predicts player movement and weapon behavior to reduce perceived latency:

1. **Client** sends user commands to server
2. **Client** immediately simulates movement locally (prediction)
3. **Server** receives commands, simulates authoritatively
4. **Server** sends back authoritative state
5. **Client** corrects prediction errors if needed

**Predicted Code:**
- Player movement (`pm_shared/`)
- Weapon animations and effects (`cl_dll/ev_hldm.cpp`)
- View bobbing and effects

**Not Predicted:**
- Damage
- Hit detection
- Other players
- Monsters

## Save/Restore System

The game uses a **data-driven save system** with type descriptors:

```cpp
// Define what data to save
TYPEDESCRIPTION CMyEntity::m_SaveData[] =
{
    DEFINE_FIELD(CMyEntity, m_iHealth, FIELD_INTEGER),
    DEFINE_FIELD(CMyEntity, m_vecPosition, FIELD_VECTOR),
    DEFINE_FIELD(CMyEntity, m_hEnemy, FIELD_EHANDLE),
};

IMPLEMENT_SAVERESTORE(CMyEntity, CBaseEntity);
```

**Saved Data:**
- Entity variables marked in TYPEDESCRIPTION
- Global state (cross-level data)
- Player inventory and stats

**Not Saved:**
- Temporary effects
- Client-side only data
- Derived/calculated values

## Memory Management

### Entity Allocation

Entities use **custom new/delete operators** that allocate from the engine's entity pool:

```cpp
void* CBaseEntity::operator new(size_t stAllocateBlock)
{
    return ::operator new(stAllocateBlock);
}

void CBaseEntity::operator delete(void* pMem)
{
    ::operator delete(pMem);
}
```

### EHANDLE System

**EHANDLE** provides safe entity references that automatically become NULL if the entity is deleted:

```cpp
EHANDLE m_hEnemy;  // Safe reference to enemy
m_hEnemy = pEnemy; // Assign
if (m_hEnemy != NULL) // Check validity
{
    CBaseEntity* pEnemy = m_hEnemy; // Get pointer
}
```

## Build System

### Visual Studio Projects

```
projects/vs2019/projects.sln
├── hldll.vcxproj        # Server DLL (hl.dll)
├── hl_cdll.vcxproj      # Client DLL (client.dll)
└── utils.sln            # Map compilation tools
```

### Build Configurations

- **Debug** - Full debugging symbols, no optimization
- **Release** - Optimized, minimal debug info

### Output Locations

After building:
```
dlls/hl.dll          # Server DLL
cl_dll/client.dll    # Client DLL
```

## Key Design Patterns

### 1. Entity Factory Pattern
Entities are registered and created by classname:
```cpp
LINK_ENTITY_TO_CLASS(monster_scientist, CScientist);
```

### 2. Think/Touch Callbacks
Entities use function pointers for delayed actions:
```cpp
SetThink(&CMyEntity::MyThinkFunction);
pev->nextthink = gpGlobals->time + 1.0;
```

### 3. Schedule/Task AI System
Monsters use schedules (sequences of tasks) for AI:
```cpp
Schedule_t* GetSchedule() override;
void StartTask(Task_t* pTask) override;
void RunTask(Task_t* pTask) override;
```

### 4. Message System
Server sends messages to clients:
```cpp
MESSAGE_BEGIN(MSG_ONE, gmsgDamage, NULL, pPlayer->pev);
    WRITE_BYTE(damage);
    WRITE_COORD(vecOrigin.x);
MESSAGE_END();
```

## Threading Model

**Single-threaded** - The game runs on a single thread with a fixed tick rate:
- Server tick rate: 100 Hz (10ms per frame)
- Client frame rate: Variable (vsync or unlimited)

## Next Steps

- **Code Structure:** See `03-CODE-STRUCTURE.md`
- **Custom Features:** See `04-CUSTOM-FEATURES.md`
- **Entity System:** See `05-ENTITY-SYSTEM.md`
- **Building:** See `02-BUILDING-AND-SETUP.md`

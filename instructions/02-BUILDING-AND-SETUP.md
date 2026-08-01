# Building and Setup

How to compile the two mod DLLs — `hl.dll` (server) and `client.dll` (client). Everything else in the solution (map compilers, model tools) is upstream tooling and is not needed to build the mod.

## Requirements

### Windows

- **Visual Studio 2019 or 2022** with the *Desktop development with C++* workload, including:
  - the **v143** platform toolset (both projects pin `<PlatformToolset>v143</PlatformToolset>`)
  - a **Windows 10 SDK** (`WindowsTargetPlatformVersion` is `10.0`, so any installed 10.x version resolves)
- Nothing else needs installing. The third-party libraries are vendored in-tree:
  - `utils/vgui/lib/win32_vc16/vgui.lib` — VGUI1
  - `lib/public/SDL2.lib` — SDL2 import library

**x86 only.** The only configurations are `Debug|Win32` and `Release|Win32`. There is no x64 configuration and there should not be one — GoldSrc only loads 32-bit mod DLLs.

### Linux

- **GCC 9 or newer** (C++17)
- `g++-multilib` (32-bit target)
- `libgl1-mesa-dev` or equivalent OpenGL headers

## Building on Windows

### From the IDE

1. Open `projects/vs2019/projects.sln`.
2. Set the configuration to `Debug` or `Release` and the platform to `Win32`.
3. Build the `hldll` and `hl_cdll` projects (right-click → Build), or Build → Build Solution to build the utilities as well.

### From the command line

Locate MSBuild:

```powershell
& "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" `
    -latest -requires Microsoft.Component.MSBuild -property installationPath
```

MSBuild lives at `<installationPath>\MSBuild\Current\Bin\MSBuild.exe`. Then:

```powershell
& "<path to>\MSBuild.exe" projects\vs2019\projects.sln `
    -t:hldll`;hl_cdll -p:Configuration=Debug -p:Platform=Win32 -m -v:minimal -nologo
```

- In PowerShell the `;` separating targets must be escaped as `` `; `` or the whole argument quoted.
- `-t:hldll:Rebuild;hl_cdll:Rebuild` forces a clean rebuild.
- `-m` builds the two projects in parallel; `MultiProcessorCompilation` is already enabled inside each project, so files within a project compile in parallel regardless.
- Use `-p:Configuration=Release` for the shipping build.

A clean rebuild of both DLLs takes roughly 25 seconds and should produce **0 warnings, 0 errors**. Treat new warnings as regressions — the tree is currently warning-clean at `/W3`.

## Build output

| | `hldll` | `hl_cdll` |
| --- | --- | --- |
| Project | `projects/vs2019/hldll.vcxproj` | `projects/vs2019/hl_cdll.vcxproj` |
| Output | `projects/vs2019/<Config>/hldll/hl.dll` | `projects/vs2019/<Config>/hl_cdll/client.dll` |
| Preprocessor | `VALVE_DLL`, `CLIENT_WEAPONS`, `QUIVER`, `VOXEL`, `QUAKE2` | `CLIENT_DLL`, `CLIENT_WEAPONS`, `HL_DLL` |
| Exports | `dlls/hl.def` module-definition file | `__declspec(dllexport)` in source |
| Extra link libs | — | `vgui.lib`, `wsock32.lib`, `SDL2.lib` |
| Include dirs | `dlls`, `engine`, `common`, `pm_shared`, `game_shared`, `public` | the same plus `cl_dll`, `cl_dll/particleman`, `utils/vgui/include`, `external` |

Shared settings: C++17 (`stdcpp17`), RTTI on, warning level 3, `_CRT_SECURE_NO_WARNINGS`, and `/Zc:threadSafeInit-`.

`/Zc:threadSafeInit-` is deliberate — it disables the thread-safe-statics guard, whose runtime support does not behave under the engine's DLL loading. Do not remove it.

`EnableClangTidyCodeAnalysis` is set on both projects, so a clang-tidy pass runs alongside compilation in the IDE.

Debug builds use the `MultiThreadedDebug` CRT with optimisations off; Release uses `MultiThreaded`, `/O2`, whole-program optimisation, COMDAT folding, and reference optimisation. Both generate PDBs.

## Installing — happens automatically

**Building the mod also installs it.** Both projects run a post-build event that invokes `filecopy.bat` in the repository root, which robocopies into a hardcoded mod directory:

```bat
rem filecopy.bat, line 5
set mod_directory=D:\Apps\steam\steamapps\common\Half-Life\topmod
```

Each successful build copies:

| From | To |
| --- | --- |
| `<OutDir>/hl.dll`, `hl.pdb` | `<mod_directory>/dlls/` |
| `<OutDir>/client.dll`, `client.pdb` | `<mod_directory>/cl_dlls/` |
| `network/delta.lst` | `<mod_directory>/` |

Consequences worth knowing:

- Every build overwrites the live mod install, including `delta.lst`. There is no separate install step to forget — and no way to build without touching the game directory unless you ask for one.
- To build **without** installing, pass `-p:PostBuildEventUseInBuild=false` on the MSBuild command line.
- To change the destination, edit line 5 of `filecopy.bat`. It is a checked-in absolute path, so changing it is a tracked modification; the file's own comment suggests using an environment variable if that matters for your setup.
- `filecopy.bat` ends with `exit /b 0` because robocopy returns non-zero exit codes on success, which MSBuild would otherwise interpret as a build failure. Keep that line.

## Building on Linux

`linux/Makefile` is a driver that delegates to `Makefile.hldll` and `Makefile.hl_cdll`.

```bash
cd linux
make              # CFG=release (default)
make CFG=debug
make clean        # removes object files only
```

Output is `hl.so` and `client.so` under `linux/release/` or `linux/debug/`. There is no install step on Linux — copy the `.so` files into the mod directory yourself.

The build targets 32-bit x86 (`g++ -m32`, `-march=pentium-m -mfpmath=387 -mno-sse`) and links `libstdc++`/`libgcc` statically. Override the compiler with `make COMPILER=clang++`.

Two GCC flags in `linux/Makefile` are load-bearing and must not be dropped:

- `-flifetime-dse=1` — without it GCC optimises away the `memset` inside `CBaseEntity::operator new`, leaving entities with uninitialised fields.
- `-fno-gnu-unique` — without it `dlclose` cannot unload the mod DLLs, so they retain state across map loads and crash the engine on invalid memory access.

## Packaging

See [BUILDING.md](../BUILDING.md). Packaging uses `dotnet script` with `scripts/packager/CreatePackage.csx`; setup for the .NET SDK is documented under [docs/tutorials/](../docs/tutorials/).

## Troubleshooting

| Symptom | Cause |
| --- | --- |
| `MSB8020` / toolset not found | v143 build tools not installed — add them via the VS Installer |
| Link errors on `vgui.lib` or `SDL2.lib` | Building a configuration other than `Win32`; the vendored libs are 32-bit only |
| Build succeeds but the game runs old code | Check that `filecopy.bat`'s `mod_directory` points at the mod you are actually launching |
| Post-build step reports failure | `filecopy.bat` lost its trailing `exit /b 0`, or the mod directory does not exist |
| Changes to `game_shared/` or `pm_shared/` seem to only half-apply | Those directories compile into *both* DLLs — rebuild both, not just one |

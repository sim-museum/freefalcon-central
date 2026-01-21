# Claude Code Guidelines

## Working Directory

You can do anything except delete files outside your working directory without asking for confirmation.

## Autonomy

Continue working without additional confirmation prompts. Keep making progress on the task at hand.

---

# FreeFalcon Linux Port - Progress Documentation

## Overview

This document tracks the progress of porting FreeFalcon (F-16 flight simulator) from Windows to Linux. The port uses SDL2 for windowing, OpenGL for rendering (replacing DirectDraw/Direct3D), and OpenAL for audio.

## Completed Work

### Session: January 21, 2026 - UI Resource Loading and Rendering

#### Problem 1: std::bad_alloc Crash During Resource Loading

**Root Cause:** The `long` type is 8 bytes on 64-bit Linux but only 4 bytes on 32-bit Windows. When reading binary .idx/.rsc resource files (created on Windows), the code was reading 8 bytes instead of 4 bytes for size/version fields, resulting in garbage values and memory allocation failures.

**Fix:** Changed `long` to `int32_t` in `cresmgr.cpp` for file I/O operations:
- `LoadIndex()`: Changed `size` variable from `long` to `int32_t`
- `LoadData()`: Changed `size` variable from `long` to `int32_t`
- Both functions: Changed `fread(&size, sizeof(long), 1, fp)` to `fread(&size, sizeof(int32_t), 1, fp)`
- Version reading: Used temporary `int32_t` variable for reading version field

**Files Modified:**
- `src/ui95/cresmgr.cpp`

#### Problem 2: Image Names Read Incorrectly from .idx Files

**Root Cause:** The `ImageHeader`, `SoundHeader`, and `FlatHeader` structs used `long` for several fields. Since `long` is 8 bytes on 64-bit Linux but the binary files were created with 4-byte fields, the struct layout was misaligned. This caused the `ID` (image name) field to be read from the wrong offset, resulting in garbled names like `'AIN_BG'` instead of `'UI_MAIN_BG'`.

**Fix:** Changed all `long` fields to `int32_t` and `short` to `int16_t` in the header structs to match the Windows 32-bit binary format:

**Files Modified:**
- `src/ui95/imagersc.h` - ImageHeader struct:
  ```cpp
  int32_t Type;         // Was: long
  char    ID[32];
  int32_t flags;        // Was: long
  int16_t centerx;      // Was: short
  int16_t centery;
  int16_t w;
  int16_t h;
  int32_t imageoffset;  // Was: long
  int32_t palettesize;  // Was: long
  int32_t paletteoffset;// Was: long
  ```

- `src/ui95/soundrsc.h` - SoundHeader struct:
  ```cpp
  int32_t Type;
  char    ID[32];
  int32_t flags;
  int16_t Channels;
  int16_t SoundType;
  int32_t offset;
  int32_t headersize;
  ```

- `src/ui95/flatrsc.h` - FlatHeader struct:
  ```cpp
  int32_t Type;
  char    ID[32];
  int32_t offset;
  int32_t size;
  ```

#### Problem 3: Resource Files Not Found on Linux

**Root Cause:** The `OpenResFile()` function in `cresmgr.cpp` was using Windows-style backslashes (`\`) and regular `fopen()`, which failed on case-sensitive Linux filesystems.

**Fix:** Added Linux-specific code path in `OpenResFile()`:
1. Convert backslashes to forward slashes in resource paths
2. Use `fopen_nocase()` for case-insensitive file lookup
3. Handle double "art/" prefix issue (when resource name starts with "art\" and directory already ends with "/art")

**Files Modified:**
- `src/ui95/cresmgr.cpp` - Added `#ifdef FF_LINUX` block in `OpenResFile()`

#### Problem 4: Missing mainbg.irc File

**Root Cause:** The game data was missing `art/resource/mainbg.irc` which is referenced in `MAIN_res.LST`.

**Fix:** Created the missing file with content:
```
[LOADRES]  MAIN_BG_RESOURCE "art\resource\mainbg"
```

**Files Created:**
- `/home/g/ese/SAT/WP/drive_c/FreeFalcon6/art/resource/mainbg.irc` (in game data, not source)

### Result

After these fixes, the landing page now renders correctly:
- UI resources (.idx/.rsc files) load successfully
- Image IDs are correctly resolved (e.g., "UI_MAIN_BG" maps to the correct image)
- The main splash screen displays with actual content instead of black screen
- Screenshot saved to: `screenshot_test.bmp`

### Session: January 21, 2026 - Segfault Fix and Mouse Event Routing

#### Problem 5: Segfault After Initial Rendering

**Root Cause:** The `IMAGE_RSC::Blit()` function (and related `Blend()`, `Scale*()` functions) were called with a NULL `surface->mem` pointer when drawing was triggered outside of a proper Lock/Unlock cycle, or when the surface wasn't properly initialized.

**Fix:** Added NULL safety checks at the start of all surface-accessing functions:
- `IMAGE_RSC::Blit()` - Added check for `!surface || !surface->mem`
- `IMAGE_RSC::Blend()` - Added check for `!surface || !surface->mem`
- `IMAGE_RSC::ScaleDown8()` - Added check
- `IMAGE_RSC::ScaleDown8Overlay()` - Added check
- `IMAGE_RSC::ScaleUp8()` - Added check
- `IMAGE_RSC::ScaleUp8Overlay()` - Added check

**Files Modified:**
- `src/ui95/imagersc.cpp`

#### Problem 6: Mouse Events Not Working

**Root Cause:** SDL mouse events were being converted to Windows messages (WM_LBUTTONDOWN, WM_LBUTTONUP, etc.) and posted to the message queue, but `ProcessGameMessages()` was not routing these messages to the UI handler (`gMainHandler->EventHandler()`). They were falling through to the `default:` case and being ignored.

**Fix:** Added explicit handling in `ProcessGameMessages()` to route mouse and keyboard events to `gMainHandler->EventHandler()`:
```cpp
case WM_LBUTTONDOWN:
case WM_LBUTTONUP:
case WM_RBUTTONDOWN:
case WM_RBUTTONUP:
case WM_MOUSEMOVE:
case WM_KEYDOWN:
case WM_KEYUP:
    if (gMainHandler != nullptr) {
        gMainHandler->EventHandler(NULL, msg.message, msg.wParam, msg.lParam);
    }
    break;
```

**Files Modified:**
- `src/ffviper/main_linux.cpp`

#### Problem 7: Mouse Coordinate Scaling

**Root Cause:** The SDL window is 640x480 but the UI surface is 1024x768. Mouse coordinates from SDL events need to be scaled to match the UI surface coordinates.

**Fix:** Added coordinate scaling in the SDL event loop:
```cpp
int scaledX = event.button.x * 1024 / WINDOW_WIDTH;  // 640
int scaledY = event.button.y * 768 / WINDOW_HEIGHT;  // 480
```

**Files Modified:**
- `src/ffviper/main_linux.cpp`

### Result After Session 2

- Segfault is fixed - application runs without crashing
- Mouse events are now routed to the UI handler
- Mouse coordinates are scaled from SDL window to UI surface dimensions
- Main menu button callbacks are hooked up (found in `ui_main.cpp::HookupControls()`)
- Exit, Setup, Campaign, Dogfight buttons should respond to clicks

### Main Menu Code Structure (Reference)

The main menu code is organized as follows:

**UI Definition Files:**
- `main_scf.lst` - Main menu window definitions
- `art/pop_scf.lst` - Popup menu definitions

**Code Files:**
- `src/ui/src/ui_main.cpp` - Main menu initialization and button callbacks
  - `LoadMainWindow()` - Loads main menu resources and windows
  - `HookupControls(ID)` - Sets up button callbacks
  - `ExitButtonCB()` - Exit button handler
  - `OpenSetupCB()` - Setup button handler
  - `OpenMainCampaignCB()` - Campaign button handler
  - `OpenDogFightCB()` - Dogfight button handler
  - `OpenInstantActionCB()` - Instant Action handler
- `src/ui/src/dogfight/dogmenus.cpp` - `HookupDogFightMenus()`
- `src/ui/src/campaign/campmenu.cpp` - `HookupCampaignMenus()`

**Button IDs (from userids.h):**
- `EXIT_CTRL` (80000) - Exit button
- `SP_MAIN_CTRL` (70003) - Setup button
- `CP_MAIN_CTRL` (40003) - Campaign button
- `DF_MAIN_CTRL` (20003) - Dogfight button
- `IA_MAIN_CTRL` (10003) - Instant Action button

## Known Issues

### Diagnostic Code
Several debug fprintf statements were added during investigation. These should be removed or wrapped in `#ifdef DEBUG` for release builds.

## Architecture Notes

### UI95 System
The UI uses a custom "UI95" windowing system with these key components:
- `C_Handler` - Main UI handler
- `C_Parser` - Script parser for .scf files
- `C_Window` - Window management
- `C_Bitmap` / `O_Output` - Image rendering
- `C_Resmgr` - Resource manager for .idx/.rsc files
- `C_Image` / `gImageMgr` - Global image manager

### Resource File Format
- `.idx` files - Index files containing headers for images/sounds/flat resources
- `.rsc` files - Actual resource data (image pixels, sound samples, etc.)
- `.irc` files - Script files that define which resources to load
- `.scf` files - UI script files defining windows, buttons, layouts
- `.id` files - Text-to-numeric ID mapping tables

### Key Directories
- `src/ui95/` - UI system implementation
- `src/compat/` - Linux compatibility layer (Windows API emulation)
- Game data: `/home/g/ese/SAT/WP/drive_c/FreeFalcon6/`

## Build Instructions

```bash
cd /home/g/ese/SAT/freefalcon-central/build
ninja
./src/ffviper/FFViper -window
```

### Session: January 21, 2026 - Button Click Detection Working

#### Problem 8: Button Clicks Not Being Detected

**Root Cause:** Mouse clicks were received by the UI system, but button hit detection was failing because:
1. The coordinates weren't reaching the correct button positions
2. The Exit button is positioned at UI coordinates (0, 728) - at the bottom of the 768-pixel tall screen

**Investigation:**
- Added debug output to `C_Handler::EventHandler()` to trace mouse events
- Added debug output to `C_Window::GetControl()` to see control hit testing
- Added debug output to `C_Button::CheckHotSpots()` to see why hits were failing
- Discovered Exit button (ID=80000) position: X=0, Y=728

**Fix:** Clicking at the correct UI coordinates (mapping SDL 640x480 to UI 1024x768):
- Exit button: UI (0, 728) -> SDL (0, 454) approximately
- Click at UI (49, 750) successfully triggers the Exit button callback

**Result:**
```
[CheckHotSpots] ID=80000 pos=(0,728) check at (49,750)
[ExitButtonCB] Called with hittype=52 (need 52 for LMOUSEUP)
[ExitButtonCB] Processing exit!
```

**Files Modified with Debug Code (to be cleaned up):**
- `src/ui95/chandler.cpp` - EventHandler debug output
- `src/ui95/cwindow.cpp` - GetControl debug output
- `src/ui95/cbuttons.cpp` - CheckHotSpots debug output
- `src/ui/src/ui_main.cpp` - Button callback debug output

### Coordinate System Summary

| Layer | Resolution | Notes |
|-------|-----------|-------|
| SDL Window | 640x480 | Physical window on screen |
| UI Surface | 1024x768 | Internal UI rendering surface |
| Scaling | X: *1024/640, Y: *768/480 | SDL to UI coordinates |

**Button Positions (UI coordinates):**
- Exit button: (0, 728) - bottom left
- Other main menu buttons: Need to discover during development

---

## High-Level Codebase Architecture

### Project Overview

FreeFalcon is a comprehensive F-16 combat flight simulator originally developed for Windows using DirectX. This Linux port replaces the Windows-specific subsystems with cross-platform alternatives.

### Directory Structure

```
freefalcon-central/
├── src/
│   ├── acmi/           # ACMI (Air Combat Maneuvering Instrumentation) replay system
│   ├── campaign/       # Campaign system - strategic layer
│   │   ├── camplib/    # Campaign library (objectives, persistence, units)
│   │   ├── camptask/   # AI task management (air, ground, naval units)
│   │   ├── camptool/   # Campaign development tools
│   │   ├── campui/     # Campaign UI screens
│   │   └── campupd/    # Campaign update logic
│   ├── codelib/        # Core utilities and shared code
│   │   └── resources/  # Resource management system
│   ├── comms/          # Network communications
│   ├── compat/         # Windows API compatibility layer (Linux port)
│   ├── crashhandler/   # Crash reporting and debugging
│   ├── falclib/        # Core Falcon library
│   │   ├── include/    # Core headers
│   │   └── msgsrc/     # Network message classes
│   ├── falcsnd/        # Sound system
│   ├── ffviper/        # Linux port main executable
│   ├── graphics/       # Graphics subsystem
│   │   ├── 3dlib/      # 3D rendering context
│   │   ├── bsplib/     # BSP (Binary Space Partition) rendering
│   │   ├── ddstuff/    # DirectDraw compatibility
│   │   ├── dxengine/   # DirectX engine wrapper (uses OpenGL on Linux)
│   │   ├── objects/    # Drawable objects (buildings, vehicles, etc.)
│   │   ├── renderer/   # Scene rendering
│   │   ├── terrain/    # Terrain rendering
│   │   ├── texture/    # Texture management
│   │   └── weather/    # Weather and time-of-day effects
│   ├── sim/            # Flight simulation core
│   │   ├── aircraft/   # Aircraft physics and systems
│   │   ├── airframe/   # Aerodynamics model
│   │   ├── cockpit/    # Cockpit instruments and panels
│   │   ├── digi/       # AI pilot (digital pilot)
│   │   ├── displays/   # MFD (Multi-Function Display) rendering
│   │   ├── fcc/        # Fire Control Computer
│   │   ├── ground/     # Ground vehicle AI
│   │   ├── guns/       # Gun ballistics
│   │   ├── missile/    # Missile guidance and physics
│   │   ├── otwdrive/   # Out-The-Window view driver
│   │   ├── radar/      # Radar simulation
│   │   └── rwr/        # Radar Warning Receiver
│   ├── ui/             # Game UI (menus, briefings, etc.)
│   │   └── src/        # UI implementation
│   │       ├── campaign/   # Campaign UI
│   │       ├── comms/      # Multiplayer UI
│   │       ├── dogfight/   # Dogfight setup
│   │       ├── instant/    # Instant action
│   │       ├── logbook/    # Pilot logbook
│   │       ├── setup/      # Settings screens
│   │       └── taceng/     # Tactical engagement
│   ├── ui95/           # Custom UI widget toolkit
│   └── vu2/            # VU (Virtual Universe) entity management
└── build/              # CMake build directory
```

### Key Subsystems

#### 1. UI95 Widget Toolkit (`src/ui95/`)

A custom UI framework with these key classes:
- `C_Handler` - Main event dispatcher and window manager
- `C_Window` - Window container for controls
- `C_Button`, `C_ListBox`, `C_EditBox`, etc. - UI controls
- `C_Resmgr` - Resource file manager (.idx/.rsc files)
- `C_Parser` - Script parser for .scf UI definition files

#### 2. Graphics Engine (`src/graphics/`)

The rendering system, originally DirectX-based:
- `DeviceManager` - Graphics device abstraction
- `Render3D`, `Render2D` - 2D/3D rendering contexts
- `ObjectLOD`, `ObjectParent` - 3D model management
- `TViewpoint` - Terrain viewpoint rendering
- `RealWeather` - Dynamic weather system

#### 3. Simulation Core (`src/sim/`)

The flight model and aircraft systems:
- `AircraftClass` - Main aircraft entity
- `AirframeClass` - Aerodynamics and flight model
- `SimVehicleClass` - Base class for simulated vehicles
- `RadarClass`, `RwrClass` - Avionics sensors
- `SMSClass` - Stores Management System (weapons)

#### 4. Campaign System (`src/campaign/`)

Strategic layer of the game:
- `CampaignClass` - Main campaign state
- `FlightClass` - Air mission flights
- `PackageClass` - Mission packages
- `ObjectiveClass` - Strategic objectives
- `UnitClass` - Military unit representation

#### 5. VU2 Entity System (`src/vu2/`)

Distributed entity management:
- `VuEntity` - Base entity class
- `VuDatabase` - Entity database
- `VuMessage` - Network message passing
- `VuSessionManager` - Multiplayer session management

### Linux Port Components

The `src/compat/` directory provides Windows API compatibility:
- `windows.h` - Windows types and macros
- `d3d.h`, `ddraw.h` - DirectX stub interfaces
- `winuser.h` - Window messaging
- `winsock.h` - Network socket compatibility

The `src/ffviper/` directory contains the Linux main entry point:
- `main_linux.cpp` - SDL2 initialization and game loop
- Replaces the Windows `WinMain()` entry point

### Build System

Uses CMake with the following structure:
- Top-level `CMakeLists.txt` - Project configuration
- Per-directory `CMakeLists.txt` - Library definitions
- Output: `build/src/ffviper/FFViper` executable

### Data Files

Game data location: `/home/g/ese/SAT/WP/drive_c/FreeFalcon6/`

Key data directories:
- `art/` - UI resources and textures
- `terrdata/` - Terrain and object data
- `campaign/` - Campaign files and saves
- `config/` - Configuration files

---

## Next Steps

1. ~~Investigate and fix the segfault after initial rendering~~ ✓ Fixed
2. ~~Get main menu functionality working~~ ✓ Exit button working
3. Test remaining main menu buttons (Setup, Campaign, Dogfight)
4. Remove or conditionalize debug output
5. Continue with other UI screens
6. ~~Generate Doxygen documentation for the codebase~~ ✓ Done

---

# Deep Architecture Analysis (from Doxygen)

## Overview

FreeFalcon is a ~3500-file C/C++ flight simulator with 5 major subsystems interconnected through a distributed entity framework (VU2). Originally Windows/DirectX, now being ported to Linux/SDL2/OpenGL.

**Statistics:**
- Source files: ~3521 (.cpp/.h)
- Documented by Doxygen: ~2918 files
- Major classes: 500+ (hierarchical inheritance)
- Lines of code: ~800K+ estimated

**Execution Model:**
- Multi-threaded: Sim thread, Graphics thread, UI thread, Network/VU thread, Campaign thread
- Event-driven UI with callback system
- Time-sliced AI with configurable update intervals
- Networked entity replication via VU2

---

## Subsystems

### 1. Simulation Core (`src/sim/`)

**Purpose:** Real-time flight physics, aircraft systems, weapons, AI pilots

**Key Entry Points:**
| Function/Class | Location | Purpose |
|----------------|----------|---------|
| `SimulationDriver::Startup()` | simloop/ | One-time initialization |
| `SimulationDriver::Enter()` | simloop/ | Enter SIM from UI |
| `SimulationLoopControl::Loop()` | simloop/ | Main simulation loop |
| `SimBaseClass::Exec()` | simlib/ | Per-entity update (virtual) |
| `AirframeClass::Exec()` | airframe/ | Physics integration |

**External Interfaces:**
- **Input:** `PilotInputs` (joystick/keyboard), VU messages (network), Campaign commands
- **Output:** VU entity updates, damage/death messages, graphics drawables, sound FX
- **Files:** Aircraft data tables (.dat), weapon configs

**Global State:**
- `SimDriver` - Global simulation controller
- `OTWDriver` - Graphics driver instance
- `SimLibElapsedTime`, `SimLibFrameCount` - Frame timing
- `gOutOfSimFlag`, `EndFlightFlag` - State flags

**Important Configuration:**
- `AuxAeroData` - Per-aircraft engine/fuel parameters
- `AeroData` - Mach/alpha lift/drag tables
- `SimLibMinorFrameTime` / `SimLibMajorFrameTime` - Update intervals

---

### 2. Campaign System (`src/campaign/`)

**Purpose:** Strategic layer - mission planning, AI commanders, unit management, persistence

**Key Entry Points:**
| Function/Class | Location | Purpose |
|----------------|----------|---------|
| `CampaignClass::InitCampaign()` | campupd/ | Load and start campaign |
| `TheCampaign.LoopStarter()` | campupd/ | Campaign thread main loop |
| `UpdateUnit()` | campupd/update.cpp | Per-unit movement/combat |
| `AirTaskingManager::Task()` | camptask/ | Air mission planning |
| `GroundTaskingManager::Task()` | camptask/ | Ground unit orders |

**External Interfaces:**
- **Input:** Scenario files (.scn), player commands, network messages
- **Output:** Unit orders, mission assignments, save files (.sav), event broadcasts
- **Files:** Theater data, objective database, unit rosters

**Global State:**
- `TheCampaign` - Global campaign instance
- `campCritical` - Thread safety critical section
- `ObjectiveNS`, `FlightNS`, etc. - ID namespace generators

**Important Configuration:**
- `CampaignTime` - Game clock
- `GroundRatio`, `AirRatio` - Force strength calculations
- `CampMapData`, `SamMapData` - Occupation grids

---

### 3. Graphics Engine (`src/graphics/`)

**Purpose:** 3D rendering, terrain, objects, effects, HUD

**Key Entry Points:**
| Function/Class | Location | Purpose |
|----------------|----------|---------|
| `RenderOTW::StartDraw()` | renderer/ | Begin frame rendering |
| `CDXEngine` (singleton) | dxengine/ | D3D7 context management |
| `TheStateStack.SetContext()` | bsplib/ | Transform state management |
| `ObjectLOD::Fetch()` | bsplib/ | Lazy model loading |
| `TextureBankClass::Reference()` | texture/ | Texture loading |

**External Interfaces:**
- **Input:** Model files (.LOD), texture files (.TEX), terrain data
- **Output:** Rendered frames to display surface
- **APIs:** DirectDraw7/Direct3D7 (Linux: OpenGL via compat layer)

**Global State:**
- `TheDXEngine` - D3D rendering context
- `TheStateStack` - Transform/lighting state (20-deep stack)
- `TheColorBank` - Color palette manager
- `TheTextureBank` - Texture cache with ref counting
- `TheObjectLODs[]` - Model LOD array

**Important Configuration:**
- Render states: 38+ predefined (STATE_SOLID, STATE_TEXTURE, etc.)
- LOD thresholds, texture quality settings
- View frustum parameters

---

### 4. UI95 Widget Toolkit (`src/ui95/`)

**Purpose:** Custom UI framework - windows, buttons, resource loading

**Key Entry Points:**
| Function/Class | Location | Purpose |
|----------------|----------|---------|
| `C_Handler::EventHandler()` | chandler.cpp | Main event dispatcher |
| `C_Window::GetControl()` | cwindow.cpp | Hit testing |
| `C_Parser::ParseScript()` | cparser.cpp | Load .scf UI definitions |
| `C_Resmgr::LoadIndex()` | cresmgr.cpp | Load .idx/.rsc resources |
| `gMainHandler` | (global) | Singleton handler |

**External Interfaces:**
- **Input:** Mouse/keyboard (SDL events → Windows messages), .scf scripts
- **Output:** Rendered UI surfaces, callback invocations
- **Files:** .idx (index), .rsc (resources), .scf (scripts), .id (ID tables)

**Global State:**
- `gMainHandler` - Global UI handler
- `UI_Critical` - Thread safety critical section
- `gImageMgr`, `gSoundMgr` - Resource managers

**Important Configuration:**
- Window layouts defined in .scf scripts
- Control IDs in userids.h (e.g., `EXIT_CTRL=80000`)
- Hotspot detection parameters

---

### 5. VU2 Entity System (`src/vu2/`)

**Purpose:** Distributed entity database, networking, message passing

**Key Entry Points:**
| Function/Class | Location | Purpose |
|----------------|----------|---------|
| `VuEntity` (base class) | vuentity.cpp | Entity lifecycle |
| `VuDatabase` | vu_database.cpp | Entity storage/lookup |
| `VuMainThread::Update()` | vu_thread.cpp | Network dispatch |
| `VuMessageQueue::DispatchMessages()` | vu_mq.cpp | Message processing |
| `VuReferenceEntity()` / `VuDeReferenceEntity()` | | Ref counting |

**External Interfaces:**
- **Input:** Network packets (UDP/reliable), local entity creation
- **Output:** Replicated entity state, position updates
- **Protocols:** Custom binary protocol over UDP + reliable channel

**Global State:**
- `vuLocalSessionEntity` - Current player session
- `vuGlobalGroup` - Broadcast target group
- `vuGameList`, `vuTargetList` - Filtered collections

**Important Configuration:**
- Entity type registry (100+ types)
- Collision radii per type
- Update priority by distance

---

## Risk & Technical Debt

### Critical Vulnerabilities

| Issue | Severity | Location | Description |
|-------|----------|----------|-------------|
| `sprintf()` overflow | CRITICAL | 50+ files | No bounds checking on format strings |
| `strcpy()` overflow | CRITICAL | 30+ files | Unbounded buffer copies |
| `fgets()` misuse | CRITICAL | realweather.cpp:1389 | Wrong buffer parameter |
| Raw pointer deref | HIGH | Throughout | No null checks before `->` |
| Global NULL ptrs | HIGH | entity.cpp:35-70 | 20+ globals used without validation |
| `new` without `delete` | HIGH | 40+ locations | Memory leaks |
| Thread safety | HIGH | 24+ files in vu2/ | Unclear synchronization |

### Memory Management Risks

**Patterns Found:**
- Manual `new`/`delete` without RAII wrappers
- Conditional `delete[]` with mismatched conditions
- No exception safety around allocations
- SmartHeap pools (optional) partially integrated

**Hotspots:**
- `src/sim/simlib/wpnstatn.cpp` - Weapon station drawables
- `src/sim/simlib/simmover.cpp` - Driver management
- `src/graphics/bsplib/` - Model loading

### Threading Risks

**Known Synchronization Points:**
- `campCritical` - Campaign thread lock
- `UI_Critical` - UI thread lock
- `SimObjectType::mutex` - Per-entity ref counting
- `OTWDriverClass::cs_update` - Graphics sync

**Potential Race Conditions:**
- Entity Wake/Sleep transitions
- Graphics object insertion during sim tick
- Network entity state replication
- Damage message application

### Legacy Code Issues

- C-style casts without type validation (25+ locations)
- `strtok()` usage modifying buffers (realweather.cpp)
- `memset()` on stack variables (ineffective clearing)
- Mixed `long`/`int32_t` causing 32/64-bit issues (partially fixed)

---

## Lint-like Checks to Perform

### Recommended Static Analysis Tools

**1. clang-tidy (Primary)**
```bash
# Suggested checks for this codebase:
clang-tidy -checks='
  bugprone-*,
  cert-*,
  clang-analyzer-*,
  cppcoreguidelines-no-malloc,
  cppcoreguidelines-owning-memory,
  cppcoreguidelines-pro-bounds-*,
  cppcoreguidelines-pro-type-cstyle-cast,
  modernize-use-nullptr,
  modernize-use-override,
  readability-implicit-bool-conversion,
  -bugprone-easily-swappable-parameters
' --header-filter='.*'
```

**Critical clang-tidy Checks:**
| Check | Purpose |
|-------|---------|
| `bugprone-unsafe-functions` | Detect sprintf, strcpy, gets |
| `bugprone-sizeof-expression` | Catch sizeof(ptr) mistakes |
| `bugprone-use-after-move` | Detect use-after-move |
| `cert-err34-c` | Check atoi/atof return values |
| `clang-analyzer-core.NullDereference` | Null pointer deref |
| `clang-analyzer-unix.Malloc` | Memory leak detection |

**2. cppcheck**
```bash
cppcheck --enable=all --std=c++17 \
  --suppress=missingIncludeSystem \
  --suppress=unusedFunction \
  -I src/falclib/include \
  -I src/graphics/include \
  -I src/sim/include \
  src/
```

**Critical cppcheck Checks:**
| Check | Purpose |
|-------|---------|
| `nullPointer` | Null pointer dereference |
| `bufferAccessOutOfBounds` | Array bounds |
| `memleak` | Memory leaks |
| `uninitvar` | Uninitialized variables |
| `resourceLeak` | File handle leaks |

**3. Custom Grep-Based Checks**
```bash
# Find sprintf without snprintf
grep -rn "sprintf\s*(" src/ --include="*.cpp" | grep -v snprintf

# Find strcpy without strncpy
grep -rn "strcpy\s*(" src/ --include="*.cpp" | grep -v strncpy

# Find raw new without smart pointer
grep -rn "=\s*new\s" src/ --include="*.cpp" | grep -v unique_ptr | grep -v shared_ptr

# Find potential null deref (-> without if)
grep -rn "\->" src/ --include="*.cpp" | head -100
```

### Compiler Warnings to Enable

```cmake
# Add to CMakeLists.txt
add_compile_options(
  -Wall -Wextra -Wpedantic
  -Wformat=2 -Wformat-security
  -Wnull-dereference
  -Wstack-protector
  -Wstrict-aliasing=2
  -Wcast-qual
  -Wconversion
  -Wshadow
  -Wdouble-promotion
  -Wundef
  -fstack-protector-strong
)
```

### Address/Memory Sanitizers

```bash
# Build with sanitizers for testing
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" ..

# Run with ASAN
ASAN_OPTIONS=detect_leaks=1:halt_on_error=0 ./FFViper
```

---

## Testing Strategy and Ideas

### Unit Testing (Per-Subsystem)

**1. Simulation Core**
| Test Area | Approach | Priority |
|-----------|----------|----------|
| `AirframeClass` physics | Golden-value tests against known aircraft data | HIGH |
| Weapon ballistics | Trajectory validation with fixed seeds | HIGH |
| Damage model | Input damage → expected state transitions | MEDIUM |
| Reference counting | Stress test `Reference()`/`Release()` | HIGH |

**2. Campaign System**
| Test Area | Approach | Priority |
|-----------|----------|----------|
| Save/Load round-trip | Serialize → deserialize → compare | HIGH |
| Unit movement | Pathfinding correctness on test maps | MEDIUM |
| AI task generation | Scenario inputs → expected missions | MEDIUM |
| Dirty flag propagation | Mark dirty → verify network message | HIGH |

**3. Graphics Engine**
| Test Area | Approach | Priority |
|-----------|----------|----------|
| Texture loading | Load all .TEX files, verify dimensions | HIGH |
| Model loading | Load all .LOD files, check for errors | HIGH |
| State stack | Push/pop correctness (20-deep) | MEDIUM |
| Ref counting | Texture/model ref count balance | HIGH |

**4. UI95**
| Test Area | Approach | Priority |
|-----------|----------|----------|
| Resource parsing | Load all .idx/.rsc pairs | HIGH |
| Script parsing | Parse all .scf files | HIGH |
| Hit detection | Coordinate → control mapping | MEDIUM |
| Event routing | Simulate click → verify callback | HIGH |

**5. VU2**
| Test Area | Approach | Priority |
|-----------|----------|----------|
| Entity lifecycle | Create → insert → remove → delete | HIGH |
| Message queue | Enqueue → dispatch → verify delivery | HIGH |
| Ref counting | Multi-threaded stress test | HIGH |
| Serialization | Entity save/load round-trip | HIGH |

### Integration Testing

**Scenario Tests:**
1. **Main Menu Flow:** Start → Load UI → Click buttons → Verify callbacks
2. **Campaign Start:** Load scenario → Verify units created → Run 1 campaign tick
3. **Flight Spawn:** Campaign → Deaggregate flight → Verify sim entities
4. **Network Sync:** Two instances → Create entity → Verify replication

**Harness Ideas:**
```cpp
// Headless test harness for campaign
class CampaignTestHarness {
    void LoadScenario(const char* file);
    void RunTicks(int n);
    void AssertUnitCount(int expected);
    void AssertObjectiveStatus(int id, int status);
};

// UI test harness with mock rendering
class UITestHarness {
    void LoadWindow(const char* scf);
    void SimulateClick(int x, int y);
    void AssertCallbackCalled(const char* name);
};
```

### Fuzz Testing

**High-Value Fuzz Targets:**
| Target | Input | Risk |
|--------|-------|------|
| `C_Parser::ParseScript()` | Malformed .scf files | Buffer overflow |
| `C_Resmgr::LoadIndex()` | Corrupted .idx files | Integer overflow |
| `LoadUnitData()` | Invalid .UCD files | Type confusion |
| `RealWeatherClass::LoadMETAR()` | Malformed METAR strings | sprintf overflow |
| VU message parsing | Random network packets | Deserialization bugs |

**Fuzzing Setup (libFuzzer):**
```cpp
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Example: fuzz .scf parsing
    FILE* tmp = tmpfile();
    fwrite(data, 1, size, tmp);
    rewind(tmp);
    C_Parser parser;
    parser.ParseScript(tmp);  // Should not crash
    fclose(tmp);
    return 0;
}
```

### Regression Testing

**Capture Known-Good States:**
- Screenshot hashes for UI screens
- Entity position snapshots after N ticks
- Campaign state checksums after scenario load

**Continuous Integration:**
```yaml
# .github/workflows/test.yml
jobs:
  build-and-test:
    steps:
      - name: Build with sanitizers
        run: cmake -DSANITIZERS=ON .. && make
      - name: Run unit tests
        run: ctest --output-on-failure
      - name: Run cppcheck
        run: cppcheck --error-exitcode=1 src/
      - name: Run clang-tidy
        run: run-clang-tidy -p build/
```

### Prioritized Test Implementation Order

1. **Phase 1 (Critical):**
   - Reference counting tests (VU2, graphics)
   - Save/load round-trip tests (campaign)
   - Resource loading tests (UI95)

2. **Phase 2 (High):**
   - Fuzz testing for parsers
   - Memory leak detection with ASAN
   - Thread safety stress tests

3. **Phase 3 (Medium):**
   - Physics golden-value tests
   - UI integration tests
   - Network sync tests

4. **Phase 4 (Low):**
   - Screenshot regression tests
   - Full campaign scenario tests
   - Performance benchmarks

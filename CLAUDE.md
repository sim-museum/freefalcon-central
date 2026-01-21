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

## Next Steps

1. Investigate and fix the segfault after initial rendering
2. Get main menu functionality working (Exit, Setup, Campaign, Dogfight buttons)
3. Remove or conditionalize debug output
4. Continue with other UI screens

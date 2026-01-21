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

## Known Issues

### Segfault After Initial Rendering
The application crashes with a segmentation fault shortly after the landing page renders. This needs investigation.

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

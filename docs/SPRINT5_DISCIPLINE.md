# Sprint 5 — Cross-platform discipline audit (Windows-build-green)

Read-only audit (no edits). Question: would the original MSVC/Windows build still
compile, given all the Linux porting changes? Discipline rule: Linux-specific code
inside `#ifdef FF_LINUX` (with `#else` for the Windows path); *portable* correctness
fixes (fixed-width ints, `delete[]`, bounds checks, `intptr_t`) may be unconditional.

## Result: GREEN — risk LOW. No HIGH/MEDIUM-risk findings.

- `FF_LINUX` is defined only under `if(UNIX AND NOT APPLE)` in the top `CMakeLists.txt`
  → strictly Linux-only; never set for MSVC.
- No unguarded Linux-only constructs in the **shared** codebase (excluding the
  intentionally Linux-only `src/compat/` and `src/ffviper/main_linux.cpp`):
  `<unistd.h>`, `<pthread.h>`, `<SDL2/...>`, `<GL/glew.h>`, `pthread_*`, `usleep`,
  `dlopen`, `__linux__` — all absent or already `#ifdef FF_LINUX`-guarded
  (e.g. `sim/simloop/simloop.cpp:11` guards its `<pthread.h>`).
- `#include <windows.h>` in shared files (183) resolves to the compat `windows.h`
  on Linux (compat dir is first on the include path) and to the real SDK on Windows.
- CMake gates Linux-only pieces: `tools/`, `movie/` under `if(WIN32)`; crash handler
  and `main_linux.cpp` vs Windows stub split by platform. No Linux-only `.cpp` is
  added to a shared target unguarded.
- Spot-checks of recently-changed shared files (objectiv.cpp, texbank.cpp,
  simloop.cpp, gmcomposit.cpp, shells.cpp) confirm: Sprint 1/2 fixes are portable
  (RAND_MAX==32767 on MSVC; `intptr_t` available since VS2005; bounds checks
  unconditional and Windows-neutral), and platform code is `#else`-paired.

## Choice-point recommendations (plan §4.3)

1. **Renderer path** — stay fixed-function for now. It works at 60 FPS; a VBO+shader
   rewrite is large and only justified if higher resolutions/refresh become a goal
   (Sprint 7 profiling should decide). Recommend: defer, revisit after profiling.
2. **`_exit(0)` at shutdown** — keep it. The legacy static-destructor order is a
   known minefield (documented exit crashes); `_exit` after orderly cleanup is a
   pragmatic, stable choice. Recommend: accept permanently; document it (done).
3. **Data compatibility layer** — keep in-place 32-bit parsing (drop-in use of
   existing installs is a core preservation value). A one-time converter would fork
   the data and break that. Recommend: do NOT convert; keep the loaders.

## Sprint 5 status: analysis DONE. No code changes required to keep Windows green.
The only standing recommendation is to wire a Windows CI job (out of scope here —
no Windows toolchain on this box) to *prove* green continuously rather than by audit.
</content>

# FreeFalcon 6 — Linux Port: packaging & preservation

The engine is open source; the **game data is not redistributable**. So packaging
ships the engine and *ingests* a user-supplied FreeFalcon/Falcon data install.

## Quick install (recommended)

```bash
# 1. Build the engine (see ../docs/PORT_STATUS.md §5 for one-time setup)
cd .. && cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && ninja -C build

# 2. Install against your data dir
packaging/install.sh --data /path/to/FreeFalcon6 [--prefix ~/.local]
```

`install.sh` verifies the build and data dir, creates the `sim -> Zips/sim`
symlink the Linux resource path needs, checks runtime libraries, and installs a
`freefalcon` launcher + a `.desktop` entry. Run `freefalcon` (or
`freefalcon -test-ia`).

## Runtime dependencies

Linked at runtime (Debian/Ubuntu package names):
`libsdl2-2.0-0`, `libglew2.2`, `libopenal1`, `libgl1`, plus a working
OpenGL driver. GLEW is also bundled under `extern/` (the binary's `RUNPATH`
points there), so a missing system GLEW is non-fatal.

## Re-creating the build-time `extern/` headers/libs (gitignored)

```bash
cd extern
apt-get download libsdl2-dev libglew-dev libglew2.2 libopenal-dev
for d in *.deb; do dpkg -x "$d" . ; done
```

## Reproducible build

- Toolchain: GCC ≥ 13, CMake ≥ 3.22, Ninja, 64-bit Linux.
- `cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && ninja -C build`
- ASAN variant: configure a second tree with
  `-DCMAKE_CXX_FLAGS="-fsanitize=address -fsanitize-recover=address -fno-omit-frame-pointer"`
  into `build-asan/` (already present on the dev box).
- The window title shows the git hash CMake was *configured* at; after a
  `ninja`-only rebuild re-run `cmake .` to refresh it.

## Full relocatable AppImage (future / optional)

The dev binary's `RUNPATH` is an absolute path (`extern/usr/...`), so it is **not**
relocatable as-is. To produce a portable AppImage:

1. Re-link with an `$ORIGIN`-relative rpath, or `patchelf --set-rpath '$ORIGIN/lib'
   FFViper` after copying the needed `.so` files next to it.
2. Stage an AppDir: `AppRun` → launcher, `FFViper` + bundled
   `libSDL2`, `libGLEW`, `libopenal` (NOT `libGL*` — use the host driver),
   `freefalcon.desktop`, an icon.
3. `appimagetool AppDir FreeFalcon6-x86_64.AppImage` (download `appimagetool`
   if not installed).

The data dir is still supplied at runtime via the launcher's `-d` flag, so even an
AppImage does not embed game data. This step is documented but not yet automated —
the `install.sh` path covers the common "I built it locally" case.

## Upstreaming

The port keeps the Windows build green (see `docs/SPRINT5_DISCIPLINE.md`) via
`#ifdef FF_LINUX` discipline, so it is structured to be merge-able back to the
parent FreeFalcon project rather than living as a permanent fork.
</content>

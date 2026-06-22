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

## Relocatable AppDir / AppImage

`packaging/build-appdir.sh` assembles a **relocatable AppDir** with no `patchelf`
needed — its `AppRun` sets `LD_LIBRARY_PATH` to the bundled libs. It bundles the
app's multimedia stack (SDL2, GLEW, OpenAL + private codec deps) and leaves the host
to provide the GL driver, X/Wayland and glibc (the standard AppImage host/bundle
split). The script verifies the bundle resolves before finishing.

```bash
packaging/build-appdir.sh                 # -> ./FreeFalcon6.AppDir (verified)
FF_DATA_DIR=/path/to/FreeFalcon6 ./FreeFalcon6.AppDir/AppRun   # run it
appimagetool ./FreeFalcon6.AppDir         # -> single-file .AppImage (needs appimagetool)
```

Only the final single-file compression needs `appimagetool` (a self-contained
download); everything up to it is automated and locally verified. Game data is still
supplied at runtime via `FF_DATA_DIR`/`-d` — even the AppImage does not embed it.
On the dev box `appimagetool`/`patchelf` are not installed and there is no clean test
VM, so cross-machine relocation is structurally correct + locally `ldd`-verified but
not yet run on a second machine.

## Upstreaming

The port keeps the Windows build green (see `docs/SPRINT5_DISCIPLINE.md`) via
`#ifdef FF_LINUX` discipline, so it is structured to be merge-able back to the
parent FreeFalcon project rather than living as a permanent fork.
</content>

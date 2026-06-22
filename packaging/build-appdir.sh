#!/bin/bash
# Sprint 6 — assemble a relocatable AppDir (no patchelf/appimagetool needed).
# Bundles the app's multimedia libraries (SDL2, GLEW, OpenAL + private codec deps)
# and uses an AppRun that sets LD_LIBRARY_PATH, while leaving the host to provide the
# GL driver, X/Wayland, and glibc (the standard AppImage host-vs-bundle split).
# A single-file .AppImage is then just:  appimagetool FreeFalcon6.AppDir
#
# Usage: packaging/build-appdir.sh [outdir]   (default: ./FreeFalcon6.AppDir)
set -euo pipefail
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$REPO/build/src/ffviper/FFViper"
APPDIR="${1:-$REPO/FreeFalcon6.AppDir}"
[ -x "$BIN" ] || { echo "build the engine first: cd $REPO/build && ninja" >&2; exit 1; }

# Host-provided libraries — MUST NOT be bundled (driver/display/base-system ABI).
DENY='^(libGL\.|libGLX|libGLdispatch|libEGL|libOpenGL|libdrm|libgbm|libc\.|libm\.|libdl|libpthread|librt|ld-linux|libmvec|libstdc\+\+|libgcc_s|libwayland|libX|libxcb|libXau|libXdmcp|libxkb|libdbus|libsystemd|libapparmor|libudev|libasound|libpulse)'

rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/lib"
cp "$BIN" "$APPDIR/usr/bin/FFViper"

echo ">> bundling libraries (excluding host-provided)..."
n=0
while read -r name _arrow path _addr; do
  [ -f "${path:-}" ] || continue
  if echo "$name" | grep -qE "$DENY"; then continue; fi
  cp -L "$path" "$APPDIR/usr/lib/" && n=$((n+1))
done < <(ldd "$BIN" | sed 's/^[[:space:]]*//')
echo ">> bundled $n libraries"

# AppRun: make the bundle relocatable via LD_LIBRARY_PATH; pass through the data dir.
cat > "$APPDIR/AppRun" <<'EOF'
#!/bin/bash
HERE="$(dirname "$(readlink -f "$0")")"
export LD_LIBRARY_PATH="$HERE/usr/lib:${LD_LIBRARY_PATH:-}"
# Game data is not bundled (not redistributable). Provide it via $FF_DATA_DIR or -d.
DATA="${FF_DATA_DIR:-}"
if [ -n "$DATA" ]; then set -- -d "$DATA" -w "$@"; fi
exec "$HERE/usr/bin/FFViper" "$@"
EOF
chmod +x "$APPDIR/AppRun"

cat > "$APPDIR/freefalcon.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=FreeFalcon 6 (Linux Port)
Exec=AppRun
Icon=freefalcon
Categories=Game;Simulation;
EOF
# Minimal placeholder icon (appimagetool requires one); replace with real art if available.
: > "$APPDIR/freefalcon.png"

echo ">> AppDir ready: $APPDIR"
echo ">> verifying the bundle resolves against itself (no leakage to system bundle set)..."
LD_LIBRARY_PATH="$APPDIR/usr/lib" ldd "$APPDIR/usr/bin/FFViper" | grep -E "not found" && {
  echo "!! unresolved libraries above — add them to the bundle"; exit 1; } || echo ">> all libraries resolve."
echo
echo "Run locally:  FF_DATA_DIR=/path/to/FreeFalcon6 $APPDIR/AppRun"
echo "Package:      appimagetool $APPDIR   # (download appimagetool if not installed)"

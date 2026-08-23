#!/usr/bin/env bash
# Build the dependency record-display.sh needs: gpu-screen-recorder.
#
# It is deliberately NOT vendored in this repository -- it is a separate
# GPL-3.0 project by dec05eba, and this repo is BSD-2-Clause. So it is
# downloaded on demand instead.
#
#   scripts/build-record-display.sh            # clone + build into ./extern
#   scripts/build-record-display.sh --install  # ...and install system-wide (sudo)
#
# Without --install the binary is left at
#   extern/gpu-screen-recorder/build/gpu-screen-recorder
# and record-display.sh will find it if that directory is on PATH.
set -euo pipefail

UPSTREAM="https://repo.dec05eba.com/gpu-screen-recorder"
# Pinned to the commit this was verified against. Set GSR_REF=master to track
# upstream instead -- unpinned, so it may not match what the videos were made with.
GSR_REF="${GSR_REF:-9c2c0e1}"

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC_DIR="$REPO_ROOT/extern/gpu-screen-recorder"
DO_INSTALL=0

for arg in "$@"; do
    case "$arg" in
        --install) DO_INSTALL=1 ;;
        -h|--help) sed -n '2,14p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "Unknown argument: $arg" >&2; exit 1 ;;
    esac
done

for tool in git meson ninja; do
    command -v "$tool" >/dev/null || {
        echo "Error: '$tool' is required but not installed." >&2
        echo "  sudo apt install git meson ninja-build" >&2
        exit 1
    }
done

if [ -d "$SRC_DIR/.git" ]; then
    echo "Updating existing checkout in $SRC_DIR"
    git -C "$SRC_DIR" fetch --quiet origin
else
    echo "Cloning $UPSTREAM -> $SRC_DIR"
    mkdir -p "$(dirname "$SRC_DIR")"
    git clone --quiet "$UPSTREAM" "$SRC_DIR"
fi

git -C "$SRC_DIR" checkout --quiet "$GSR_REF"
echo "At $(git -C "$SRC_DIR" rev-parse --short HEAD)"

meson setup --reconfigure "$SRC_DIR/build" "$SRC_DIR" >/dev/null
ninja -C "$SRC_DIR/build"

BIN="$SRC_DIR/build/gpu-screen-recorder"
[ -x "$BIN" ] || { echo "Error: build finished but $BIN is missing." >&2; exit 1; }
echo "Built: $BIN"

if [ "$DO_INSTALL" -eq 1 ]; then
    echo "Installing system-wide (needs root)..."
    sudo meson configure --prefix=/usr --buildtype=release "$SRC_DIR/build"
    sudo ninja -C "$SRC_DIR/build" install
    echo "Installed. record-display.sh can now be run directly."
else
    echo
    echo "Not installed. Either add it to PATH for this shell:"
    echo "    export PATH=\"$SRC_DIR/build:\$PATH\""
    echo "or re-run with --install to install system-wide."
fi

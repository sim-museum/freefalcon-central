#!/bin/bash
# FreeFalcon Linux launcher
# Usage: ./run-freefalcon.sh            (normal windowed)
#        ./run-freefalcon.sh -test-ia   (auto-launch Instant Action)
# Output (including crash backtraces) is captured to /tmp/freefalcon-session.log
# Paths are derived from this script's location so the repo works from any
# checkout dir; override either with FF_GAMEDATA / FF_BIN.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GAMEDATA="${FF_GAMEDATA:-$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6}"
BIN="${FF_BIN:-$SCRIPT_DIR/build/src/ffviper/FFViper}"
cd "$GAMEDATA" || { echo "Game data not found: $GAMEDATA"; exit 1; }
[ -x "$BIN" ] || { echo "FFViper binary not found: $BIN (build it with: cd $SCRIPT_DIR/build && ninja)"; exit 1; }
# Keep the previous session log for post-mortem
[ -f /tmp/freefalcon-session.log ] && mv -f /tmp/freefalcon-session.log /tmp/freefalcon-session.prev.log
exec "$BIN" -d "$GAMEDATA" -w "$@" > /tmp/freefalcon-session.log 2>&1

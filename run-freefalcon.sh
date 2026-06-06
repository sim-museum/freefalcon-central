#!/bin/bash
# FreeFalcon Linux launcher
# Usage: ./run-freefalcon.sh            (normal windowed)
#        ./run-freefalcon.sh -test-ia   (auto-launch Instant Action)
# Output (including crash backtraces) is captured to /tmp/freefalcon-session.log
GAMEDATA="/home/g/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
BIN="/home/g/ff/build/src/ffviper/FFViper"
cd "$GAMEDATA" || { echo "Game data not found: $GAMEDATA"; exit 1; }
# Keep the previous session log for post-mortem
[ -f /tmp/freefalcon-session.log ] && mv -f /tmp/freefalcon-session.log /tmp/freefalcon-session.prev.log
exec "$BIN" -d "$GAMEDATA" -w "$@" > /tmp/freefalcon-session.log 2>&1

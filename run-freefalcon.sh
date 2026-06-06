#!/bin/bash
# FreeFalcon Linux launcher
# Usage: ./run-freefalcon.sh            (normal windowed)
#        ./run-freefalcon.sh -test-ia   (auto-launch Instant Action)
GAMEDATA="/home/g/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
BIN="/home/g/ff/build/src/ffviper/FFViper"
cd "$GAMEDATA" || { echo "Game data not found: $GAMEDATA"; exit 1; }
exec "$BIN" -d "$GAMEDATA" -w "$@"

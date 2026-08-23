#!/bin/bash
# Drive the Wine (Windows) FreeFalcon build and capture frames, so the gold
# standard can be produced by the agent instead of only by the PO recording
# video. The prefix is 32-bit and system wine defaults to wow64, so `wine32`
# is required -- plain `wine` fails with "cannot support 64-bit applications".
set -u
export DISPLAY=:0
export WINEPREFIX="$HOME/sgl/SAT/freeFalcon/WP"
unset WINEARCH
GAMEDATA="$WINEPREFIX/drive_c/FreeFalcon6"
OUT="${1:-/tmp/wine-shot}"
WAIT="${2:-40}"
cd "$GAMEDATA" || exit 1
timeout 120 wine32 FFViper.exe > /tmp/wine-run.log 2>&1 &
WPID=$!
for i in $(seq 1 "$WAIT"); do
    W=$(xdotool search --name "Falcon 4 - FreeFalcon" 2>/dev/null | head -1)
    [ -n "$W" ] && break
    sleep 1
done
[ -z "${W:-}" ] && { echo "no game window appeared"; kill $WPID 2>/dev/null; exit 1; }
echo "window $W  geometry: $(xdotool getwindowgeometry --shell $W | tr '\n' ' ')"
sleep 12
import -window "$W" "$OUT.png" 2>/dev/null && echo "captured $OUT.png $(identify -format '%wx%h' $OUT.png)"
sleep 3
pkill -f FFViper.exe 2>/dev/null
wait $WPID 2>/dev/null
echo "done"

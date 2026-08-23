#!/bin/bash
# Drive the Wine (Windows) build through the same UI path the Linux harness's
# FF_UI_CLICK follows, using xdotool. Coordinates are WINDOW-RELATIVE so the
# window's position on the root does not matter.
#   $1 = output prefix   $2 = click script "x,y@sec;x,y@sec;..."   $3 = total secs
set -u
export DISPLAY=:0
export WINEPREFIX="$HOME/sgl/SAT/freeFalcon/WP"
unset WINEARCH
GAMEDATA="$WINEPREFIX/drive_c/FreeFalcon6"
OUT="${1:-/tmp/wine-drive}"
CLICKS="${2:-}"
TOTAL="${3:-90}"
cd "$GAMEDATA" || exit 1

timeout $((TOTAL + 30)) wine32 FFViper.exe > /tmp/wine-drive.log 2>&1 &
for i in $(seq 1 45); do
    W=$(xdotool search --name "^Falcon 4 - FreeFalcon$" 2>/dev/null | head -1)
    [ -n "$W" ] && break
    sleep 1
done
[ -z "${W:-}" ] && { echo "no game window"; pkill -f FFViper.exe; exit 1; }
eval "$(xdotool getwindowgeometry --shell "$W")"
echo "window=$W pos=$X,$Y size=${WIDTH}x${HEIGHT}"
START=$SECONDS
xdotool windowactivate "$W" 2>/dev/null; sleep 1

IFS=';' read -ra STEPS <<< "$CLICKS"
for step in "${STEPS[@]}"; do
    [ -z "$step" ] && continue
    coord="${step%@*}"; at="${step#*@}"
    while [ $((SECONDS - START)) -lt "$at" ]; do sleep 1; done
    cx="${coord%,*}"; cy="${coord#*,}"
    xdotool mousemove --window "$W" "$cx" "$cy" click 1
    echo "  clicked $cx,$cy at $((SECONDS - START))s"
    sleep 1
    import -window "$W" "${OUT}-after-${cx}x${cy}.png" 2>/dev/null
done

while [ $((SECONDS - START)) -lt "$TOTAL" ]; do sleep 2; done
import -window "$W" "${OUT}-final.png" 2>/dev/null && echo "final: ${OUT}-final.png"
pkill -f FFViper.exe 2>/dev/null
sleep 2
echo "done"

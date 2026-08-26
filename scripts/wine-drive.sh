#!/bin/bash
# Launch the Wine (Windows) FreeFalcon build and drive it through a UI timeline,
# so the gold standard can be produced by script rather than only by hand.
# See wine-capture.sh for the three Wayland/Wine pitfalls this works around.
#
# Usage: wine-drive.sh "<timeline>"
#   timeline entries, semicolon separated, seconds measured from window-ready:
#     c:X,Y@S   click at window-relative X,Y     (XSendEvent -- XTEST is dropped)
#     k:KEY@S   send KEY                          (same reason)
#     s:NAME@S  capture NAME.png
#
# Known window-relative coordinates (1024x768):
#   TACTICAL ENGAGEMENT 674,750 | THEATER 573,750 | CAMPAIGN 973,750
#   TE mission row N     140,(94 + N*17)
#   COMMIT 822,748 | TAKEOFF/FLY 984,748
# Keys: N = NVG (DIK_N 0x31), G = gear (DIK_G 0x22), 0 = orbit camera, 1 = cockpit.
set -u
export DISPLAY=:0
RUNNER_DIR="$HOME/.local/share/lutris/runners/wine/lutris-GE-Proton8-26-x86_64"
[ -x "$RUNNER_DIR/bin/wine" ] || { echo "pinned runner missing: $RUNNER_DIR"; exit 1; }
export PATH="$RUNNER_DIR/bin:$PATH"
export WINE="$RUNNER_DIR/bin/wine" WINELOADER="$RUNNER_DIR/bin/wine" WINESERVER="$RUNNER_DIR/bin/wineserver"
export LD_LIBRARY_PATH="$RUNNER_DIR/lib64:$RUNNER_DIR/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export WINEDLLPATH="$RUNNER_DIR/lib64/wine/x86_64-unix:$RUNNER_DIR/lib/wine/i386-unix"
export WINEPREFIX="$HOME/sgl/SAT/freeFalcon/WP" WINEARCH=win32
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TIMELINE="${1:-}"

setsid nohup "$RUNNER_DIR/bin/wine" explorer /desktop=FreeFalcon,1024x768 \
    'C:\FreeFalcon6\FFViper.exe' > /tmp/wine-drive.log 2>&1 < /dev/null &
disown

# The game window spawns OUTSIDE the virtual desktop (a fixed +3815,-62 offset) and
# must be moved back or it renders invisibly while still playing audio. It also
# passes through a 640x480 splash and is RECREATED on entering 3D, so this has to be
# re-run after the sim loads, not just at startup.
place() {
    for _ in $(seq 1 60); do
        W=$(xdotool search --name "^Falcon 4 - FreeFalcon$" 2>/dev/null | tail -1)
        if [ -n "$W" ] && eval "$(xdotool getwindowgeometry --shell "$W" 2>/dev/null)"; then
            if [ "${WIDTH:-0}" -eq 1024 ] && [ "${HEIGHT:-0}" -eq 768 ]; then
                { [ "$X" -gt 3840 ] || [ "$X" -lt 0 ]; } && { xdotool windowmove "$W" 0 0; sleep 2; }
                eval "$(xdotool getwindowgeometry --shell "$W" 2>/dev/null)"
                [ "${X:-99999}" -le 3840 ] && { echo "window $W at $X,$Y"; return 0; }
            fi
        fi
        sleep 2
    done
    echo "window never became ready"; return 1
}
place || exit 1
START=$SECONDS
IFS=';' read -ra STEPS <<< "$TIMELINE"
for step in "${STEPS[@]}"; do
    [ -z "$step" ] && continue
    kind="${step%%:*}"; rest="${step#*:}"; arg="${rest%@*}"; at="${rest#*@}"
    while [ $((SECONDS - START)) -lt "$at" ]; do sleep 1; done
    W=$(xdotool search --name "^Falcon 4 - FreeFalcon$" 2>/dev/null | tail -1)
    OUTER=$(xdotool search --name "FreeFalcon - Wine desktop" 2>/dev/null | head -1)
    [ -n "$OUTER" ] && { xdotool windowactivate "$OUTER" 2>/dev/null; sleep 1; }
    case "$kind" in
      c) xdotool mousemove --window "$W" "${arg%,*}" "${arg#*,}"; sleep 0.4
         xdotool click --window "$W" 1; echo "  [$((SECONDS-START))s] click $arg" ;;
      k) xdotool key --window "$W" "$arg"; echo "  [$((SECONDS-START))s] key $arg" ;;
      s) "$HERE/wine-capture.sh" "$arg" >/dev/null && echo "  [$((SECONDS-START))s] shot $arg.png" ;;
      p) place ;;
    esac
done
echo "done (build left running; pkill -f '[F]FViper\.exe' to stop)"

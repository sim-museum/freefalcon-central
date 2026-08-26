#!/bin/bash
# Capture one frame from the Wine (Windows) FreeFalcon build -- the gold standard
# used to check the Linux port's rendering against.
#
# THREE THINGS FAIL SILENTLY ON THIS MACHINE, all discovered the hard way:
#
#  1. LAUNCH: system `wine`/`wine32` cannot boot this 32-bit prefix at all. The
#     pinned Lutris runner below is required (see freeFalcon.sh for the history).
#  2. INPUT: the session is GNOME *Wayland*, which silently discards XTEST fake
#     input -- `xdotool click` moves the pointer and nothing happens, even with the
#     correct coordinates and the window on top. Use XSendEvent
#     (`xdotool click --window <id>`), which Wine does accept.
#  3. CAPTURE: X11 grabs (`import`, `ffmpeg -f x11grab`) return an ALL-BLACK image
#     under Wayland regardless of what is on screen -- do not read that as a
#     sleeping monitor. gpu-screen-recorder works; its *window* capture does not
#     (unsupported under XWayland), so record the monitor and crop.
#
# Usage: wine-capture.sh <out-name> [seconds]
set -u
export DISPLAY=:0
OUT="${1:-wine-shot}"; DUR="${2:-4}"
W=$(xdotool search --name "^Falcon 4 - FreeFalcon$" 2>/dev/null | tail -1)
[ -z "$W" ] && { echo "no game window (is the build running?)"; exit 1; }
eval "$(xdotool getwindowgeometry --shell "$W")"
if [ "$X" -ge 1920 ]; then MON=HDMI-A-1; MX=1920; else MON=DP-1; MX=0; fi
# raise the Wine DESKTOP window: raising the inner game window is not enough,
# an XWayland window can sit under native ones and the crop then captures those.
OUTER=$(xdotool search --name "FreeFalcon - Wine desktop" 2>/dev/null | head -1)
[ -n "$OUTER" ] && { xdotool windowactivate "$OUTER" 2>/dev/null; sleep 1; }
rm -f /tmp/_wine_shot.mp4
# -q ultra / -tune quality / -cr full: default H.264 blurs HUD digits past legibility.
gpu-screen-recorder -w "$MON" -f 30 -q ultra -tune quality -cr full -bm qp \
    -o /tmp/_wine_shot.mp4 >/tmp/gsr.log 2>&1 &
G=$!
sleep "$DUR"
kill -INT $G 2>/dev/null; sleep 2; wait $G 2>/dev/null
[ -s /tmp/_wine_shot.mp4 ] || { echo "capture failed"; tail -3 /tmp/gsr.log; exit 1; }
ffmpeg -loglevel error -sseof -1.5 -i /tmp/_wine_shot.mp4 -frames:v 1 \
       -vf "crop=${WIDTH}:${HEIGHT}:$((X - MX)):${Y}" -y "$OUT.png" 2>/dev/null
rm -f /tmp/_wine_shot.mp4
identify -format "$OUT.png %wx%h mean=%[fx:mean]\n" "$OUT.png"

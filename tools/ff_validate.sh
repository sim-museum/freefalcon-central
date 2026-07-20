#!/usr/bin/env bash
# Objective frame-validation harness for the FreeFalcon 6 Linux port.
#
# Drives the game to a fixed state (Instant Action via -test-ia, or a scripted UI
# click sequence), captures ONE frame from inside the game, and prints objective
# statistics so "did the 3D world render?" is a number instead of an impression.
#
# Frame capture uses the in-game hooks (no external window grab -- window grabs
# come back black under Wayland/XWayland while the sim thread is presenting):
#   sim mode : FF_SIM_SCREENSHOT / FF_VIEW_SCRIPT 's'  -> captured by the SIM
#              thread inside its own swap path, on the GL-context-owning thread,
#              immediately before SDL_GL_SwapWindow.
#   UI mode  : FF_UI_SCREENSHOT                        -> captured by the main thread.
#
# Usage:
#   tools/ff_validate.sh <tag> [options]
#     -m sim|ui       mode (default sim)
#     -t <sec>        capture time, seconds after entering the mode (default 30)
#     -r <sec>        total run time before the game is killed (default: capture+20)
#     -v <mode>       sim view mode to select before capturing
#                     (0=HUD 1=2D pit 2=chase 3=orbit 4=3D virtual pit)
#     -c <clicks>     FF_UI_CLICK script, e.g. "870,745@12;900,750@20"
#     -e K=V          extra env var (repeatable), e.g. -e FF_DEBUG_RUNWAY=1
#
# Output:
#   /tmp/ffval/<tag>.png   the captured frame
#   /tmp/ffval/<tag>.log   the full game log
#   plus a printed report: size, distinct colours, non-black/non-white %,
#   and per-band (top/middle/bottom) average RGB + distinct colour counts.
#
# Exit status: 0 if a frame was captured AND it has real content
#              (>5% non-black, >5% non-white, >100 distinct colours), else 1.
set -u

FF_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="${FF_BIN:-$FF_ROOT/build/src/ffviper/FFViper}"
GAME_DIR="${FF_GAME_DIR:-$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6}"
OUT="${FF_VAL_OUT:-/tmp/ffval}"

TAG="${1:-shot}"; shift || true
MODE=sim
AT=30
RUN=
VIEW=
CLICKS=
EXTRA_ENV=()

while getopts "m:t:r:v:c:e:" opt; do
    case "$opt" in
        m) MODE="$OPTARG" ;;
        t) AT="$OPTARG" ;;
        r) RUN="$OPTARG" ;;
        v) VIEW="$OPTARG" ;;
        c) CLICKS="$OPTARG" ;;
        e) EXTRA_ENV+=("$OPTARG") ;;
        *) echo "see header for usage"; exit 2 ;;
    esac
done

RUN="${RUN:-$(( AT + 20 ))}"
mkdir -p "$OUT"
SHOT="$OUT/$TAG.bmp"
LOG="$OUT/$TAG.log"
rm -f "$SHOT" "$OUT/$TAG.png" "$LOG"

[ -x "$BIN" ] || { echo "no binary at $BIN (build first: cd $FF_ROOT/build && ninja)"; exit 2; }
[ -d "$GAME_DIR" ] || { echo "no game data dir at $GAME_DIR (set FF_GAME_DIR)"; exit 2; }

ENVV=(DISPLAY="${DISPLAY:-:0}")
ARGS=(-d "$GAME_DIR" -w)

if [ "$MODE" = sim ]; then
    ARGS+=(-test-ia)
    if [ -n "$VIEW" ]; then
        # Switch view a couple of seconds before the capture so the mode has settled.
        SW=$(python3 -c "print(max(0,$AT-3))")
        ENVV+=(FF_VIEW_SCRIPT="$VIEW@$SW")
    fi
    ENVV+=(FF_SIM_SCREENSHOT="$AT:$SHOT")
else
    # UI mode: FF_UI_SCREENSHOT is periodic (every N seconds) and always writes
    # /tmp/ff_ui.bmp; we let it tick and copy the last one out after the run.
    ENVV+=(FF_UI_SCREENSHOT="$AT")
    rm -f /tmp/ff_ui.bmp
fi
[ -n "$CLICKS" ] && ENVV+=(FF_UI_CLICK="$CLICKS")
for e in "${EXTRA_ENV[@]:-}"; do [ -n "$e" ] && ENVV+=("$e"); done

echo "=== $TAG: mode=$MODE capture@${AT}s run=${RUN}s ${VIEW:+view=$VIEW }${CLICKS:+clicks=\"$CLICKS\"}"
( cd "$GAME_DIR" && timeout -s INT "$RUN" env "${ENVV[@]}" "$BIN" "${ARGS[@]}" ) >"$LOG" 2>&1
echo "  game exited ($?; 124=killed by harness timeout, expected)"

if [ "$MODE" = ui ] && [ -s /tmp/ff_ui.bmp ]; then cp /tmp/ff_ui.bmp "$SHOT"; fi

grep -aE '^\[Screenshot\]|Segmentation|SIGSEGV|SIGABRT' "$LOG" | tail -5 | sed 's/^/  /'

if [ ! -s "$SHOT" ]; then
    echo "  NO FRAME captured -- see $LOG"
    exit 1
fi

python3 - "$SHOT" "$OUT/$TAG.png" <<'PY'
import sys
from PIL import Image
im = Image.open(sys.argv[1]).convert('RGB')
im.save(sys.argv[2])
w, h = im.size
px = list(im.tobytes())
px = [tuple(px[i:i+3]) for i in range(0, len(px), 3)]
n = len(px)
distinct = len(set(px))
nonblack = sum(1 for p in px if p != (0, 0, 0))
nonwhite = sum(1 for p in px if p != (255, 255, 255))
print(f"  size={w}x{h}  distinct_colours={distinct}")
print(f"  non-black={100.0*nonblack/n:.1f}%  non-white={100.0*nonwhite/n:.1f}%")
def band(y0, y1):
    s = [px[y*w+x] for y in range(y0, y1, 2) for x in range(0, w, 7)]
    avg = tuple(sum(c[i] for c in s)//len(s) for i in range(3))
    return avg, len(set(s))
for name, (y0, y1) in [("top   ", (0, h//3)), ("middle", (h//3, 2*h//3)), ("bottom", (2*h//3, h))]:
    avg, nc = band(y0, y1)
    print(f"  {name}: avg_rgb={avg} distinct={nc}")
print(f"  saved {sys.argv[2]}")
ok = (nonblack > 0.05*n) and (nonwhite > 0.05*n) and (distinct > 100)
print("  VERDICT: " + ("REAL CONTENT" if ok else "BLANK/DEGENERATE FRAME"))
sys.exit(0 if ok else 1)
PY

#!/bin/bash
# Generic ASAN soak driven by an FF_UI_CLICK script — exercise any UI-reachable mode
# (Tactical Engagement, Dogfight, Campaign...) under AddressSanitizer to surface
# mode-specific heap bugs / crashes. Crashes are backtraces (no visual capture needed).
#
# Usage: run-asan-ui-soak.sh <label> "<FF_UI_CLICK script>" [timeout_secs]
#   e.g. run-asan-ui-soak.sh dogfight "870,745@12;201,134@18;884,741@24;900,750@32" 200
set -u
[ $# -ge 2 ] || { echo "usage: $0 <label> \"<FF_UI_CLICK>\" [secs]"; exit 2; }
LABEL="$1"; CLICKS="$2"; T="${3:-200}"
export DISPLAY=:0
GAMEDATA="/home/g/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
BIN="/home/g/ff/build-asan/src/ffviper/FFViper"
LOG="/tmp/ff-asan-${LABEL}.log"
pgrep -f mutter-x11-frames >/dev/null || { setsid /usr/libexec/mutter-x11-frames >/dev/null 2>&1 & sleep 1; }
cd "$GAMEDATA" || { echo "no game data"; exit 1; }
export ASAN_OPTIONS="halt_on_error=0:detect_leaks=0:abort_on_error=0"
export FF_UI_CLICK="$CLICKS"
echo "=== ASAN ${LABEL} soak: timeout ${T}s, clicks=$CLICKS ===" > "$LOG"
timeout -s INT "$T" "$BIN" -d "$GAMEDATA" -w >> "$LOG" 2>&1
echo "=== exited rc=$? ===" >> "$LOG"
echo "--- ${LABEL}: ASAN summaries ---"
grep -E "SUMMARY: AddressSanitizer|ASSearch|ERROR: AddressSanitizer.*SEGV" "$LOG" | sort | uniq -c
echo "--- exit ---"; tail -1 "$LOG"

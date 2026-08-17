#!/bin/bash
# Campaign flight -> 3D under AddressSanitizer.
#
# WHY THIS EXISTS: the other soaks (run-asan-soak.sh = Instant Action,
# and the TE-02 repro) both reported clean for an entire session while four
# heap-use-after-frees and five alloc-dealloc-mismatches sat in the campaign
# path -- ATCBrain::ProcessQueue's runway queue and the loadout chain. Neither
# TE 2 nor Instant Action runs the campaign ATC queue or the AI weapons path,
# so neither could ever have found them. A sanitiser only finds what you run.
#
# Usage: ./run-asan-campaign-flight-soak.sh [seconds]   (default 420)
set -u
export DISPLAY=:0
GD="${FF_GAMEDATA:-$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6}"
BIN="${FF_BIN:-/home/g/ff/build-asan/src/ffviper/FFViper}"
LOG="${FF_LOG:-/tmp/ff-campfly-asan.log}"

pgrep -f mutter-x11-frames >/dev/null || { setsid /usr/libexec/mutter-x11-frames >/dev/null 2>&1 & sleep 1; }
cd "$GD" || { echo "no game data at $GD"; exit 1; }

export ASAN_OPTIONS="halt_on_error=0:detect_leaks=0:abort_on_error=0:print_stats=0"

# Campaign -> COMMIT -> mission priorities OK -> START CAMPAIGN -> pick the first
# ATO flight row -> take a pilot slot -> TAKEOFF -> runway start tile.
# Timings are deliberately generous: the ASAN build is slow enough that a schedule
# tuned on the release build drifts and the later clicks miss their targets.
export FF_UI_CLICK="924,745@14;905,758@26;563,751@34;495,390@42;110,135@56;95,340@66;976,750@78;200,595@92"

timeout -s INT "${1:-420}" "$BIN" -d "$GD" -w > "$LOG" 2>&1
echo "=== exited rc=$? ===" >> "$LOG"

echo "ASAN errors : $(grep -c 'ERROR: AddressSanitizer' "$LOG")"
grep -oE "ERROR: AddressSanitizer: [a-z-]+" "$LOG" | sort | uniq -c
echo "reached 3D  : $(grep -ci 'RunningGraphics\|\[GROUND\]' "$LOG")"
echo "crashes     : $(grep -ciE 'SIGSEGV|SIGABRT|core dumped' "$LOG")"
echo "(a run that never reached 3D also reports zero errors -- check the line above)"

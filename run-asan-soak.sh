#!/bin/bash
# Sprint 2 — ASAN soak of the Instant Action flow.
# Validates the correctness-sweep changes live and catches memory errors / the
# intermittent ASSearch SIGSEGV via backtrace (no visual capture needed).
# Usage: ./run-asan-soak.sh [timeout_secs]
set -u
export DISPLAY=:0
GAMEDATA="/home/g/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
BIN="/home/g/ff/build-asan/src/ffviper/FFViper"
LOG=/tmp/ff-asan-soak.log
T="${1:-200}"

# Heal the documented mutter-x11-frames gotcha (its absence hangs SDL_CreateWindow).
pgrep -f mutter-x11-frames >/dev/null || { setsid /usr/libexec/mutter-x11-frames >/dev/null 2>&1 & sleep 1; }

cd "$GAMEDATA" || { echo "no game data"; exit 1; }
# halt_on_error=0 -> log-and-continue (binary built with -fsanitize-recover);
# detect_leaks=0 -> suppress noisy exit-time leak dump, focus on real memory errors.
export ASAN_OPTIONS="halt_on_error=0:detect_leaks=0:abort_on_error=0:print_stats=0"
echo "=== ASAN soak: timeout ${T}s, -test-ia ===" > "$LOG"
timeout -s INT "$T" "$BIN" -d "$GAMEDATA" -w -test-ia >> "$LOG" 2>&1
echo "=== exited rc=$? ===" >> "$LOG"

echo "--- ASAN errors / crashes ---"
grep -nE "ERROR: AddressSanitizer|SUMMARY: AddressSanitizer|runtime error:|SIGSEGV|ASSearch|#0 0x" "$LOG" | head -40 || true
echo "--- tail ---"; tail -8 "$LOG"

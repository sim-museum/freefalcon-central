#!/bin/bash
# Reproduce TE "02 Takeoff" ground start and capture the spawn/ground elevation
# numbers. Click script is the TE-02 repro recorded in docs/STATUS.md:1050.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DISPLAY=:0
GAMEDATA="${FF_GAMEDATA:-$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6}"
# Release build by default: the FF_DEBUG_* traces are FF_LINUX-gated, not
# ASAN-gated, and the release tree rebuilds incrementally in ~1min vs ~7min for
# build-asan (every commit changes the git hash CMake stamps in, forcing a full
# ASAN regen). Set FF_BIN to build-asan when memory checking is the point.
BIN="${FF_BIN:-$REPO/build-relg/src/ffviper/FFViper}"
LOG=/tmp/ff-te02.log
T="${1:-150}"

pgrep -f mutter-x11-frames >/dev/null || { setsid /usr/libexec/mutter-x11-frames >/dev/null 2>&1 & sleep 1; }

cd "$GAMEDATA" || { echo "no game data"; exit 1; }

export ASAN_OPTIONS="halt_on_error=0:detect_leaks=0:abort_on_error=0:print_stats=0"
export FF_DEBUG_SPAWN=1
export FF_DEBUG_DEATH=1
export FF_DEBUG_GROUND=1
export FF_DEBUG_RUNWAY=1
export FF_DEBUG_DEAG=1
# Orbit camera (mode 3, same as the PO's gold shot) then capture frames.
export FF_VIEW_SCRIPT="${FF_VIEW_SCRIPT:-3@12;s@16;s@22}"
# TE-02 repro: Tactical(624,745) -> mission 02(140,128) -> COMMIT(825,750)
#              -> start tile(160,343) -> TAKEOFF(975,750) -> runway tile(200,595)
export FF_UI_CLICK="${FF_UI_CLICK:-624,745@6;140,128@10;825,750@13;160,343@17;975,750@20;200,595@24}"

echo "=== TE-02 repro: timeout ${T}s ===" > "$LOG"
timeout -s INT "$T" "$BIN" -d "$GAMEDATA" -w >> "$LOG" 2>&1
echo "=== exited rc=$? ===" >> "$LOG"

echo "--- SPAWN / DEATH ---"
grep -nE "\[SPAWN\]|\[DEATH\]|\[DEAGCHK\]" "$LOG" | head -40

#!/bin/bash
# CRASH-1 validation: drive a campaign flight into 3D under ASAN. The PO's crash
# was an AI bomb release using a dangling SimObjectType, so this needs the
# campaign AI running with real entity churn, not TE 2.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DISPLAY=:0
GD="$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
BIN=$REPO/build-asan/src/ffviper/FFViper
LOG=/tmp/ff-campfly-asan.log
pgrep -f mutter-x11-frames >/dev/null || { setsid /usr/libexec/mutter-x11-frames >/dev/null 2>&1 & sleep 1; }
cd "$GD" || exit 1
export ASAN_OPTIONS="halt_on_error=0:detect_leaks=0:abort_on_error=0:print_stats=0"
# Campaign -> COMMIT -> priorities OK -> START CAMPAIGN -> flight row -> pilot slot
# -> TAKEOFF -> runway start tile.   Generous spacing: the ASAN build is slow.
export FF_UI_CLICK="924,745@14;905,758@26;563,751@34;495,390@42;110,135@56;95,340@66;976,750@78;200,595@92"
timeout -s INT "${1:-420}" "$BIN" -d "$GD" -w > "$LOG" 2>&1
echo "=== rc=$? ===" >> "$LOG"

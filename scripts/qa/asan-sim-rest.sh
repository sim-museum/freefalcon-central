#!/bin/bash
# ASAN-7: the 26 TE missions not covered by asan-sim-pass.sh (which took rows
# 1,2,9,11,15,19,22,26 and found the HARMs heap-buffer-overflow -- 1 in 8).
# TESWEEP-4 ran all 34 in the RELEASE build: function, not memory safety.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DISPLAY=:0
GD="$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
BIN=$REPO/build-asan/src/ffviper/FFViper
pgrep -f mutter-x11-frames >/dev/null || { setsid /usr/libexec/mutter-x11-frames >/dev/null 2>&1 & sleep 1; }
cd "$GD" || exit 1

for row in 3 4 5 6 7 8 10 12 13 14 16 17 18 20 21 23 24 25 27 28 29 30 31 32 33 34; do
    y=$(( 94 + row * 17 ))
    log=/tmp/asan-rest-$row.log
    ( export ASAN_OPTIONS="halt_on_error=0:detect_leaks=0:abort_on_error=0:print_stats=0"
      export FF_UI_CLICK="574,750@12;225,171@18;896,743@24;677,748@36;140,${y}@42;824,750@48;973,750@54"
      timeout -s INT 190 "$BIN" -d "$GD" -w > "$log" 2>&1 )
    name=$(grep -a "StartReadCampFile: type" "$log" | head -1 | sed "s/.*filename='//;s/'.*//")
    n=$(grep -ac 'ERROR: AddressSanitizer' "$log")
    printf "row %2d sim=%s asan=%-3s %s\n" "$row" \
        "$(grep -ac RunningGraphics "$log")" "$n" "${name:-<none>}"
    # keep only logs that found something, to avoid filling /tmp
    [ "$n" = "0" ] && rm -f "$log"
done
echo "=== ASAN REST PASS COMPLETE ==="

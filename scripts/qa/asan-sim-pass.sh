#!/bin/bash
# ASAN-5: sim-mode ASAN pass over a representative spread of TE missions.
# TESWEEP-4 covered all 34 in the RELEASE build (functional only); this covers
# memory safety in flight code, which had only two incidental ASAN runs.
# Rows chosen to exercise distinct subsystems, not just to be many.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DISPLAY=:0
GD="$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
BIN=$REPO/build-asan/src/ffviper/FFViper
pgrep -f mutter-x11-frames >/dev/null || { setsid /usr/libexec/mutter-x11-frames >/dev/null 2>&1 & sleep 1; }
cd "$GD" || exit 1

# row : what it exercises
ROWS="1:basic-handling 2:takeoff 9:landing 11:flameout-landing 15:aim9 19:bombs-ccrp 22:guns-ag 26:harms"

for entry in $ROWS; do
    row="${entry%%:*}"; what="${entry#*:}"
    y=$(( 94 + row * 17 ))
    log=/tmp/asan-sim-$row.log
    ( export ASAN_OPTIONS="halt_on_error=0:detect_leaks=0:abort_on_error=0:print_stats=0"
      export FF_UI_CLICK="574,750@12;225,171@18;896,743@24;677,748@36;140,${y}@42;824,750@48;973,750@54"
      timeout -s INT 200 "$BIN" -d "$GD" -w > "$log" 2>&1 )
    name=$(grep -a "StartReadCampFile: type" "$log" | head -1 | sed "s/.*filename='//;s/'.*//")
    printf "row %2d %-18s sim=%s asan=%s  %s\n" "$row" "$what" \
        "$(grep -ac RunningGraphics "$log")" \
        "$(grep -ac 'ERROR: AddressSanitizer' "$log")" "${name:-<none>}"
done
echo "=== ASAN SIM PASS COMPLETE ==="

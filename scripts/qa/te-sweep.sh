#!/bin/bash
# Full TE regression sweep, rows 1..34, on the current build.
# Forces the KOREA theater first -- TE row coordinates index the CURRENT
# theater's mission list, and curTheater persists (see ff-verification-method).
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DISPLAY=:0
GAMEDATA="$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
BIN=$REPO/build-relg/src/ffviper/FFViper
pgrep -f mutter-x11-frames >/dev/null || { setsid /usr/libexec/mutter-x11-frames >/dev/null 2>&1 & sleep 1; }
cd "$GAMEDATA" || exit 1
FIRST=${1:-1}
LAST=${2:-34}
for row in $(seq "$FIRST" "$LAST"); do
    # measured: mission N sits at y = 94 + N*17 (row 22 -> y 468, "22 20mm Cannon")
    y=$(( 94 + row * 17 ))
    log=/tmp/sw-$row.log
    ( export FF_UI_CLICK="574,750@12;225,171@18;896,743@24;677,748@36;140,${y}@42;824,750@48;973,750@54"
      timeout -s INT 105 "$BIN" -d "$GAMEDATA" -w > "$log" 2>&1 )
    name=$(grep -a "StartReadCampFile: type" "$log" | head -1 | sed "s/.*filename='//;s/'.*//")
    printf "row %2d  sim=%s crash=%s asserts=%s  %s\n" "$row" \
        "$(grep -ac RunningGraphics $log)" \
        "$(grep -ac 'Segmentation fault\|Aborted\|buffer overflow' $log)" \
        "$(grep -ac 'Assertion at' $log)" "${name:-<none>}"
done
echo "=== SWEEP COMPLETE ==="

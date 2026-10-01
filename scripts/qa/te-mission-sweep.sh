#!/bin/bash
# FF-TESWEEP-1: host-only sweep of the mission TEs (campaign/SAVE/*.tac, alphabetical TE list).
# Usage: te-mission-sweep.sh [first_idx] [last_idx]   (0-based list index; row y = 111 + 17*idx,
# measured: idx 8 "Havin' Fun Strike" = 247, idx 9 "Sink the Kuz" = 264). Run under ~/bin/gl-lock.
# Per TE: load (StartReadCampFile), host takeoff, first 3D frame, crashes, [abop] lines (flights
# cancelled at load are TE data -- record, don't fix). Seat click 215,340 is the default first seat;
# a TE whose seat sits elsewhere shows takeoff=0 and needs its own seat (Sink the Kuz: 160,340).
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DISPLAY=${DISPLAY:-:0}
GAMEDATA="$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
BIN=${BIN:-$REPO/build/src/ffviper/FFViper}
OUT=${OUT:-$HOME/ff-gates/temission}
mkdir -p "$OUT"
cd "$GAMEDATA" || exit 1
for idx in $(seq "${1:-0}" "${2:-11}"); do
    y=$(( 111 + idx * 17 ))
    seat=${TE_SEAT:-160,340}
    frow=${TE_FROW:-98}
    log=$OUT/te-$idx.log
    ( export FF_UI_CLICK="674,748@10;212,16@16;170,${y}@24;824,748@32;562,748@40;110,${frow}@60;${seat}@66;976,750@168"
      export FF_DEBUG_STARTCAMP=1 FF_DEBUG_ABOP=1 FF_SIM_SCREENSHOT="215:$OUT/te-$idx.bmp"
      timeout -k 5 -s INT 230 "$BIN" -d "$GAMEDATA" -w > "$log" 2>&1 )
    name=$(grep -a "StartReadCampFile: type" "$log" | head -1 | sed "s/.*filename='//;s/'.*//")
    printf "te %2d  takeoff=%s 3D=%s crash=%s asserts=%s abop=%s  %s\n" "$idx" \
        "$(grep -ac 'STARTCAMP\] takingOff' $log)" "$(grep -ac 'RenderFirstFrame\] Exit' $log)" \
        "$(grep -ac '=== CRASH\|Segmentation fault\|Aborted' $log)" "$(grep -ac 'Assertion at' $log)" \
        "$(grep -ac '\[abop\]' $log)" "${name:-<none>}"
done
echo "=== MISSION SWEEP COMPLETE ==="

#!/bin/bash
# FF-TEMP-1: host-only flight inventory of each Tactical Engagement (FF_DEBUG_TEINV=1).
# Usage: te-inventory.sh train|saved FIRST LAST     Run under ~/bin/gl-lock.
#   train: Training tab rows, y = 94 + 17*row (te-sweep.sh geometry)
#   saved: SAVED tab (campaign/SAVE/*.tac), y = 111 + 17*idx (te-mission-sweep.sh geometry)
# Forces the KOREA theater first (TE rows index the current theater). Disposable tree only: GAMEDATA
# defaults to ~/ff-crawl, never the PO's tree.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DISPLAY=${DISPLAY:-:0}
GAMEDATA=${GAMEDATA:-$HOME/ff-crawl}
BIN=${BIN:-$REPO/build/src/ffviper/FFViper}
OUT=${OUT:-$HOME/ff-gates/teinv}
TAB=$1; mkdir -p "$OUT"
cd "$GAMEDATA" || exit 1
theater="574,750@12;225,171@18;896,743@24"
for i in $(seq "$2" "$3"); do
    if [ "$TAB" = train ]; then y=$((94 + 17*i)); c="$theater;677,748@36;140,${y}@42;824,750@48"
    else y=$((111 + 17*i)); c="$theater;674,748@36;212,16@42;170,${y}@50;824,748@56;562,748@62"; fi
    log=$OUT/$TAB-$i.log
    # The inventory prints when the flight list is first built; stop the run (its own PID) right after.
    FF_UI_CLICK="$c" FF_DEBUG_TEINV=1 FF_DEBUG_STARTCAMP=1 FF_DEBUG_ABOP=1 \
        timeout -k 5 -s INT 110 "$BIN" -d "$GAMEDATA" -w > "$log" 2>&1 &
    pid=$!
    for _ in $(seq 1 110); do grep -aq '^\[teinv\] end' "$log" 2>/dev/null && { sleep 3; break; }; kill -0 $pid 2>/dev/null || break; sleep 1; done
    kill -INT $pid 2>/dev/null; wait $pid 2>/dev/null
    name=$(grep -a "StartReadCampFile: type" "$log" | head -1 | sed "s/.*filename='//;s/'.*//")
    inv=$(grep -a '^\[teinv\] flt' "$log" | awk '{for(k=1;k<=NF;k++){split($k,a,"="); v[a[1]]=a[2]}
           if (v["final"]==1) {f[v["team"]]++; ac[v["team"]]+=v["ac"]; t[v["team"]]=t[v["team"]] v["type"] "x" v["ac"] ","}}
           END {for (k in f) printf "team%s:%dflt/%dac[%s] ", k, f[k], ac[k], t[k]}')
    printf "%s %2d crash=%s abop=%s inv=%s | %s | %s\n" "$TAB" "$i" \
        "$(grep -ac '=== CRASH\|Segmentation fault\|Aborted' $log)" "$(grep -ac '\[abop\]' $log)" \
        "$(grep -ac '^\[teinv\] end' $log)" "${name:-<none>}" "$inv"
done

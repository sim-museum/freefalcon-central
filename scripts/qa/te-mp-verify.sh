#!/bin/bash
# FF-TEMP-1 acceptance: every "* MP.tac" in ~/ff-crawl/campaign/SAVE hosted by peer A and joined + flown by peer B
# (te-mp-two.sh). The SAVED tab lists *.tac (except te_new) case-insensitively sorted, rows y = 111 + 17*idx, about
# 33 rows visible; rows past that are reported "not visible" instead of being clicked blind. Run under ~/bin/gl-lock.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
S=${GAMEDATA:-$HOME/ff-crawl}/campaign/SAVE; OUTB=${OUTB:-$HOME/ff-gates/temp1/verify}; mkdir -p "$OUTB"
mapfile -t list < <(cd "$S" && printf '%s\n' *.tac | grep -av '^te_new\.tac$' | sed 's/\.tac$//' | LC_ALL=C sort -f)
for i in "${!list[@]}"; do
    n=${list[$i]}
    case "$n" in *" MP") ;; *) continue ;; esac
    [ -n "${ONLY:-}" ] && ! echo "$n" | grep -aqE -- "$ONLY" && continue
    if [ "$i" -gt 32 ]; then printf "%-28s row %d not visible -- skipped\n" "$n" "$i"; continue; fi
    o="$OUTB/$(echo "$n" | tr -c 'A-Za-z0-9\n' '_')"; mkdir -p "$o"
    # Probe (host only, offline): the mission tree 6130 lists one row per flight, item id = the flight's camp id;
    # row order is by takeoff time, so it differs per TE. Seats are only taken by the seat click (lead = ctrl 6108
    # at 131,342) -- without it a player silently gets the FIRST row's flight.
    ( cd "${GAMEDATA:-$HOME/ff-crawl}" && FF_UI_CLICK="674,748@10;212,16@16;170,$((111 + 17*i))@24;824,748@32;562,748@40" \
        FF_DUMP_UI=50 FF_DEBUG_TEINV=1 timeout -k 5 -s INT 53 "$REPO/build/src/ffviper/FFViper" -d "${GAMEDATA:-$HOME/ff-crawl}" -w \
        > "$o/probe.log" 2>&1 )
    esc=$(grep -a '^\[teinv\] flt' "$o/probe.log" | grep -a ' team=1 final=1 ac=3 .* mission=3 ' | tail -n1 | sed 's/.*camp=\([0-9]*\).*/\1/')
    ply=$(grep -a '^\[teinv\] flt' "$o/probe.log" | grep -a ' team=1 final' | grep -avE "camp=$esc |type=(E-3|KC-10|E-2C)" | head -n1 | sed 's/.*camp=\([0-9]*\).*/\1/')
    ey=$(grep -a "^\[UIDUMP\]     item id=$esc " "$o/probe.log" | tail -n1 | sed 's/.*click=[0-9]*,//')
    py=$(grep -a "^\[UIDUMP\]     item id=$ply " "$o/probe.log" | tail -n1 | sed 's/.*click=[0-9]*,//')
    if [ -z "$ey" ] || [ -z "$py" ]; then printf "%-28s probe: escort=%s(row %s) player=%s(row %s) -- not found, skipped\n" "$n" "$esc" "$ey" "$ply" "$py"; continue; fi
    OUT="$o" TE_ROW=$((111 + 17*i)) A_ROW=$py A_SEAT=131,342 TE_JROW=$ey TE_SEAT=131,342 A_SECS=${A_SECS:-340} B_SECS=${B_SECS:-260} \
        "$REPO/scripts/qa/te-mp-two.sh" > /dev/null 2>&1
    te=$(grep -a 'StartReadCampFile: type' "$o/a.log" | head -1 | sed "s/.*filename='//;s/'.*//")
    fa=$(grep -a '^\[playerflt\] session .* local=1 -> flight' "$o/a.log" | tail -n1 | sed 's/.*flight //')
    fb=$(grep -a '^\[playerflt\] session .* local=1 -> flight' "$o/b.log" | tail -n1 | sed 's/.*flight //')
    nesc=$(grep -a '^\[teinv\] flt' "$o/a.log" | grep -ac 'ac=3 type=.* mission=3 ')
    printf "%-28s escort=%s host3D=%s join3D=%s joined=%s crash=%s/%s hosted='%s' host[%s]=player%s joiner[%s]=escort%s\n" "$n" "$nesc" \
        "$(grep -ac 'RenderFirstFrame\] Exit' "$o/a.log")" "$(grep -ac 'RenderFirstFrame\] Exit' "$o/b.log")" \
        "$(grep -ac 'FM_JOIN_SUCCEEDED' "$o/b.log")" "$(grep -ac '=== CRASH' "$o/a.log")" "$(grep -ac '=== CRASH' "$o/b.log")" \
        "$te" "$fa" "$ply" "$fb" "$esc"
done

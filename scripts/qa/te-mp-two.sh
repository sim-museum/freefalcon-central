#!/bin/bash
# FF-TEMP-1: two-instance multiplayer run of one SAVED-tab Tactical Engagement (from ~/ff-gates/mp26 camp2.sh +
# crun.sh "hostfly te", MP-TEJOIN-1). Host A hosts the TE and flies its first flight; joiner B joins, picks a flight
# row and seat, flies. Disposable trees only: GA=~/ff-crawl (host), GB=~/ff2 (joiner) -- never the PO's tree.
# Knobs: TE_ROW (host SAVED-list y), TE_JROW (joiner's flight row y), TE_SEAT (joiner seat), A_SEAT, OUT.
# A_CONNECT/B_CONNECT override the FF_MP_CONNECT specs (B_CONNECT=2944:1:sgw joins via the matchmaker).
# Run under ~/bin/gl-lock. Result line: host/joiner 3D, crashes, and the flight each player sits in.
set -u
OUT=${OUT:?}; mkdir -p "$OUT/a" "$OUT/b"
BIN=${FF_BIN:-$HOME/freefalcon-central/build/src/ffviper/FFViper}
GA=${GA:-$HOME/ff-crawl}; GB=${GB:-$HOME/ff2}
A_SECS=${A_SECS:-520}; B_SECS=${B_SECS:-420}; B_DELAY=${B_DELAY:-80}
export DISPLAY=${DISPLAY:-:0}
A="674,748@10;212,16@16;170,${TE_ROW:?}@24;824,748@32;562,748@40;110,${A_ROW:-85}@60;${A_SEAT:-215,340}@66;976,750@168${A_EXTRA:+;$A_EXTRA}"
B="674,748@10;287,16@20;140,112@32;824,748@44;562,748@54;110,${TE_JROW:-98}@70;${TE_SEAT:-160,340}@77;976,750@85"
DBG="FF_DEBUG_TEAMS=1 FF_DEBUG_MPCOMMS=1 FF_DEBUG_STARTCAMP=1 FF_DEBUG_TEINV=1 FF_DEBUG_PLAYERFLT=1 ${XENV:-}"
( cd "$GA" && env FF_WINPOS=0,0 FF_MP_CONNECT=${A_CONNECT:-2934} $DBG FF_UI_CLICK="$A" FF_UI_SCREENSHOT=${SHOT:-10} FF_UI_SHOT_DIR="$OUT/a" \
    FF_SIM_SCREENSHOT="200:$OUT/a_sim200.bmp" timeout -s INT "$A_SECS" "$BIN" -d "$GA" -w > "$OUT/a.log" 2>&1 ) &
sleep "$B_DELAY"
( cd "$GB" && env FF_WINPOS=1920,0 FF_MP_CONNECT=${B_CONNECT:-2944:2934:127.0.0.1} $DBG FF_UI_CLICK="$B" FF_UI_SCREENSHOT=${SHOT:-10} \
    FF_UI_SHOT_DIR="$OUT/b" FF_SIM_SCREENSHOT="120:$OUT/b_sim120.bmp" timeout -s INT "$B_SECS" "$BIN" -d "$GB" -w > "$OUT/b.log" 2>&1 ) &
wait
a=$OUT/a.log; b=$OUT/b.log
printf "%s: host3D=%s join3D=%s crashA=%s crashB=%s joined=%s te=%s\n" "$(basename "$OUT")" \
  "$(grep -ac 'RenderFirstFrame\] Exit' $a)" "$(grep -ac 'RenderFirstFrame\] Exit' $b)" \
  "$(grep -ac '=== CRASH' $a)" "$(grep -ac '=== CRASH' $b)" "$(grep -ac 'FM_JOIN_SUCCEEDED\|JOIN_SUCCEEDED' $b)" \
  "$(grep -a 'StartReadCampFile: type' $a | head -1 | sed "s/.*filename='//;s/'.*//")"

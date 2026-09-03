#!/bin/bash
# GNDMOVE-1 / GMT regression: the Maverick TE's tank column must actually drive,
# and must therefore show up in the GMT mover list.
#
# Pre-fix: every ground sim object reports Vt 0.0 and moves 0 ft, because the TE's
# battalions load with dest_x/dest_y == 0 and nothing ever computes a destination
# (see docs/STATUS.md, GNDMOVE-1). The GMT mover list is then empty and the scope
# is blank -- which is what the PO saw.
# Post-fix: ~480 vehicles at ~60 ft/s and a non-empty mover list.
#
# FF_NO_GNDMOVE_DEST_FIX=1 reproduces the old (stationary) behaviour.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DISPLAY=${DISPLAY:-:0}
GD="$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
BIN=${FF_BIN:-$REPO/build/src/ffviper/FFViper}
log=${GMT_LOG:-/tmp/gmt-movers.log}

pgrep -f mutter-x11-frames >/dev/null || { setsid /usr/libexec/mutter-x11-frames >/dev/null 2>&1 & sleep 1; }

( export FF_UI_CLICK="574,750@12;225,171@18;896,743@24;677,748@36;140,502@42;824,750@48;973,750@54"
  export FF_AP_MODE=1 FF_DEBUG_GM=1
  export FF_SIM_KEY="S0x53@15;0x3C@20"
  timeout -s INT 100 "$BIN" -d "$GD" -w > "$log" 2>&1 ) 2>/dev/null

# Last GMT (mode 16) mover census of the run.
line=$(grep "\[GM\] movers mode=16" "$log" | tail -1)
[ -n "$line" ] || { echo "GMT movers: never reached GMT (mode 16)  FAIL"; exit 1; }

list=$(sed 's/.* list=\([0-9]*\) .*/\1/' <<<"$line")
speedok=$(sed 's/.*simSpeedOK=\([0-9]*\) .*/\1/' <<<"$line")
vtmax=$(sed 's/.*simVt=\[[0-9.]*\.\.\([0-9.]*\)\].*/\1/' <<<"$line")
moved=$(grep "\[GM\] trk0" "$log" | tail -1 | sed 's/.*moved=\([0-9.]*\)ft.*/\1/')
moved=${moved:-0}

ok=PASS
awk -v v="$vtmax" 'BEGIN{exit !(v > 3.0)}'    || ok=FAIL   # inside the GMT speed gate
awk -v m="$moved" 'BEGIN{exit !(m > 1.0)}'    || ok=FAIL   # vehicles actually translate
[ "${list:-0}" -gt 0 ]                         || ok=FAIL   # mover list non-empty
[ "${speedok:-0}" -gt 0 ]                      || ok=FAIL

printf 'GMT movers: list=%s simSpeedOK=%s simVtMax=%s movedPerInterval=%sft  %s\n' \
       "$list" "$speedok" "$vtmax" "$moved" "$ok"
[ "$ok" = PASS ]

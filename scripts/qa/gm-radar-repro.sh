#!/bin/bash
# GMRADAR-1 regression: drive the Maverick TE into A-G / GM radar and assert no
# crash. Pre-fix (e29f4151^) this SIGSEGVs in DrawGMsquare -> DrawPrimitive
# because &v0 was indexed as a 4-element array; that only held under Win32's
# stack-contiguous argument passing.
#   S0x53 SimICPAG (modifier 1 = SHIFT -- bare 0x53 is SimDropTrack, a different
#   binding entirely), 0x3C SimRadarAGModeStep (modifier 0, so bare is correct),
#   0x1F SimNextWaypoint (the PO's "S")
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DISPLAY=:0
GD="$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
BIN=${1:-$REPO/build-relg/src/ffviper/FFViper}
log=/tmp/gm-radar-repro.log
( export FF_UI_CLICK="574,750@12;225,171@18;896,743@24;677,748@36;140,502@42;824,750@48;973,750@54"
  export FF_AP_MODE=1 FF_SIM_KEY="S0x53@15;0x3C@20;0x1F@25;0x1F@30;0x3C@35" FF_DEBUG_GM=1
  timeout -s INT 150 "$BIN" -d "$GD" -w > "$log" 2>&1 ) 2>/dev/null
c=$(grep -ac 'CRASH: SIGSEGV' "$log")
gm=$(grep -ac 'RenderGMComposite::Setup' "$log")
printf 'GM radar repro: crash=%s gmSetup=%s  %s\n' "$c" "$gm" \
  "$([ "$c" = 0 ] && echo PASS || echo FAIL)"
[ "$c" = 0 ]

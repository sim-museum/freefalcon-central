#!/bin/bash
# GMRADAR-4 regression: the GM radar noise overlay must be drawn at its vertex
# alpha (0.3), not at 1.0. Flies the Maverick TE into A-G / GM, dumps the sweep
# render target (FF_GM_DUMP) at each beam reversal, and measures the green mean
# of the post-noise image. Pre-fix: ~36/255 (alpha ignored). Post-fix: ~11/255.
# Threshold 20. FF_NO_RHW_ALPHAMOD_FIX=1 reproduces the old level.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DISPLAY=${DISPLAY:-:0}
GD="$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
BIN=${1:-$REPO/build/src/ffviper/FFViper}
D=${GM_DUMP_DIR:-/tmp/gm-noise-dump}
rm -rf "$D"; mkdir -p "$D"
log=/tmp/gm-noise-level.log
( export FF_UI_CLICK="574,750@12;225,171@18;896,743@24;677,748@36;140,502@42;824,750@48;973,750@54"
  export FF_AP_MODE=1 FF_DEBUG_GM=1 FF_GM_DUMP="$D"
  export FF_SIM_KEY="S0x53@15;0x3C@20;0x1F@25"
  timeout -s INT 110 "$BIN" -d "$GD" -w > "$log" 2>&1 ) 2>/dev/null
n=$(ls "$D"/gm_post_*.ppm 2>/dev/null | wc -l)
[ "$n" -ge 3 ] || { echo "GM noise level: only $n dumps (harness never reached GM)  FAIL"; exit 1; }
mean=$(convert "$D"/gm_post_0002.ppm -channel G -separate -format '%[fx:mean*255]' info:)
ok=$(awk -v m="$mean" 'BEGIN{print (m < 20) ? "PASS" : "FAIL"}')
printf 'GM noise level: dumps=%s postGreenMean=%.1f (threshold 20)  %s\n' "$n" "$mean" "$ok"
[ "$ok" = PASS ]

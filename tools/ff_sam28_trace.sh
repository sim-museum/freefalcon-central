#!/usr/bin/env bash
# SAM-1: TE 28 on autopilot with the SAM refusal trace, 5 minutes.
set -u
FF=$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6; OUT=$HOME/ff-gates/taiwan; TAG=${1:-sam28}; BIN=/home/admin/freefalcon-central/build/src/ffviper/FFViper
cp -a "$FF/config/registry.ini" "$OUT/registry.ini.$TAG.bak"; sed -i "s/^curTheater=.*/curTheater=1,4B6F72656100/" "$FF/config/registry.ini"
( cd "$FF" && timeout -k 5 -s INT ${RUN:-340} env DISPLAY=:0 FF_DEBUG_SAM=1 \
    FF_TE_FILE="28 Missile Threat" FF_UI_CLICK="677,748@12;140,128@18;824,750@24;973,750@30" FF_SIM_KEY="0x1e@8" \
    FF_SIM_SCREENSHOT_SIMREL=1 FF_SIM_SCREENSHOT="60:$OUT/$TAG-60.bmp;120:$OUT/$TAG-120.bmp;180:$OUT/$TAG-180.bmp;240:$OUT/$TAG.bmp" "$BIN" -d "$FF" -w ) > "$OUT/$TAG.log" 2>&1
echo "exit $?"; cp -a "$OUT/registry.ini.$TAG.bak" "$FF/config/registry.ini"
grep -a -c "\[sam\]" "$OUT/$TAG.log"; grep -a "\[sam\]" "$OUT/$TAG.log" | sed 's/ t=[0-9]*//' | sort | uniq -c | sort -rn | head -12 | cut -c1-160

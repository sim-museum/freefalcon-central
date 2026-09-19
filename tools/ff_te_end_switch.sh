#!/usr/bin/env bash
# Arm C1: fly TE 28 (Korea), press Esc in the sim, see where the game lands.
set -u
FF=$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6; OUT=$HOME/ff-gates/taiwan; TAG=${1:-te_end}; BIN=${FF_BIN:-/home/admin/freefalcon-central/build/src/ffviper/FFViper}
cp -a "$FF/config/registry.ini" "$OUT/registry.ini.$TAG.bak"
sed -i "s/^curTheater=.*/curTheater=1,${PRESET:-4B6F72656100}/" "$FF/config/registry.ini"
rm -f /tmp/ff_ui.bmp
( cd "$FF" && timeout -k 5 -s INT ${RUN:-150} env DISPLAY=:0 FF_DEBUG_THEATER=1 FF_UI_SCREENSHOT=10 ${EXTRA_ENV:-} \
    FF_TE_FILE="${TE:-28 Missile Threat}" FF_UI_CLICK="${CLICKS:-677,748@12;140,128@18;824,750@24;973,750@30}" FF_SIM_KEY="${SIMKEY:-0x01@30}" \
    "$BIN" -d "$FF" -w ) > "$OUT/$TAG.log" 2>&1
echo "exit $?"
cp -a "$OUT/registry.ini.$TAG.bak" "$FF/config/registry.ini"
[ -s /tmp/ff_ui.bmp ] && python3 -c "from PIL import Image; Image.open('/tmp/ff_ui.bmp').convert('RGB').save('$OUT/$TAG.png'); print('ui shot saved')"; rm -f /tmp/ff_ui.bmp
grep -a -n "\[FF_UI_CLICK\] firing\|FF_SIM_KEY\|EndFlight\|OTWDriver.Exit\|UI_Startup\] ENTRY\|FM_START_UI\|FM_END_UI\|CRASH\|double free\|Screenshot\] Saved" "$OUT/$TAG.log" | tail -14 | cut -c1-150

#!/bin/bash
# Drive a real weapon release from the harness. Every gate below was found by
# measurement (FF_DEBUG_PICKLE), not from docs.
#
#   1. Master Arm ON      shift + DIK 0x32   (SimArmMasterArm, keystrokes.key mod 1)
#   2. ICP A-G            shift + DIK 0x53   (SimICPAG, callback id 1014)
#   3. Pickle             DIK 0x39 = SPACE   (SimPickle)
#
# THE PICKLE MUST BE HELD LONGER THAN FCC->GetPickleTime(), which is 1000ms.
# pilotinputs.cpp latches PickleTime on the first frame the key is down and only
# raises pickleButton once (SimLibElapsedTime - PickleTime) > 1000. A 600ms hold
# produced gate lines every frame and never armed. 2500ms works:
#     [PICKLE] pickleButton -> ON
# which sets FCC->releaseConsent = TRUE (aircraftinputs.cpp:278).
#
# STILL NOT ENOUGH FOR A BOMB TO LEAVE THE JET. releaseConsent is consent, not
# release -- the FCC still needs a firing solution, which in CCIP means the pipper
# on a target. Level flight with nothing designated consents and never releases.
# That is the remaining gap for anyone wanting an automated impact.
#
# FF_SIM_KEY times are seconds AFTER SIM ENTRY (fixed in 276522ff; before that the
# clock ran from process start and everything below ~55s fired at once).
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DISPLAY=:0
GD="$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
BIN=$REPO/build-relg/src/ffviper/FFViper
ROW=${1:-20}            # 20 = "20 Bombs with CCIP"
HOLD=${2:-2500}
pgrep -f mutter-x11-frames >/dev/null || { setsid /usr/libexec/mutter-x11-frames >/dev/null 2>&1 & sleep 1; }
cd "$GD" || exit 1
y=$(( 94 + ROW * 17 ))
log=/tmp/pickle-$ROW.log
( export FF_UI_CLICK="574,750@12;225,171@18;896,743@24;677,748@36;140,${y}@42;824,750@48;973,750@54"
  export FF_SIM_KEY="S0x32@8;S0x53@11;0x39@15+${HOLD};0x39@30+${HOLD}"
  export FF_DEBUG_PICKLE=1 FF_DEBUG_LODZ=1 FF_DEBUG_MSLEND=1
  timeout -s INT 180 "$BIN" -d "$GD" -w > "$log" 2>&1 )
printf 'row %s hold=%sms  sim=%s  pickleON=%s  impacts=%s  crash=%s\n' \
  "$ROW" "$HOLD" \
  "$(grep -ac RunningGraphics "$log")" \
  "$(grep -ac 'pickleButton -> ON' "$log")" \
  "$(grep -ac '\[MSLEND\]' "$log")" \
  "$(grep -ac 'Segmentation fault\|Aborted' "$log")"

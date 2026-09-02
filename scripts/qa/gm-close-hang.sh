#!/bin/bash
# GMRADAR-3 hang hunt: fly the Maverick TE all the way to the target with the
# GM radar up and time accel, under gdb, so a wedge yields thread stacks
# instead of a frozen desktop. Hard timeout -> SIGINT stops the inferior ->
# gdb prints all stacks and exits (killing the game). The PO's machine froze
# on this scenario 2026-09-01; every run here is bounded.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DISPLAY=:0
GD="$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
BIN=${1:-$REPO/build-relg/src/ffviper/FFViper}
RUNSECS=${RUNSECS:-420}
log=/tmp/gm-close-hang.log

export FF_UI_CLICK="574,750@12;225,171@18;896,743@24;677,748@36;140,502@42;824,750@48;973,750@54"
export FF_AP_MODE=1 FF_DEBUG_GM=1
# A-G at 15s, GM up at 20s, direct-to-target waypoint stepping, 4x accel at 30s,
# GMT at 180s (close in), accel off at 200s.
export FF_SIM_KEY="S0x53@15;0x3C@20;0x1F@25;0x3A@30;0x3C@180;0x3A@200"
export FF_SIM_SCREENSHOT="200:/tmp/gmch_200.bmp;260:/tmp/gmch_260.bmp;320:/tmp/gmch_320.bmp"

# ulimit guards a runaway allocation (12 GB address space).
ulimit -v 12582912
timeout -s INT --kill-after=30 "$RUNSECS" \
  gdb -batch -ex 'set pagination off' -ex 'handle SIGINT stop' \
      -ex run \
      -ex 'echo \n=== STACKS AT STOP ===\n' \
      -ex 'thread apply all bt 25' \
      --args "$BIN" -d "$GD" -w > "$log" 2>&1

grep -c 'STACKS AT STOP' "$log" && echo "stacks in $log"

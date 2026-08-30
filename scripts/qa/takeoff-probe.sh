#!/bin/bash
# Scripted takeoff on a TE, for measuring anything that only happens once the
# jet is actually airborne (SINK-2 and the ground-handling family).
#
# FF_AP_MODE=1 alone does NOT take off -- the autopilot holds the jet at idle
# on the runway indefinitely. The takeoff has to be flown by injected keys.
# Codes are from keystrokes.key, where the trailing field is the modifier
# (0 none, 1 shift, 2 ctrl, 4 alt):
#   SimParkingBrakeToggle  0x19 mod 4  -> "A0x19"  release the parking brake
#   AFCoarseThrottleUp     0x0D mod 1  -> "S0xD"   step the throttle up
#   AFABOn                 0x0C mod 2  -> "C0xC"   minimum afterburner
# Times in FF_SIM_KEY are relative to SIM ENTRY, not process start -- the
# harness latches the clock on the first !doUI frame.
#
# TAKEOFF IS INTERMITTENT: roughly half of runs rotate, the rest sit on the
# runway. Budget at least two runs per data point and check the marker before
# trusting a null result. That is a property of the scripted roll, not of the
# build under test.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DISPLAY=:0
GAMEDATA="$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
BIN=$REPO/build-relg/src/ffviper/FFViper

ROW=${1:-2}                       # TE row; 2 is "02 Takeoff"
LOG=${2:-/tmp/takeoff.log}
SECS=${3:-260}                    # needs to outlast sim entry (~70s) + the roll

y=$(( 94 + ROW * 17 ))

pgrep -f mutter-x11-frames >/dev/null || {
    setsid /usr/libexec/mutter-x11-frames >/dev/null 2>&1 &
    sleep 1
}

pkill -x FFViper 2>/dev/null
sleep 1

cd "$GAMEDATA" || exit 1
(
    export FF_AP_MODE=1
    export FF_DEBUG_LIFT=1
    export FF_VIEW_SCRIPT="3@6"
    export FF_SIM_KEY="A0x19@5;S0xD@7;S0xD@9;S0xD@11;S0xD@13;C0xC@15;S0xD@17;S0xD@19;C0xC@22"
    export FF_UI_CLICK="574,750@12;225,171@18;896,743@24;677,748@36;140,${y}@42;824,750@48;973,750@54"
    timeout -s INT "$SECS" "$BIN" -d "$GAMEDATA" -w > "$LOG" 2>&1
)

if grep -aq "WHEELS OFF" "$LOG"; then
    echo "ROTATED. Ring dump present: 96 frames either side of wheels-off."
    grep -a "^\[LIFT\] t-" "$LOG" | grep -o "clr=[0-9.]*" | cut -d= -f2 | awk '
        {if(NR==1){mn=mx=$1} if($1<mn)mn=$1; if($1>mx)mx=$1; s+=$1; n++}
        END{printf "  ground-roll clearance: min=%.2f max=%.2f swing=%.2fft mean=%.2f n=%d\n",
                   mn, mx, mx-mn, s/n, n}'
    printf "  OnGround after wheels-off: "
    grep -a "^\[LIFT\] t+" "$LOG" | grep -o "OnGround=[01]" | uniq -c | tr '\n' ' '
    echo
else
    echo "DID NOT ROTATE (expected ~half the time) -- rerun before concluding anything."
    grep -a "^\[LIFT\] OnGround" "$LOG" | grep -o "clr=[0-9.-]*" | sort | uniq -c |
        sort -rn | head -5 | sed 's/^/  on-ground /'
fi

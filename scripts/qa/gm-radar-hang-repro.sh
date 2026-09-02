#!/bin/bash
# GMRADAR-3 repro: fly the Maverick TE (row 24) to the target area with the GM
# radar up, stepping GM->GMT close in -- the PO's hang scenario (2026-09-01,
# post-GMRADAR-2: radar draws at range, "diffuse blobs" up close, GMT inert,
# then a machine-killing hang as the jet pointed at the vehicle column).
#
# Differences from gm-radar-repro.sh:
#  - long run (default 480s) + 4x time accel so we actually reach the target
#  - GMT / mode steps late in the flight, when the footprint holds the convoy
#  - a watchdog samples ALL thread stacks with gdb when the log stops growing,
#    and once unconditionally late in the run -- a hang becomes a named frame
#  - MemoryMax scope so an allocation runaway cannot take the machine down
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DISPLAY=:0
GD="$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
BIN=${1:-$REPO/build-relg/src/ffviper/FFViper}
RUNSECS=${RUNSECS:-480}
log=/tmp/gm-hang-repro.log
stacks=/tmp/gm-hang-stacks.log
: > "$stacks"

# Click track: TE -> row 24 Mavericks -> commit -> fly (sim entry ~55s).
# Keys (relative to sim entry):
#   15s SHIFT+SimICPAG (A-G master mode), 20s AG mode step (GM up),
#   25s next waypoint, 30s CAPS = 4x time accel,
#   150s/210s AG mode step (GM->GMT etc.) near the target,
#   240s CAPS again (drop accel while close in).
( export FF_UI_CLICK="574,750@12;225,171@18;896,743@24;677,748@36;140,502@42;824,750@48;973,750@54"
  export FF_AP_MODE=1 FF_DEBUG_GM=1
  export FF_SIM_KEY="S0x53@15;0x3C@20;0x1F@25;0x3A@30;0x3C@150;0x3C@210;0x3A@240"
  export FF_SIM_SCREENSHOT="120:/tmp/gm_close_120.bmp;180:/tmp/gm_close_180.bmp;240:/tmp/gm_close_240.bmp;300:/tmp/gm_close_300.bmp"
  systemd-run --user --scope -p MemoryMax=10G -q \
    timeout -s KILL "$RUNSECS" "$BIN" -d "$GD" -w > "$log" 2>&1 ) &
runner=$!

# Wait for the game process to appear (child of the scope, not our shell).
pid=""
for i in $(seq 1 60); do
    pid=$(pgrep -x FFViper | head -1)
    [ -n "$pid" ] && break
    sleep 1
done
echo "[watchdog] FFViper pid=${pid:-NONE}" | tee -a "$stacks"

dump_stacks() {
    local tag="$1"
    [ -n "$pid" ] && kill -0 "$pid" 2>/dev/null || return
    echo "=== STACKS ($tag) $(date +%T) ===" >> "$stacks"
    gdb -p "$pid" --batch -ex 'set pagination off' \
        -ex 'thread apply all bt 20' >> "$stacks" 2>&1
}

# Watchdog: if the log stops growing for 20s, the game is likely wedged.
last=0; still=0; sampled_idle=0; t=0
while kill -0 "$pid" 2>/dev/null; do
    sleep 10; t=$((t+10))
    sz=$(stat -c%s "$log" 2>/dev/null || echo 0)
    if [ "$sz" = "$last" ]; then still=$((still+10)); else still=0; fi
    last=$sz
    if [ "$still" -ge 20 ] && [ "$sampled_idle" -lt 3 ]; then
        dump_stacks "log-stalled-${t}s"
        sampled_idle=$((sampled_idle+1))
        # Two stalled samples 10s apart with identical top frames = a real spin.
    fi
    # One unconditional late sample: even if logging keeps trickling, show
    # where the render/sim threads live when close to the target.
    if [ "$t" -eq 300 ]; then dump_stacks "scheduled-300s"; fi
done
wait "$runner" 2>/dev/null

c=$(grep -ac 'CRASH: SIGSEGV' "$log"); gm=$(grep -ac '\[GM\] stage' "$log")
printf 'gm-hang repro: crash=%s gmTicks=%s stacksFile=%s\n' "$c" "$gm" "$stacks"

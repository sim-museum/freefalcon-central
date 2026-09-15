#!/usr/bin/env bash
# tools/ff_landap_approach.sh -- LANDAP-1's TE-09 "09 Landing Final Approach" recipe, written down.
#
# Four sprints re-typed this by hand and one of them (TE2-7 S1) lost a run to the missing second
# keypress. The recipe IS the finding of LANDAP-1 S1: the front-end clicks reach the TE and fly it,
# and FF_SIM_KEY's SECOND key (Ctrl+1 = SimLeftAPSwitch) is what puts the autopilot in FollowWP
# instead of RollHold. Without it the aeroplane flies wings-level from wherever the AP engaged.
#
#   gl-lock tools/ff_landap_approach.sh [outdir]
#
#   SECS=N   seconds of wall clock (default 620 -- S2 measured the approach needs more than 340)
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="${BIN:-$ROOT/build/src/ffviper/FFViper}"
GD="${GD:-/home/admin/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6}"
OUT="${1:-${OUT:-/tmp/ff_landap}}"
SECS="${SECS:-620}"
mkdir -p "$OUT"
[ -x "$BIN" ] || { echo "no binary at $BIN" >&2; exit 2; }
if pgrep -x FFViper >/dev/null 2>&1; then
  echo "  REFUSING TO RUN: FFViper is already running (pid $(pgrep -x FFViper | tr '\n' ' '))." >&2
  exit 2
fi
log="$OUT/approach.log"
echo "TE-09 autopilot approach -- ${SECS}s, FollowWP recipe (LANDAP-1 S1)"
echo "  FF_SIM_KEY=${FF_SIM_KEY:-0x1e@5;C0x02@12}"
( cd "$GD" && timeout -k 5 -s KILL "$SECS" env \
    DISPLAY="${DISPLAY:-:0}" \
    FF_UI_CLICK="624,745@8;210,247@14;825,750@18;976,750@30" \
    FF_SIM_KEY="${FF_SIM_KEY:-0x1e@5;C0x02@12}" \
    FF_DEBUG_GROUND=1 FF_DEBUG_AP="${FF_DEBUG_AP:-1}" \
    ${FF_GROUND_PROBE:+FF_GROUND_PROBE="$FF_GROUND_PROBE"} \
    "$BIN" -d "$GD" -w ) >"$log" 2>&1
# Prove the run reached the state it claims to measure BEFORE any number in it is believed.
nap=$(grep -ac "^\[AP-1\]" "$log")
ng=$(grep -ac "^\[GROUND\]" "$log")
nn=$(grep -ac "^\[NAV\]" "$log")
echo "  [AP-1] samples: $nap   [GROUND] samples: $ng   [NAV] samples: $nn"
if [ "$nap" -gt 0 ]; then
  echo "  AP first: $(grep -a '^\[AP-1\]' "$log" | head -1)"
  echo "  AP last : $(grep -a '^\[AP-1\]' "$log" | tail -1)"
fi
grep -aq "Strg=1" "$log" \
  && echo "  route-following ENGAGED (Strg=1 seen)" \
  || echo "  ⚠ route-following NEVER engaged -- every distance below is a wings-level drift, not the plan"
echo "  onGround=1 samples: $(grep -ac "onGround=1" "$log")"

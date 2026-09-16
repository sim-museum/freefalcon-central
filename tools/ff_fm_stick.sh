#!/usr/bin/env bash
# tools/ff_fm_stick.sh -- fly a TE by STICK ONLY, with the autopilot never engaged.
#
# FM-GOLD-1 S7 (2026-09-15): S6 and S7 both measured the AUTOPILOT and not the flight model.
# ff_landap_approach.sh exists to engage the AP -- its FF_SIM_KEY default "0x1e@5" is DIK_A,
# the autopilot toggle, and its own header says that without the second key (Ctrl+1 = FollowWP)
# "the aeroplane flies wings-level from wherever the AP engaged".  That is exactly what S7's
# tape showed: roll EXACTLY 0.000000 and yaw EXACTLY 90.000 deg across all 701 samples, with a
# ground-track turn rate never above 1.43 deg/s.  RollHold, not a jet.  A flight-model number
# can never come out of that harness, so FM work gets its own: same front-end clicks to enter
# the TE, and NO sim key at all.
#
#   gl-lock tools/ff_fm_stick.sh [outdir]
#
#   SECS=N          wall clock (default 260)
#   FF_STICK2=r,p[,start_s[,dur_s]]   the held stick input (pilotinputs.cpp)
#   FF_TE_FILE=...  which tactical engagement to load
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="${BIN:-$ROOT/build/src/ffviper/FFViper}"
GD="${GD:-/home/admin/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6}"
OUT="${1:-${OUT:-/home/admin/ff-fm-stick}}"
SECS="${SECS:-260}"
mkdir -p "$OUT"
[ -x "$BIN" ] || { echo "no binary at $BIN" >&2; exit 2; }
if pgrep -x FFViper >/dev/null 2>&1; then
  echo "  REFUSING TO RUN: FFViper is already running (pid $(pgrep -x FFViper | tr '\n' ' '))." >&2
  exit 2
fi
log="$OUT/stick.log"
echo "stick-only TE flight -- ${SECS}s, NO autopilot key (FM-GOLD-1 S7)"
echo "  FF_STICK2=${FF_STICK2:-<unset>}"
( cd "$GD" && timeout -k 5 -s KILL "$SECS" env \
    DISPLAY="${DISPLAY:-:0}" \
    FF_UI_CLICK="624,745@8;210,247@14;825,750@18;976,750@30" \
    FF_DEBUG_GROUND=1 FF_DEBUG_AP=1 FF_DEBUG_STICK=1 \
    "$BIN" -d "$GD" -w ) >"$log" 2>&1
# The gate S6 and S7 both needed: prove the AUTOPILOT IS OFF before any number is believed.
nap=$(grep -ac "^\[AP-1\]" "$log")
nst=$(grep -ac "^\[stick2\]" "$log")
echo "  [AP-1] samples: $nap   [stick2] writes (print capped at 10): $nst"
if [ "$nap" -gt 0 ]; then
  echo "  ⚠⚠ AUTOPILOT ENGAGED ($nap samples) -- this run measures the AP, NOT the flight model."
  echo "     Every g and turn-rate number from this tape is void.  Do not report it."
  exit 3
fi
[ "$nst" -gt 0 ] || { echo "  ⚠ FF_STICK2 never wrote -- no input reached PilotInputs::Update."; exit 4; }
echo "  autopilot OFF and stick input reached the FM -- tape is admissible."

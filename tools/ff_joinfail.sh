#!/usr/bin/env bash
# tools/ff_joinfail.sh — JOINFAIL-1 verification.
#
# THE ITEM: "the graceful-failure path segfaults". CampaignJoinFail() ->
# C_Handler::RemoveUserCallback() -> SIGSEGV when a load failure arrives with no main UI handler
# up — i.e. the try/catch that was supposed to turn a failed load into a clean return to the menu
# crashed inside its own recovery. Both sites were guarded (campjoin.cpp:174, :360) and the fix has
# sat UNVERIFIED ever since, because nothing in the harness could make a load fail on demand.
#
# FF_TEST_JOINFAIL=<sec> raises FM_JOIN_FAILED through the same dispatch a real failure uses.
#
#   tools/ff_joinfail.sh [sec-to-raise] [run-sec]
#
# PASS = the process is still alive after the failure and has not printed a crash signature.
# A run that dies before the scheduled time is NOT a pass and is reported as such: the whole point
# is what happens AFTER the message, so the harness proves it got there first.
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="${FF_BIN:-$ROOT/build/src/ffviper/FFViper}"
GD="${FF_GAME_DIR:-$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6}"
AT="${1:-20}"; RUN="${2:-$((AT + 25))}"
# FF_TEST_JOINFAIL_NULLHANDLER=1 (default here) makes the failure arrive with NO main UI handler --
# the state the reported crash needs, and the one that cannot be reached by timing because
# gMainHandler is already up on the first frame of the main loop.
NULLH="${NULLH:-1}"
# NOGUARD=1 restores the pre-fix dereference: the CONTROL. It is expected to CRASH.
NOGUARD="${NOGUARD:-0}"
# ⚠️ `${NOGUARD:+...}` sets the variable when NOGUARD=0 as well — "0" is a NON-EMPTY string, and
# the code under test only checks getenv() presence. That made both arms run with the guard
# bypassed, and the "control did not crash" that followed was an artefact of this line.
NULLENV=""; [ "$NULLH" = "1" ] && NULLENV="FF_TEST_JOINFAIL_NULLHANDLER=1"
# NOGUARD names the site: "stop", "find", or "1"/"all" for both. Empty/0 = the shipped guards.
NOGUARDENV=""; case "$NOGUARD" in ""|0) ;; *) NOGUARDENV="FF_TEST_JOINFAIL_NOGUARD=$NOGUARD";; esac
OUT="${OUT:-/tmp/ff_joinfail}"; mkdir -p "$OUT"
LOG="$OUT/joinfail.log"
[ -x "$BIN" ] || { echo "no binary at $BIN" >&2; exit 2; }

echo "JOINFAIL-1 — raising FM_JOIN_FAILED at ${AT}s, run ${RUN}s"
( cd "$GD" && timeout -k 5 -s INT "$RUN" env DISPLAY="${DISPLAY:-:0}" \
    FF_TEST_JOINFAIL="$AT" \
    $NULLENV $NOGUARDENV \
    "$BIN" -d "$GD" -w ) >"$LOG" 2>&1
rc=$?
echo "  exit $rc (124 = still running when the harness stopped it — that is the healthy outcome)"

raised=$(grep -ac "FF_TEST_JOINFAIL\] raising" "$LOG")
handler=$(grep -a "FF_TEST_JOINFAIL\] raising" "$LOG" | sed -n 's/.*gMainHandler=\([A-Za-z]*\).*/\1/p')
recv=$(grep -ac "FM_JOIN_FAILED received" "$LOG")
# NB "Stop Campaign Load" is a MonoPrint, which does not reach stderr here -- reported, never
# asserted on. A counter that is structurally blind must not be allowed to read as evidence.
stopped=$(grep -ac "Stop Campaign Load" "$LOG")
survived=$(grep -ac "survived the no-handler recovery path" "$LOG")
seen=$(grep -a "\[joinfail\] CampaignJoinFail entered" "$LOG" | tail -1)
nulled=$(grep -ac "handing CampaignJoinFail a NULL gMainHandler" "$LOG")
crash=$(grep -acE "Segmentation fault|SIGSEGV|SIGABRT|=== CRASH" "$LOG")
echo "  message raised: $raised   (gMainHandler was: ${handler:-?})"
echo "  dispatch reached CampaignJoinFail: $recv    (no-handler state forced: $nulled, returned: $survived)"
echo "  what the recovery path saw: ${seen:-<never entered>}"
echo "  StopCampaignLoad MonoPrint seen: $stopped  (MonoPrint does not reach stderr — not evidence)"
echo "  crash signatures in the log: $crash"

if [ "$raised" -eq 0 ]; then
  echo "  INCONCLUSIVE: the failure was never raised — the run did not reach ${AT}s (see $LOG)"; exit 2
fi
if [ "$recv" -eq 0 ]; then
  echo "  FAIL: the message was posted but never dispatched"; exit 1
fi
if [ -n "$NOGUARDENV" ]; then
  # The control arm EXPECTS the pre-fix crash. A control that passes proves nothing.
  if [ "$crash" -gt 0 ] || [ "$rc" -ge 128 ]; then
    echo "  CONTROL REPRODUCED THE FAULT (rc=$rc): the guard is what keeps the recovery path alive"; exit 0
  fi
  echo "  ⚠ CONTROL ARM DID NOT CRASH — the guard is not what is keeping this alive; investigate"; exit 1
fi
if [ "$crash" -gt 0 ] || [ "$rc" -ge 128 ]; then
  echo "  FAIL: the recovery path crashed (rc=$rc)"; exit 1
fi
if [ "${NULLH:-0}" = "1" ] && [ "$survived" -eq 0 ]; then
  echo "  FAIL: the no-handler recovery path was entered and never returned"; exit 1
fi
echo "  PASS: the join failure was handled with no main UI handler, and the process survived it"

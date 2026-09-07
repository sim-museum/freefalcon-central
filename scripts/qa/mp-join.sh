#!/bin/bash
# GATE: MPTEST-FF S6y (2026-09-06) -- a second FreeFalcon instance JOINS the first's campaign game.
# Runs scripts/qa/mp-two-instance.sh with PEER_B_JOIN=1 and asserts the joiner's log reaches
# FM_JOIN_SUCCEEDED (all CAMP_NEED_* data received, the campaign UI opened). ~5 min, needs a
# display (both instances open windows). Logs: /tmp/mp-peerA.log, /tmp/mp-peerB.log.
set -u
cd "$(dirname "$0")/../.."
export PEER_B_SCREEN=campaign PEER_B_JOIN=1 B_SECS=${B_SECS:-120} A_SECS=${A_SECS:-240}
scripts/qa/mp-two-instance.sh > /tmp/mp-join.log 2>&1
n=$(grep -ac 'FM_JOIN_SUCCEEDED received' /tmp/mp-peerB.log)
pre=$(grep -ac 'requesting campaign preload' /tmp/mp-peerB.log)
win=$(grep -ac 'window id=5004 ' /tmp/mp-peerB.log)
printf "  mp-join: preload=%s info_window=%s FM_JOIN_SUCCEEDED=%s\n" "$pre" "$win" "$n"
if [ "$n" -ge 1 ]; then echo "  mp-join                PASS"; exit 0; else echo "  mp-join                FAIL  (see /tmp/mp-peerB.log)"; exit 1; fi

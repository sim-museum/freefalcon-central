#!/bin/bash
# MP-1: drive two peers through the REAL comms UI on one machine.
#
# Everything here was measured rather than guessed; see the notes at each step.
#
#   peer A (server): COMMS -> COMM_MODE_SERV -> PB_CONNECT, then enter a campaign.
#                    Entering a campaign IS hosting -- it creates the
#                    FalconGameEntity that peer B has to find.
#   peer B (client): COMMS -> PB_CONNECT. No mode click and no row click are
#                    needed: a seeded phonebkn.da2 alone puts the game in client
#                    mode, because loading an entry sets localData.ip_address = 1
#                    (phonebk.cpp:203) which flips CopyDataToWindow to the client
#                    branch and pre-fills the address.
#
# THE PORT MATTERS. Both peers default to CAPI_UDP_PORT (2934) for their LOCAL
# port, so on one machine they fight over it. -port moves peer B. That option was
# parsed only in winmain.cpp until this session; main_linux.cpp ignored it.
#
# KNOWN LIMITATION, not yet resolved: both instances share one game data
# directory and both write config/registry.ini. If results look erratic, suspect
# that before suspecting the network layer.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DISPLAY=:0
GD="$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
BIN=${FF_BIN:-$REPO/build-relg/src/ffviper/FFViper}

A_LOG=/tmp/mp-peerA.log
B_LOG=/tmp/mp-peerB.log
A_SECS=${A_SECS:-240}
B_SECS=${B_SECS:-150}
B_DELAY=${B_DELAY:-95}          # let peer A reach the campaign before B connects

pgrep -f mutter-x11-frames >/dev/null || {
    setsid /usr/libexec/mutter-x11-frames >/dev/null 2>&1 &
    sleep 1
}

pkill -x FFViper 2>/dev/null
sleep 1

# Peer B needs a seeded book to default to client mode. Peer A clicks SERV
# explicitly, so the same book does not hurt it.
python3 "$REPO/scripts/qa/seed-phonebook.py" "$GD/phonebkn.da2" 127.0.0.1 >/dev/null

echo "=== peer A (server + campaign host) starting ==="
(
    cd "$GD" || exit 1
    export FF_DEBUG_MPCOMMS=1 FF_DEBUG_CAMPCODEC=0
    # COMMS, SERV, CONNECT, dismiss COMMLINK_WIN, then the campaign track.
    # The dismiss (ALERT_CANCEL 577,468) is essential: PB_CONNECT raises
    # COMMLINK_WIN (357,282 310x203) over the screen, and without dismissing it
    # every campaign click lands on that dialog -- the first run of this script
    # reported "campaign files read: 0" for exactly that reason.
    export FF_UI_CLICK="487,748@12;733,476@17;512,748@22;577,468@28;924,745@40;905,758@52;563,751@60;495,390@68;110,135@82"
    timeout -s INT "$A_SECS" "$BIN" -d "$GD" -w > "$A_LOG" 2>&1
) &
A_PID=$!

sleep "$B_DELAY"

echo "=== peer B (client) starting on port 2936 ==="
(
    cd "$GD" || exit 1
    export FF_DEBUG_MPCOMMS=1
    # COMMS, CONNECT, then dismiss COMMLINK_WIN. Peer A always dismissed it;
    # peer B did not, so peer B spent every run sitting behind the connect
    # dialog -- including both UI dumps, which is why "no game list" was
    # observed. Nothing in the comms screen auto-hides that dialog; it is
    # dismissed by its own ALERT_CANCEL (577,468). Only the campaign and
    # dogfight join paths hide it programmatically.
    export FF_UI_CLICK="487,748@12;512,748@18;577,468@26"
    export FF_DUMP_UI="30;45"          # after connect: find the game-list screen
    timeout -s INT "$B_SECS" "$BIN" -d "$GD" -w -port 2936 > "$B_LOG" 2>&1
) &
B_PID=$!

wait $B_PID
wait $A_PID

echo
echo "=== peer A: comms + hosting ==="
grep -a "\[PBOOK\]\|\[MP\]" "$A_LOG" | head -3
printf "  campaign files read: %s   crash: %s\n" \
    "$(grep -ac StartReadCampFile "$A_LOG")" \
    "$(grep -ac 'Segmentation fault\|Aborted' "$A_LOG")"

echo "=== peer B: comms + what it saw ==="
grep -a "\[PBOOK\]\|\[MP\]" "$B_LOG" | head -3
printf "  crash: %s\n" "$(grep -ac 'Segmentation fault\|Aborted' "$B_LOG")"
echo "  windows visible after connect:"
grep -a "UIDUMP. window" "$B_LOG" | sort -u | head -12

echo
echo "=== did any game cross the wire? ==="
# Count what the code actually prints. The first version of this grepped for
# "type=10|type=11" and reported 0 while peer B was demonstrably decoding a real
# remote game -- the metric was measuring a string the build does not emit.
printf "  peer B session decodes: %s\n" "$(grep -ac 'MPGAMEID. session decode' "$B_LOG")"
printf "  peer B remote games remembered: %s\n" \
    "$(grep -ac 'remembered REMOTE game' "$B_LOG")"
grep -ao "gameId=[0-9]*/[0-9]*" "$B_LOG" | sort -u | sed 's/^/    /'
echo "  (a local placeholder is 0/2; a real remote game has the form <n>/28007)"
grep -a "JoinGame\|IsGame\|GotJoinData" "$B_LOG" | tail -3
echo "=== MP TWO-INSTANCE COMPLETE ==="

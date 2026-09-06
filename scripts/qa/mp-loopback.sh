#!/bin/bash
# MPTEST-FF: TWO INSTANCES, ONE MACHINE, over loopback.
#
# PO 2026-09-05: "follow the same general process used to test bob multiplayer, to test ff
# multiplayer". docs/MULTIPLAYER.md says "a real two-machine connection has NEVER been tested from
# this box", and MP-1 called what remains "genuinely not answerable on one machine". BoB disproved
# that shape of claim by running two instances over loopback, so this asks the same question here.
#
# FF is far better placed than BoB or MA were: FF_MP_CONNECT drives the connect path with NO
# phonebook dialog, so this needs no click recipe at all and sidesteps MP-1's text-entry defects.
#
# ⚠️ THE PORTS. docs/STATUS.md proposed the client run "2934:2934:127.0.0.1", but on ONE machine
# that asks the client to bind the port the host already holds. The first fix here -- client on
# 2935 -- was ALSO wrong, and the run said so: each instance binds its local port AND the next one
# for reliable traffic:
#     host   localPort=2934 recvPort=2934 reliableRecvPort=2935
#     client localPort=2935 recvPort=2935 reliableRecvPort=2936   <-- 2935 twice
# (and vuevent.cpp pokes recvPort+1..+3, so allow a small block per instance). The client is now
# ten ports clear at 2944 and still TALKS to the host's 2934.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DISPLAY=${DISPLAY:-:0}
GD="$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
BIN=${FF_BIN:-$REPO/build/src/ffviper/FFViper}
OUT=${MP_LOG_DIR:-/tmp/ff-mp}
SECS=${MP_SECS:-70}
mkdir -p "$OUT"
[ -x "$BIN" ] || { echo "  FAIL: no FFViper at $BIN"; exit 1; }

run() {  # $1 = role, $2 = FF_MP_CONNECT spec
  ( export FF_MP_CONNECT="$2" FF_DEBUG_MPCOMMS=1
    # MPTEST-FF S3: FF_STRACE=1 wraps each instance in strace (network syscalls only) so the
    # gate can show WHAT each end sends (address:port) and what recvfrom returns -- packet truth.
    W=""; [ -n "${FF_STRACE:-}" ] && W="strace -f -e trace=socket,bind,sendto,recvfrom,connect -o $OUT/$1.strace"
    timeout -s INT "$SECS" $W "$BIN" -d "$GD" -w > "$OUT/$1.log" 2>&1 ) 2>/dev/null
}

echo "MPTEST-FF loopback gate (two instances: host 2934, client 2944 -> 2934)"
run host   "2934"                 &  hpid=$!
sleep 3
run client "2944:2934:127.0.0.1"  &  cpid=$!
wait $hpid $cpid 2>/dev/null

fails=0
say() { printf "  %-4s %-46s %s\n" "$1" "$2" "$3"; [ "$1" = FAIL ] && fails=$((fails+1)); return 0; }

for r in host client; do
  L="$OUT/$r.log"
  sc=$(grep -ac '\[MPCONNECT\] StartComms' "$L" 2>/dev/null || echo 0)
  on=$(grep -a '\[MPCONNECT\] returned, Online=' "$L" 2>/dev/null | tail -1 | sed -n 's/.*Online=\([0-9]*\).*/\1/p')
  mp=$(grep -a '\[MPCOMMS\] localPort=' "$L" 2>/dev/null | tail -1)
  echo "  $r: $(basename "$L")  ${mp:-（no [MPCOMMS] line）}"
  [ "${sc:-0}" -ge 1 ] && say PASS "$r reached StartComms" "" || say FAIL "$r reached StartComms" "never called"
  # Online is sampled the INSTANT StartComms returns, so a listening host can legitimately read 0
  # there and still receive later (this run: host Online=0 with 8200 recv calls). Report it; the
  # packet evidence below is what decides.
  [ "${on:-0}" = 1 ] && say PASS "$r reports Online" "" || say INFO "$r reports Online" "Online=${on:-none} (snapshot at StartComms return)"
done

# ⭐ THE ACTUAL QUESTION. `Online=1` is a LOCAL flag -- it says this process opened its own
# transport, not that anything reached the other one. capi.c already carries counters that separate
# the two explanations MP-1 wrote down ("packets never move (transport open but inert)" vs "packets
# move and the session/game-list handshake never happens"), behind FF_DEBUG_MPCOMMS.
#
# ⚠️ READ THE RETURN VALUES, NOT THE COUNTS. `[MPIO] send n=8200` is the call NUMBER, and the
# counter only prints for n<=3 or every 200th call -- so a big n means "this was called a lot",
# NOT "8200 packets crossed". Asserting on n gave a confident PASS while every logged recv was
# returning 0. A send returns BYTES on success; -9 is COMAPI_EMPTYGROUP (the group has no members
# to send to) and -2 another refusal.
echo "  ---- did packets actually move? ----"
for r in host client; do
  L="$OUT/$r.log"
  sok=$(grep -a '\[MPIO\] send' "$L" 2>/dev/null | sed -n 's/.*ret=\(-\?[0-9]*\).*/\1/p' | awk '$1>0' | wc -l)
  rok=$(grep -a '\[MPIO\] recv' "$L" 2>/dev/null | sed -n 's/.*ret=\(-\?[0-9]*\).*/\1/p' | awk '$1>0' | wc -l)
  eg=$(grep -ac 'ret=-9' "$L" 2>/dev/null || echo 0)
  printf "  %-6s sends with bytes=%-4s recvs with bytes=%-4s EMPTYGROUP refusals=%s\n" "$r" "$sok" "$rok" "$eg"
  eval "${r}_sok=$sok; ${r}_rok=$rok; ${r}_eg=$eg"
done
if [ "$(( ${host_sok:-0} + ${client_sok:-0} ))" -eq 0 ]; then
  say FAIL "either end sends a byte" "no send ever returned a length"
else
  say PASS "at least one end sends real bytes" "host=$host_sok client=$client_sok"
fi
if [ "$(( ${host_rok:-0} + ${client_rok:-0} ))" -eq 0 ]; then
  say FAIL "either end RECEIVES a byte" "nothing was ever received -- the two peers do not meet"
else
  say PASS "packets cross the loopback" "host=$host_rok client=$client_rok"
fi
# EMPTYGROUP is reported, NOT failed on. S1 read these refusals as "the peer is never added to the
# group". [MPSEND] members= shows that is wrong: members=0 only for the first three sends, BEFORE
# discovery, and then 1 and 2. The group fills. These are a startup ORDERING artefact -- the game
# sends before it has anyone to send to -- not the blocker.
say INFO "COMAPI_EMPTYGROUP refusals (startup only)" "host=${host_eg:-0} client=${client_eg:-0}"
for r in host client; do
  m=$(grep -a '\[MPSEND\] call=' "$OUT/$r.log" 2>/dev/null | tail -1 | sed -n 's/.*members=\([0-9]*\).*/\1/p')
  printf "  %-6s final send-group members=%s\n" "$r" "${m:-0}"
  [ "${m:-0}" -ge 1 ] || say FAIL "$r ends with members in its send group" "members=${m:-0}"
done

echo "  ---- evidence that the two ends met ----"
grep -a '\[MPCOMMS\]\|\[MPCONNECT\]' "$OUT/host.log"   | tail -6 | sed 's/^/    host   /'
grep -a '\[MPCOMMS\]\|\[MPCONNECT\]' "$OUT/client.log" | tail -6 | sed 's/^/    client /'

[ "$fails" -eq 0 ] && echo "MP LOOPBACK: both ends started comms" || echo "MP LOOPBACK: FAIL ($fails)"
exit $([ "$fails" -eq 0 ] && echo 0 || echo 1)

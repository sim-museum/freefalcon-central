#!/bin/bash
# MP-DMG-2 S4: two FreeFalcon "machines" on one box -- runs INSIDE `unshare -rn` (its own network stack). The host
# stays in this namespace (10.9.0.1), the joiner runs in a nested one (10.9.0.2), joined by a veth pair: both use
# UDP 2934 like the PO's two PCs, and every packet crosses an interface, not loopback.
set -u
OUT=${OUT:?}; BIN=${FF_BIN:-$HOME/freefalcon-central/build/src/ffviper/FFViper}
GA=${FF1:-$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6}; GB=${FF2:-$HOME/ff2}
A_SECS=${A_SECS:-300}; B_SECS=${B_SECS:-260}; B_DELAY=${B_DELAY:-30}
A_CLICK="874,748@10;892,748@20;562,748@28;880,750@90"
B_CLICK="874,748@10;212,16@18;140,122@24;892,748@28;562,748@34;880,750@50"
ip link set lo up
unshare -n sleep 100000 & PEER=$!; sleep 0.5
ip link add v0 type veth peer name v1 && ip link set v1 netns $PEER
ip addr add 10.9.0.1/24 dev v0 && ip link set v0 up
nsenter -t $PEER -n sh -c 'ip link set lo up; ip addr add 10.9.0.2/24 dev v1; ip link set v1 up'
ping -c1 -W2 10.9.0.2 >/dev/null && echo "[ns] veth up: 10.9.0.1 <-> 10.9.0.2" || { echo "[ns] veth FAILED"; kill $PEER; exit 2; }
( cd "$GA" && env FF_WINPOS=0,0 FF_MP_CONNECT=2934 FF_DEBUG_MPCOMMS=1 FF_DEBUG_MPMSG=1 \
    FF_UI_CLICK="$A_CLICK" FF_UI_SCREENSHOT=10 FF_UI_SHOT_DIR="$OUT/a" ${A_ENV:-} \
    timeout -s INT "$A_SECS" "$BIN" -d "$GA" -w > "$OUT/a.log" 2>&1 ) & APID=$!
sleep "$B_DELAY"
nsenter -t $PEER -n sh -c "cd '$GB' && exec env FF_WINPOS=1920,0 FF_MP_CONNECT=2934:2934:10.9.0.1 FF_DEBUG_MPCOMMS=1 FF_DEBUG_MPMSG=1 \
    FF_UI_CLICK='$B_CLICK' FF_UI_SCREENSHOT=10 FF_UI_SHOT_DIR='$OUT/b' ${B_ENV:-} \
    timeout -s INT $B_SECS '$BIN' -d '$GB' -w > '$OUT/b.log' 2>&1" & BPID=$!
wait $APID $BPID   # not a bare wait: the namespace-holder sleep is a child too
kill $PEER 2>/dev/null
echo "[ns] done"

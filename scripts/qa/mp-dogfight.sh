#!/bin/bash
# MP-2INST (2026-09-26): two FreeFalcon instances on ONE box, each with its OWN data tree and window,
# driven through the real UI. Written for MP-DMG-1 / MP-CLOCK-1; replaces mp-two-instance.sh's shared
# data dir (both peers wrote config/registry.ini) and its centred windows (a covered XWayland window
# runs at 1 frame/s, which reads exactly like a sync defect).
#
#   peer A (host):   ~/sgl/.../FreeFalcon6, UDP 2934, window at 0,0     (monitor 1)
#   peer B (joiner): $FF2 (default ~/ff2, a copy of the tree), UDP 2944, window at 1920,0 (monitor 2)
#
# MODE=dogfight (default) -- host: DOGFIGHT, HOST, OK on the rules window, TAKEOFF.
#                            joiner: DOGFIGHT, ONLINE tab, the host's game, COMMIT, OK, TAKEOFF.
# MODE=campaign            -- host: CAMPAIGN, COMMIT, OK, OK (priorities), START CAMPAIGN.
#                            joiner: CAMPAIGN, JOIN tab, the host's game, COMMIT, COMPLY.
#
# Extra per-peer environment goes in A_ENV / B_ENV (space-separated VAR=value words), extra clicks in
# A_CLICK / B_CLICK (replace the recipe). Logs: $OUT/a.log, $OUT/b.log; UI shots in $OUT/a, $OUT/b.
#
# Harness lifetimes: B starts B_DELAY after A, so A_SECS must be >= B_DELAY + B_SECS + margin or the
# joiner is measuring a dead host.
#
# Useful:  A_ENV="FF_TEST_MPDMG=45"          one proximity hit on the joiner's jet (MP-DMG-1)
#          B_ENV="FF_TEST_MPFORMUP=10:3000"  joiner holds 3000 ft ahead of the host's nose, so the
#                                            host can shoot it with a boresight AIM-9 ('D', pickle)
#          FF_DEBUG_MPMSG=1 (always on here) [mpdmg] lock/release/flags/damage lines
#          FF_DEBUG_MPCLOCK=1                [mpclock] the joiner's clock following the host's
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
OUT=${OUT:-/tmp/ff-mp-dogfight}; mkdir -p "$OUT/a" "$OUT/b"
BIN=${FF_BIN:-$REPO/build/src/ffviper/FFViper}
GA=${FF1:-$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6}
GB=${FF2:-$HOME/ff2}
MODE=${MODE:-dogfight}
A_SECS=${A_SECS:-300}; B_SECS=${B_SECS:-260}; B_DELAY=${B_DELAY:-30}
[ "$A_SECS" -ge $((B_DELAY + B_SECS)) ] || { echo "A_SECS=$A_SECS < B_DELAY+B_SECS: the host would die first" >&2; exit 2; }
[ -d "$GB" ] || { echo "no second data tree at $GB (cp -a --reflink=auto \"$GA\" \"$GB\", ~14 GB)" >&2; exit 2; }

case "$MODE" in
    dogfight)
        A_CLICK=${A_CLICK:-"874,748@10;892,748@20;562,748@28;880,750@90"}
        B_CLICK=${B_CLICK:-"874,748@10;212,16@18;140,122@24;892,748@28;562,748@34;880,750@50"} ;;
    campaign)
        A_CLICK=${A_CLICK:-"974,748@10;905,758@22;563,751@30;563,751@40;495,390@50"}
        B_CLICK=${B_CLICK:-"974,748@10;269,16@22;121,122@26;892,748@32;562,748@40"} ;;
    *) echo "MODE must be dogfight or campaign" >&2; exit 2 ;;
esac

export DISPLAY=${DISPLAY:-:0}
( cd "$GA" && env FF_WINPOS=0,0 FF_MP_CONNECT=2934 FF_DEBUG_MPCOMMS=1 FF_DEBUG_MPMSG=1 FF_DEBUG_MPCLOCK=1 \
    FF_UI_CLICK="$A_CLICK" FF_UI_SCREENSHOT=${SHOT:-10} FF_UI_SHOT_DIR="$OUT/a" ${A_ENV:-} \
    timeout -s INT "$A_SECS" "$BIN" -d "$GA" -w > "$OUT/a.log" 2>&1 ) &
sleep "$B_DELAY"
( cd "$GB" && env FF_WINPOS=1920,0 FF_MP_CONNECT=2944:2934:127.0.0.1 FF_DEBUG_MPCOMMS=1 FF_DEBUG_MPMSG=1 FF_DEBUG_MPCLOCK=1 \
    FF_UI_CLICK="$B_CLICK" FF_UI_SCREENSHOT=${SHOT:-10} FF_UI_SHOT_DIR="$OUT/b" ${B_ENV:-} \
    timeout -s INT "$B_SECS" "$BIN" -d "$GB" -w > "$OUT/b.log" 2>&1 ) &
wait

echo "=== $MODE: host ==="
printf "  crash: %s   in 3-D: %s\n" "$(grep -ac '=== CRASH' "$OUT/a.log")" "$(grep -ac 'FM_START_DOGFIGHT\|FM_START_CAMPAIGN' "$OUT/a.log")"
grep -a '\[mpdmg\]' "$OUT/a.log" | head -12 | sed 's/^/  /'
echo "=== $MODE: joiner ==="
printf "  crash: %s   joined: %s   in 3-D: %s\n" "$(grep -ac '=== CRASH' "$OUT/b.log")" \
    "$(grep -ac 'FM_JOIN_SUCCEEDED' "$OUT/b.log")" "$(grep -ac 'FM_START_DOGFIGHT\|FM_START_CAMPAIGN' "$OUT/b.log")"
grep -a '\[mpdmg\]' "$OUT/b.log" | head -12 | sed 's/^/  /'
grep -a '\[mpclock\]' "$OUT/b.log" | tail -2 | sed 's/^/  /'
echo "=== MP $MODE COMPLETE ==="

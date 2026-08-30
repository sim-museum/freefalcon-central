#!/bin/bash
# Drive the campaign UI in each theater and report what campaign actually loads.
# Click track is the one that reaches the Korea frag order (MP-1 sprint 4).
# NOTE: plain subshell, never setsid -- setsid detaches and the shell then races
# the run, greps an empty log, and a stale FFViper rewrites registry.ini.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DISPLAY=:0
GD="$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
# FF_LINUX: build-relg is the current build; build/ went stale and is a different
# config, so results from it did not reflect the tree under test (same defect found
# in theater-sweep.sh, 5274414d).
BIN=${FF_BIN:-$REPO/build-relg/src/ffviper/FFViper}
R="$GD/config/registry.ini"
cp "$R" /tmp/registry.ini.campbak
pgrep -f mutter-x11-frames >/dev/null || { setsid /usr/libexec/mutter-x11-frames >/dev/null 2>&1 & sleep 1; }

for name in "$@"; do
    hex=$(printf '%s' "$name" | xxd -p | tr -d '\n')00
    sed -i "s/^curTheater=.*/curTheater=1,$hex/" "$R"
    log=/tmp/camp-$(printf '%s' "$name" | tr -c 'A-Za-z0-9' '_').log
    ( export FF_UI_CLICK="924,745@14;905,758@26;563,751@34;495,390@42;110,135@56"
      export FF_DEBUG_THEATER=1
      timeout -s INT 100 "$BIN" -d "$GD" -w > "$log" 2>&1 ) 2>/dev/null
    printf '%-22s theater=%-18s camps_read=%-3s asserts=%-3s crash=%s\n' \
        "[$name]" \
        "$(grep -a matched "$log" | head -1 | sed "s/.*matched '//;s/'.*//")" \
        "$(grep -ac 'StartReadCampFile' "$log")" \
        "$(grep -ac 'Assertion at' "$log")" \
        "$(grep -ac 'Segmentation fault\|Aborted' "$log")"
done

cp /tmp/registry.ini.campbak "$R"
echo "=== CAMPAIGN SWEEP COMPLETE (registry restored) ==="

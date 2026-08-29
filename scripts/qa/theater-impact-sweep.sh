#!/bin/bash
# ACMI/BOOM follow-up: every explosion and terrain-Z measurement so far has been
# KOREA ONLY. Terrain data, LOD spreads and airbase geometry differ per theater, so
# the physics-vs-drawn ground disagreement (the TERRAIN-Z family) may differ too.
# Spawns ground bursts in each theater and reports the disagreement.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DISPLAY=:0
GD="$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
BIN=$REPO/build-relg/src/ffviper/FFViper
R="$GD/config/registry.ini"
cp "$R" /tmp/registry.ini.impbak
pgrep -f mutter-x11-frames >/dev/null || { setsid /usr/libexec/mutter-x11-frames >/dev/null 2>&1 & sleep 1; }
y=$(( 94 + 1 * 17 ))   # TE row 1 exists in every theater's list

for name in "Korea" "Balkans" "Israeli" "Israel 2012" "Israel Classic" "EuroWar Theater" "Korea 2012 Theater "; do
    hex=$(printf '%s' "$name" | xxd -p | tr -d '\n')00
    sed -i "s/^curTheater=.*/curTheater=1,$hex/" "$R"
    log=/tmp/imp-$(printf '%s' "$name" | tr -c 'A-Za-z0-9' '_').log
    ( export FF_UI_CLICK="574,750@12;225,171@18;896,743@24;677,748@36;140,${y}@42;824,750@48;973,750@54"
      export FF_AP_MODE=1 FF_TEST_EXPLOSION=ground FF_TEST_EXPL_DIST=2500
      export FF_DEBUG_DRAWNGND=1 FF_DEBUG_LODZ=1
      timeout -s INT 150 "$BIN" -d "$GD" -w > "$log" 2>&1 ) 2>/dev/null
    # largest physics-vs-drawn raise seen, and the burst ground heights
    raise=$(grep -a DRAWNGND "$log" | sed 's/.*raise=\([0-9.-]*\).*/\1/' | sort -g | tail -1)
    printf '%-22s theater=%-18s bursts=%-3s drawnRaises=%-3s maxRaise=%-6s crash=%s\n' \
        "[$name]" \
        "$(grep -a matched "$log" | head -1 | sed "s/.*matched '//;s/'.*//")" \
        "$(grep -ac 'GROUND burst' "$log")" \
        "$(grep -ac DRAWNGND "$log")" \
        "${raise:-none}" \
        "$(grep -ac 'CRASH: SIGSEGV' "$log")"
done
cp /tmp/registry.ini.impbak "$R"
echo "=== THEATER IMPACT SWEEP COMPLETE (registry restored) ==="

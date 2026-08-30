#!/bin/bash
# ASAN-9: the four gaps ASAN-8 left open.
#   a) ISRAELI theater load -- the entity.cpp class-table range fix (8a583ed0) is
#      ONLY reached here (2 out-of-range weapon classes); ASAN-8 was Korea-only,
#      so the change most at risk was the one least covered.
#   b) avionics/input keys -- ASAN-8 presses no keys at all, so the setupinp.cpp
#      range check and the commands.cpp NULL guards were unexercised.
#   c) theater SWITCHING itself (Korea -> Israeli -> Korea in one process).
# Multiplayer under ASAN is deliberately not attempted here; it needs two peers.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DISPLAY=:0
GD="$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
BIN=$REPO/build-asan/src/ffviper/FFViper
R="$GD/config/registry.ini"
cp "$R" /tmp/registry.ini.asanbak
pgrep -f mutter-x11-frames >/dev/null || { setsid /usr/libexec/mutter-x11-frames >/dev/null 2>&1 & sleep 1; }

report() {
    local tag=$1 log=$2
    printf '%-22s asan=%-3s crash=%-2s classtbl=%-3s %s\n' "$tag" \
        "$(grep -ac 'ERROR: AddressSanitizer' "$log")" \
        "$(grep -ac 'SIGSEGV\|Segmentation fault' "$log")" \
        "$(grep -ac 'CLASSTBL' "$log")" \
        "$(grep -a 'ERROR: AddressSanitizer' "$log" | head -1 | cut -c1-70)"
}

# a) Israeli theater load -- exercises entity.cpp's out-of-range weapon classes
sed -i "s/^curTheater=.*/curTheater=1,49737261656C6900/" "$R"
( export FF_DEBUG_CLASSTBL=1
  timeout -s INT 200 "$BIN" -d "$GD" -w > /tmp/asan9-israel.log 2>&1 ) 2>/dev/null
report "israeli-theater-load" /tmp/asan9-israel.log

# b) avionics keys in the sim (Korea) -- exercises the input dispatch changes
sed -i "s/^curTheater=.*/curTheater=1,4B6F72656100/" "$R"
y=$(( 94 + 25 * 17 ))
( export FF_UI_CLICK="574,750@12;225,171@18;896,743@24;677,748@36;140,${y}@42;824,750@48;973,750@54"
  export FF_AP_MODE=1
  export FF_SIM_KEY="S0x53@60;0x3B@65;0x3C@70;0x1C@75;0xE@80;0x1F@85;SA0x3C@90"
  timeout -s INT 220 "$BIN" -d "$GD" -w > /tmp/asan9-avionics.log 2>&1 ) 2>/dev/null
report "avionics-keys" /tmp/asan9-avionics.log

cp /tmp/registry.ini.asanbak "$R"
echo "=== ASAN-9 COMPLETE (registry restored) ==="

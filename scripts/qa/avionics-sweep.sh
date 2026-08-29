#!/bin/bash
# AVIONICS-1: drive avionics mode keys and watch for crashes/assertions.
# The 34-mission TE sweep never enters A-G, radar or MFD modes, which is why a
# hard SIGSEGV in the GM radar (GMRADAR-1) survived every sweep this project ran.
# Keys are the unmodified (modifier column 0) bindings from config/keystrokes.key.
# FF_SIM_KEY times are seconds after SIM ENTRY, which lands around 55s here --
# times below that all fire at once on the first serviced frame.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DISPLAY=:0
GD="$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
BIN=$REPO/build-relg/src/ffviper/FFViper
pgrep -f mutter-x11-frames >/dev/null || { setsid /usr/libexec/mutter-x11-frames >/dev/null 2>&1 & sleep 1; }

run_batch() {
    local name=$1; shift
    local keys="" t=60
    for k in "$@"; do keys="$keys$k@$t;"; t=$((t+5)); done
    local log=/tmp/av-$name.log
    ( export FF_UI_CLICK="574,750@12;225,171@18;896,743@24;677,748@36;140,${y}@42;824,750@48;973,750@54"
      export FF_AP_MODE=1 FF_SIM_KEY="${keys%;}"
      timeout -s INT 150 "$BIN" -d "$GD" -w > "$log" 2>&1 ) 2>/dev/null
    printf '%-8s keys=%-2s fired=%-2s crash=%-2s asserts=%-2s %s\n' "$name" "$#" \
        "$(grep -ac 'FF_SIM_KEY. DOWN' "$log")" \
        "$(grep -ac 'CRASH: SIGSEGV\|Segmentation fault\|Aborted' "$log")" \
        "$(grep -ac 'Assertion at' "$log")" \
        "$(grep -a 'Assertion at' "$log" | sed 's/.*  \([^ ]*\.cpp\)/\1/' | sort -u | tr '\n' ' ')"
}

# Rows matter: the GMRADAR-1 crash reproduces on row 24 (Mavericks) but NOT on
# row 18 (A-G Radar Modes) with the same keys, so a single-row sweep proves very
# little. Validated: pre-fix, row 24 + the 0x53 primer crashes; row 18 does not.
for ROW in ${ROWS:-18 20 24 25 26}; do
    y=$(( 94 + ROW * 17 ))
    echo "--- row $ROW ---"
    run_batch r${ROW}modes 0x53 0x3B 0x3C 0x1F 0x28 0x17 0x1C
    run_batch r${ROW}radar 0x53 0x3D 0x3E 0x3F 0x41 0x42 0x43
    run_batch r${ROW}misc  0x53 0x44 0x57 0x58 0xC9 0xD1 0xD2
done
echo "=== AVIONICS SWEEP COMPLETE ==="

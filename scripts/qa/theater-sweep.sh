#!/bin/bash
# Load each installed theater through the registry path and report what it
# actually loaded. curTheater is a hex, NUL-terminated REG_SZ (see
# ff-verification-method); FindTheaterByName silently returns m_first on a
# miss, so "matched X when we asked for Y" is the failure to look for.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DISPLAY=:0
GD="$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
# FF_LINUX: build-relg is the current build (te-sweep.sh and every other QA script
# use it). This pointed at build/, which went stale -- so a "theaters still work"
# result from this script could have been measured against an old binary.
BIN=${FF_BIN:-$REPO/build-relg/src/ffviper/FFViper}
R="$GD/config/registry.ini"
cp "$R" /tmp/registry.ini.sweepbak

names=("Korea" "Balkans" "Israeli" "Israel 2012" "Israel Classic" "EuroWar Theater" "Korea 2012 Theater ")

for name in "${names[@]}"; do
    hex=$(printf '%s' "$name" | xxd -p | tr -d '\n')00
    sed -i "s/^curTheater=.*/curTheater=1,$hex/" "$R"
    log=/tmp/th-sweep-$(printf '%s' "$name" | tr -c 'A-Za-z0-9' '_').log
    # FF_LINUX: the [THEATER] trace this script greps for is gated on
    # FF_DEBUG_THEATER (theaterdef.cpp:587). Without it every row reported
    # matched=<none> and therefore "FELL BACK" -- a detector that could only ever
    # return one answer, so a clean sweep and a broken build looked identical.
    ( export FF_DEBUG_THEATER=1
      timeout -s INT 40 "$BIN" -d "$GD" -w > "$log" 2>&1 ) 2>/dev/null
    matched=$(grep -a "matched" "$log" | head -1 | sed "s/.*matched '//;s/'.*//")
    themap=$(grep -a "MapClass::Setup" "$log" | tail -1 | sed "s|.*FreeFalcon6/||;s|/terrain/Theater.map.*||")
    camp=$(grep -a "  campaign  =" "$log" | tail -1 | sed "s|.*FreeFalcon6/||")
    ok="OK"
    [ "$matched" = "$name" ] || ok="FELL BACK"
    printf '%-22s -> matched=%-18s map=%-28s camp=%-24s %s\n' \
        "[$name]" "${matched:-<none>}" "${themap:-<none>}" "${camp:-<none>}" "$ok"
done

cp /tmp/registry.ini.sweepbak "$R"
echo "=== THEATER SWEEP COMPLETE (registry restored) ==="

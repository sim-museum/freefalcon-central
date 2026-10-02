#!/bin/bash
# FF-TEMP-1: make "<TE> MP" with the game's own TE editor (recipe measured 2026-10-02 on 01 Basic Handling):
#   stage the TE as campaign/SAVE/!edit.tac (sorts to SAVED row 0; click its TEXT at 115,111) -> EDIT 437,16 ->
#   ATO 610,749 -> right-click the player's ATO row (150,ROWY) -> "Add Flight" (+49 px) -> ADD FLIGHT window
#   (defaults: same type, role HAVCAP, target = that flight, status Target) -> Size 485,297 -> "3" 347,343 ->
#   OK 537,438 -> SAVE 536,750 -> name field 400,586, type "<TE> MP" -> SAVE 536,540.
# Usage: te-mp-make.sh "<source file in campaign/SAVE>" [ROWY=121] [shot]   ("shot": only photograph the ATO)
# Disposable tree only (GAMEDATA defaults to ~/ff-crawl). Run under ~/bin/gl-lock.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DISPLAY=${DISPLAY:-:0}
GAMEDATA=${GAMEDATA:-$HOME/ff-crawl}; BIN=${BIN:-$REPO/build/src/ffviper/FFViper}
SRC=$1; ROWY=${2:-121}; MODE=${3:-make}
S="$GAMEDATA/campaign/SAVE"; base="${SRC%.*}"; name="$base MP"
OUT=${OUT:-$HOME/ff-gates/temake}/$(echo "$base" | tr -c 'A-Za-z0-9\n' '_'); mkdir -p "$OUT"
[ -f "$S/$SRC" ] || { echo "no such TE: $SRC"; exit 1; }
cp "$S/$SRC" "$S/!edit.tac"
c="674,748@10;212,16@16;115,111@22;437,16@28;610,749@46"
if [ "$MODE" = shot ]; then
    ( cd "$GAMEDATA" && FF_UI_CLICK="$c" FF_UI_SCREENSHOT=51 FF_UI_SHOT_DIR="$OUT" timeout -k 5 -s INT 54 "$BIN" -d "$GAMEDATA" -w > "$OUT/shot.log" 2>&1 )
    echo "shot $base -> $OUT/ui_0001.bmp"; exit 0
fi
# SQUAD=new (default): pick "New" in the Squadron list (485,253 -> 350,271) -- some training TEs' own squadron has
# sptype 255, which has no flight class, so OK silently does nothing (te_units.cpp tactical_make_flight).
if [ "${SQUAD:-new}" = new ]; then sq="485,253@57;350,271@58;"; else sq=""; fi
c="$c;150,${ROWY}@52r;185,$((ROWY + 49))@55;${sq}485,297@60;347,343@63;537,438@66;536,750@70;400,586@74;536,540@81"
rm -f "$S/$name.tac" "$S/$name.frc" "$S/$name.his"
( cd "$GAMEDATA" && FF_UI_CLICK="$c" FF_DEBUG_TEMAKE=1 FF_UI_TYPE="$name@76" FF_UI_SCREENSHOT=67 FF_UI_SHOT_DIR="$OUT" \
    timeout -k 5 -s INT 90 "$BIN" -d "$GAMEDATA" -w > "$OUT/make.log" 2>&1 )
printf "make %-32s saved=%s crash=%s flight=[%s]\n" "\"$name\"" "$([ -f "$S/$name.tac" ] && echo yes || echo NO)" \
    "$(grep -ac '=== CRASH' "$OUT/make.log")" "$(grep -a '^\[temake\] \(BuildMission\|no flight\)' "$OUT/make.log" | head -n1 | cut -c10-60)"

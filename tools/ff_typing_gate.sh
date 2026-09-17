#!/usr/bin/env bash
# MP-1 regression gate: can a human type into the FreeFalcon UI?
#
# WHY. The PO's report was "text cannot be typed into the UI at all". It resolved into FIVE
# defects across sprints 5-11 -- the scancode posted in the wrong parameter; GetKeyState stubbed,
# then its 0x80/0x8000 mask; BuildAscii() corrupting the ascii table on Linux; the modifier
# re-read at handle time instead of carried with the message; and DIK_Z colliding with
# VK_SNAPSHOT so a letter Z took a screenshot. Five defects in one path, and until this script
# there was NO automated check that any of them stayed fixed.
#
# It asserts the whole chain on one run, at the three levels S10 separated:
#   decode   -- ui95 receives the right Key/Ascii/ShiftStates  ([keys])
#   storage  -- the focused C_EditBox accumulates them exactly ([edit])
#   absence  -- typing does NOT trigger the screenshot path    ([keys] SNAPSHOT)
#
# The string "ZQ7x" is chosen, not arbitrary: Z is DIK 0x2C (the VK_SNAPSHOT collision) AND needs
# shift; Q needs shift; 7 is a digit; x is lowercase. A single pass covers every defect above.
#
# Headless and hermetic: it types into the logbook PILOT field and never clicks OK, so nothing is
# written to the player's install. (S11 did write, and backed up/restored to do it.)
#   FF_NO_SNAPFIX=1 bash tools/ff_typing_gate.sh   <- NEGATIVE CONTROL: restores the pre-S11
#       VK_SNAPSHOT comparison, so assertion 3 must go RED. A gate never seen to fail is not a gate.
set -u
ROOT=/home/admin/freefalcon-central
GAME="${FF_GAME_DIR:-/home/admin/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6}"
BIN="${FF_BIN:-$ROOT/build/src/ffviper/FFViper}"
OUT="${OUT:-/home/admin/ff-typing-gate}"
WORD="${WORD:-ZQ7x}"
mkdir -p "$OUT"; LOG="$OUT/run.log"

echo "MP-1 typing gate -- typing \"$WORD\" into the logbook PILOT field"
"$ROOT"/../bin/gl-lock 2>/dev/null >/dev/null || true
/home/admin/bin/gl-lock systemd-run --user --scope -p MemoryMax=10G bash -c \
  "cd '$GAME' && timeout -k 5 -s INT 75 env DISPLAY=:0 \
   FF_DEBUG_KEYS=1 FF_DEBUG_EDIT=1 ${FF_NO_SNAPFIX:+FF_NO_SNAPFIX=$FF_NO_SNAPFIX} \
   FF_UI_CLICK='150,750@8;75,97@14' FF_UI_TYPE='$WORD@20' \
   '$BIN' -d '$GAME' -w" > "$LOG" 2>&1

fail=0
say() { printf '  %-46s %s\n' "$1" "$2"; }

# 0. the instrument must have spoken at all -- a silent log is a harness failure, not a pass
if ! grep -aq '\[FF_UI_TYPE\] typing' "$LOG"; then
    echo "  CANNOT MEASURE: FF_UI_TYPE never fired -- see $LOG"; exit 2
fi
if ! grep -aq '^\[edit\]' "$LOG"; then
    echo "  CANNOT MEASURE: no edit box received anything (field never focused?) -- see $LOG"; exit 2
fi

# 1. decode: the capitals must arrive as capitals
if grep -aq "Ascii=90('Z') Shift=1" "$LOG"; then say "decode: Z arrives shifted" "OK"
else say "decode: Z arrives shifted" "FAIL"; fail=1; fi

# 2. storage: the field holds exactly what was typed
if grep -aq "text=\"$WORD\"" "$LOG"; then say "storage: field reads \"$WORD\"" "OK"
else say "storage: field reads \"$WORD\"" "FAIL"; fail=1
     grep -a '^\[edit\].*text=' "$LOG" | tail -1 | sed 's/^/      last: /'; fi

# 3. absence: typing must not trigger the screenshot path (DIK_Z vs VK_SNAPSHOT, S11)
if grep -aq 'SNAPSHOT trigger fired' "$LOG"; then
    say "no screenshot triggered by typing" "FAIL"; fail=1
    grep -a 'SNAPSHOT trigger fired' "$LOG" | head -2 | sed 's/^/      /'
else say "no screenshot triggered by typing" "OK"; fi

echo "----------------------------------------"
[ "$fail" -eq 0 ] && echo "PASS: typing works end to end" || echo "FAIL: see $LOG"
exit "$fail"

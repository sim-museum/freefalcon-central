#!/bin/bash
# FUNC-SWEEP-FF: press EVERY binding in config/keystrokes.key once during one Instant Action flight
# (tools/ff_keysweep_gen.py builds the list; flight-ending keys are held back) and report crash /
# assert / which key was the last to fire. Runs on the disposable data copy. Use under gl-lock.
#   DATA (default ~/ff-crawl)  OUT (default ~/ff-gates/keysweep)  START/STEP (s, default 25/0.6)
set -u
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
DATA="${DATA:-$HOME/ff-crawl}"; OUT="${OUT:-$HOME/ff-gates/keysweep}"; mkdir -p "$OUT"
BIN="${BIN:-$ROOT/build/src/ffviper/FFViper}"
python3 "$ROOT/tools/ff_keysweep_gen.py" "$DATA/config/keystrokes.key" "${START:-25}" "${STEP:-0.6}" "$OUT/keys.map" > "$OUT/keys.txt"
LAST=$(tail -n 1 "$OUT/keys.map" | cut -f1); SECS=$(python3 -c "print(int($LAST)+110)")
echo "events=$(tr ';' '\n' < "$OUT/keys.txt" | wc -l) bindings=$(wc -l < "$OUT/keys.map") last@${LAST}s timeout=${SECS}s"
( cd "$DATA" && FF_TEST_IA_EXIT_SEC=$((SECS - 20)) FF_SIM_KEY="@$OUT/keys.txt" timeout -k 5 -s KILL "$SECS" "$BIN" -d "$DATA" -w -test-ia > "$OUT/run.log" 2>&1 ); rc=$?
crash=$(grep -ac '=== CRASH' "$OUT/run.log"); asserts=$(grep -ac 'Assertion at' "$OUT/run.log")
downs=$(grep -ac '\[FF_SIM_KEY\] DOWN' "$OUT/run.log")
lastdown=$(grep -a '\[FF_SIM_KEY\] DOWN' "$OUT/run.log" | tail -n 1)
echo "rc=$rc crash=$crash asserts=$asserts key-downs=$downs"
echo "last: $lastdown"
if [ -n "$lastdown" ]; then
  ms=$(sed -E 's/.* at ([0-9]+)ms/\1/' <<<"$lastdown")
  awk -F'\t' -v t="$ms" '$1*1000 <= t+1 {l=$0} END {print "last binding fired: " l}' "$OUT/keys.map"
fi
grep -a 'Assertion at' "$OUT/run.log" | sort | uniq -c | sort -rn | head -n 8

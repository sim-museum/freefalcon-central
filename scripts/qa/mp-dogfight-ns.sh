#!/bin/bash
# usage: OUT=<dir> [A_ENV=..] [B_ENV=..] run.sh  -- two-namespace FF dogfight (see inner.sh)
set -u; OUT=${OUT:?}; mkdir -p "$OUT/a" "$OUT/b"; export OUT
unshare -rn --kill-child bash "$(dirname "$0")/mp-dogfight-ns-inner.sh"
for s in a b; do printf "%s: crash=%s joined=%s 3d=%s\n" $s "$(grep -ac '=== CRASH' $OUT/$s.log)" \
  "$(grep -ac 'FM_JOIN_SUCCEEDED' $OUT/$s.log)" "$(grep -ac 'FM_START_DOGFIGHT' $OUT/$s.log)"; done

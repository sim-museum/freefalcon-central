#!/bin/bash
# AVIONICS-1 full sweep: drive EVERY drivable binding in config/keystrokes.key,
# including the modified ones FF_SIM_KEY could not reach before 81ea0a08.
# Modifier bitmask -> prefix: 1=S 2=C 4=A 3=SC 5=SA 6=CA 7=SCA.
# Terminal/destructive bindings (exit/quit/eject/abort/drop-all/nuclear) are
# excluded -- they end the run and produce noise, not coverage.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export DISPLAY=:0
GD="$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
BIN=$REPO/build-relg/src/ffviper/FFViper
K="$GD/config/keystrokes.key"
ROW=${ROW:-25}
y=$(( 94 + ROW * 17 ))
PER=${PER:-7}
pgrep -f mutter-x11-frames >/dev/null || { setsid /usr/libexec/mutter-x11-frames >/dev/null 2>&1 & sleep 1; }

awk '$1 ~ /^Sim/ && $4 ~ /^0[Xx]/ {
  m=$5; p="";
  if (m==1) p="S"; else if (m==2) p="C"; else if (m==4) p="A";
  else if (m==3) p="SC"; else if (m==5) p="SA"; else if (m==6) p="CA";
  else if (m==7) p="SCA"; else if (m!=0) next;
  print p $4, $1
}' "$K" | grep -viE "exit|quit|eject|abort|end.?flight|SimDropAll|Nuclear" > /tmp/allkeys.txt

total=$(wc -l < /tmp/allkeys.txt)
echo "row $ROW: $total bindings, $PER per batch"
b=0
while read -r spec name rest; do
  batch[$((b % PER))]="$spec"
  names[$((b % PER))]="$name"
  b=$((b+1))
  if [ $((b % PER)) -eq 0 ]; then
    keys=""; t=60
    for k in "${batch[@]}"; do keys="$keys$k@$t;"; t=$((t+5)); done
    log=/tmp/afs-$((b/PER)).log
    ( export FF_UI_CLICK="574,750@12;225,171@18;896,743@24;677,748@36;140,${y}@42;824,750@48;973,750@54"
      export FF_AP_MODE=1 FF_DEBUG_OTWTHREAD=1 FF_SIM_KEY="${keys%;}"
      timeout -s INT 150 "$BIN" -d "$GD" -w > "$log" 2>&1 ) 2>/dev/null
    c=$(grep -ac 'CRASH: SIGSEGV\|Segmentation fault\|Aborted' "$log")
    a=$(grep -ac 'Assertion at' "$log")
    printf 'batch %-3s crash=%-2s asserts=%-2s %s  [%s]\n' "$((b/PER))" "$c" "$a" \
      "$(grep -a 'Assertion at' "$log" | sed 's/.*  \([^ ]*\.cpp\).*/\1/' | sort -u | tr '\n' ' ')" \
      "${names[*]}"
  fi
done < /tmp/allkeys.txt
echo "=== FULL AVIONICS SWEEP COMPLETE ==="

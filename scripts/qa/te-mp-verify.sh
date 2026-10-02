#!/bin/bash
# FF-TEMP-1 acceptance: every "* MP.tac" in ~/ff-crawl/campaign/SAVE hosted by peer A and joined + flown by peer B
# (te-mp-two.sh). The SAVED tab lists *.tac (except te_new) case-insensitively sorted, rows y = 111 + 17*idx, about
# 33 rows visible; rows past that are reported "not visible" instead of being clicked blind. Run under ~/bin/gl-lock.
set -u
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
S=${GAMEDATA:-$HOME/ff-crawl}/campaign/SAVE; OUTB=${OUTB:-$HOME/ff-gates/temp1/verify}; mkdir -p "$OUTB"
mapfile -t list < <(cd "$S" && printf '%s\n' *.tac | grep -av '^te_new\.tac$' | sed 's/\.tac$//' | LC_ALL=C sort -f)
for i in "${!list[@]}"; do
    n=${list[$i]}
    case "$n" in *" MP") ;; *) continue ;; esac
    [ -n "${ONLY:-}" ] && ! echo "$n" | grep -aq -- "$ONLY" && continue
    if [ "$i" -gt 32 ]; then printf "%-28s row %d not visible -- skipped\n" "$n" "$i"; continue; fi
    o="$OUTB/$(echo "$n" | tr -c 'A-Za-z0-9\n' '_')"
    OUT="$o" TE_ROW=$((111 + 17*i)) A_ROW=98 TE_JROW=85 A_SECS=${A_SECS:-340} B_SECS=${B_SECS:-260} \
        "$REPO/scripts/qa/te-mp-two.sh" > /dev/null 2>&1
    te=$(grep -a 'StartReadCampFile: type' "$o/a.log" | head -1 | sed "s/.*filename='//;s/'.*//")
    fa=$(grep -a '^\[playerflt\] session .* local=1 -> flight' "$o/a.log" | tail -n1 | sed 's/.*flight //')
    fb=$(grep -a '^\[playerflt\] session .* local=1 -> flight' "$o/b.log" | tail -n1 | sed 's/.*flight //')
    esc=$(grep -a '^\[teinv\] flt' "$o/a.log" | grep -ac 'ac=3 type=.* mission=3 ')
    printf "%-28s escort=%s host3D=%s join3D=%s joined=%s crash=%s/%s hosted='%s' host[%s] joiner[%s]\n" "$n" "$esc" \
        "$(grep -ac 'RenderFirstFrame\] Exit' "$o/a.log")" "$(grep -ac 'RenderFirstFrame\] Exit' "$o/b.log")" \
        "$(grep -ac 'FM_JOIN_SUCCEEDED' "$o/b.log")" "$(grep -ac '=== CRASH' "$o/a.log")" "$(grep -ac '=== CRASH' "$o/b.log")" \
        "$te" "$fa" "$fb"
done

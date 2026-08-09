#!/usr/bin/env bash
# Extract a 1024x768 game-client frame from the Wine gold VIDEOS.
#
# The 2026-08-08 gold set is screen-recorded at 1920x1080/60fps with FreeFalcon
# running WINDOWED at 1024x768 ("FreeFalcon - Wine desktop"). That is the same
# resolution our own captures use, so frames crop out pixel-for-pixel with no
# rescaling -- unlike the older PNG golds, whose 1011x771 client area forced a
# resize and made every size-based verdict unsafe.
#
# The window sits at a DIFFERENT desktop offset in each recording, so the crop
# origin is per-video and recorded below. Re-derive with --probe if a new
# recording is added.
#
# Usage:
#   tools/gold_video.sh <clip> <timestamp> <out.png>
#   tools/gold_video.sh --probe <file.mp4>          # find the client origin
#   tools/gold_video.sh --list
#
#   <clip> is one of: views | landing | ia
#   <timestamp> is anything ffmpeg -ss accepts (e.g. 585 or 00:09:45)
#
# Example:
#   tools/gold_video.sh ia 585 /tmp/gold_pit_sunup.png
set -uo pipefail

GOLD_DIR="${FF_GOLD_VIDEO_DIR:-$HOME/gold standard/free falcon/260808}"

# clip -> file : client_x : client_y   (client is always 1024x768)
clip_spec() {
    case "$1" in
        views)   echo "260808_different_viwes.mp4:100:114" ;;
        landing) echo "260808_landing_final_approach.mp4:100:114" ;;
        ia)      echo "260808_instant_action_5am_ending_at_16x_speed.mp4:233:226" ;;
        *)       return 1 ;;
    esac
}

if [ "${1:-}" = "--list" ]; then
    printf '%-8s %-58s %s\n' CLIP FILE "CLIENT ORIGIN"
    for c in views landing ia; do
        s=$(clip_spec "$c"); f=${s%%:*}; xy=${s#*:}
        printf '%-8s %-58s (%s)\n' "$c" "$f" "${xy/:/, }"
    done
    cat <<'EOF'

Contents (2026-08-08 session, FreeFalcon 6.0 / FFViper 2.3.3.44):
  views    1:33  view-mode cycling
  landing  3:44  TE "09 Landing Final Approach" -- the RWY-2 acceptance flight
  ia      10:25  Instant Action from 05:04 dawn, ending at 16x time accel with
                 a visible sunrise. Timeline: 0-6s main menu, 9-27s IA setup,
                 33s LOADING screen (the blue blueprint/cobra art that the
                 still golds 1 and 5 mislabelled as the main menu), 36s+ 2D pit
                 at dawn, ~520-550s HUD view, 560-595s pit with the sun up,
                 610s "prepare to debrief".
EOF
    exit 0
fi

if [ "${1:-}" = "--probe" ]; then
    [ $# -ge 2 ] || { echo "usage: $0 --probe <file.mp4> [timestamp]"; exit 2; }
    src="$2"; at="${3:-30}"
    tmp=$(mktemp --suffix=.png)
    ffmpeg -v error -ss "$at" -i "$src" -frames:v 1 -y "$tmp" || exit 1
    python3 - "$tmp" <<'PY'
import sys, numpy as np
from PIL import Image
a=np.asarray(Image.open(sys.argv[1]).convert('RGB'),dtype=int)
light=(a.min(2)>200); runs=light.sum(1)
cand=np.where(runs>600)[0]
if len(cand)==0:
    print("no title bar found -- pass a timestamp where the window is visible"); raise SystemExit(1)
yt=cand[0]; xs=np.where(light[yt])[0]
ybot=cand[cand<yt+80].max()
# the rounded title bar is ~9px inset from the client edges
print(f"title bar y{yt}..{ybot} x{xs.min()}..{xs.max()}")
print(f"=> client origin approx ({xs.min()-9}, {ybot+1}), size 1024x768")
PY
    rm -f "$tmp"; exit 0
fi

[ $# -eq 3 ] || { echo "usage: $0 <views|landing|ia> <timestamp> <out.png>   (see --list)"; exit 2; }
spec=$(clip_spec "$1") || { echo "unknown clip '$1' (see --list)"; exit 2; }
file=${spec%%:*}; rest=${spec#*:}; cx=${rest%%:*}; cy=${rest##*:}
src="$GOLD_DIR/$file"
[ -f "$src" ] || { echo "missing gold video: $src"; exit 1; }

ffmpeg -v error -ss "$2" -i "$src" -frames:v 1 \
       -vf "crop=1024:768:$cx:$cy" -y "$3" || exit 1
echo "$1 @ $2 -> $3  (1024x768 client, crop origin $cx,$cy)"

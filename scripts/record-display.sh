#!/usr/bin/env bash
set -euo pipefail

OUTDIR="$HOME/Videos"
FPS=60
CONTAINER=mp4
DEFAULT_MONITOR=2   # previous fixed behaviour: the HDMI monitor

# gpu-screen-recorder prints one "NAME|WIDTHxHEIGHT" line per connected monitor.
mapfile -t MONITORS < <(gpu-screen-recorder --list-monitors)
if (( ${#MONITORS[@]} == 0 )); then
    echo "Error: gpu-screen-recorder reported no monitors." >&2
    exit 1
fi

list_monitors() {
    local i
    for i in "${!MONITORS[@]}"; do
        printf '  %d  %-12s %s\n' "$((i + 1))" "${MONITORS[i]%%|*}" "${MONITORS[i]#*|}"
    done
}

usage() {
    cat <<EOF
Usage: ${0##*/} [monitor]

Record one monitor of the dual-monitor display with gpu-screen-recorder.

  monitor       Which monitor to record: an index (1, 2, ...) or a monitor
                name such as DP-1. Defaults to $DEFAULT_MONITOR.

  -l, --list    List the connected monitors and exit.
  -h, --help    Show this help and exit.

Connected monitors:
$(list_monitors)
EOF
}

if (( $# > 1 )); then
    echo "Error: expected at most one argument, got $#." >&2
    usage >&2
    exit 1
fi

ARG="${1:-$DEFAULT_MONITOR}"
case "$ARG" in
    -h|--help) usage; exit 0 ;;
    -l|--list) list_monitors; exit 0 ;;
esac

INDEX=0
if [[ "$ARG" =~ ^[0-9]+$ ]]; then
    if (( ARG < 1 || ARG > ${#MONITORS[@]} )); then
        echo "Error: no monitor $ARG; ${#MONITORS[@]} monitor(s) connected." >&2
        list_monitors >&2
        exit 1
    fi
    INDEX="$ARG"
else
    for i in "${!MONITORS[@]}"; do
        if [[ "${MONITORS[i]%%|*}" == "$ARG" ]]; then
            INDEX=$((i + 1))
            break
        fi
    done
    if (( INDEX == 0 )); then
        echo "Error: no monitor named '$ARG'." >&2
        list_monitors >&2
        exit 1
    fi
fi

ENTRY="${MONITORS[INDEX - 1]}"
MONITOR="${ENTRY%%|*}"
RESOLUTION="${ENTRY#*|}"

STAMP="$(date +%Y%m%d-%H%M%S)"
OUTFILE="$OUTDIR/display$INDEX-$STAMP.$CONTAINER"

mkdir -p "$OUTDIR"

echo "Recording monitor: $MONITOR ($RESOLUTION, monitor $INDEX of ${#MONITORS[@]})"
echo "Saving to: $OUTFILE"
echo "Press Ctrl+C to stop and save gracefully."

gpu-screen-recorder \
  -w "$MONITOR" \
  -f "$FPS" \
  -c "$CONTAINER" \
  -o "$OUTFILE"

#!/usr/bin/env bash
# PO crash hunt: FreeFalcon, dev build, run under gdb so a crash yields a SYMBOL backtrace.
# Interactive -- the PO drives it. No harness env, so po_idle.sh treats the display as busy.
set -u
R=/home/admin/freefalcon-central
BIN=$R/build/src/ffviper/FFViper
GD=$HOME/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6
OUT=$HOME/ff-gates/po_crash
STAMP=$(date +%y%m%d-%H%M%S)
LOG=$OUT/ff_$STAMP.log
ln -sfn "$LOG" "$OUT/latest.log"
ulimit -c unlimited
echo "=== FreeFalcon (dev build $(stat -c %y $BIN | cut -c1-16)) under gdb, started $(date +%H:%M:%S)" | tee -a "$LOG"
echo "=== git $(cd $R && git rev-parse --short HEAD) + uncommitted: $(cd $R && git status --short | tr '\n' ' ')" | tee -a "$LOG"
cd "$GD" || exit 1
gdb -batch \
    -ex 'set pagination off' -ex 'set confirm off' \
    -ex 'handle SIGPIPE nostop noprint pass' \
    -ex 'handle SIGUSR1 nostop noprint pass' -ex 'handle SIGUSR2 nostop noprint pass' \
    -ex 'run' \
    -ex 'echo \n======== CRASH: gdb caught a fatal signal ========\n' \
    -ex 'bt full' \
    -ex 'echo \n======== all threads ========\n' \
    -ex 'thread apply all bt' \
    -ex 'echo \n======== registers ========\n' \
    -ex 'info registers' \
    -ex 'echo \n======== faulting source ========\n' \
    -ex 'list' \
    --args "$BIN" -d "$GD" -w >> "$LOG" 2>&1
rc=$?
echo "=== exited rc=$rc at $(date +%H:%M:%S)" >> "$LOG"
if grep -aq 'CRASH: gdb caught' "$LOG"; then echo "CRASHED -- backtrace captured in $LOG"; else echo "exited without a fatal signal (rc=$rc)"; fi

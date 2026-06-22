#!/bin/bash
# Sprint 2/4 — ASAN soak of CAMPAIGN mode (not covered by -test-ia).
# Drives Main menu -> Campaign -> COMMIT -> START CAMPAIGN via FF_UI_CLICK, then
# lets the strategic sim run under ASAN to surface campaign-mode heap bugs and the
# intermittent AS_DataClass::ASSearch (ground-unit A*) crash. Timings stretched for ASAN.
set -u
export DISPLAY=:0
GAMEDATA="/home/g/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"
BIN="/home/g/ff/build-asan/src/ffviper/FFViper"
LOG=/tmp/ff-asan-campaign.log
T="${1:-220}"
pgrep -f mutter-x11-frames >/dev/null || { setsid /usr/libexec/mutter-x11-frames >/dev/null 2>&1 & sleep 1; }
cd "$GAMEDATA" || { echo "no game data"; exit 1; }
export ASAN_OPTIONS="halt_on_error=0:detect_leaks=0:abort_on_error=0"
# UI coords are 1024x768. Navigate into a running campaign (ASAN-stretched timings):
#   Campaign btn -> COMMIT -> mission-priorities OK -> START CAMPAIGN (center map button)
export FF_UI_CLICK="924,745@12;905,758@20;563,751@26;495,390@34"
echo "=== ASAN CAMPAIGN soak: timeout ${T}s ===" > "$LOG"
timeout -s INT "$T" "$BIN" -d "$GAMEDATA" -w >> "$LOG" 2>&1
echo "=== exited rc=$? ===" >> "$LOG"
echo "--- ASAN summaries ---"
grep -E "SUMMARY: AddressSanitizer|ASSearch|SEGV|runtime error:" "$LOG" | sort | uniq -c | head -40
echo "--- did we reach campaign? ---"
grep -cE "InitCampaign|LoadCampaign|CampaignClass|START_CAMPAIGN|HandleCampaignThread" "$LOG"
echo "--- tail ---"; tail -6 "$LOG"

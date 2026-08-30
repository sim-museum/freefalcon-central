#!/usr/bin/env python3
"""Audit every theater's stations.dat for TACAN channels outside 1..126.

tacan.cpp:168 asserts channel > 0 and channel <= NUM_CHANNELS (126) and then --
because F4Assert is live but NON-HALTING in this build -- stores the value anyway
via StoreStation(). So a bad row is reported once and then silently kept.

VALIDATED AGAINST A KNOWN POSITIVE: this catches Balkans line 170 (channel 130),
which is the row that trips the assertion in a Balkans campaign load. A scanner
that has never caught anything it must catch is worthless, so if this stops
reporting Balkans, the scanner is broken, not the data.

Format: stationId channel band ... (channel is the second whitespace field).
"""
import os, sys

GD = os.path.expanduser(os.environ.get(
    "FF_GAMEDATA", "~/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6"))
NUM_CHANNELS = 126

def scan(path):
    bad, rows = [], 0
    with open(path, errors="replace") as fh:
        for lineno, line in enumerate(fh, 1):
            t = line.strip()
            if not t or t.startswith("#"):
                continue
            f = t.split()
            if len(f) < 3:
                continue
            try:
                ch = int(f[1])
            except ValueError:
                continue
            rows += 1
            if not (0 < ch <= NUM_CHANNELS):
                bad.append((lineno, ch, t[:60]))
    return rows, bad

def main():
    found = []
    for root, _dirs, files in os.walk(GD):
        for fn in files:
            if fn.lower() == "stations.dat":
                found.append(os.path.join(root, fn))
    if not found:
        print(f"no stations.dat under {GD}", file=sys.stderr)
        return 2
    total_bad = 0
    for p in sorted(found):
        rows, bad = scan(p)
        rel = p.replace(GD + "/", "")
        print(f"{rel}: {rows} stations, {len(bad)} out of range")
        for lineno, ch, txt in bad:
            print(f"    line {lineno}: channel={ch}   {txt}")
        total_bad += len(bad)
    print(f"=== {len(found)} files scanned, {total_bad} bad channels ===")
    if total_bad == 0:
        print("WARNING: zero findings. Balkans is a known positive -- "
              "if it is present and clean, suspect the scanner.", file=sys.stderr)
    return 0

if __name__ == "__main__":
    sys.exit(main())

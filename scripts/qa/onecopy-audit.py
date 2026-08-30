#!/usr/bin/env python3
"""Find FF_LINUX fixes that were applied to only ONE of several identical sites.

This shape has bitten twice, both times found by hand:

  BOOM-3   ground-impact particles are placed at exactly GroundLevel in FOUR
           places across two parallel emitter implementations; the lift reached
           one of them.
  MAV-2    objectiv.cpp has two feature-creation functions that both set
           simdata.z = 0; only Deaggregate() got the terrain placement, while
           DeaggregateFromData() -- the path a joining peer takes -- kept
           burying every feature at sea level.

Both were near-duplicate code where a fix landed in one copy. The heuristic:
take the line immediately preceding each `#ifdef FF_LINUX`, treat it as the
"anchor" the fix attaches to, and report any OTHER occurrence of that same
anchor line in the tree that is NOT followed by an FF_LINUX block.

Deliberately crude. It is a lead generator, not a prover -- an anchor that
appears in unrelated contexts will produce noise, and every hit needs reading.

HONEST TRACK RECORD, so nobody over-trusts it: as of 2026-08-30 this tool has
found ZERO defects on its own. Both instances of the pattern it exists for
(BOOM-3's four emitter sites, objectiv.cpp's two deaggregation paths) were found
by hand first; the tool reproduces the objectiv.cpp one when pointed at the
pre-fix file, which is how it is validated, but it did not discover it.

Of the 94 anchors it currently reports, the ones triaged were all false
positives of one shape: BOILERPLATE anchors shared by sibling functions that
legitimately differ. dxengine.cpp pairs the solid-stack drain with the alpha-stack
drain because both open with `ObjectInstance *LastObj = NULL; float LastFog = 0;`,
and the FF_LINUX difference is a diagnostic counter. That is not filterable by
another regex -- distinguishing "same code, fix missing" from "different code,
same boilerplate" needs a human.

Use it as a place to start looking, not as a list of defects.

Reads latin-1: ~30 files in this tree hold non-UTF-8 bytes (see CLAUDE.md), and
an audit that skips them is not an audit.
"""
import os, re, sys
from collections import defaultdict

ROOT = sys.argv[1] if len(sys.argv) > 1 else "/home/g/ff/src"

# Anchors this weak are meaningless -- braces, blank-ish lines, common keywords.
TRIVIAL = re.compile(r'^\s*(?:[{}]|else|break|return;?|continue;?|\*/|/\*|//|#|$)')
MIN_LEN = 18


def norm(line):
    """Collapse whitespace so indentation differences do not hide a match."""
    return re.sub(r'\s+', ' ', line.strip())


anchored = defaultdict(list)   # anchor -> [(path, lineno)] where a fix follows
plain = defaultdict(list)      # anchor -> [(path, lineno)] where none does

for dirpath, dirnames, filenames in os.walk(ROOT):
    # camptool duplicates camptask wholesale as an offline tool; tools/ likewise.
    # Their divergence from the shipping copy is expected, not a missing fix.
    dirnames[:] = [d for d in dirnames
                   if d not in ("extern", ".git", "camptool", "tools", "bspbuild")]

    for fn in filenames:
        if not fn.endswith((".cpp", ".c", ".h")):
            continue

        p = os.path.join(dirpath, fn)

        if os.path.islink(p):
            continue

        try:
            with open(p, encoding="latin-1") as f:
                lines = f.readlines()
        except OSError:
            continue

        for i, line in enumerate(lines):
            s = norm(line)

            if TRIVIAL.match(line) or len(s) < MIN_LEN:
                continue

            # does an FF_LINUX block open within the next 2 lines?
            nxt = "".join(lines[i + 1:i + 3])
            rel = os.path.relpath(p, "/home/g/ff")

            if "#ifdef FF_LINUX" in nxt:
                # A block that only traces is not a fix. This tree is full of
                # `#ifdef FF_LINUX fprintf(...)` debug blocks, and counting them
                # made unit.cpp's Deaggregate/DeaggregateFromData pair look like a
                # missing fix when the only difference is a [Deaggregate] printf.
                body = []
                for k in range(i + 1, min(len(lines), i + 16)):
                    t = lines[k].strip()

                    if t.startswith("#endif"):
                        break

                    if (not t or t.startswith(("//", "*", "/*", "#"))):
                        continue

                    body.append(t)

                meaningful = [t for t in body
                              if not t.startswith(("fprintf", "fflush", "MonoPrint"))]

                if not meaningful:
                    continue

                anchored[s].append((rel, i + 1))
            else:
                # If the "unfixed" site already sits inside an FF_LINUX region,
                # it is not a missing fix -- most often it IS the fix, reported
                # against itself.
                near = "".join(lines[max(0, i - 3):i + 4])

                if "FF_LINUX" in near:
                    continue

                plain[s].append((rel, i + 1))

hits = []

for anchor, fixed in anchored.items():
    missing = plain.get(anchor)

    if not missing:
        continue

    # An anchor that appears everywhere is a common idiom, not a duplicated site.
    if len(missing) > 4:
        continue

    hits.append((len(missing), anchor, fixed, missing))

hits.sort(key=lambda h: h[0])

print("--- %d anchors fixed in one place but present unfixed elsewhere ---"
      % len(hits))

for n, anchor, fixed, missing in hits:
    print("\n  anchor: %s" % anchor[:100])
    for path, ln in fixed:
        print("    FIXED   %s:%d" % (path, ln))
    for path, ln in missing:
        print("    unfixed %s:%d" % (path, ln))

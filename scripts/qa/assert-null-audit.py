#!/usr/bin/env python3
"""
ASSERTNULL-1: find functions that ASSERT a lookup succeeded and then RETURN the
possibly-NULL result anyway.

CPBTN-1 is the pattern:

    CPButtonObject* CockpitManager::GetButtonPointer(int buttonId) {
        BOOL found = FALSE; CPButtonObject* preturnValue = NULL;
        ... search ...
        F4Assert(found);          // live but NON-HALTING in this build
        return(preturnValue);     // NULL when not found
    }

ShiAssert/F4Assert do not halt here, so the assertion is a log line and the NULL
escapes to callers. GUARD-1 (#79) swept vuDatabase->Find() call sites; this is the
same family one level up -- the PRODUCER asserts and returns NULL anyway.

Validate against the known positive before believing any zero:
    python3 scripts/qa/assert-null-audit.py | grep cpmanager

RESULT 2026-08-29: 4 candidates, 1 real. Triage is cheap because the set is tiny,
but do not skip it -- 3 of 4 are false positives with distinct shapes:

  cpmanager.cpp:4744  GetButtonPointer  REAL -> CPBTN-1 (#91). Asserts `found`,
                      returns a NULL-initialised pointer, and two consumers
                      (SimNextAAWeapon/SimNextAGWeapon) dereference it unguarded.

  f4vu.cpp:373        VuxType           FALSE. Asserts the range and then
                      RE-CHECKS the same range in an if before assigning. That is
                      the CORRECT pattern -- assertion plus guard.
  laserpod.cpp:568    FindLaserPod      FALSE. Asserts its ARGUMENT, then guards
                      `if (not theObject) return retval;`. Returning NULL for
                      "no pod fitted" is a legitimate optional result, and all
                      four call sites check it (three `if (laserPod)`, one
                      short-circuit `laserPod and laserPod->IsSOI()`).
  objectparent.cpp:547 ChooseLOD        FALSE. The assertion concerns a loop
                      variable, not the returned value.

The producer returning NULL is not itself the bug -- an optional result is fine.
The bug is a consumer that does not check. So a hit here is only actionable after
its call sites are read.
"""
import os, re, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'src'))

# a function whose return type is a pointer
FUNC = re.compile(r'^[A-Za-z_][\w:<>,\s]*\*\s*([A-Za-z_]\w*::)?([A-Za-z_]\w*)\s*\([^;{]*\)\s*\{', re.M)
ASSERT = re.compile(r'\b(ShiAssert|F4Assert|assert)\s*\(([^;]*)\)\s*;')
RET = re.compile(r'\breturn\s*\(?\s*([A-Za-z_]\w*)\s*\)?\s*;')


def body_of(src, start):
    """Return the text of the function body beginning at the brace at `start`."""
    depth, i = 0, start
    while i < len(src):
        if src[i] == '{':
            depth += 1
        elif src[i] == '}':
            depth -= 1
            if depth == 0:
                return src[start:i]
        i += 1
    return src[start:start + 4000]


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else ROOT
    findings = 0
    for dirpath, _, names in os.walk(root):
        for n in names:
            if not n.endswith(('.cpp', '.c')):
                continue
            path = os.path.join(dirpath, n)
            try:
                src = open(path, errors='ignore').read()
            except OSError:
                continue
            if 'Assert' not in src:
                continue
            for m in FUNC.finditer(src):
                body = body_of(src, src.index('{', m.end() - 1))
                for am in ASSERT.finditer(body):
                    tail = body[am.end(): am.end() + 220]
                    rm = RET.search(tail)
                    if not rm:
                        continue
                    var = rm.group(1)
                    # the returned variable must be initialised to NULL somewhere
                    # in the body -- that is what makes the escape possible
                    if not re.search(r'\b' + re.escape(var) +
                                     r'\s*=\s*(NULL|0|nullptr)\s*;', body):
                        continue
                    # and the assert must not itself be the null check on it
                    if re.search(r'\b' + re.escape(var) + r'\b', am.group(2)):
                        continue
                    line = src[:m.start()].count('\n') + body[:am.start()].count('\n') + 1
                    print(f'{os.path.relpath(path, root)}:{line}: '
                          f'{m.group(2)}() asserts ({am.group(2).strip()[:44]}) '
                          f'then returns `{var}`, which is NULL-initialised')
                    findings += 1
                    break
    print(f'\n=== {findings} site(s) ===')


if __name__ == '__main__':
    sys.exit(main())

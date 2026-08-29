#!/usr/bin/env python3
"""
SAVESIZE-1: find Save()/SaveSize() pairs whose field lists disagree.

VUADDR-1 (b7ade5e7) overran a send buffer because VuSessionEntity::LocalSize
counted sizeof(address_) = 8 while Save() called address_.Encode(), which wrote 12.
The buffer is allocated from SaveSize(), so any field Save writes that SaveSize
does not count is a heap overflow waiting for the right data -- ASAN caught this
one only because multiplayer was finally run under it.

Compares the multiset of sizeof(...) terms on each side. A term present in Save but
absent from SaveSize is the dangerous direction (writes more than budgeted); the
reverse merely wastes space.

TRIAGE 2026-08-29: 9 candidates, 0 defects.
  EMPIRICAL: ASAN now runs clean across every path these classes live on --
  two-peer multiplayer (session/game/group entities), bombing and CCRP, guns,
  HARMs, the Israeli theater, avionics keys and 3D-pit clicks. No overflow
  manifests anywhere covered.
  STATIC spot-check, BombClass: SaveSize counts sizeof(BombType) while Save writes
  sizeof(int). BombType is `enum BombType { None, Chaff, Flare, Debris }` and the
  member is declared `int bombType` -- both 4 bytes. Same width, different
  spelling. FALSE POSITIVE.

PRECISION IS LOW IN THIS CODEBASE and the reason is structural: the same width is
routinely spelled two ways (enum vs int, uint32_t vs a variable name, typedef vs
underlying type), and this tool compares SPELLINGS. Its genuine value is narrow but
real -- it catches a field written through a HELPER CALL, which contributes no
sizeof to the Save side at all. That is exactly how VUADDR-1 presented, and it is
the one shape worth acting on. Read the "counted but not written as sizeof" column
first.

KNOWN LIMITS, so a hit is read rather than believed:
  - a field written via a helper call (address_.Encode) has no sizeof on the Save
    side at all, so it shows as SaveSize-only -- that is how VUADDR-1 presents, and
    it is a hit worth reading, not a false positive
  - Save often advances the stream with a second sizeof per field; multiset
    comparison tolerates that only if SaveSize repeats it too, so counts are
    reported rather than compared strictly
  - base-class Save/SaveSize pairs are compared independently of derived ones
"""
import os, re, sys, collections

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'src'))
# Save has TWO overloads in this codebase: Save(VU_BYTE**) writes the network
# stream sized by SaveSize, and Save(FILE*) writes a file. Comparing the FILE
# overload against SaveSize is apples-to-oranges -- it produced a bogus
# FalconGameEntity hit on the first run. Match only the stream overload.
FUNC = re.compile(r'^[A-Za-z_][\w:<>\*\s&]*?\b([A-Za-z_]\w*)::(SaveSize|LocalSize)\s*\(\s*(?:void)?\s*\)\s*\{'
                  r'|^[A-Za-z_][\w:<>\*\s&]*?\b([A-Za-z_]\w*)::(Save)\s*\(\s*(?:VU_BYTE|unsigned char)\s*\*\*[^;{)]*\)\s*\{', re.M)
SIZEOF = re.compile(r'sizeof\s*\(\s*([^)]+?)\s*\)')


def strip_comments(t):
    t = re.sub(r'/\*.*?\*/', ' ', t, flags=re.S)
    return re.sub(r'//[^\n]*', ' ', t)


def body(src, brace):
    d, i = 0, brace
    while i < len(src):
        if src[i] == '{':
            d += 1
        elif src[i] == '}':
            d -= 1
            if d == 0:
                return src[brace:i]
        i += 1
    return src[brace:brace + 8000]


def terms(b):
    return collections.Counter(m.group(1).strip() for m in SIZEOF.finditer(strip_comments(b)))


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else ROOT
    found = collections.defaultdict(dict)
    for dp, _, fs in os.walk(root):
        for f in fs:
            if not f.endswith(('.cpp', '.c')):
                continue
            p = os.path.join(dp, f)
            try:
                src = open(p, errors='ignore').read()
            except OSError:
                continue
            for m in FUNC.finditer(src):
                cls = m.group(1) or m.group(3)
                kind = m.group(2) or m.group(4)
                b = src.index('{', m.end() - 1)
                found[(p, cls)][kind] = (terms(body(src, b)), src[:m.start()].count('\n') + 1)

    hits = 0
    for (p, cls), k in sorted(found.items()):
        # SaveSize commonly DELEGATES -- e.g. VuSessionEntity::SaveSize is just
        # VuTargetEntity::SaveSize() + LocalSize(), so on its own it contains almost
        # no sizeof terms and every field looks uncounted. Union whichever size
        # functions the class defines instead of picking one.
        names = [n for n in ('SaveSize', 'LocalSize') if n in k]
        if 'Save' not in k or not names:
            continue
        sizefn = '+'.join(names)
        lv = collections.Counter()
        for n in names:
            lv.update(k[n][0])
        sv = k['Save'][0]
        sizeline = k[names[0]][1]
        only_save = {t: c for t, c in sv.items() if t not in lv}
        only_size = {t: c for t, c in lv.items() if t not in sv}
        if only_save or only_size:
            print(f'{os.path.relpath(p, root)}: {cls}::Save (line {k["Save"][1]}) vs '
                  f'{cls}::{sizefn} (line {sizeline})')
            if only_save:
                print(f'    written but NOT counted (overflow risk): {sorted(only_save)}')
            if only_size:
                print(f'    counted but not written as sizeof: {sorted(only_size)}')
            hits += 1
    print(f'\n=== {hits} pair(s) with a field-list difference ===')


if __name__ == '__main__':
    sys.exit(main())

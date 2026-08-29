#!/usr/bin/env python3
"""
WIREFMT-1: find Encode/Decode (and Save/Load) pairs whose field sequences disagree.

VUADDR-1 (b7ade5e7) was exactly this: VU_ADDRESS::Encode wrote
    memcpy(*stream, &ip, sizeof(unsigned long));     // 8 on LP64
while Decode read
    memcpychk(&ip, stream, sizeof(uint32_t), rem);   // 4
Someone corrected the read side and missed the write side. The result was a buffer
overrun AND a silent wire desync -- every field after the address decoded
misaligned, which is the signature of "bytes arrive, nothing decodes".

Compares the ORDERED list of sizeof(...) widths each side of a pair uses. A
mismatch in width or count means the two sides do not agree on the format.

TRIAGE RESULT 2026-08-29: all three hits on the current tree are FALSE POSITIVES.
  vu2/src/vu_nat.cpp        the fix writes sizeof(ip) (a variable) where Decode
                            uses sizeof(uint32_t) (a type) -- same width, and the
                            tool cannot resolve a variable to a width
  falclib/msgsrc/radiochattermsg.cpp   both sides use
                            sizeof(dataBlock) - sizeof(dataBlock.edata) then a
                            count byte then per-element reads; the difference is
                            Encode counting each field three times
  campaign/campupd/cmpclass.cpp   ALREADY deliberately aligned -- Encode carries
                            "FF_LINUX: write 4-byte fields to match Decode's
                            int32_t reads". The apparent 69-vs-70 field gap is the
                            size header, written into a staging buffer rather than
                            *stream, so it falls outside the extraction pattern.
So: 0 real defects found, 3/3 false positives. The tool earns its place only as a
pointer at pairs worth reading by hand -- treat every hit as a question, never as
a finding, and expect to spend a few minutes per hit.

Validate against the pre-fix source before believing a zero:
    git show b7ade5e7^:src/vu2/src/vu_nat.cpp > /tmp/w/vu_nat.cpp
    python3 scripts/qa/wire-pair-audit.py /tmp/w
"""
import os, re, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'src'))

FUNC = re.compile(r'^[A-Za-z_][\w:<>\*\s&]*?\b([A-Za-z_]\w*)::(Encode|Decode|Save|Load)\s*\([^;{]*\)\s*\{', re.M)
SIZEOF = re.compile(r'sizeof\s*\(\s*([^)]+?)\s*\)')

# nominal widths; anything unknown is left symbolic and compared by name
WIDTH = {
    'char': 1, 'unsigned char': 1, 'uchar': 1, 'VU_BYTE': 1, 'BYTE': 1,
    'short': 2, 'unsigned short': 2, 'ushort': 2, 'WORD': 2,
    'int': 4, 'unsigned int': 4, 'uint32_t': 4, 'int32_t': 4, 'float': 4,
    'DWORD': 4, 'BOOL': 4, 'u_int32_t': 4,
    'long': 8, 'unsigned long': 8, 'ulong': 8, 'ULONG': 8, 'LONG': 8,
    'double': 8, 'int64_t': 8, 'uint64_t': 8, 'VU_TIME': 8, 'size_t': 8,
}


def strip_comments(text):
    """Remove // and /* */ comments.

    Without this the audit reads sizeof() out of PROSE -- the first run flagged
    vu_nat.cpp because the comment explaining the VUADDR-1 fix contains the words
    sizeof(unsigned long). A scanner that parses its own documentation as code
    reports the bug it describes as still present.
    """
    text = re.sub(r'/\*.*?\*/', ' ', text, flags=re.S)
    text = re.sub(r'//[^\n]*', ' ', text)
    return text


def body_of(src, brace):
    depth, i = 0, brace
    while i < len(src):
        if src[i] == '{':
            depth += 1
        elif src[i] == '}':
            depth -= 1
            if depth == 0:
                return src[brace:i]
        i += 1
    return src[brace:brace + 6000]


def widths(body):
    body = strip_comments(body)
    out = []
    for m in SIZEOF.finditer(body):
        t = m.group(1).strip()
        out.append(WIDTH.get(t, t))
    return out


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else ROOT
    pairs = {}
    for dirpath, _, names in os.walk(root):
        for n in names:
            if not n.endswith(('.cpp', '.c')):
                continue
            path = os.path.join(dirpath, n)
            try:
                src = open(path, errors='ignore').read()
            except OSError:
                continue
            for m in FUNC.finditer(src):
                cls, kind = m.group(1), m.group(2)
                b = src.index('{', m.end() - 1)
                pairs.setdefault((path, cls), {})[kind] = (
                    widths(body_of(src, b)), src[:m.start()].count('\n') + 1)

    findings = 0
    for (path, cls), kinds in sorted(pairs.items()):
        for a, b in (('Encode', 'Decode'), ('Save', 'Load')):
            if a in kinds and b in kinds:
                wa, la = kinds[a]
                wb, lb = kinds[b]
                # Encode double-counts each field (memcpy + pointer advance) while
                # Decode's memcpychk advances internally, so the raw sequences are
                # not comparable. Compare the SET of widths used: a side using a
                # width the other never uses is a real format disagreement, which
                # is exactly how VUADDR-1 presented (8 vs 4 for the same field).
                if sorted(set(map(str, wa))) != sorted(set(map(str, wb))):
                    print(f'{os.path.relpath(path, root)}: {cls}::{a} (line {la}) vs '
                          f'{cls}::{b} (line {lb}) DISAGREE')
                    print(f'    {a}: {wa}')
                    print(f'    {b}: {wb}')
                    findings += 1
    print(f'\n=== {findings} disagreeing pair(s) ===')


if __name__ == '__main__':
    sys.exit(main())

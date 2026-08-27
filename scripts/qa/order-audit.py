#!/usr/bin/env python3
"""ORDER-1: find guards sequenced AFTER the access they protect.

Three shapes, all found live in this codebase during the 2026-08 sessions:
  (a) array[idx] used before the bounds check on idx   (AttachChild/DetachChild)
  (b) ptr dereferenced before its NULL check
  (c) an index read before the > -1 check on it        (GetAvailablePilot)

Correctness notes, both learned the hard way:
  * Reads latin-1. ~30 files in this tree hold non-UTF-8 bytes that the default
    ripgrep-style grep silently skips (CLAUDE.md), so an audit through it is not
    a completeness audit.
  * Tracks brace depth. A flat backward window crosses function boundaries and
    reports a correctly-guarded setter as the unguarded access for the getter
    below it -- that produced ~20 false positives on the first run.
"""
import os, re, sys

ROOT   = sys.argv[1] if len(sys.argv) > 1 else "/home/g/ff/src"
WINDOW = 10

G_BOUND = re.compile(r'\bif\s*\(\s*(\w+)\s*(>=|>|<|<=)\s*([\w.>\-]+)\s*\)')
G_NULL  = re.compile(r'\bif\s*\(\s*(?:not\s+|!)\s*(\w+)\s*\)\s*$|'
                     r'\bif\s*\(\s*(\w+)\s*(?:==|not_eq|!=)\s*(?:NULL|nullptr|0)\s*\)')
BAILS   = re.compile(r'\b(return|continue|break|goto|throw)\b')
DECL    = re.compile(r'^\s*(?:const\s+|static\s+|unsigned\s+)*[\w:]+\s*[*&]?\s*\**\s*%s\s*(?:[;=,)]|\[)')

def depth_map(lines):
    """Brace depth BEFORE each line (comments/strings ignored -- good enough)."""
    d, out = 0, []
    for ln in lines:
        out.append(d)
        code = re.sub(r'//.*', '', ln)
        code = re.sub(r'"(?:\\.|[^"\\])*"', '""', code)
        code = re.sub(r"'(?:\\.|[^'\\])*'", "''", code)
        d += code.count('{') - code.count('}')
    return out

def accesses(line, name):
    hits = set()
    if re.search(r'\b\w+\s*\[[^\]]*\b' + re.escape(name) + r'\b[^\]]*\]', line):
        hits.add("index")
    if re.search(r'\b' + re.escape(name) + r'\s*(?:->|\[)', line):
        hits.add("deref")
    return hits



def short_circuit(line, name):
    """True if name is truth-tested before being dereferenced on the same line,
    i.e. `p and p->x` / `p && p->x` -- the deref is protected by short-circuit."""
    m = re.search(r'\b' + re.escape(name) + r'\b\s*(?:and|&&)', line)
    if not m:
        return False
    d = re.search(r'\b' + re.escape(name) + r'\s*(?:->|\[)', line)
    return bool(d) and m.start() < d.start()

def split_if(line):
    """Split `if (cond) body` into (cond, body). An access inside COND is not
    protected by that if; an access in BODY is. Conflating the two is what made
    the first pass-2 miss GetAvailablePilot -- its unguarded access lives in the
    condition of the next if."""
    k = line.find("if")
    if k < 0 or "(" not in line[k:]:
        return line, ""
    i = line.index("(", k); d = 0
    for j in range(i, len(line)):
        if line[j] == "(": d += 1
        elif line[j] == ")":
            d -= 1
            if d == 0:
                return line[i:j+1], line[j+1:]
    return line, ""

def self_guarded(line, name):
    """True only if name is validity-tested in the condition AND used in the body."""
    cond, body = split_if(line)
    if not body.strip():
        return False
    return (re.search(r'\b' + re.escape(name) + r'\b', cond) is not None
            and not accesses(cond, name)
            and bool(accesses(body, name)))

def scan(path, out):
    try:
        with open(path, encoding="latin-1") as f:
            lines = f.readlines()
    except OSError:
        return
    depth = depth_map(lines)
    for i, line in enumerate(lines):
        s = line.strip()
        if s.startswith(("//", "*", "/*", "#")):
            continue
        if not BAILS.search("".join(lines[i:i+3])):
            continue
        cands = []
        m = G_BOUND.search(s)
        if m: cands.append((m.group(1), "bounds"))
        m = G_NULL.search(s)
        if m: cands.append((m.group(1) or m.group(2), "null"))

        for name, kind in cands:
            if not name or len(name) < 2 or name in ("if", "for", "while"):
                continue
            gd = depth[i]
            if gd == 0:
                continue
            for j in range(i - 1, max(-1, i - 1 - WINDOW), -1):
                if depth[j] < gd:            # left the guard's block
                    break
                prev, p = lines[j], lines[j].strip()
                if p.startswith(("//", "*", "/*", "#")) or name not in prev:
                    continue
                if re.match(DECL.pattern % re.escape(name), prev):
                    continue                 # a declaration, not an access
                if self_guarded(prev, name) or short_circuit(prev, name):
                    continue          # guarded in its own condition -- fine
                how = accesses(prev, name)
                if how:
                    out.append((os.path.relpath(path, "/home/g/ff"), i + 1, kind,
                                name, j + 1, "/".join(sorted(how)), p[:100], s[:100]))
                    break

res, n = [], 0
for dirpath, dirnames, filenames in os.walk(ROOT):
    dirnames[:] = [d for d in dirnames if d not in ("extern", ".git")]
    for fn in filenames:
        if fn.endswith((".cpp", ".c", ".h")):
            p = os.path.join(dirpath, fn)
            if os.path.islink(p):
                continue
            scan(p, res); n += 1

for r in res:
    print("%s:%d  guard=%s(%s)  access@%d=%s" % r[:6])
    print("      access: %s" % r[6])
    print("      guard : %s" % r[7])
print("--- %d candidates in %d files ---" % (len(res), n), file=sys.stderr)

# ---------------------------------------------------------------------------
# Pass 2 -- shape (d): a validity guard whose single unbraced statement covers
# only the FIRST access, with further accesses to the same name following it at
# the same depth. This is what GetAvailablePilot was: "if (best_pilot > -1)"
# guarded the usage++ and nothing else, so PilotInfo[-1] was read two lines
# later. Pass 1 cannot see it -- the guard is correctly placed, it is merely
# too small, which is exactly why it reads as safe.
# ---------------------------------------------------------------------------
VALID = re.compile(r'\bif\s*\(\s*(?:not\s+|!)?\s*(\w+)\s*'
                   r'(?:(?:!=|not_eq|==)\s*(?:NULL|nullptr|0)\s*'
                   r'|>\s*-1\s*|>=\s*0\s*|<\s*[A-Z_][A-Z0-9_]*\s*)?\)\s*$')

def scan2(path, out):
    try:
        with open(path, encoding="latin-1") as f:
            lines = f.readlines()
    except OSError:
        return
    depth = depth_map(lines)
    for i, line in enumerate(lines):
        s = line.strip()
        if s.startswith(("//", "*", "/*", "#")) or depth[i] == 0:
            continue
        m = VALID.match(s)
        if not m:
            continue
        name = m.group(1)
        if len(name) < 2 or name in ("if", "for", "while", "return"):
            continue
        # The body is the first real line after the guard. Taking lines[i+1]
        # blindly reports the true body as "outside the guard" whenever a
        # commented-out line sits between -- which it does at several sites here.
        b = i + 1
        while b < len(lines) and (not lines[b].strip()
                                  or lines[b].strip().startswith(("//", "/*", "*"))):
            b += 1
        if b >= len(lines):
            continue
        body = lines[b].strip()
        if body.startswith("{") or not accesses(body, name):
            continue                      # braced body, or body is not the use

        # Nested unbraced guard: "if (cur)\n if (cur->Label_)\n return cur->..."
        # The inner if is the outer's body, so the line after it is the INNER body and
        # is still protected. Without this, correctly-nested guards read as violations
        # -- cpopup.cpp:330 was flagged this way through every earlier run.
        if re.match(r'\s*if\s*\(', body):
            continue
        if BAILS.search(body):
            continue                      # early-out guard: inverse shape
        # further unguarded accesses at the guard's own depth?
        for j in range(b + 1, min(len(lines), b + 1 + WINDOW)):
            if depth[j] < depth[i]:
                break
            nxt, p = lines[j], lines[j].strip()
            if p.startswith(("//", "*", "/*", "#")) or name not in nxt:
                continue
            if depth[j] != depth[i]:
                continue
            if self_guarded(p, name) or short_circuit(p, name):
                break                     # genuinely guarded -- fine
            if accesses(p, name):
                out.append((os.path.relpath(path, "/home/g/ff"), j + 1, "narrow",
                            name, i + 1, "outside-guard", p[:100], s[:100]))
            break

res2, n2 = [], 0
for dirpath, dirnames, filenames in os.walk(ROOT):
    dirnames[:] = [d for d in dirnames if d not in ("extern", ".git")]
    for fn in filenames:
        if fn.endswith((".cpp", ".c", ".h")):
            p = os.path.join(dirpath, fn)
            if os.path.islink(p):
                continue
            scan2(p, res2); n2 += 1

print("\n=== pass 2: guard too narrow (access outside a single-statement body) ===")
for r in res2:
    print("%s:%d  %s(%s)  guard@%d=%s" % r[:6])
    print("      guard : %s" % r[7])
    print("      access: %s" % r[6])
print("--- pass2: %d candidates ---" % len(res2), file=sys.stderr)

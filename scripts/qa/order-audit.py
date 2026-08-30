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

# Blind spot found 2026-08-30, validated against a known positive: G_NULL only
# matches when the NULL test IS the whole condition. A guard written as
#     if ((win == NULL) or (a == NULL) or (b == NULL))
# never matched, so phonebk.cpp's CopyDataFromWindow -- which dereferences `win`
# twice via FindControl before that very line -- scanned clean through the whole
# ORDER-1 sweep. Match a NULL test ANYWHERE inside an if-condition and offer
# every name it tests as a candidate.
IF_LINE     = re.compile(r'\bif\s*\(')
G_NULL_ANY  = re.compile(r'(\w+)\s*(?:==|not_eq|!=)\s*(?:NULL|nullptr|0)\b')
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

        # compound conditions -- see G_NULL_ANY above
        if IF_LINE.search(s):
            for nm in G_NULL_ANY.findall(s):
                if (nm, "null") not in cands:
                    cands.append((nm, "null"))

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

                # Linked-list walk: `curr = curr->next;` followed by `if (!curr)`
                # is advance-then-test, the correct idiom, not a guard sequenced
                # after its access. This shape was most of the noise once the
                # compound-condition match was added.
                if re.match(r'^\s*%s\s*=\s*%s\s*(?:->|\.)'
                            % (re.escape(name), re.escape(name)), prev):
                    continue
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

# ---------------------------------------------------------------------------
# Pass 3 -- ORDER-3: an assertion that performs the very access it is checking.
# This matters here specifically because ShiAssert is LIVE in this build
# (shi/assert.h promotes DEBUG/_DEBUG, CMake defines _DEBUG), so an assert that
# indexes out of range IS the crash, not a debug-only annoyance. Found
# FarTexDB::Deactivate and division.cpp, the latter sitting directly above a
# bounds check tagged "// JB 010223 CTD" -- crash to desktop.
# ---------------------------------------------------------------------------
def scan3(path, out):
    try:
        with open(path, encoding="latin-1") as f:
            lines = f.readlines()
    except OSError:
        return
    for a, b in _funcs(lines):
        body = lines[a:b+1]
        for k, ln in enumerate(body):
            if ln.lstrip().startswith("//"):
                continue
            m = re.search(r'\bShiAssert\s*\(([^;]*)\)\s*;', ln)
            if not m:
                continue
            for arr, ix in re.findall(r'\b(\w+)\s*\[\s*([\w\.\->]+)\s*\]', m.group(1)):
                rest = "".join(body[k+1:])
                gv = re.search(r'if\s*\(\s*(?:not\s+|!)?\s*' + re.escape(arr)
                               + r'\s*(?:==\s*(?:NULL|0|nullptr)\s*)?\)', rest)
                gi = re.search(r'if\s*\(\s*' + re.escape(ix) + r'\s*(?:>=|<|>)\s*', rest)
                if gv or gi:
                    out.append((os.path.relpath(path, "/home/g/ff"), a + k + 1,
                                "assert-indexes", "%s[%s]" % (arr, ix),
                                "guard is LATER in the same function", "",
                                ln.strip()[:100], ""))

def _funcs(lines):
    d, start = 0, None
    for i, ln in enumerate(lines):
        code = re.sub(r'//.*', '', ln)
        code = re.sub(r'"(?:\\.|[^"\\])*"', '""', code)
        o, c = code.count('{'), code.count('}')
        if d == 0 and o:
            start = i
        d += o - c
        if d <= 0 and start is not None:
            yield start, i
            start, d = None, 0

res3 = []
for dirpath, dirnames, filenames in os.walk(ROOT):
    dirnames[:] = [d for d in dirnames if d not in ("extern", ".git")]
    for fn in filenames:
        if fn.endswith((".cpp", ".c")):
            p = os.path.join(dirpath, fn)
            if os.path.islink(p):
                continue
            scan3(p, res3)

print("\n=== pass 3: assertion performs the access it checks (ShiAssert is LIVE here) ===")
for r in res3:
    print("%s:%d  %s %s -- %s" % (r[0], r[1], r[2], r[3], r[4]))
    print("      %s" % r[6])
print("--- pass3: %d candidates ---" % len(res3), file=sys.stderr)


# ---------------------------------------------------------------------------
# Pass 4 -- GUARD-1: a lookup that CAN return NULL whose result is dereferenced
# with no NULL check at all. Distinct from pass 1 (guard present but sequenced
# after the access) -- here there is no guard.
#
# Seed found by hand in atm.cpp:
#     abe = (CampEntity) vuDatabase->Find(airbase->id);
#     if (abe->IsObjective())            <-- no NULL test
# vuDatabase->Find() returns NULL for an absent id, and ids go stale across
# deaggregation and entity removal.
#
# ShiAssert does NOT count as a guard: it is live but non-halting in this build,
# so "ShiAssert(p); p->x;" is this same defect with a printout.
# ---------------------------------------------------------------------------
LOOKUPS = re.compile(
    r'=\s*(?:\([^)]*\)\s*)?'                       # optional cast
    r'(?:[\w\.\->]*?)\b('
    r'vuDatabase\s*->\s*Find|'
    r'FindATMAirbase|'
    r'FindEntity|GetEntity|'
    r'FindName|FindObjective|FindUnit'
    r')\s*\(')

def scan4(path, out):
    try:
        with open(path, encoding="latin-1") as f:
            lines = f.readlines()
    except OSError:
        return
    depth = depth_map(lines)
    for a, b in _funcs(lines):
        for k in range(a, b + 1):
            ln = lines[k]
            if ln.lstrip().startswith("//"):
                continue
            if not LOOKUPS.search(ln):
                continue
            # The optional type prefix made this ambiguous: on `abe = ...` the
            # regex backtracks and captures 'e' (splitting "ab|e ="), so the walk
            # then looks for derefs of `e` and misses `abe->`. That silently hid
            # the very instance this pass was written from. Capture the identifier
            # immediately before '=' instead.
            m = re.search(r'(\w+)\s*=(?!=)', ln)
            if not m:
                continue
            name = m.group(1)
            # Assignment INSIDE a condition, tested on the same line:
            #     (assoc = vuDatabase->Find(...)) not_eq 0 and assoc->OwnerId()
            # That is guarded. Fourth false-positive shape found while triaging.
            if re.search(r'\b' + re.escape(name) + r'\b[^;]*?(?:==|!=|not_eq)\s*(?:NULL|nullptr|0)', ln):
                continue
            if short_circuit(ln, name):
                continue
            base = depth[k]
            # walk forward in the same or deeper scope
            for j in range(k + 1, min(k + 1 + WINDOW, b + 1)):
                if depth[j] < base:
                    break
                nxt = lines[j]
                if nxt.lstrip().startswith("//"):
                    continue
                # a real NULL test on `name` clears it
                if re.search(r'if\s*\(\s*(?:not\s+|!)?\s*' + re.escape(name)
                             + r'\s*(?:==|!=|not_eq)?\s*(?:NULL|nullptr|0)?\s*[\)&|]', nxt):
                    break
                if re.search(r'\b' + re.escape(name) + r'\s*(?:==|not_eq|!=)\s*(?:NULL|nullptr|0)', nxt):
                    break
                # ShiAssert is NOT a guard here
                # `p and p->x` / `not p or not p->x` protect the deref by
                # short-circuit -- the first pass-4 run reported ~all of these as
                # defects. Reuse the helper written for pass 1.
                if short_circuit(nxt, name):
                    break
                # A truth-test of `name` in an if CONDITION guards the whole body,
                # including a deref on a LATER line:
                #     if (o and ...)
                #         o->GetFullName(...);
                # short_circuit() only sees one line, so it missed these and all
                # four gtm.cpp hits were false positives. `name` must appear BARE in
                # the condition -- if it is dereferenced there (if (abe->IsObj()))
                # that is the defect itself, not a guard.
                if 'if' in nxt:
                    cond, _body = split_if(nxt)
                    # strip redundant parens so `if ((un) and (un->IsFlight()))`
                    # reads as a bare operand -- waypoint.cpp:2144 was reported as a
                    # defect purely because of the extra brackets.
                    flat = re.sub(r'\(\s*(\w+)\s*\)', r'\1', cond)
                    # `(un) and (un->IsFlight())`: the name is BOTH truth-tested and
                    # dereferenced in the same condition, so accesses() alone rejects
                    # it. Short-circuit order is what matters -- the bare test comes
                    # first, so the deref is protected.
                    if short_circuit(flat, name):
                        break
                    if re.search(r'\b' + re.escape(name) + r'\b', flat) and not accesses(flat, name):
                        break
                if re.search(r'(?:not\s+|!)\s*' + re.escape(name) + r'\b\s*(?:or|\|\|)', nxt):
                    break
                deref = re.search(r'\b' + re.escape(name) + r'\s*(?:->|\[)', nxt)
                if deref and 'ShiAssert' not in nxt:
                    out.append((os.path.relpath(path, "/home/g/ff"), k + 1, j + 1,
                                name, ln.strip()[:90], nxt.strip()[:90]))
                    break

res4 = []
for dirpath, dirnames, filenames in os.walk(ROOT):
    # camptool is built only under if(WIN32) (src/campaign/CMakeLists.txt:6) and is
    # not compiled in this port, so its copies of camptask sources are dead code
    # here. They were inflating the pass-4 count with duplicates of already-fixed
    # sites. FF_AUDIT_ALL=1 includes them.
    dirnames[:] = [d for d in dirnames
                   if d not in ("extern", ".git")
                   and not (d == "camptool" and not os.environ.get("FF_AUDIT_ALL"))]
    for fn in filenames:
        if fn.endswith((".cpp", ".c")):
            p = os.path.join(dirpath, fn)
            if os.path.islink(p):
                continue
            scan4(p, res4)

print("\n=== pass 4: lookup result dereferenced with NO NULL check ===")
for r in res4:
    print("%s:%d  '%s' assigned here, dereferenced at line %d with no NULL test" % (r[0], r[1], r[3], r[2]))
    print("      %s" % r[4])
    print("      %s" % r[5])
print("--- pass4: %d candidates ---" % len(res4), file=sys.stderr)

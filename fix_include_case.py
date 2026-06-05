#!/usr/bin/env python3
"""Create symlinks for case-mismatched #include directives.

Scans every #include in src/, and when the target doesn't exist with the
exact case but DOES exist case-insensitively (relative to the includer's
directory or any include path), creates a symlink with the requested name
pointing at the real file. Lost-and-recreated infrastructure for the
FreeFalcon Linux port (the original tree had such symlinks, untracked).
"""
import os
import re
import sys

SRC = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'src')

INCLUDE_DIRS = [
    SRC,
    os.path.join(SRC, 'compat'),
    os.path.join(SRC, 'include'),
    os.path.join(SRC, 'falclib/include'),
    os.path.join(SRC, 'codelib/include'),
    os.path.join(SRC, 'vu2/include'),
    os.path.join(SRC, 'sim/include'),
    os.path.join(SRC, 'campaign/include'),
    os.path.join(SRC, 'ui/include'),
    os.path.join(SRC, 'graphics/include'),
    os.path.join(SRC, 'ui95'),
    os.path.join(SRC, 'ui95_ext'),
    os.path.join(SRC, 'acmi/src/include'),
    os.path.join(SRC, 'mathlib'),
    os.path.join(SRC, 'comms'),
    os.path.join(SRC, 'comms/include'),
    os.path.join(SRC, 'codelib/resources/reslib/src'),
    os.path.join(SRC, 'ui/src'),
]

inc_re = re.compile(r'^\s*#\s*include\s+["<]([^">]+)[">]', re.MULTILINE)

def case_resolve(base, rel):
    """Resolve rel against base case-insensitively. Return (resolved_relpath, exact) or None."""
    parts = rel.replace('\\', '/').split('/')
    cur = base
    out = []
    exact = True
    for p in parts:
        if p == '..':
            cur = os.path.dirname(cur)
            out.append('..')
            continue
        if p == '.':
            continue
        if not os.path.isdir(cur):
            return None
        try:
            entries = os.listdir(cur)
        except OSError:
            return None
        if p in entries:
            out.append(p)
            cur = os.path.join(cur, p)
            continue
        match = None
        for e in entries:
            if e.lower() == p.lower():
                match = e
                break
        if match is None:
            return None
        exact = False
        out.append(match)
        cur = os.path.join(cur, match)
    return ('/'.join(out), exact)

created = 0
scanned = 0
for root, dirs, files in os.walk(SRC):
    dirs[:] = [d for d in dirs if d not in ('.git',)]
    for f in files:
        if not f.endswith(('.cpp', '.h', '.c', '.hpp', '.inl')):
            continue
        path = os.path.join(root, f)
        scanned += 1
        try:
            text = open(path, encoding='utf-8', errors='replace').read()
        except OSError:
            continue
        for m in inc_re.finditer(text):
            target = m.group(1).replace('\\', '/')
            if target.startswith(('sys/', 'GL/', 'SDL2/', 'AL/')):
                continue
            # search includer dir first, then include paths
            bases = [root] + INCLUDE_DIRS
            found_exact = False
            found_ci = None  # (base, resolved)
            for b in bases:
                fp = os.path.join(b, target)
                if os.path.exists(fp):
                    found_exact = True
                    break
                r = case_resolve(b, target)
                if r and not r[1] and found_ci is None:
                    found_ci = (b, r[0])
            if found_exact or found_ci is None:
                continue
            b, resolved = found_ci
            # Walk component-by-component; create a symlink at the FIRST
            # mismatching component (file or directory), then re-resolve.
            tparts = target.split('/')
            rparts = resolved.split('/')
            cur = b
            for tp, rp in zip(tparts, rparts):
                if tp == rp:
                    cur = os.path.join(cur, rp)
                    continue
                linkpath = os.path.join(cur, tp)
                try:
                    os.symlink(rp, linkpath)
                    created += 1
                    print(f"LINK {linkpath} -> {rp}")
                except FileExistsError:
                    pass
                except OSError as e:
                    print(f"FAIL {linkpath}: {e}")
                cur = os.path.join(cur, rp)

print(f"\nscanned {scanned} files, created {created} symlinks")

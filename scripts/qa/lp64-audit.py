#!/usr/bin/env python3
"""
LP64-1: find writes sized sizeof(long)/sizeof(DWORD)/sizeof(ULONG) whose
DESTINATION is a struct field narrower than 8 bytes.

Win32 long is 4 bytes; Linux long is 8. A memcpy/fread sized on `long` into a
32-bit field runs 4 bytes past it and corrupts whatever is adjacent. That is
exactly what killed multiplayer: ComIPHostIDGet wrote sizeof(long) into
ComGROUP::HostID (unsigned int) and zeroed the low half of the GroupHead
pointer sitting next to it.

Serialization pairs where BOTH the read and the write use sizeof(long) are
self-consistent and are NOT this defect -- they round-trip within one build.
Only the destination width matters here.

Read the output before believing it. Every scanner in this repo has had a bug
found by reading its own output.
"""
import re, sys, os, collections

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'src')
ROOT = os.path.normpath(ROOT)

# Types that are 4 bytes or fewer on LP64.
NARROW = re.compile(r'^\s*(?:volatile\s+|const\s+)*'
                    r'(?:(?:unsigned|signed)\s+)?'
                    r'(?:int|short|char|float|BOOL|DWORD|UINT|WORD|BYTE|uchar|ushort|'
                    r'u_int32_t|uint32_t|int32_t|u_short|VU_BYTE|GridIndex)\b'
                    r'(?!\s*\*)')

WIDE = re.compile(r'\b(?:long|double|__int64|int64_t|uint64_t|size_t|time_t|'
                  r'CampaignTime|VU_TIME|ULONG|LONG)\b|\*')

SIZEOF = re.compile(r'sizeof\s*\(\s*(?:unsigned\s+)?(long|DWORD|ULONG|LONG)\s*\)')
# memcpy(dst, src, sizeof(long))  /  fread(dst, sizeof(long), n, f)
CALL = re.compile(r'\b(memcpy|memmove|fread|fwrite|memset)\s*\(', re.S)


def collect_fields(files):
    """member name -> set of declared type strings, from struct/class bodies."""
    fields = collections.defaultdict(set)
    decl = re.compile(r'^\s*((?:(?:volatile|const|unsigned|signed|struct|static)\s+)*'
                      r'[A-Za-z_][\w:]*(?:\s*\*+)?)\s+'
                      r'([A-Za-z_]\w*)\s*(?:\[[^\]]*\])?\s*;')
    for path in files:
        try:
            src = open(path, errors='ignore').read()
        except OSError:
            continue
        for ln in src.splitlines():
            m = decl.match(ln)
            if m:
                fields[m.group(2)].add(m.group(1).strip())
    return fields


def split_args(s):
    """Split a call's argument list on top-level commas."""
    out, depth, cur = [], 0, ''
    for ch in s:
        if ch in '([': depth += 1
        elif ch in ')]': depth -= 1
        if ch == ',' and depth == 0:
            out.append(cur); cur = ''
        else:
            cur += ch
    out.append(cur)
    return [a.strip() for a in out]


def arg_text(src, open_paren):
    depth, i = 0, open_paren
    while i < len(src):
        if src[i] == '(': depth += 1
        elif src[i] == ')':
            depth -= 1
            if depth == 0:
                return src[open_paren + 1:i]
        i += 1
    return ''


def find_wide_writers(files):
    """Functions that write sizeof(long) into a char*/void* PARAMETER.

    This is the shape the real defect took: ComIPHostIDGet(c, char *buf, int)
    did memcpy(buf, &internalId, sizeof(long)). The destination width is not
    visible here at all -- it is decided by each caller.
    Returns {func_name: arg_index_of_buffer_param}.
    """
    sig = re.compile(r'\b(?:int|void|BOOL|long|short|char)\s+\*?\s*'
                     r'([A-Za-z_]\w*)\s*\(([^;{)]*)\)\s*\{', re.S)
    writers = {}
    for path in files:
        if path.endswith(('.h', '.hpp')):
            continue
        try:
            src = open(path, errors='ignore').read()
        except OSError:
            continue
        if not SIZEOF.search(src):
            continue
        for m in sig.finditer(src):
            fname, params = m.group(1), m.group(2)
            body = src[m.end(): m.end() + 4000]
            plist = split_args(params)
            for idx, pdecl in enumerate(plist):
                pm = re.match(r'.*?\b(?:char|void|BYTE|LPBYTE)\s*\*\s*([A-Za-z_]\w*)\s*$',
                              pdecl.strip())
                if not pm:
                    continue
                pname = pm.group(1)
                w = re.search(r'\bmemcpy\s*\(\s*' + re.escape(pname) +
                              r'\s*,[^;]*?' + SIZEOF.pattern, body)
                if w:
                    writers[fname] = (idx, os.path.basename(path),
                                      src[:m.start()].count('\n') + 1)
    return writers


def scan_direct(files, fields, root):
    """memcpy/fread(&obj.field, ..., sizeof(long)) written straight into a
    narrow field -- the intraprocedural shape, where the width mismatch is
    visible at the write itself."""
    member_ref = re.compile(r'&\s*[A-Za-z_][\w\[\]\.:()>-]*?(?:\.|->)([A-Za-z_]\w*)\s*$')
    findings = 0
    for path in files:
        if path.endswith(('.h', '.hpp')):
            continue
        try:
            src = open(path, errors='ignore').read()
        except OSError:
            continue
        if not SIZEOF.search(src):
            continue
        for m in CALL.finditer(src):
            args_raw = arg_text(src, m.end() - 1)
            if not SIZEOF.search(args_raw):
                continue
            args = split_args(args_raw)
            if len(args) < 2:
                continue
            size_args = [a for a in args[1:] if SIZEOF.search(a)]
            if not size_args:
                continue
            # sizeof(long)*N is an array copy, not a single-field write
            if any('*' in re.sub(r'\(\s*(?:unsigned\s+)?char\s*\*\s*\)', '', a)
                   for a in size_args):
                continue
            dst = re.sub(r'\((?:unsigned\s+)?(?:char|BYTE|void|LPBYTE|VU_BYTE)\s*\*\s*\)',
                         '', args[0]).strip()
            mm = member_ref.match(dst)
            if not mm:
                continue
            name = mm.group(1)
            types = fields.get(name)
            if not types:
                continue
            narrow = sorted(t for t in types if NARROW.match(t) and not WIDE.search(t))
            wide = sorted(t for t in types if WIDE.search(t))
            if narrow and not wide:
                line = src[:m.start()].count('\n') + 1
                print(f'{os.path.relpath(path, root)}:{line}: {m.group(1)} sized on long '
                      f'into `{name}` declared {narrow}')
                print(f'    {" ".join(args_raw.split())[:110]}')
                findings += 1
    return findings


def main():
    root = sys.argv[1] if len(sys.argv) > 1 else ROOT
    files = []
    for dirpath, _, names in os.walk(root):
        if 'camptool' in dirpath and not os.environ.get('FF_AUDIT_ALL'):
            continue
        for n in names:
            if n.endswith(('.c', '.cpp', '.h', '.hpp')):
                files.append(os.path.join(dirpath, n))

    fields = collect_fields(files)
    writers = find_wide_writers(files)
    print(f'-- {len(writers)} function(s) write sizeof(long) into a char* parameter:')
    for f, (idx, fl, ln) in sorted(writers.items()):
        print(f'     {f}()  arg{idx}  [{fl}:{ln}]')
    print()

    print('-- direct writes into a narrow field:')
    findings = scan_direct(files, fields, root)
    print()
    print('-- narrow destinations passed to those writers:')

    member_ref = re.compile(r'&\s*[A-Za-z_][\w\[\]\.:()>-]*?(?:\.|->)([A-Za-z_]\w*)\s*$')
    for path in files:
        if path.endswith(('.h', '.hpp')):
            continue
        try:
            src = open(path, errors='ignore').read()
        except OSError:
            continue
        for fname, (argidx, _, _) in writers.items():
            for m in re.finditer(r'\b' + re.escape(fname) + r'\s*\(', src):
                args = split_args(arg_text(src, m.end() - 1))
                if len(args) <= argidx:
                    continue
                dst = re.sub(r'\((?:unsigned\s+)?(?:char|BYTE|void|LPBYTE|VU_BYTE)\s*\*\s*\)',
                             '', args[argidx]).strip()
                mm = member_ref.match(dst)
                if not mm:
                    continue
                name = mm.group(1)
                types = fields.get(name)
                if not types:
                    continue
                narrow = sorted(t for t in types if NARROW.match(t) and not WIDE.search(t))
                wide = sorted(t for t in types if WIDE.search(t))
                if narrow and not wide:
                    line = src[:m.start()].count('\n') + 1
                    print(f'{os.path.relpath(path, root)}:{line}: {fname}() writes '
                          f'8 bytes into `{name}` declared {narrow}')
                    print(f'    {" ".join(args[argidx].split())[:110]}')
                    findings += 1
    print(f'\n=== {findings} candidate site(s) total ===')
    return 0


if __name__ == '__main__':
    sys.exit(main())

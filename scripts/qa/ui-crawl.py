#!/usr/bin/env python3
"""FF UI crawler (FUNC-SWEEP-FF): click every control reachable from the main menu, each in a FRESH
launch, and record whether the game crashed, asserted, quit, or opened new windows.

Discovery uses the game's own control table (FF_DUMP_UI: every visible window and control with its
click point), so no coordinates are guessed. Depth 1 = every control of every main-menu screen;
depth 2 = every control of each window a depth-1 click opened (Setup tabs, TacRef pages, popups...).

Runs against a DISPOSABLE data copy (default ~/ff-crawl) -- crawling clicks Delete/Apply/Save.
Usage (under gl-lock):  ui-crawl.py [--data DIR] [--out DIR] [--max-runs N] [--depth 1|2]
Output: <out>/crawl.tsv (one row per click path) + <out>/runs/*.log
"""
import argparse, os, re, subprocess, sys, time

ap = argparse.ArgumentParser()
ap.add_argument('--data', default=os.path.expanduser('~/ff-crawl'))
ap.add_argument('--bin', default=os.path.expanduser('~/freefalcon-central/build/src/ffviper/FFViper'))
ap.add_argument('--out', default=os.path.expanduser('~/ff-gates/crawl'))
ap.add_argument('--max-runs', type=int, default=450)
ap.add_argument('--depth', type=int, default=2)
ap.add_argument('--only', default='', help='comma list of main-menu ctrl ids to crawl')
A = ap.parse_args()
os.makedirs(A.out + '/runs', exist_ok=True)

T_MAIN = 10      # main-menu click (s after launch)
T_STEP = 9       # seconds between successive clicks in a path
T_DUMP = 7       # dump/screenshot this long after the last click
nruns = 0

def run(path_clicks, tag):
    """Launch, click each (x,y) at T_MAIN, T_MAIN+T_STEP, ...; dump UI after the last. Return dict."""
    global nruns
    nruns += 1
    times = [T_MAIN + i * T_STEP for i in range(len(path_clicks))]
    tdump = (times[-1] if times else T_MAIN) + T_DUMP
    clicks = ';'.join('%d,%d@%d' % (x, y, t) for (x, y), t in zip(path_clicks, times))
    log = '%s/runs/%s.log' % (A.out, tag)
    env = dict(os.environ, DISPLAY=os.environ.get('DISPLAY', ':0'), FF_DUMP_UI=str(tdump))
    if clicks: env['FF_UI_CLICK'] = clicks
    shot_dir = '%s/runs/%s_shots' % (A.out, tag)
    os.makedirs(shot_dir, exist_ok=True)
    env['FF_UI_SCREENSHOT'] = str(tdump); env['FF_UI_SHOT_DIR'] = shot_dir
    t0 = time.time()
    with open(log, 'wb') as f:
        p = subprocess.run(['timeout', '-k', '5', '-s', 'KILL', str(tdump + 6), A.bin, '-d', A.data, '-w'],
                           cwd=A.data, env=env, stdout=f, stderr=subprocess.STDOUT)
    txt = open(log, 'rb').read().decode('latin-1')
    crash = txt.count('=== CRASH') + txt.count('Segmentation fault') + txt.count('Aborted')
    asserts = txt.count('Assertion at')
    dump = txt[txt.rfind('[UIDUMP] ===='):] if '[UIDUMP] ====' in txt else ''
    wins = {}
    cur = None
    for ln in dump.split('\n'):
        m = re.match(r'\[UIDUMP\] window id=(-?\d+) at (-?\d+),(-?\d+) (\d+)x(\d+) vis=1', ln)
        if m: cur = int(m.group(1)); wins[cur] = []; continue
        m = re.match(r'\[UIDUMP\]\s+ctrl id=(-?\d+) rect=-?\d+,-?\d+ (\d+)x(\d+) click=(-?\d+),(-?\d+)', ln)
        if m and cur is not None:
            cid, w, h, x, y = map(int, m.groups())
            if cid != -2 and w > 0 and h > 0 and 0 <= x < 1024 and 0 <= y < 768:
                wins[cur].append((cid, x, y))
    # exit 124/137 = still running at the timeout (alive); anything else = it quit or died
    state = 'CRASH' if crash else ('alive' if p.returncode in (124, 137, -9) else 'exited(%d)' % p.returncode)
    return dict(state=state, asserts=asserts, wins=wins, secs=round(time.time() - t0), log=log, dumped=bool(dump))

tsv = open(A.out + '/crawl.tsv', 'a')
def record(path_desc, r, opened):
    tsv.write('\t'.join([path_desc, r['state'], str(r['asserts']), 'dump' if r['dumped'] else 'NODUMP',
                         ','.join(map(str, sorted(opened))), os.path.basename(r['log'])]) + '\n')
    tsv.flush()
    print('%-40s %-12s asserts=%-3d opened=%s' % (path_desc, r['state'], r['asserts'], sorted(opened)), flush=True)

base = run([], 'main')
if 5000 not in base['wins']:
    print('main menu not found in dump; abort'); sys.exit(1)
main_ctrls = [c for c in base['wins'][5000]]
if A.only:
    keep = set(int(x) for x in A.only.split(','))
    main_ctrls = [c for c in main_ctrls if c[0] in keep]
print('main menu controls:', [c[0] for c in main_ctrls], flush=True)

for (mid, mx, my) in main_ctrls:
    if nruns >= A.max_runs: break
    r1 = run([(mx, my)], 'm%d' % mid)
    base_ids = set(base['wins'])
    opened1 = set(r1['wins']) - base_ids if r1['dumped'] else set()
    record('main:%d' % mid, r1, opened1)
    if r1['state'] != 'alive' or not r1['dumped']:
        continue
    # depth 1: every control in every window visible on this screen except the main menu's own
    screen_ctrls = []
    for wid, cl in r1['wins'].items():
        if wid == 5000 and 5000 in base_ids and len(r1['wins']) > 1:
            continue
        screen_ctrls += [(wid, c) for c in cl]
    seen = set()
    for wid, (cid, x, y) in screen_ctrls:
        if nruns >= A.max_runs: break
        key = (cid, x, y)
        if key in seen: continue
        seen.add(key)
        r2 = run([(mx, my), (x, y)], 'm%d_w%d_c%d' % (mid, wid, cid))
        opened2 = set(r2['wins']) - set(r1['wins']) if r2['dumped'] else set()
        record('main:%d > w%d:c%d' % (mid, wid, cid), r2, opened2)
        if A.depth < 2 or r2['state'] != 'alive' or not opened2:
            continue
        # depth 2: controls of the window(s) that click opened
        for wid2 in sorted(opened2):
            for (cid2, x2, y2) in r2['wins'][wid2]:
                if nruns >= A.max_runs: break
                r3 = run([(mx, my), (x, y), (x2, y2)], 'm%d_w%d_c%d_w%d_c%d' % (mid, wid, cid, wid2, cid2))
                opened3 = set(r3['wins']) - set(r2['wins']) if r3['dumped'] else set()
                record('main:%d > w%d:c%d > w%d:c%d' % (mid, wid, cid, wid2, cid2), r3, opened3)
print('=== CRAWL COMPLETE runs=%d ===' % nruns, flush=True)

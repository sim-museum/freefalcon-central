#!/usr/bin/env python3
"""FUNC-SWEEP-FF: one FF_SIM_KEY event list that presses EVERY binding in config/keystrokes.key once.
Format per event: [S][C][A]0xDIK@sec+holdms (main_linux.cpp FF_SIM_KEY). Chords press the prefix first.
Held back (they end or freeze the flight; test separately): SimEndFlight, SimEject, SimTogglePaused,
SimMotionFreeze. Usage: ff_keysweep_gen.py keystrokes.key [start_sec] [step_sec] > keys.txt
Also writes keys.txt.map (event index -> callback) so a crash can be attributed to the key that fired."""
import os, re, sys
HOLD = {'SimEndFlight', 'SimEject', 'SimTogglePaused', 'SimMotionFreeze'}
LINE = re.compile(r'^(\S+)\s+(-?\d+)\s+(-?\d+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(-?\d+)\s+"([^"]*)"')
def mods(m): return ('S' if m & 1 else '') + ('C' if m & 2 else '') + ('A' if m & 4 else '')
t = float(sys.argv[2]) if len(sys.argv) > 2 else 25.0
step = float(sys.argv[3]) if len(sys.argv) > 3 else 0.6
ev, mp = [], []
skip = int(os.environ.get('SKIP', '0'))   # resume after a flight ended: drop the first N bindings
nb = 0
for raw in open(sys.argv[1], encoding='latin-1'):
    m = LINE.match(raw.strip())
    if not m: continue
    func, _, _, key, mod, pk, pm, _, desc = m.groups()
    key, mod, pk, pm = (int(x, 16) for x in (key, mod, pk, pm))
    if key == 0xFFFFFFFF or func in HOLD: continue
    nb += 1
    if nb <= skip: continue
    if pk == 0xFFFFFFFF:       # radio menu entry: open the menu key, then digit 1
        ev.append('%s0x%02X@%.1f+150' % (mods(mod), key, t)); ev.append('0x02@%.1f+150' % (t + 0.25))
    elif pk != 0:
        ev.append('%s0x%02X@%.1f+150' % (mods(pm), pk, t)); ev.append('%s0x%02X@%.1f+150' % (mods(mod), key, t + 0.25))
    else:
        ev.append('%s0x%02X@%.1f+150' % (mods(mod), key, t))
    mp.append('%.1f\t%s\t%s' % (t, func, desc))
    t += step
print(';'.join(ev))
open(sys.argv[4] if len(sys.argv) > 4 else '/dev/stderr', 'w').write('\n'.join(mp) + '\n')

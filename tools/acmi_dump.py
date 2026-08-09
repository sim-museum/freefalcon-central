#!/usr/bin/env python3
"""
Read a FreeFalcon ACMI tape (.vhs) and emit the flight as NUMBERS.

Why this exists (PO, 2026-08-09): "acmi ... gives a quantitative measure of
sim/pilot performance that can be tracked and optimized". Screenshots make
approach quality a matter of opinion; a tape makes it a time series.

The .vhs on-disk layout is 32-bit Windows (`long` == 4 bytes). Sizes below are
confirmed against real PO tapes, whose own block offsets are exactly consistent
with them -- the same constants that are static_assert-pinned in
src/acmi/src/include/acmitape.h:

    ACMITapeHeader          80 bytes
    ACMIEntityData          36
    ACMIEntityPositionData  41

Coordinates are Falcon NED feet: x north, y east, z DOWN, so altitude = -z.

Usage:
    tools/acmi_dump.py <tape.vhs>                 # summary + entity list
    tools/acmi_dump.py <tape.vhs> --track         # player altitude/speed series
    tools/acmi_dump.py <tape.vhs> --approach      # last 90 s: the landing profile
    tools/acmi_dump.py <tape.vhs> --csv out.csv   # full position series as CSV
"""
import struct
import sys

HDR_FMT = '<17i'
HDR_SIZE = 80
ENT_SIZE = 36
POS_SIZE = 41

HDR_FIELDS = [
    'fileID', 'fileSize', 'numEntities', 'numFeat', 'entityBlockOffset',
    'featBlockOffset', 'numEntityPositions', 'timelineBlockOffset',
    'firstEntEventOffset', 'firstGeneralEventOffset', 'firstEventTrailerOffset',
    'firstTextEventOffset', 'firstFeatEventOffset', 'numEvents', 'numEntEvents',
    'numTextEvents', 'numFeatEvents',
]

POS_TYPE_POS = 0


def read_header(d):
    vals = struct.unpack_from(HDR_FMT, d, 0)
    h = dict(zip(HDR_FIELDS, vals))
    h['startTime'], h['totPlayTime'], h['todOffset'] = struct.unpack_from('<3f', d, 68)
    return h


def check_layout(h):
    """The tape's own offsets must agree with the 32-bit struct sizes."""
    checks = [
        ('header',    HDR_SIZE,                                      h['entityBlockOffset']),
        ('entities',  h['entityBlockOffset'] + h['numEntities'] * ENT_SIZE, h['featBlockOffset']),
        ('features',  h['featBlockOffset'] + h['numFeat'] * ENT_SIZE,      h['timelineBlockOffset']),
        ('positions', h['timelineBlockOffset'] + h['numEntityPositions'] * POS_SIZE,
                      h['firstEntEventOffset']),
    ]
    return [(n, a, b, a == b) for n, a, b in checks]


def read_entities(d, h):
    out = []
    for i in range(h['numEntities']):
        off = h['entityBlockOffset'] + i * ENT_SIZE
        uid, typ, count, flags, lead = struct.unpack_from('<5i', d, off)
        slot, special = struct.unpack_from('<2i', d, off + 20)
        firstPos, firstEvent = struct.unpack_from('<2i', d, off + 28)
        out.append(dict(idx=i, uniqueID=uid, type=typ, count=count, flags=flags,
                        firstPositionDataOffset=firstPos))
    return out


def walk_positions(d, first_offset, limit=200000):
    """Follow one entity's position chain via nextPositionUpdateOffset."""
    out = []
    off = first_offset
    seen = set()
    while off and 0 < off < len(d) - POS_SIZE and len(out) < limit:
        if off in seen:
            break
        seen.add(off)
        t = struct.unpack_from('<f', d, off)[0]
        ptype = d[off + 4]
        nxt = struct.unpack_from('<i', d, off + 33)[0]
        if ptype == POS_TYPE_POS:
            x, y, z, pitch, roll, yaw = struct.unpack_from('<6f', d, off + 5)
            out.append(dict(time=t, x=x, y=y, z=z, pitch=pitch, roll=roll, yaw=yaw))
        off = nxt
    return out


def fmt_hms(s):
    s = int(s)
    return f'{s // 3600:02d}:{s % 3600 // 60:02d}:{s % 60:02d}'


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    path = sys.argv[1]
    opts = sys.argv[2:]
    d = open(path, 'rb').read()
    h = read_header(d)

    print(f'{path}  ({len(d)} bytes)')
    print(f'  entities={h["numEntities"]}  features={h["numFeat"]}  '
          f'positions={h["numEntityPositions"]}')
    print(f'  startTime={h["startTime"]:.1f}s ({fmt_hms(h["startTime"])} game time)  '
          f'playTime={h["totPlayTime"]:.1f}s')

    print('  32-bit layout self-check:')
    ok_all = True
    for name, expect, actual, ok in check_layout(h):
        ok_all &= ok
        print(f'    {name:10} {expect:>10} == {actual:<10} {"OK" if ok else "MISMATCH"}')
    if not ok_all:
        print('  !! layout mismatch -- this tape is not the expected 32-bit format')
        return 1

    ents = read_entities(d, h)
    # The player/ownship is normally the entity with the longest position chain.
    tracks = []
    for e in ents[:min(len(ents), 400)]:
        if e['firstPositionDataOffset'] <= 0:
            continue
        pts = walk_positions(d, e['firstPositionDataOffset'])
        if pts:
            tracks.append((e, pts))
    tracks.sort(key=lambda ep: len(ep[1]), reverse=True)

    print(f'\n  entities with position chains: {len(tracks)}')
    for e, pts in tracks[:5]:
        alt = [-p['z'] for p in pts]
        print(f'    id={e["uniqueID"]:<12} type={e["type"]:<6} samples={len(pts):<6} '
              f'alt {min(alt):8.0f}..{max(alt):8.0f} ft   '
              f't {pts[0]["time"]:.1f}..{pts[-1]["time"]:.1f}s')

    if not tracks:
        print('  (no position chains found)')
        return 0

    ent, pts = tracks[0]

    if '--track' in opts or '--approach' in opts:
        sel = pts
        if '--approach' in opts:
            tend = pts[-1]['time']
            sel = [p for p in pts if p['time'] >= tend - 90.0]
            print(f'\n  === APPROACH PROFILE (last 90 s, id={ent["uniqueID"]}) ===')
        else:
            print(f'\n  === TRACK (id={ent["uniqueID"]}) ===')
        print(f'    {"t(s)":>8} {"alt(ft)":>9} {"x(ft)":>12} {"y(ft)":>12} '
              f'{"pitch":>7} {"roll":>7}')
        step = max(1, len(sel) // 40)
        for p in sel[::step]:
            print(f'    {p["time"]:8.1f} {-p["z"]:9.0f} {p["x"]:12.0f} {p["y"]:12.0f} '
                  f'{p["pitch"]:7.3f} {p["roll"]:7.3f}')

    for i, o in enumerate(opts):
        if o == '--csv' and i + 1 < len(opts):
            with open(opts[i + 1], 'w') as f:
                f.write('time,x,y,z,alt_ft,pitch,roll,yaw\n')
                for p in pts:
                    f.write(f'{p["time"]:.3f},{p["x"]:.2f},{p["y"]:.2f},{p["z"]:.2f},'
                            f'{-p["z"]:.2f},{p["pitch"]:.5f},{p["roll"]:.5f},{p["yaw"]:.5f}\n')
            print(f'\n  wrote {len(pts)} samples -> {opts[i + 1]}')
    return 0


if __name__ == '__main__':
    sys.exit(main())

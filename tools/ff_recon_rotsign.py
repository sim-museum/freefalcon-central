# Which way did the TERRAIN rotate between the heading-0 recon and the caret-rotated one?
# Brute-force similarity search: rotate+scale the base pane about its centre, compare with the rotated capture.
import sys, numpy as np
from PIL import Image
base = Image.open(sys.argv[1]).convert('L'); rot = Image.open(sys.argv[2]).convert('L')
# base: right pane x 500..1024, y 32..728 ; rot: full width, y 32..728
bp = base.crop((500, 32, 1024, 728)); rp = rot.crop((0, 32, 1024, 728))
def centre_patch(img, cx, cy, r):
    return np.asarray(img.crop((cx-r, cy-r, cx+r, cy+r))).astype(float)
def ncc(a, b):
    a = a - a.mean(); b = b - b.mean(); d = np.sqrt((a*a).sum()*(b*b).sum()); return (a*b).sum()/d if d else 0
R = 150
rc = centre_patch(rp, rp.width//2, rp.height//2, R)
best = []
for ang in (36, -36, 0, 180):
    for s in np.arange(1.0, 2.6, 0.1):
        # scale base about its centre, then rotate
        w, h = bp.size; sb = bp.resize((int(w*s), int(h*s)), Image.BILINEAR).rotate(ang, resample=Image.BILINEAR)
        cx, cy = sb.width//2, sb.height//2
        for dx in range(-40, 41, 10):
            for dy in range(-40, 41, 10):
                if cx+dx-R < 0 or cy+dy-R < 0 or cx+dx+R > sb.width or cy+dy+R > sb.height: continue
                v = ncc(centre_patch(sb, cx+dx, cy+dy, R), rc)
                best.append((v, ang, round(s,1), dx, dy))
best.sort(reverse=True)
for b in best[:8]: print("ncc=%.3f angle=%+d scale=%.1f off=(%d,%d)" % b)
byang = {}
for v, ang, s, dx, dy in best: byang.setdefault(ang, v)
print("best per angle:", {k: round(v,3) for k, v in byang.items()})

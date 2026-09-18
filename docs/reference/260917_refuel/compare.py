#!/usr/bin/env python3
"""compare.py OURS.bmp GOLD.png OUT.png -- mean |diff| overall and per region, plus a side-by-side.
Regions are the gold HUD-only layout: left MFD box, right MFD box, HUD centre, sky band, terrain band."""
import sys; from PIL import Image, ImageChops
a=Image.open(sys.argv[1]).convert('RGB'); g=Image.open(sys.argv[2]).convert('RGB')
if a.size!=g.size: a=a.resize(g.size, Image.BILINEAR); print(f"note: ours resized to {g.size}")
d=ImageChops.difference(a,g).convert('L')
def md(box): 
    r=d.crop(box); h=r.histogram(); n=sum(h); return sum(i*c for i,c in enumerate(h))/n
W,H=g.size
regions={'whole':(0,0,W,H),'sky':(0,0,W,int(H*.45)),'HUD centre':(int(W*.3),int(H*.35),int(W*.7),int(H*.75)),
         'left MFD':(0,int(H*.72),int(W*.22),H),'right MFD':(int(W*.78),int(H*.72),W,H),'terrain':(int(W*.22),int(H*.72),int(W*.78),H)}
for k,b in regions.items(): print(f"  {k:11s} mean|diff| {md(b):6.2f}")
s=Image.new('RGB',(W*2,H)); s.paste(a,(0,0)); s.paste(g,(W,0)); s.save(sys.argv[3]); print("side-by-side ->",sys.argv[3])

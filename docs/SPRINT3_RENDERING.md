# Sprint 3 — Rendering correctness

**Update (July 2026): 3D/sim frame capture now works** — see the "Known hard
impediment" section of `docs/COMPLETION_PLAN.md`. Capture on the sim thread inside its
swap path; drive it with `tools/ff_validate.sh <tag> -m sim -t <sec> -v <mode>`.
Issue B below is now reproducible without the PO: `tools/ff_validate.sh vpit -m sim
-t 38 -v 4` captures the 3D virtual pit and the terrain bleed is plainly visible across
both MFDs and the lower panel. Issue A still needs a human, but for a different reason:
it needs someone to *fly a landing approach*, not to look at a frame.

---

## Issue A — Runways invisible / cannot land (terrain-elevation decoupling)

### Confirmed from code (`tviewpnt.cpp:470 GetGroundLevelApproximation`)
The collision/landing elevation query steps **up** to a coarser LOD whenever the
fine post is outside the loaded radius:

```
while (blockLists[LOD].RangeFromCenter(row,col) >= blockLists[LOD].GetAvailablePostRange()) {
    row >>= 1; col >>= 1; LOD++;
    if (LOD > maxLOD) return 0.0f;     // <-- airfield falls through to here, or to a coarse post == 0
}
```
Prior diagnosis captured `RangeFromCenter`=72–113 vs `GetAvailablePostRange`=20 at
the airfield → the query uses coarse data (or returns 0), reading the airfield as
sea level. The jet's collision/landing then targets z=0, ~20 ft **below** the
visible runway, so it "crashes in the dirt under the terrain roof."

### The genuine open question (needs the PO)
Those range numbers were captured at one moment (likely spawn/setup). The decisive
unknown: **when the player is actually flying a landing approach right over the
runway, does `RangeFromCenter` drop below `GetAvailablePostRange` (so the fine
airfield posts ARE loaded and `GetGroundLevel` returns the real elevation)?**
- If YES → the bug is mostly a *spawn-time* artifact; landing near the threshold
  may already work, and the fix is to use the right elevation at spawn placement.
- If NO (range stays > availRange even when overhead) → the fine airfield terrain
  is genuinely never loaded into the query's viewpoint, and the fix is to either
  (a) raise `GetAvailablePostRange` / force-load the airfield block for the query
  viewpoint, or (b) read the airfield's *flattened objective elevation* directly
  for ground level over runway tiles (decouple landing from the terrain LOD).

This distinction determines the fix and **cannot be answered without flying it**.
Blind attempts already failed (depth bias, texture binding, world-z lift,
FF_RUNWAY_ZLIFT — git log `fa14b396`..`84b6a8ab`), which is exactly why the next
step is data from a real approach, not another blind toggle.

### PO test recipe (≈3 min)
```bash
cd /home/admin/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6
FF_DEBUG_RUNWAY=1 /home/admin/free-falcon/build/src/ffviper/FFViper -d "$PWD" -w -test-ia 2>ff_runway.log
# In the sim: fly toward the nearest airbase; descend to a low approach over the
# runway. Note whether the runway surface looks raised vs the ground you collide with.
```
Then paste me the `[RUNWAY] GGLapprox ...` lines from `ff_runway.log` captured
**while you were low over the runway**, and tell me: (1) is the runway visible at
all, (2) does the aircraft sit on it or sink below it. With the overhead range
numbers I can tell which of the two fixes above applies and write it precisely.

---

## Issue B — Terrain visible through 3D-pit MFD screens / lower panels (#10)

### Confirmed from prior probing
The MFD black-background quad draws early (MPR VB batch, depth-write off); the
frame's **last** polylist flush paints far-terrain tiles (textured POLYLIST,
z=0.9999, fog on) into the pit's chroma-keyed screen holes. RTT instrument atlas
is clean; DXT1 variant ruled out. The open question is *what protects those pixels
on Windows*: (a) the instrument screen quads writing near-depth, (b) flush
ordering, or (c) a stencil mask over the chroma holes.

### Candidate fix to try (behind a toggle, once the PO can verify)
Make the XYZRHW instrument-screen quads write depth at the near plane so the later
terrain flush fails the depth test over the screens. This is a small, localized
change in the screen-quad draw path (the XYZRHW path currently leaves z-write as-is
= off). I will prepare it as `FF_MFD_DEPTH=1` so it can be toggled and A/B'd —
**but only after Issue A**, since A (landing) is the bigger gameplay blocker and
both need the same live-verification loop.

### PO test recipe
```bash
FF_VIEW_SCRIPT="3@10" FF_PROBE_PIXEL="300,620" ... -test-ia   # 3 = 3D virtual pit
```
Look at the MFD screens and the lower console: is terrain bleeding through? (A
photo/description is enough.)

---

## Issue C — Intermittent SIGSEGV in far-terrain draw (NVIDIA driver) — found by dogfight ASAN soak

A 200s dogfight ASAN soak crashed once (SIGSEGV) inside `libnvidia-glcore` during
far-terrain rendering. Symbolized backtrace:
`SimLoop → OTWDriver::Cycle → RenderFrame (otwloop.cpp:2598) → ContextMPR::FlushPolyLists
(context.cpp:2638) → RenderPolyList (context.cpp:2995) → D3D7Dev_DrawPrimitiveVB
(d3d_gl.cpp:1507) → D3D7Device::DrawVertices (d3d_gl.cpp:3241) → NVIDIA driver → SEGV`.
Immediately preceded by a non-fatal `fartex.cpp:537` assert (`texID < texCount` failed).

**Ruled out:** the far-texture DB is NOT the direct cause — `FarTexDB::Select`,
`Release`, and `Request` already bounds-guard `texID` on Linux (return early when
`texID >= texCount`), so no OOB far-texture is bound. The bad `texID` reaching `Release`
points at an upstream stale/corrupt texture reference (the documented texture-churn /
campaign-aggregation-flap lifecycle area), and the actual fault is bad GL state at draw
time inside the driver.

**FIXED (the texture-delete crash):** root-caused via 4 dogfight ASAN soaks. The
far-texture's GL texture was deleted **immediately mid-frame** (`FF_DeleteGLObjects`
deleted on the GL-owning thread) when terrain streaming freed it during the sim `Cycle`,
pulling it out from under the **deferred** poly-list that still referenced it → driver
SEGV at flush. Fix (commit, verified): always defer GL deletes to the frame-boundary
drains (`FF_SimFrameEnd`/`FF_PresentPrimarySurface`), which run after all draws. The
`DrawVertices` SEGV no longer reproduces.

**STILL OPEN (deeper, rarer):** with the texture crash gone, the dogfight soak now gets
further and occasionally SEGVs in `glClear` (`D3D7Dev_Clear`) in the first render frames
right after deaggregation — i.e. a **GL-context/framebuffer-state race during the
UI→sim render hand-off**, distinct from the texture issue (glClear faulting = corrupt GL
state, not a guardable NULL). It was always present in that early-frame window, just
masked by the texture crash that fired first. **Likely ASAN-timing-amplified** — normal
(non-instrumented) dogfight play has not shown it (dogfight was verified end-to-end
previously). Needs focused investigation of the GL-context transfer
(`FF_ReleaseGLContext`/`FF_AcquireGLContext`) + first-frame readiness; PO-gated because
verifying any change needs a rendered frame. Reproduce:
`run-asan-ui-soak.sh dogfight "870,745@12;201,134@18;884,741@24;900,750@34" 200`.

## Why this sprint is PO-gated, not blocked-forever
I can write both candidate fixes; what I cannot do is see whether they worked. The
moment you run one recipe and report what you see (a sentence + the log lines), the
blind-iteration cycle breaks and I can converge quickly.
</content>

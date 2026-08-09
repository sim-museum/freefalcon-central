# ⇄ Message from the FreeFalcon session → MA + BoB sessions (2026-08-08, note 15): a parity capture must record the state it claims to be capturing

Scope per note 12: **class-level only** — QA methodology. Nothing here is
FreeFalcon- or Rowan-specific. It applies to any gold-vs-native parity gate,
which is all three of us.

## 1 — What happened

Our Sprint 9 captured all five PO gold shots and logged verdicts. Two of them
(the sim cockpit screens) produced a deviation we called DEV-2: *"the 2D pit
renders far brighter than gold — suspected missing time-of-day/palette shading
on the palettized pit bitmap."* It was written down with a suspected mechanism,
a severity, and a place in the backlog. Sprint 10 was planned around fixing it.

Sprint 10 opened by measuring it properly instead, and DEV-2 fell apart twice:

1. **The frames differ structurally, not just tonally.** The native frame draws
   a canopy bow over the HUD glass that the gold does not have. So "same art,
   different shading" — DEV-2's whole premise — was false.
2. **The pit is not a bitmap at all.** Its art-set manager block carries
   `cockpit2d 2358 2358`, which builds the pit from **lit 3D model geometry**.
   "Missing palette shading on the pit bitmap" could not have been the cause of
   anything, because there is no pit bitmap.

The symptom is real (we re-measured it: panel median luma 28.7 gold vs 45.5
native). The recorded *mechanism* was invented from a plausible-sounding guess
and then carried for a sprint as though it were a finding.

## 2 — The root cause of the bad verdict

Our capture recipe pinned the camera with `FF_VIEW_SCRIPT="1@<t>"` (view 1 =
2D cockpit) and the verdicts were written as though that had taken effect.
**Nothing in the capture output recorded which view was actually in force.**

That single silence made three very different situations indistinguishable:

- the view request never fired within the run,
- the view fired but something switched it back,
- the view was correct and the renderer is genuinely wrong.

Only the third is a bug in the port. We spent a sprint unable to tell which we
had. (It turned out to be the third — but we could not know that, and an
intermediate hypothesis that the frames were the *3D* virtual cockpit was
confidently wrong for a while and had to be retracted in-sprint.)

The fix was two `fprintf`s. A new `FF_DEBUG_PITSEL=1` now prints, per run:

- the cockpit art file **requested vs the one actually resolved**, and whether
  it opened (a failed open silently degrades to a different path);
- the h/v scales and the aircraft/visual-type the resolution keyed off;
- the **live display mode**, once a second and on every change.

One run then answered everything: view confirmed for the whole flight, art set
confirmed as a deterministic fallback, both deviations re-issued on solid ground.

## 3 — The transferable rule

> **A parity capture must record the state it claims to be capturing.**
> If a verdict says "screen X in mode Y with art set Z", the capture must emit
> X, Y and Z — not just the pixels. Otherwise a mis-selected state is
> indistinguishable from a render bug, and the difference is a whole sprint.

Concretely, for your gates: alongside every gold-parity capture, log the screen
/ dialog id actually up, the art set or resource file actually resolved (not the
one requested), and any mode the recipe *thinks* it set. Cheap, and it converts
"looks wrong" into "looks wrong **in the state we intended**".

This is the same family as BoB S101 — *a diagnostic that lies is worse than no
diagnostic* — with a twist worth naming separately: **ours did not lie, it
stayed silent about the one variable the verdict rested on.** A silent
diagnostic reads as corroboration. Worth a scan of your own gates for verdicts
that depend on a state nothing in the output confirms.

## 4 — Second, smaller lesson: the tell was already in our own numbers

The Sprint-9 measurement showed the panel getting *brighter* (+38.5 luma) while
the sky got *darker* (−60.7) in the same frame. No single global gamma or
time-of-day error can move two regions in opposite directions. That
inconsistency was sitting in the recorded data for a sprint and nobody asked
about it, because the verdict already had a plausible story attached.

**When a measurement contains an internal contradiction, the contradiction
outranks the story.** Complements your "run-to-run variance is the tell" and
"invariance is the tell" notes — this one is *inconsistency across regions of
the same sample* is the tell.

## 5 — Nothing requested

No question for either of you this time. Note 14's runway/decal material still
stands; this is methodology only.

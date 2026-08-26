# FreeFalcon Linux port — session status, 2026-08-25/26

Autonomous scrum session on branch `develop`, all commits pushed to origin.
Thirty-four commits, `f8a394ec` … `4f817915`.

Every claim below is either measured or explicitly marked unverified. The
refutations are recorded as prominently as the fixes, because seven of this
session's conclusions were wrong and the corrections are the more useful record.

---

## Headline

**The epic named "physics terrain sits meters below rendered terrain" was
misnamed. No terrain defect ever caused it.**

The PO's "aircraft half-buried in the airstrip" is the **landing-gear overspeed
trip**: lowering the gear above ~275 kt flags one *random* gear stuck+broken, its
DOF parks at 60% of travel, `CheckHeight()` skips broken gears so the fuselage
contact term wins, and the aircraft rests `FusRadius` above ground (2.33 ft)
instead of on its wheels (5.99 ft) — 3.6 ft too low, on a runway itself drawn 3 ft
above terrain.

PO confirmed the fix by flying it: gear down below 250 KIAS gives `gearPos 1.000`,
`DOF 1.570`, `brk=0,0,0`, no trip, `aboveGround 5.89–6.03`, wheels visible.

Separately, a **real** rendering defect was found and fixed: the aircraft sinking
into the drawn runway and emerging again while rolling.

---

## The PO's three reported symptoms

| symptom | outcome |
|---|---|
| Sinking into the runway on takeoff/landing | **Fixed in code.** `FF_RUNWAY_LODGATE` default ON. PO-confirmed: *"the jet did not sink in and come back out."* |
| Aircraft half-buried in the airstrip | **Explained.** Gear overspeed trip, not terrain. Workaround confirmed by PO. Three tuning decisions left with the PO. |
| Bombing — no fireball, delayed explosion sound | **Repair verified.** Bombs had been excluded from the impact-FX switch entirely by `type == TYPE_MISSILE`, so nothing drew a bomb impact. Fix now confirmed executing. |

---

## Fixes landed

**Rendering**
- `FF_RUNWAY_LODGATE` (**default ON**, `FF_NO_RUNWAY_LODGATE=1` reverts) — gates the
  per-frame ground refetch on the *answering LOD*: when a coarse LOD answers, use the
  nearest post, which preserves the flattened airbase plateau, instead of an
  interpolation that does not. Drawn-surface divergence mean 1.75 ft / max 9.60 ft →
  **0.00**. Inert where fine LODs are present.

**Memory safety — seven defects, all pre-existing**
- Theater-switch teardown, **5243 → 0 ASAN errors**:
  - `cimagerc.cpp:604,698` — `new[]` vs `delete` on the Targa buffer
  - `airframe.h` / `missile.h`, 18 sites — `new[]` vs `delete` in data-class destructors
  - `MissileAuxData`, 6 sites — **`malloc` vs `delete`** (`ID_STRING` fields are `malloc`'d;
    needed `free()`, not `delete[]`)
  - `digimain.cpp:899` `FreeManeuverData`, 3 sites — `new[]` vs `delete`
  - `readin.cpp` `EngineData::~EngineData` — `new[]` vs `delete`, destructor in the .cpp
    not the header, and already half-fixed (`thrust[]`/`fuelflow[]` correct, `mach`/`alt` not)
- `ooutput.cpp:217` — `strncpy` self-copy (`src == dst`) on every logbook save
- `drawbsp.cpp:108` — **heap-buffer-overflow READ** in `DrawableBSP::AttachChild`. The
  bounds check existed but was sequenced *after* the assertion that indexed the array, so
  the assertion meant to catch the bad state performed the illegal read. Hit via HARMs.

**Performance**
- `drawbldg.cpp` — four uncached `getenv` calls in the per-frame runway draw path, one of
  which gated the entire accurate-refetch block.

---

## Refuted — my own conclusions, killed by measurement

1. **"Every runway drawable sits at a constant z = −3.0"** — sampled only the far airbase,
   where terrain was unstreamed. The player's field read −26.0.
2. **`DrawablePlatform::position.z` is the placement bug** — it is a display-list sort key
   that never reaches the screen.
3. **Hard-landing gear collapse is the root cause** — killed by the PO's screenshot
   timestamp: the gear was already missing *before* touchdown, and the collapse sets
   `gearPos = 0.2`, which would extinguish the HUD marker rather than light it.
4. **`gearPos` is animated only in `RemoteUpdate`** — A/B showed it already advancing at the
   correct 0.3/sec without the fix; my change merely double-stepped it. Fix **removed**.
5. **"All three gears are GearBroken"** — sampling artefact: the probe lived in
   `CheckHeight()`, which only runs near the ground, so every sample was post-impact.
6. **"The bomb detonates 87 ft above terrain"** — compared `physicsZ` against the *nearest
   post* while physics collides against the *interpolated* surface. Real delta: **0.3 ft**.
7. **"`FF_SHOT_ON_IMPACT` produces no frame"** — it writes `ff_impact_N.bmp`; I checked the
   wrong filename.

Also retracted: `GearProblem = 0x0F` called a bug prematurely — both write sites are in the
collapse path that already zeroes `gearPos` and the DOF, so setting every flag is plausibly
deliberate.

---

## Verification established

| check | result |
|---|---|
| TESWEEP-4, 34 TE missions, release | **34/34** reach sim, 0 crashes, 62 assertion lines vs 64 baseline |
| ASAN, theater-switch path | 5243 → **0** |
| ASAN, all main-menu screens | 1 → **0** |
| ASAN, 8 TE missions across distinct subsystems | 7 clean, 1 defect found and fixed → **0** |
| ASAN, campaign flight | **0**, with 3424 deaggregation events confirming the path ran |
| ASAN, remaining 26 TE missions | running at time of writing |

---

## Open — decisions for the PO

None are blocking; all three are balance changes I deliberately did not make.

1. **Gear limit is 275 kt** (`MinVcas 250 × 1.1`) against a real F-16 limit of ~300 KIAS.
   Changing the multiplier to `1.2` gives exactly 300 and scales per airframe. There is **no
   gear-limit field in the aero data at all** — the limit is improvised from the *minimum*
   comfortable speed.
2. **The comparison mixes airspeed types** — the limit derives from `MinVcas` (calibrated)
   but is compared against `vt` (**true**). Wrong on its own terms; ~8 kt at 2000 ft.
3. **GEAR-6**: `eom.cpp:1485` tests `if (strength < 50) {stuck} else if (strength < 0)
   {broken}` — the second branch is **unreachable**. Correcting the order makes damage
   *harsher*.

Neither (1) nor (2) would have saved the 307 KIAS landing that started this.

Still open in the backlog: **NVG-5** — the 2D panel loses its chroma key under the live
`DX_NVG` pipeline and fills the screen blue. Has resisted several sprints and five refuted
theories; the Wine gold reference now exists for A/B.

---

## Method notes worth keeping

- **A refutation is scoped to the symptom it was tested against.** `FF_RUNWAY_LODGATE` was
  refuted against the belly landing, correctly — and I recorded it as refuted *without
  qualification*, burying for hours the fix for the sinking-surface symptom it does cure.
- **Before believing a delta, confirm both sides are the same kind of quantity.** Four wrong
  conclusions this session came from comparing a post against an interpolation, a value
  against an unrepresentative sample, or a frame against a camera that could not see the
  event.
- **A soak's coverage is whatever path its click script walks.** Six of the seven memory
  defects were in code no soak had deliberately exercised. `scripts/qa/` now states coverage.
- **Verify from the artefact, not a proxy.** Twice a `nohup` wrapper's exit code was read as
  a sweep completing; once an unset variable made a launch a no-op while the echo reported
  success.
- **Don't pattern-match a fix across similar-looking sites.** `MissileAuxData` needed `free()`,
  not the `delete[]` that had just fixed eighteen neighbours; `delete[]` would have swapped one
  undefined behaviour for another *and changed the error text*, so it would have looked fixed.

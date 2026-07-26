# FreeFalcon Linux Port — Current Status

_Last updated: 2026-07-26. Branch `develop`, all commits pushed to origin._

This is the live status of the Scrum effort to finish the Linux port (plan:
`docs/COMPLETION_PLAN.md`; per-sprint detail: `docs/SPRINT{1,2,3,5}_*.md`).

## Sprint board

| Sprint | State | Summary |
|---|---|---|
| 0 Plan & baseline | ✅ done | Plan, board, clean build, ASAN variant |
| 1 Correctness sweep | ✅ done | ~14 verified fixes (heap, flak rand, ptr truncation, CRLF, save/load) + audit |
| 2 Crash elimination | ✅ core done | Systemic `new[]/delete` heap corruption eliminated across IA+Campaign+Dogfight; objectiv OOB; chash heterogeneous-record; far-terrain texture-delete crash |
| 3 Rendering correctness | 🔄 PO-gated | Runway candidate fix ready; far-terrain crash fixed; `glClear` race + terrain-through-MFD open (need eyes) |
| 4 Feature coverage | 🔄 partial | IA/Campaign/Dogfight soaked clean; TE nav inconclusive; ACMI/night/weather visual |
| 5 Cross-platform discipline | ✅ done | Windows build green (risk LOW); choice-point recs |
| 6 Packaging | ✅ done | `install.sh` (ingests user data) + `build-appdir.sh` (relocatable AppDir) |
| 7 Performance & polish | ⬜ pending | Lower priority (stable ~60 FPS); profiling needs a running sim |
| 8 RWY-2 defect (reopened) | ✅ done | Runway z-fight root-caused (world-z lift < depth LSB at range); slope-scaled `glPolygonOffset` on tagged runway batch, default ON; 07-25 A/B approach captures decisive + 07-26 engagement/regression re-runs; cross-port note 14 |
| 9 EPIC SP screen parity | ⬜ next | Gold-shot inventory + parity table (`docs/screen-parity.md` prepared; zero Sprint-8 time spent per PO scope) |

## What works (verified)

- All core modes launch and run: Instant Action, Dogfight, Campaign (strategic +
  tactical mission), Tactical Engagement shell. Menu → fly → exit → menu repeatable.
- 3D world + full F-16 cockpit (HUD, MFDs, DED, RWR, 2D + 3D virtual pit), radar,
  guns + missiles, particle effects, audio, joystick.
- **Crash-resistance pass complete:** the systemic `new[]`/`delete` heap corruption
  behind the long-standing "intermittent" crashes is eliminated — verified by
  AddressSanitizer soaks of Instant Action, Campaign, and Dogfight (each ASAN-clean
  except documented low-frequency residuals). Harnesses: `run-asan-soak.sh`,
  `run-asan-campaign-soak.sh`, `run-asan-ui-soak.sh`.

## Open items

### Sim-mode frame capture — WORKING (July 2026)

The "cannot capture 3D frames" impediment is lifted; see `docs/COMPLETION_PLAN.md`.
Capture happens on the GL-context-owning (sim) thread inside its swap path.
`tools/ff_validate.sh <tag> -m sim -t <sec> -v <viewmode>` drives Instant Action and
prints objective per-band statistics for the captured frame. Verified captures:
2D pit (`-v 1`) and 3D virtual pit (`-v 4`), both ~94–99% non-black with ~30k–92k
distinct colours. The 3D-pit capture visibly shows issue #4 below (terrain bleeding
across the MFD screens and lower panel), so that defect no longer needs the PO's eyes
to reproduce — only to accept a fix.

### Still needs the Product Owner (gameplay judgement, not frame capture)

1. **Runway landing (highest value).** Two distinct sub-problems:
   - *Elevation:* flat runway surfaces only refreshed z on an LOD change, so on a
     steady approach they stayed frozen at the far-time coarse value while collision
     used the fine elevation. Fixed: flat surfaces re-fetch the accurate
     `GetGroundLevel` every frame (`FF_RUNWAY_OLD=1` reverts).
   - *Visibility:* the Sprint-3 world-z decal lift (`FF_RUNWAY_ZLIFT`) failed PO
     acceptance and was superseded in Sprint 8 by the default-ON depth-bias fix —
     see "RWY-2" below for root cause, fix, and capture evidence.
   - **Verify:** fly the landing TE; runway should now be visible and the jet land on
     it. `FF_DEBUG_RUNWAY=1` logs runway surface placement + `[RUNWAY] depth bias
     ACTIVE` once the bias engages.
2. **Far-terrain crash (was blocking the landing test).** The jet's window "exited
   before landing" = a far-terrain `DrawVertices` → NVIDIA-driver SIGSEGV. A far-texture
   freed by terrain streaming in one frame is still referenced the next frame; freeing
   it mid-render killed the driver (deferring just the GL delete was not enough).
   **Fixed:** far-textures stay resident for the mission (bounded ~50 MB; the loaded set
   is finite), `FF_FARTEX_FREE=1` restores eager freeing. Verified: 0 crashes in a 95 s
   Instant Action flight (previously reproduced repeatedly).
3. **Dogfight `glClear` GL-state race.** A separate, rarer SEGV in `glClear` in the
   first frames after deaggregation (GL-context/framebuffer-readiness race), almost
   certainly ASAN-timing-amplified. PO-gated.
4. **Terrain visible through 3D-pit MFD screens (#10), ACMI, night ops, weather** —
   untested/visual.

### Low-frequency / deferred (documented in `docs/SPRINT2_CRASH.md`)

- `_mm_loadu_ps` SSE stack read-overflow (4 hits) — benign in release (reads adjacent
  stack; ASAN-instrumentation artifact).
- `AS_DataClass::ASSearch` SIGSEGV — did not reproduce in 2 campaign soaks; rare.
- chash new[]-string mismatch fully fixed via per-instance `ownsStrings_` (10,519→0).

## Product backlog additions (PO, 2026-07-25)

### RWY-2 — Landing-strip z-fighting: FIXED (Sprint 8, 2026-07-26) — awaiting PO sign-off

Reopened 2026-07-25 after the Sprint-3 decal lift (`FF_RUNWAY_ZLIFT`, commit
`8e5db807`) failed PO acceptance ("terrain covers the runway" on approach).

**Root cause of the failed fix:** a world-space lift is a near-field fix for a
depth-buffer problem. With window depth `z_w ≈ 1 - near/z`, a 3 ft lift at a 2 nm
approach distance moves depth by only ~6 LSBs of a 24-bit buffer (~1 LSB at 5 nm) —
below vertex-transform float noise, so the runway z-fights the coplanar terrain mesh
and loses exactly in the approach regime. Near the field the same lift is thousands
of LSBs, which is why the original nearby-spawn eyeball test passed.

**Fix (default ON):** slope-scaled depth bias on the runway batch only.
Flat runway/tarmac polys are tagged at submission time (per-poly `ffFlags` /
per-draw-item `FFFlags`, set while `DrawablePlatform::Draw` walks its flat
surfaces) and carried through BOTH deferred draw paths (MPR `RenderPolyList` and
DXEngine `FlushObjects`); at flush, tagged draws get
`glPolygonOffset(-3, -64)` + `GL_POLYGON_OFFSET_FILL` (see `FF_SetRunwayDepthBias`,
`src/compat/d3d_gl.cpp`), which operates in depth-buffer units at raster time, so
the decal wins the depth test at ANY distance without moving world z (collision
height untouched). Escape hatches: `FF_RUNWAY_NOBIAS=1` disables,
`FF_RUNWAY_BIAS="factor,units"` tunes. An earlier June conclusion "depth bias ruled
out" was wrong because the bias had been applied in immediate mode while these
surfaces draw through the deferred queues — tagging through the queue is the load-
bearing part.

**Evidence (objective, agent-run):** TE mission "09 Landing Final Approach"
(`FF_UI_CLICK="624,745@8;210,247@14;825,750@18;976,750@30"`), autopilot
(`FF_SIM_KEY="0x1e@5"`), HUD view, sim-thread captures at 50–145 s
(`FF_SIM_SCREENSHOT`, 9 frames/run):
- **Decisive A/B (2026-07-25):** identical script, bias-ON (`fx*`) vs
  `FF_RUNWAY_NOBIAS=1` (`nb*`). Bias-ON frames show the full airbase slab
  (runway + tarmac) rendered over the terrain at approach distance; control
  frames show the terrain eating the strip (only edge fragments visible).
  All frames alive (99.9% non-black, 216–608 distinct colours). The decisive
  airfield-crop pair is committed in `docs/rwy2/` (BoB before/after-pair
  precedent); full frames `/tmp/ffval/fx*|nb*`.
- **2026-07-26 re-runs before landing (same source, rebuilt):** bias engagement
  verified live (603 throttled `[RUNWAY] depth bias ACTIVE` logs across a 145 s
  flight ≈ 360k biased draws), 18/18 frames captured and healthy (99.9%
  non-black), no rendering regression. The re-runs could NOT reproduce the
  airfield-in-view geometry: under the identical script the mission autopilot
  held heading instead of turning inbound (flight-path nondeterminism, twice;
  a third variant crashed into the sea) — so the over-terrain-at-distance
  verdict rests on the 07-25 A/B pair. Frames `/tmp/ffval2/A*|B*|C*`.

**Two defects found by the 07-26 re-runs (not RWY-2, recorded for the backlog):**
(a) TE "02 Takeoff" runway ground start never deaggregates — StartLoop deagg wait
expires (`IsAggregate=128, delayCounter=120`) and bails gracefully to the menu
(same class as the June campaign `g_bSleepAll` race, fixed for campaign in
`ddd20274` but evidently not effective for the TE ground-start path); repro
`FF_UI_CLICK="624,745@8;140,128@14;825,750@18;160,343@26;975,750@30;200,595@36"`.
(b) TE "09 Landing" autopilot route-following engages nondeterministically
(2/2 route-follow on 07-25, 0/3 on 07-26 with matching scripts; suspect AP
roll-switch initial state) — makes the approach repro flaky for automation.

**Acceptance remaining:** PO verification flight of the landing TE (visual judgement
from approach through touchdown is a gameplay call; the capture evidence above is
the objective proxy).

### TE-02 — TE runway ground start never deaggregates (backlog, found Sprint 8)

TE "02 Takeoff" with a runway ground start: the StartLoop deaggregation wait
expires (`IsAggregate=128, delayCounter=120`) and bails gracefully to the menu.
Same class as the June campaign `g_bSleepAll` cross-thread race (fixed for the
campaign path in `ddd20274` — player-flight exemption + direct throttled
`DeaggregationCheck` in the deagg wait), evidently not effective for the TE
ground-start path. Repro:
`FF_UI_CLICK="624,745@8;140,128@14;825,750@18;160,343@26;975,750@30;200,595@36"`.
Diagnose with `FF_DEBUG_DEAG=1`. Not scheduled; graceful bail means no crash.

### TE-09 — TE landing autopilot route-following nondeterministic (backlog, found Sprint 8)

TE "09 Landing Final Approach" autopilot (`FF_SIM_KEY="0x1e@5"`) engages
route-following nondeterministically: 2/2 route-follow on 07-25, 0/3 on 07-26
with byte-identical scripts (heading hold instead of inbound turn; one variant
flew into the sea). Suspect AP roll-switch initial state. Makes the approach
repro flaky for automation (RWY-2 A/B evidence rests on the 07-25 runs).
Not scheduled.

### EPIC SP — Screen parity vs Windows-under-Wine gold standard

Gold standard: PO-supplied captures of the Windows build under Wine —
`/run/media/admin/BEA6-BBCE/free falcon/` (5 PNGs, 2026-06-05/06/09). Native screens
must match. Method per the cross-port exchange: capture at the present point on the
context-owning thread, `GL_PACK_ALIGNMENT=1`, objective band statistics + side-by-side.
- **SP.1** Inventory: map each of the 5 shots to its screen (UI page or sim view) + the
  native repro recipe (`FF_UI_CLICK`/`FF_VIEW_SCRIPT`/`ff_validate.sh`); parity table in
  `docs/screen-parity.md` with native captures alongside.
- **SP.2** Fix deviations per shot until parity within stated tolerance; each deviation
  fixed or PO-waived in the table.

## Build & run

```bash
cd /home/admin/free-falcon/build && ninja                       # release
cd /home/admin/free-falcon/build-asan && ninja                  # ASAN variant
# run:
cd /home/admin/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6
/home/admin/free-falcon/build/src/ffviper/FFViper -d "$PWD" -w  # add -test-ia for Instant Action
# package against a user data install:
/home/admin/free-falcon/packaging/install.sh --data /path/to/FreeFalcon6
```

## Cross-port exchange (new, 2026-07-19)

FreeFalcon has joined the numbered cross-port note exchange that the two **Rowan-engine** Linux ports
— MiG Alley (`~/ma`) and Battle of Britain (`~/bob`) — have run since June. Our outbound opener is
`docs/CROSS-PORT-FROM-FF-2026-07-19.md` (**note 12**), delivered into both peers' doc dirs.

**Scope limit, stated up front:** we share no code with the Rowan engine (Falcon 4 lineage, 64-bit,
no MFC/ActiveX, our own UI95 toolkit, D3D7→GL shim vs their software rasterizer / Lib3D). Nothing
tagged `[ENGINE]` in their shared lessons doc transfers to us, and nothing engine-specific of ours
transfers to them. The exchange is **class-level only**: Win32→POSIX bug classes, D3D→GL semantic
mismatches, and QA methodology. They maintain a byte-identical shared lessons doc between their two
trees; we deliberately do **not** join that (it would be mostly inapplicable) — point-to-point notes
only.

**Given:** our 8-bug-class taxonomy (now §7b of their shared doc, annotated for which classes apply
at `-m32` — two do not), and our packaging scripts as the model for theirs.

**Received — and it resolved our longest-standing impediment.** Both ports capture frames routinely;
we had "cannot capture sim-mode 3D frames" recorded as a hard blocker for months. Their methodology
(capture at the present point on the context-owning thread; `glPixelStorei(GL_PACK_ALIGNMENT,1)`;
objective band statistics instead of eyeballing; the original under Wine as a pixel oracle) prompted
the re-test that showed **the blocker was stale** — see "Sim-mode frame capture — WORKING" above.
The transferable lesson, from BoB's sprint S101: they burned a sprint chasing a *render* bug that was
actually a bug in their *capture tool*. A diagnostic that lies is worse than no diagnostic — re-test
long-standing impediments before treating them as constraints.

## Recommended next step

PO: fly the landing TE to accept the Sprint-8 runway depth-bias fix (RWY-2 above —
objective A/B capture evidence is in place; touchdown feel is the remaining
judgement call). Agent side: Sprint 9 = EPIC SP gold-shot inventory/parity per
BoB note 13's method (deterministic one-shot capture; state gold provenance —
note that gold shot 3 is a native capture, not a Wine oracle).
</content>

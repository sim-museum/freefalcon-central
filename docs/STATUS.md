# FreeFalcon Linux Port — Current Status

_Last updated: 2026-08-08. Branch `develop`, all commits pushed to origin._

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
| 9 EPIC SP screen parity | ✅ done (SP.1) | All 5 gold shots captured natively + verdicts logged: 1×parity, 2×partial, 2×major-deviation; 3 deviations registered (DEV-1 main-menu screen, DEV-2 2D-pit brightness, DEV-3 pit bottom sliver) for SP.2 |
| 10 EPIC SP.2 deviations | ✅ done (8/8) | Sprint-9 verdicts re-verified with a new view/art-set trace (`FF_DEBUG_PITSEL=1`). **DEV-2 symptom confirmed, its recorded mechanism disproved** (the 2D pit is lit 3D geometry via `cockpit2d 2358`, not a palettized bitmap — so it is a lighting question, not a palette one). View confirmed `Mode2DCockpit`, art set the deterministic generic `16_ckpit.dat` fallback. **DEV-4 registered** (native draws a canopy bow the gold lacks); DEV-3 suspended pending a gold-4 re-shot. No fix shipped — deliberate |
| 11 EPIC SP.2 (cont.) | ✅ done (7/8) | **DEV-2 and DEV-4 both RESOLVED as non-defects; gold 2 upgraded to PARITY.** DEV-4 = the canopy-reflection visual cue (`SimVisualCueMode=VCReflection`), A/B-proven by forcing it off — a player option, not a renderer bug. DEV-2 = a time-of-day difference: forcing the gold's light level reproduces the gold's panel almost exactly (p95 106.0 vs 107.0), so the panel path is correct. Both recommended for PO waiver. New backlog: LIGHT-1 (cockpit never re-lights), LIGHT-2 (unclamped `cLight`). DEV-3 carried |
| 12 EPIC SP.2 (DEV-1) | ⚠️ partial (5/8) | **DEV-1 reclassified, not fixed.** The native correctly renders `main_win.scf` → `UI_MAIN_BG` (the F-16 photo); a wrong Sprint-9 finding is corrected (`MAIN_SCRN` is a radar scope, and the window never asks for it). Searched all 751 `.idx` files / 69 unique 1024×768 images: **nothing in this install matches the gold's menu**, so golds 1 & 5 look like a **provenance problem**, same class as gold 3. PO question raised. Retro: validate a similarity metric on a known-positive before believing a negative |
| 13 Video gold standard | ✅ done (8/8) | **DEV-1 CLOSED as PARITY** — the video shows golds 1 & 5 were a *loading screen*, not the main menu; native vs gold main menu correlates **0.9908** at matched 1024×768. Overturns both the Sprint-9 finding and my own Sprint-12 conclusion. ~~LIGHT-1 confirmed~~ — **that claim was WITHDRAWN in Sprint 14: the defect does not exist** (the callback was already registered; a truncated grep hid it). New `tools/gold_video.sh` extracts pixel-exact 1024×768 client frames from the Wine recordings |
| 14 LIGHT-1 (withdrawn) | ❌ reverted | **LIGHT-1 does not exist.** The claim that `CockpitManager::TimeUpdateCallback` was never registered was wrong — cpmanager.cpp:677–679 has always registered it. A `grep … | head -20` truncated the answer at line 21. The "fix" double-registered the manager (two registers, one release → **use-after-free** on the next TOD tick after a mission) and is reverted. Caught by its own control run: predicted 9-vs-1, measured 9-vs-15 |
| 15 EPIC SP closed | ✅ done (8/8) | **DEV-3 CLOSED — does not reproduce.** Against the video gold's 2D pit (`views` t=20s, matched 1024×768) the native pit art reaches the bottom edge exactly as the gold's; bottom rows track within a few luma units to y=766 and neither shows pilot hands. **At matched daytime TOD the panel means agree (gold 59.9 vs native 58.6)** — independent corroboration of DEV-2's waiver. **EPIC SP is now COMPLETE: all 5 gold shots resolved, DEV-1..4 all closed as non-defects, zero renderer bugs found.** Done entirely offline — no display lock |
| 16 RWY-2 + ACMI-1 | ⚠️ partial | **ACMI-1 found: our ACMI cannot read Windows tapes.** `ACMITapeHeader` is a packed struct of 17 `long`s — 80 bytes on Win32, 148 here — so every field misparses; proven by parsing a real PO tape both ways (32-bit gives numEntities=1, numFeat=724, totPlayTime=195.9s; 64-bit gives garbage). `src/acmi/` has 48 `long`s, zero `int32_t`, no `FF_LINUX` guards — never swept. RWY-2 **blocked on TE-09** — two attempts diverged (one my own view-mismatch error, one autopilot). Depth bias confirmed engaging (701 activations). TE-09 promoted to the next sprint |
| 17 ACMI-1 fix | ✅ done (8/8) | **On-disk format restored to 32-bit.** 35 packed-struct fields `long`→`int32_t` (in-memory structs deliberately untouched), plus the callsign count in the `.flt` read, the tape write and `GetCallsignList`'s embedded count/stride. **Five `static_assert`s now pin the layout** (80/36/41/8/16) against sizes derived from a real PO tape's own block offsets — the format contract fails the build instead of failing silently. Playback path still untested (needs display) |
| 18 TE-09 root-caused | ✅ done (7/8) | **Not nondeterminism — persisted options.** (1) With `SimAvionicsType=ATRealisticAV` and `SimAutopilotType=APNormal` (both from `Viper.pop`), `SimToggleAutopilot` routes the AP key to **`SimRightAPSwitch`** and never calls `ToggleAutopilot` — proven with entry-level tracing after a first, badly-instrumented attempt wrongly blamed key timing. **AP-1 registered:** a duplicated `(not StrgSel) and (not StrgSel)` guard (two sibling sites show `(not StrgSel) and (not HDGSel)`) clobbers HDG Select — the very roll-switch-initial-state TE-09 suspected. Upstream; PO call. (2) `Viper.pop` persists `SimAutopilotType=APNormal` → `ThreeAxisAP`, an attitude hold that does not follow the route. The game rewrites that file on exit, so the value drifts between sessions — which is what read as nondeterminism. Shipped `FF_AP_MODE` + `FF_DEBUG_AP`; corrected recipe recorded |

## Sprint 9 Planning (2026-07-27, re-planned after interruption)

Project re-activated in the rotation after a short pause. Baseline re-verified
before planning: release build green (`ninja: no work to do`), repo clean on
`develop`, and the previously wedged GLX gate has healed (NVIDIA direct
rendering: Yes) — real-GL capture runs are unblocked.

**Goal:** EPIC SP.1 — fill the prepared parity table in `docs/screen-parity.md`
with native captures and verdicts for all 5 PO gold shots (deterministic
one-shot captures per the cross-port method; deviations classified
renderer-bug / authentic-asset / asset-gap / prior-decision, fixes deferred to
SP.2 unless trivial).

**Selected stories (~8 pts):**
- **SP.1-U (2 pts):** UI main-menu parity — golds 1 (06-05 16-31-46) and
  5 (06-09 15-32-20), one native capture serves both.
- **SP.1-S (3 pts):** Sim 2D-cockpit Instant Action parity — gold 2
  (06-05 16-32-02), real-GL run under the display lock.
- **SP.1-D (3 pts):** Dogfight setup screen (gold 3, native-provenance
  capture, see inventory note) + dogfight arena entry 2D pit (gold 4).

Handoff check (class-level, from MA/BoB 2026-07-27): FreeFalcon hosts no
DoPropExchange/CPropExchange persisted-property controls (zero source hits;
no MFC/ActiveX by design) — the uninitialized-OCX-member class does not apply.

## Sprint 9 Review (2026-07-27) — DONE, 8/8 pts

**Shipped:** SP.1 complete — all 5 PO gold shots have native counterpart
captures and verdicts in `docs/screen-parity.md` (evidence thumbnails committed
in `docs/screen-parity/`, full frames in `/tmp/ffval/`, all recipes verified
and corrected to working parameters).

- **SP.1-U (2 pts, done):** golds 1 & 5 (UI main menu) = **major deviation
  DEV-1** — native shows the legacy Falcon4 photo menu, Windows shows the
  FF-themed cobra/aircraft-column menu. Classified as UI selection divergence:
  missing `mainbg.irc` ruled out by experiment (recreated, no change); themed
  `MAIN_SCRN` art confirmed present in the data but never displayed natively.
- **SP.1-S (3 pts, done):** gold 2 (IA 2D pit) = **partial** — HUD/MFD/DED
  symbology and layout parity; **DEV-2**: native 2D-pit art far brighter than
  gold (suspected missing TOD/palette shading on the palettized pit bitmap).
- **SP.1-D (3 pts, done):** gold 3 (dogfight lobby) = **parity** (identical
  layout/art/options; the gold is native-provenance, so this certifies
  no-regression-since-June, not Windows parity). Gold 4 (dogfight arena 2D
  pit, same-scenario pair) = **partial** — confirms DEV-2 decisively, plus
  minor **DEV-3** (pit bottom sliver/hands not visible).

**Verified/demoed:** every capture run objective (band statistics, all frames
94–99.9 % non-black with 600–92 000 distinct colours); build baseline green;
GLX gate re-probed healed (NVIDIA direct rendering: Yes) before planning per
the MA/BoB handoff. No code changes this sprint — capture/analysis only, so
no gate regressions possible; game data untouched except restoring the
documented-in-January `art/resource/mainbg.irc` (no visual effect).

**Carry-over:** none. SP.2 (fix/waive DEV-1..3) is the natural Sprint 10.

**Operational finds (recorded in the parity doc):** sim-mission load now takes
~80–100 s in this build, so process-relative `FF_SIM_SCREENSHOT` under ~110 s
captures the load screen (the "white frame" here was timing, not the old
RTT-readback bug); scripted TAKEOFF clicks that fire before FM_JOIN_SUCCEEDED
is processed silently no-op (dogfight flow needs the click ≥6 s after COMMIT,
plus a backup click).

**Retro (one line):** Same-scenario capture pairs (gold 4) turn "looks
different" into a decisive single-variable verdict — prefer them over
different-instant pairs (gold 2) when choosing what to capture first.

## Sprint 10 Review (2026-08-08) — DONE, 8/8 pts

**Goal:** EPIC SP.2 — root-cause and fix DEV-2 (2D-pit brightness). **Goal
partially met and deliberately re-scoped:** DEV-2 is root-*caused* to the extent
that its recorded mechanism is disproved and the real one localised, but no fix
shipped — the sprint's main output is that the Sprint-9 verdicts could not be
trusted as written, and the capture method that produced them is now fixed.

- **SP.2-A (2 pts, done):** objective metric for DEV-2, and the finding that
  killed the sprint's original plan — native and gold differ **structurally**,
  not just tonally (native draws a canopy bow the gold lacks; same combiner
  posts, same panel top ~y340, so not a scale difference). The tell that should
  have caught this in Sprint 9 was already in the numbers: **the sky moved the
  opposite way from the panel** (−60.7 vs +38.5 luma), which no single global
  gamma/TOD error can produce.
- **SP.2-B (4 pts, done):** new `FF_DEBUG_PITSEL=1` trace (cpmanager.cpp +
  otwloop.cpp) records the pit art set actually resolved, whether it opened, the
  scales, and the live display mode. One view-confirmed re-capture then settled
  it: **view is `Mode2DCockpit` for the whole flight, hybrid off**, art set is
  the deterministic generic `art/ckptart/16_ckpit.dat` fallback at scale 0.6399
  (the aircraft is `F-16CJ` and no `F-16CJ/` cockpit directory exists).
- **SP.2-C (2 pts, done):** re-measured against gold 2 with the view confirmed.
  **DEV-2's symptom is real and reinstated** (panel median 28.7 gold vs 45.5
  native; p95 107 vs 189) but **its stated mechanism is wrong** — the 2D pit is
  not a palettized bitmap, it is lit 3D geometry (`cockpit2d 2358 2358` →
  `CreateCockpitGeometry`), so this is a pit-geometry *lighting* question, not a
  palette one. New **DEV-4** registered for the canopy bow. DEV-3 stays
  suspended until gold 4 is re-shot with the trace.

**Two corrections made inside the sprint, both recorded rather than quietly
fixed:** (1) an intermediate conclusion that the native captures were the *3D
virtual cockpit* was wrong — it inferred from "no 2D-pit `.dat` declares a
`MIRROR`" without noticing the 2D pit is itself model geometry, so mirror shapes
can belong to it; (2) DEV-2 was briefly withdrawn wholesale when only its
mechanism was unsupported.

**Retro:** *a parity capture must record the state it claims to be capturing.*
Sprint 9 pinned the view with `FF_VIEW_SCRIPT="1@<t>"` and wrote verdicts as
though that had taken effect; nothing in the output recorded the view in force,
so "unset view" and "render bug" were indistinguishable for a whole sprint.
Cheap to fix, and it turned a three-hypothesis guess into a one-run answer.
Same family as BoB S101 — except the diagnostic here did not lie, it stayed
silent about the one variable the verdict rested on.

**Carry-over into Sprint 11:** DEV-4 (dump model 2358's switch/DOF components;
the 2D path sets only masks 7 and 3 vs the 3D pit's 263 — a component left at
its default-visible state is the leading candidate and is the port's recurring
"nothing ever wrote this" class); DEV-2's lighting path (check whether commit
`3bc96916`'s 3D-pit lighting fixes cover the 2D pit's geometry pass); DEV-3
re-capture; DEV-1 still untouched.

## Sprint 11 Review (2026-08-08) — DONE, 7/8 pts

**Goal:** EPIC SP.2 — fix DEV-2 and DEV-4 on the 2D pit. **Goal met, with the
answer being that neither is a port defect.** Both of Sprint 9's "major" sim
deviations are resolved as non-defects, and gold 2 is upgraded to **PARITY**.

- **S11-A (3 pts, done): DEV-4 SOLVED.** A/B under `gl-lock`, single variable:
  forcing `SimVisualCueMode=0` makes the canopy bow vanish (clean sky) while the
  panel is untouched (58.6 → 58.8). The bow is switch 3 of the pit's external
  geometry — the **canopy-reflection visual cue** — enabled because
  `SimVisualCueMode=2` (`VCReflection`), a legal in-range value that is both the
  ctor default and a valid user setting. That the panel didn't move also
  **proves the bow and the brightness were two independent deviations**, which
  is exactly why DEV-2's "shading difference only" premise failed.
  → **Recommend WAIVE** (player option, nothing to fix in the renderer).
- **S11-B (3 pts, done): DEV-2 RESOLVED — the panel path is CORRECT.** Forcing
  the environment light to the gold's implied value reproduces the gold almost
  exactly (p95 106.0 vs 107.0; mean 35.8 vs 38.0), against a prediction of
  32–40 stated before the run. `todLight=1.0` is genuine theater data (TOD ctor
  defaults sum to 0.9), so Windows computes 1.0 too. The deviation is a
  **time-of-day difference between the captures**. → **Recommend WAIVE.**
- **S11-C (1/2 pts):** two view-confirmed captures plus the A/B and the light
  test all landed; **gold 4 not re-shot**, so DEV-3 stays suspended. Carried.

**New backlog items — real, but neither caused DEV-2:**
- **LIGHT-1:** `CockpitManager::TimeUpdateCallback` is never registered, so
  `lightLevel` is set once in `RenderFirstFrame` and the cockpit never re-lights
  as the sun moves. Invisible in a 2-min capture; wrong on a long or dawn/dusk
  flight.
- **LIGHT-2:** `ComputeLightFactors` assigns `cLight[i] = eLight` with no upper
  clamp (the 1.0 clamp is only in the flood-light branch) while
  `GetLightLevel()` returns `Ambient + Diffuse`, which can exceed 1.0.

**Four plausible hypotheses were killed by checking rather than banked** — worth
reading before trusting a "found it" in this area: the `.pop` backslash path
(compat does `#define fopen fopen_nocase`, which converts them); the `.pop`
`fread(this,…)` 32/64-bit hazard (all members are int/float/enum/char, so the
layout is portable — the 228 vs 240-byte gap is *version*); `CPPanel::SetTOD`'s
unreachable TOD body (`git log -L` shows it predates the port, so Windows
short-circuits identically); and the hybrid-pit auto-switch (gated on a flag
that initialises to 0).

**Retro:** *state the prediction before the run.* S11-B committed to "32–40" in
advance, so 35.8 confirmed a model instead of being retro-fitted to a story —
the failure mode that produced DEV-2's wrong mechanism in the first place.

**Tooling:** `FF_PIT_VISCUE=<n>` and `FF_PIT_LIGHT=<f>` diagnostics added;
`ff_validate.sh` now uses `timeout -k 5` so a run that ignores SIGINT cannot
outlive its cap holding the display lock.

## Sprint 12 Review (2026-08-08) — PARTIAL, 5/8 pts

**Goal:** EPIC SP.2 — root-cause and fix DEV-1 (main menu is a different screen).
**Outcome: DEV-1 reclassified, not fixed.** The native is behaving correctly for
this game data; the gold appears to come from a different install.

- **S12-A (1/3 pts):** no UI trace shipped — the question was answered by data
  analysis instead, so the instrumentation story is only partly delivered.
- **S12-B (3 pts, done):** **corrected a wrong Sprint-9 finding.** Sprint 9
  recorded that the data carries FF-themed menu art the native never displays
  (`MAIN_SCRN`, "blueprint/logo composition"). Both halves are wrong:
  `MAIN_SCRN` is a **radar-scope** image, and the window does not ask for it —
  `main_win.scf:20` names **`UI_MAIN_BG`**, which resolves to
  `art/UISkin/ff4/UIMAINBG` and decodes to the **F-16-in-shelter photo the
  native shows**. The native renders exactly what the script names. Every
  `main_win.scf` in the install (base + 4 theater trees) puts the buttons on the
  **bottom bar at y=728**, which is what the native draws; nothing in this data
  produces the gold's right-hand aircraft column.
- **S12-C (1/2 pts):** fresh main-menu capture taken under `gl-lock`
  (`sp12-main`) and both gold verdicts re-issued. Full-install search: 751
  `.idx` files, every 1024×768 entry decoded, 69 unique full-screen images, none
  matching the gold; the only two near-black candidates are entirely empty.
  → **PO question raised:** were golds 1 & 5 shot against a different install /
  version / UI skin? Same class as the Sprint-9 note on gold 3.

**Stated limitation:** the search covered full-screen 1024×768 resources only.
A *composed* menu (tiled/smaller background plus placed elements) would evade it.

**Retro — the important one: validate a similarity metric on a known-positive
control before believing a negative.** The first search ranked resources by
correlation against the gold and reported "no match, best 0.447", which read
like a finding. Run against the **native** capture, whose correct answer is
known, the same metric put the true background **outside the top 6**. The metric
could not find a known-positive, so its negatives meant nothing; the conclusion
was withdrawn and redone by decode + contact sheet. A negative result from an
unvalidated metric is not evidence.

**Also registered (upstream, deliberately NOT fixed):** `theaterdef.cpp:278`
assigns the **wrong variable** in its else branch —
`strcpy(FalconUISoundDirectory, …)` where it means `FalconUIArtThrDirectory`
(copy-paste from the block above). `git log -L` shows it arrived in the original
import, so **Windows has it too**; fixing it would diverge from the oracle.
Unreachable in this data anyway (every theater sets a non-empty `artdir`).
Related: `main_linux.cpp:1280` hardcodes `FalconUIArtThrDirectory` to
`<data>/art`, which matches Korea's `artdir art` but not `korea_2012`
(`artkorea2012`), `eurowar` (`artEurowar`) or Balkans — latent for non-Korea
theaters if the theater loader does not override it in time.

## Sprint 13 Review (2026-08-08) — DONE, 8/8 pts

**Goal:** stand up the new PO **video** gold standard and use it. Both headline
results landed, and one of them corrects this session's own work.

- **S13-A (3 pts, done):** `tools/gold_video.sh` — pulls a **pixel-exact
  1024×768 client frame** from the Wine recordings at any timestamp
  (`--list` inventory, `--probe` to derive the crop for a new clip). The
  recordings are 1920×1080/60 with the game windowed at 1024×768, i.e. **our own
  capture resolution**, so no rescaling is needed. That removes the PNG golds'
  worst flaw (1011×771 client forced a resize, making every size-based verdict
  unsafe). Window offset differs per clip and is recorded in the tool.
- **S13-B (3 pts, ❌ WITHDRAWN in Sprint 14 — the defect does not exist).**
  Windows re-lights the cockpit as the sun rises; we never do. Measured
  glare-free (left console, away from the canopy): six dawn samples pinned at
  38.2/34.0, rising monotonically to 47.0/40.0 once the sun is up. Cause is
  attributed to `CockpitManager::TimeUpdateCallback` never being registered.
  **That attribution was false** — it has always been registered
  (cpmanager.cpp:677–679). The brightening is normal behaviour on both sides,
  not a defect. See Sprint 14.
- **S13-C (2 pts, done): DEV-1 CLOSED as PARITY, and DEV-2/DEV-4 corroborated.**
  The `ia` clip shows the Windows main menu at t=0–6 s is the **legacy F-16
  photo with the bottom bar** — what we render — and the blue blueprint/cobra
  art of golds 1 & 5 appears at **t=33 s as a transient LOADING screen**.
  Native vs gold main menu: **0.9908** full-frame, 0.999 per region, mean
  luminance within 2 units. Separately the dawn cockpit frames show the gold
  **with the canopy bow present** (DEV-4's reflection cue on) and a **dark
  panel** (DEV-2 = time of day), so both waiver recommendations now rest on the
  video rather than on inference alone.

**A correction to my own Sprint-12 conclusion.** Sprint 12 concluded the gold's
menu artwork "cannot be reproduced from this install" and was probably a
different install. Wrong — this build produces it, on a screen I never thought
to look for. The exhaustive resource search was sound; the *hypothesis space*
was too narrow. Searching harder inside a wrong assumption does not escape it.

**Retro:** *a still cannot tell you what state produced it* — third time in four
sprints. A 33-second loading screen and a main menu are indistinguishable as
PNGs, and that ambiguity alone sustained DEV-1 through three sprints of
speculation. **The video gold standard is strictly better and should be the
default oracle from here.**

## Sprint 14 Review (2026-08-09) — ❌ WITHDRAWN, work reverted

**Goal:** fix LIGHT-1. **Outcome: there was nothing to fix, and the attempted
fix was a regression. Reverted the same day.**

**The claim.** That `CockpitManager::TimeUpdateCallback` was never registered,
leaving `lightLevel` written once by `RenderFirstFrame` and frozen for the whole
mission — supported by an apparently damning asymmetry: the destructor released
a callback that nothing registered.

**The reality.** cpmanager.cpp:677–679, in the constructor:

```c
// Initialize the lighting conditions and register for future time of day updates
TimeUpdateCallback(this);
TheTimeManager.RegisterTimeUpdateCB(TimeUpdateCallback, this);
```

It has always been there, correctly paired with the destructor's release, using
the same idiom as every other TOD consumer. **The cockpit re-lights already.**

**Root cause of the error: a truncated grep.** The search for `TheTimeManager`
was piped through `head -20`; the `cpmanager.cpp` registration was line 21.
Every downstream conclusion — the missing-half argument, the destructor-proves-
intent argument, the fix itself — rested on output that had silently dropped the
answer.

**This is the third time in one session.** The same trap produced a wrong
"the TOD clock is never advanced" conclusion earlier in this very sprint (a
`head -10` hid the two `otwloop.cpp` call sites), and the "filter, don't cap"
lesson was already banked from the MA side before the session began. Reading it
was not enough; the habit had to change. **Rule adopted: never pipe a grep
through `head` when the question is "does X exist anywhere" — completeness is
the whole point of the search. Cap output only when browsing.**

**What the attempted fix actually did.** It added a *second* registration.
The callback then fired twice per cycle, and since the destructor releases only
one `CBlist` entry, the leftover slot kept a pointer to the freed
`CockpitManager` — a **use-after-free on the next TOD tick after a mission
ends**. Reverted; a comment at the site now warns against re-adding it.

**What saved it: the control run.** The A/B predicted 9 calls with the fix vs
**1** without. It measured 9 vs **15**. Nine callback-driven re-lights in the
"disabled" arm is exactly what an already-registered callback looks like. Had I
run only the treatment arm, "15 calls, works!" would have shipped as verified.
**The control is not a formality — it is the only thing that tested the premise
rather than the change.**

**Salvage:** the Sprint-13 measurement that the gold cockpit brightens as the
sun rises stands as a fact, but it is now expected behaviour on both sides
rather than evidence of a defect. Whether our port matches that curve is still
untested and needs a dawn mission — a legitimate future sprint, with no
presumption of a bug.

## Sprint 15 Review (2026-08-09) — DONE, 8/8 pts

**Goal:** close DEV-3, the last open screen-parity deviation. Done — and it
completes EPIC SP.

- **DEV-3 CLOSED, does not reproduce.** The `views` clip at t=20 s is the 2D
  cockpit at exactly 1024×768, and pit art is screen-fixed, so it compares
  like-for-like against our view-confirmed `sp11-base`. Bottom-edge row means
  track within a few luma units all the way to y=766; the bottom strips are
  structurally identical (same MFD bezels, standby ASI/altimeter, button rows,
  `MASTER ARM`/`SIMULATE` panel) and **neither shows pilot hands**. There is no
  missing sliver.
- **DEV-2 independently corroborated.** This clip is daytime, and at matched TOD
  the panel means agree: **gold 59.9 vs native 58.6**. Sprint 9's "far brighter
  than gold" came entirely from comparing a noon native against a dimmer-TOD
  gold.
- **Whole sprint ran offline** — no display lock, while three other sessions
  held it. The video gold makes most parity work display-free.

### EPIC SP — COMPLETE

All five PO gold shots resolved. **Every one of the four registered deviations
closed as a non-defect; no renderer bug was ever found.**

| ID | was | outcome |
|---|---|---|
| DEV-1 | main menu is a different screen | the golds were a **loading screen**; real menu at 0.9908 parity |
| DEV-2 | 2D pit far brighter than gold | **time of day**; matched-TOD panels agree |
| DEV-3 | pit bottom sliver / hands missing | **does not reproduce** |
| DEV-4 | native draws a canopy bow | the **`VCReflection` player option**; gold shows it too |

The scoreboard is worth stating plainly: **Sprint 9 logged three "major"
deviations and a fourth was added in Sprint 10; all four were artefacts of the
comparison method, not of the port.** Every one traced to the same root cause —
comparing frames without recording the state that produced them (view mode, time
of day, player options, or even which screen it was).

## Sprint 16 (2026-08-09) — RWY-2 vs the landing gold + ACMI-1

**ACMI-1 (new defect, high confidence, found offline): our ACMI cannot read
Windows-recorded tapes, and ours are unreadable on Windows.**

`ACMITapeHeader` (acmitape.h:124-148) is a `#pragma pack(1)` struct of 17
`long`s followed by 3 floats. `long` is **4 bytes on 32-bit Windows, 8 on 64-bit
Linux**, so the header is 80 bytes there and 148 bytes here, and every field
after the first misparses. This is the port's signature bug class — the same one
already fixed in `cresmgr.cpp`, the campaign save path, WAV loading and the
terrain block offsets — but **`src/acmi/` was never swept: 48 `long` uses, zero
`int32_t`, and not one `FF_LINUX` guard in the whole directory.**

Verified against a real PO tape (`260808_landing_final_approach.vhs`, 919 525 B)
by parsing it both ways:

| field | 32-bit layout | 64-bit layout |
|---|---|---|
| numEntities | **1** | 498216206416 |
| numFeat | **724** | 112442243810720 |
| entityBlockOffset | **80** | 3852847657602276 |
| numEntityPositions | **1440** | 897061 |
| totPlayTime | **195.861 s** | 0.000 |
| startTime | **32417.0 s** (09:00:17) | 0.000 |

The 32-bit reading is entirely self-consistent — ascending in-file offsets, and
195.9 s of play time matching the 3:44 clip minus its menu and load. The 64-bit
reading is garbage. The tapes are fine; our reader is not.

Scope beyond the header: `ACMIEntityData`, `ACMIEntityPositionData`,
`ACMIEventHeader`, `ACMIEventTrailer` and the rest of the packed structs in
`acmitape.h` have the same problem, so this is a directory-wide sweep
(`long` → `int32_t` for everything that touches the file format), not a one-line
fix. It also means **ACMI tapes we write today are not interchangeable with
Windows** — worth knowing before anyone treats a `.vhs` as a portable artefact.

Cost of *not* fixing: the five PO-supplied tapes (`TAPE0006-0010.vhs` plus the
landing tape) are unusable as an oracle, and ACMI playback parity cannot be
tested at all.

**RWY-2: still BLOCKED, now with the blocker pinned to TE-09.** The landing clip
is the acceptance flight outstanding since Sprint 8, and gold reference frames at
t=130/150/165/180 s are committed (`docs/screen-parity/gold-video-landing-t*.png`)
showing the runway rendered continuously from approach distance through touchdown.

Two capture attempts, neither comparable:

1. **My error:** captured in the default 2D pit while the gold pilot flew the
   approach in the **HUD view**. State not matched to the oracle — the very
   discipline banked in Sprint 10. Corrected by pinning `FF_VIEW_SCRIPT="0@40"`.
2. **TE-09:** with the view corrected, the flight path still diverged — out over
   water, and in the sea by t=170 s. This is the documented nondeterministic
   approach autopilot (2/2 route-follow 07-25, 0/3 on 07-26, one variant into the
   sea). Two more attempts today, both diverged.

**What the attempts did establish:** the Sprint-8 depth-bias fix is alive and
engaging — **701 `[RUNWAY] depth bias ACTIVE` activations** in the corrected run,
with flat runway/tarmac surfaces being tagged and submitted
(`InsertStaticSurface … visType=54/53`). So the mechanism works; what is missing
is a flight that actually reaches the runway.

**Conclusion: RWY-2 cannot be accepted by automation until TE-09 is fixed.**
Retrying the flaky path burns a contended display slot per attempt at roughly a
1-in-3 success rate. **TE-09 is therefore promoted to the next sprint** — it is
the enabler for an acceptance that has been open since July, and the suspect
(AP roll-switch initial state) is diagnosable by reading the autopilot engage
path rather than by repeated flights.

## Sprint 18 (2026-08-09) — TE-09 root-caused: it was never nondeterminism

**TE-09 has two causes, and neither is a race.** It has blocked the RWY-2
acceptance since Sprint 8 and was recorded as "autopilot engages route-following
nondeterministically (suspect AP roll-switch initial state)". The roll switch is
not involved.

**Cause 1 (CORRECTED) — the AP key does not reach `ToggleAutopilot` at all,
because realistic avionics route it to the AP SWITCH instead.**

_The first version of this entry claimed the key fired during the mission load
and was therefore lost. That was wrong, and the evidence for it was bad: the
`FF_DEBUG_AP` trace sat inside the `autopilotType == APOff` branch, so it could
not print when the AP was already on. "No trace" was read as "never called".
Re-instrumented at function entry, and with the key moved to `@105` (well after
load), the trace **still** never printed — so the timing hypothesis was not the
explanation._

The real chain, all from persisted options:

- `Viper.pop` has `SimAvionicsType = 3` = `ATRealisticAV` → **`g_bRealisticAvionics = true`**
- `Viper.pop` has `SimAutopilotType = 2` = `APNormal`
- `SimToggleAutopilot` (commands.cpp) therefore takes
  `case APNormal:` → `g_bRealisticAvionics` → **`SimRightAPSwitch(...)`**,
  and never calls `AircraftClass::ToggleAutopilot`.

So with realistic avionics the AP key works the F-16's **right AP switch**
(AltHold ↔ AttHold) rather than engaging a route-following autopilot. Route
following is the **left** switch's `StrgSel` (steering select) position.
`0x1E` is correctly bound (`SimToggleAutopilot … 0X1E` in `keystrokes.key`), so
the binding was never the problem.

**AP-1 (new defect, upstream): a duplicated condition in the AP-switch guard.**
`SimRightAPSwitch` guards the "jet was entered with the switch off centre" case
with the same test twice:

```c
line 8130:  (not StrgSel) and (not StrgSel)   // duplicated
line 8162:  (not StrgSel) and (not StrgSel)   // duplicated
line 8209:  (not StrgSel) and (not HDGSel)    // correct
line 8260:  (not StrgSel) and (not HDGSel)    // correct
```

Two sibling sites in the same file show the intended form, so this is a
copy-paste typo, not intent. Effect: when the left switch is in **HDG Select**,
the guard still passes and force-sets `RollHold`, silently clobbering the
heading/steering mode — i.e. the autopilot's behaviour depends on the switch
position the jet spawned with. **That is exactly the "AP roll-switch initial
state" that TE-09 suspected from the start, and which this sprint initially
dismissed.** `git log -L` shows the duplication predates the port (only a
`!`→`not` reformat since), so **Windows has it too** — same
fix-or-match-the-oracle dilemma as the `theaterdef.cpp` wrong-variable bug,
except this one is reachable and affects flight behaviour. **Registered, not
fixed; PO call.**

**Cause 2 — even when it fires, the mode is wrong.** `ToggleAutopilot`
(aircraftinputs.cpp) selects behaviour from a **player option**:

| `SimAutopilotType` | engages | follows route? |
|---|---|---|
| `APIntelligent` (0) | `CombatAP` | yes |
| `APEnhanced` (1) | `WaypointAP` | yes |
| **`APNormal` (2)** | **`ThreeAxisAP`** | **no — attitude/heading hold** |

Decoding `config/Viper.pop` directly gives **`SimAutopilotType = 2` = `APNormal`**,
so the AP holds heading instead of turning inbound — exactly the recorded
symptom. The offset mapping was cross-checked before being trusted: the same
decode yields `SimVisualCueMode=2` and `ObjDetailLevel=2.000`, both matching
values printed live by `FF_DEBUG_PITSEL` in an earlier run.

**Why it looked nondeterministic.** The game **rewrites `Viper.pop` on exit**, so
the persisted AP mode can differ between sessions. Runs that route-followed had a
route-following mode saved at the time; later runs did not. Not a race —
**persisted state drifting between runs**.

**That is the fourth "port defect" this session that turned out to be captured
state or a player option** (DEV-2 time-of-day, DEV-4 `VCReflection`, DEV-1 the
wrong screen, now TE-09). The pattern is worth naming: this engine keeps a lot of
behaviour in a binary options file it rewrites on exit, so anything not pinned
drifts, and drift reads as nondeterminism.

**Shipped:** `FF_DEBUG_AP=1` traces `ToggleAutopilot` at function entry, and
`FF_AP_MODE=<0|1|2>` forces the mode — but note the override is applied inside
`AircraftClass::ToggleAutopilot`, one level **below** where the dispatch
actually happens (`SimToggleAutopilot`), so it does not change which branch is
taken. To make the approach repeatable, the mode must be forced at the command
layer, or the persisted options changed. Left as-is and documented rather than
half-fixed.

**Recipe status:** still not repeatable. `0x1e@105` (after load) is the right
timing and should replace the stale `@5`, but on its own it changes nothing
while the persisted options send the key to the AP switch. A repeatable approach
needs either the command-layer override or `SimAutopilotType`/`SimAvionicsType`
set to a route-following combination.

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

PO: (a) fly the landing TE to accept the Sprint-8 runway depth-bias fix (RWY-2
above — objective A/B capture evidence is in place; touchdown feel is the
remaining judgement call); (b) eyeball the Sprint-9 parity verdicts in
`docs/screen-parity.md` — in particular whether DEV-1 (native shows the legacy
photo main menu, not the FF cobra menu) should be fixed or waived, and whether
the Wine golds came from this exact game-data install. Agent side: Sprint 10 =
SP.2, root-cause DEV-2 (2D-pit palette/TOD shading — biggest visual payoff,
affects both sim screens) then DEV-1.
</content>

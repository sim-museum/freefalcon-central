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
| 19 PO smoke test | ✅ done (8/8) | **AUTOSAVE-1 FIXED** — our own `Auto Save.cam` was unreadable: `Encode` wrote 32-byte native event nodes while `Decode` read the 20-byte 32-bit-Windows layout, drifting 12 bytes per event → garbage count → `bad_alloc` → SIGSEGV on **every** campaign mission end. Both encode sites fixed. Five more defects registered from the PO's flight: JOINFAIL-1 (graceful-failure path segfaults), **RWY-3** (12/31 runway posts step through coarse elevations — the retracting airfield, now quantified), TERRAIN-1 (grey untextured dogfight terrain), LOAD-1 (white loading screen regression), PIT-1/GEAR-1. **ACMI promoted to top of backlog per PO** — it is a quantitative performance instrument, not just an oracle |
| 20 ACMI end-to-end | ✅ done (8/8) | **A Windows-recorded tape now LOADS in the player.** `tools/acmi_dump.py` turns any tape into a time series (validated on 5 tapes); the gold landing decodes to a textbook approach with **ground altitude pinned at 28 ft through rollout** — the oracle RWY-3 needed. Three further fixes to reach playback: unbounded `ACMI_Callsigns[uniqueID]` index (the SIGSEGV; ids run to 477), `ACMI_CallRec.teamColor` `long`→`int32_t` (24→20 bytes, wrong memcpy stride), and two `delete`/`new[]` mismatches. In-game clock `05:04:03` matches the Python decode exactly |
| 21 ACMI capture chain | ✅ done (7/8) | **ACMI-3 found: a recorded flight can never become a loadable tape.** The recorder writes only `acmi*.flt`; `ACMI_ImportFile()` converts it and **nothing calls it** (both `acmiui.cpp` sites commented out; only live caller is a multiplayer chat command). `FindFirstFileA` checked and exonerated. Also: the tape is flushed by **StopRecording**, so any flight cut short leaves nothing on disk — likely why the PO's landing produced no tape. New hooks `FF_AP_MODE` (now at the dispatch site), `FF_ACMI_RECORD`, `FF_ACMI_STOP`, `FF_ACMI_IMPORT`. Sprint 18's diagnosis retro-validated: `ToggleAutopilot ENTER` fired for the first time ever |
| 22 Landing diff | ✅ done (8/8) | **First quantitative parity result: our landing matches the Windows landing.** Touchdown 36 ft both; rollout minimum 28.3 ft both; spread 8.8 ft gold vs 7.6 ft ours. **Correction:** Sprint 20's "gold pinned at 28 ft" was a subsampling artifact — the gold varies 8.8 ft too, so RWY-3's criterion was built on a contrast that does not exist. **RWY-3 reframed as a RENDERING defect** (12/31 runway posts step elevation mid-approach); ACMI measures the aircraft, not the scenery, so the correct instrument is `FF_DEBUG_RUNWAY` |
| 22 PO landing vs gold | ✅ done (8/8) | **Our landing is quantitatively equivalent to Windows**: touchdown 36 ft both, rollout min 28.3 ft both, spread 7.6 vs 8.8 ft (ours tighter), touchdown points ~670 ft apart. First hard evidence the port's flight/collision matches the oracle. **CORRECTION:** Sprint 20's "gold pinned at 28 ft" was a subsampling artifact — the gold varies 28.3–37.1 ft too, so RWY-3's criterion was wrong. **RWY-3 still reproduces** (12/31 runway posts step elevation) but **ACMI cannot measure it** — tapes record aircraft state, not rendered scenery. RWY-3 is a rendering defect; its criterion is the runway-post series |

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

## Sprint 19 (2026-08-09) — PO smoke test: 6 defects found, autosave round-trip FIXED

The PO test-drove the build under gdb. Dogfight flew clean; the campaign crashed;
TE "09 Landing Final Approach" **loaded and was landed successfully**. Six
distinct defects, all with evidence attached.

### ⭐ AUTOSAVE-1 — FIXED. Our own autosave could not be read back

**Deterministic crash on every campaign mission end.** Sequence from the gdb log:

```
FM_START_DOGFIGHT   -> flew fine
FM_START_CAMPAIGN   -> OCA strike
EndFlight           -> aircraft destroyed
OTWDriver.Exit      -> sim cleanup OK
FM_REVERT_CAMPAIGN  -> return to campaign
FM_LOAD_CAMPAIGN    -> reload 'Auto Save.cam'
   Decode: "Reading 15360 standard events (newRem=27327)"  <-- garbage count
   -> std::bad_alloc -> FM_JOIN_FAILED -> SIGSEGV
```

The first load read the shipped `save0.cam` and worked; the reload read
`Auto Save.cam`, written by the game minutes earlier. **Root cause: an exact
encode/decode mismatch on the UI event queues.**

| | bytes per `uieventnode` |
|---|---|
| `Encode` wrote | `sizeof(uieventnode)` = **32** (native; two 8-byte pointers + padding) |
| `Decode` read | **20** (six real fields, then a 10-byte skip for 32-bit Windows pointers) |

`Decode` had been carefully patched for the 32-bit Windows layout; **`Encode`
never was.** Every event node drifted the stream 12 bytes, so a later count was
read as garbage. Fixed at both encode sites (standard *and* priority queues —
the priority decoder was checked first and uses the identical layout).

Note this is the **inverse** of ACMI-1: there, Windows files were unreadable by
us; here **our own files were unreadable by us**. Same root cause (a struct with
pointers serialised raw), differing only in which side got fixed.

### JOINFAIL-1 — the graceful-failure path segfaults (open)

`CampaignJoinFail()` → `C_Handler::RemoveUserCallback()` → SIGSEGV. The try/catch
added to turn a failed load into a clean return-to-menu **does** catch, and then
the handler itself crashes. AUTOSAVE-1 removes the main trigger, but the safety
net still has a hole and any other load failure will hit it.

### RWY-3 — runway posts step through coarse elevations (open, quantified)

The PO's report: *"just before touchdown the airfield retracts into the distance,
the landing point I had chosen shows up as terrain, with the airfield foreshortened
in the distance."* Now measured — **12 of 31 runway posts change elevation during
the approach**, converging on the true ~−26 ft only as terrain LOD refines:

```
post (780980,1309143):  -14.3 -> -20.2 -> -23.4 -> -24.6
post (781101,1307747):  -13.7 -> -19.3 -> -22.5 -> -24.1
post (778593,1311827):    0.0 -> -22.2 -> -26.0      (starts at sea level)
```

Feature data carries `ORIGINAL z = 0.00` for every post, so elevation comes
entirely from `GetGroundLevel`, which is coarse at range. The far end of the
airfield can sit 26 ft below the near end and then step up on final — which is
exactly the retracting touchdown point. This is the known runway-elevation
decoupling class, but with a **quantified, reproducible signature** for the first
time.

**RWY-2's visibility half is effectively accepted:** the PO landed, with 664
`depth bias ACTIVE` logs. The remaining landing defect is RWY-3, not z-fighting.

### Also open, from the same session

- **TERRAIN-1:** dogfight arena shows a uniformly grey surface below the
  aircraft, with airfields sitting on flat grey polygons. Untextured terrain.
  **2026-08-14: probable root cause found** — the M/L LOD terrain textures are
  absent from the install (416 unique misses: 300 `L*`, 131 `M*`; the DDS set is
  `H*`-only, the PCX set is still inside an unextracted 178 MB `texture.zip`), so
  every medium/low LOD tile ends at `ShiError` with `bits[res] = NULL`. Data fix,
  PO call. See `docs/STATUS_2026-08-14_session.md`.
- **LOAD-1 (regression):** clicking Fly shows a **white** screen instead of the
  animated aircraft-icon progress bar. That animation was fixed in June
  (`cc4e2517`) and `FF_LoadingClear` is supposed to paint black — never white.
  **2026-08-14: quantified** — TE 09 is white to ~31 s and renders the full
  cockpit by 50 s, so the mission does load; the setup phase simply presents
  nothing. Note the deagg-wait splash animation runs **zero** iterations on a
  healthy launch, so the presents must come from `OTWDriver::Enter` itself.
- **PIT-1:** in view 1 the 2D cockpit sits low, peeking up from the bottom of the
  screen at low resolution. Possibly the 16_ckpit.dat art set scaled 0.64.
- **GEAR-1:** view 0 shows no landing gear; the aircraft appears to rest on the
  terrain.

### Found 2026-08-14 (PO test drive)

- **CAMP-1 (NEW, top of the backlog):** entering 3D from the campaign **deadlocks
  permanently** on a white screen. The deagg wait exits after zero iterations
  (`IsAggregate=128 delayCounter=120`, so `flight->IsDead()` is true), which takes
  the failed-launch bail path into
  `WaitForSingleObject(wait_for_sim_cleanup, INFINITE)` (`simloop.cpp:1386`). That
  event is only ever set from `SimulationDriver::Cycle()` (`simdrive.cpp:857`),
  which the Loop thread never calls because the launch never reached
  `RunningGraphics` — every thread parks at 0 % CPU and the last (white) frame
  stays up. **Third instance of the signal-less-INFINITE-wait class (cf. issues #6,
  #13).** Fix: bound the wait, fall through to `FM_START_UI`. Distinct from LOAD-1;
  TE 09 is unaffected (it deaggregates normally and flies).
- **SETUP-1 (NEW):** clicking **Setup** on the main menu SIGSEGVs in
  `UpdateKeyMapButton` (`controltab.cpp:2887`) — `strcat(totalDescrip,
  KeyDescrips[Map.key2])` with a NULL entry, since `KeyDescrips` is a sparse
  `char*[256]` memset to 0 and the guard only rejects `key2 == -1`. Line 2882 has
  the same exposure via `_stprintf`; neither bounds-checks `key2 < 256`.
  Data-dependent on `keystrokes.key`, which is why Setup opened fine before.

### ⭐ ACMI promoted to TOP of the backlog (PO direction)

> *"make sure acmi is prioritized in the backlog - it's important because it gives
> a quantitative measure of sim/pilot performance that can be tracked and optimized"*

Re-framed accordingly: ACMI is not merely a parity oracle, it is a
**measurement instrument**. A readable tape yields position, altitude, attitude
and events (1440 position samples in the PO's landing tape), which turns
subjective judgements into tracked numbers — and would let the PO's own landing
be compared directly against the Wine gold's. It also makes RWY-3 measurable
end-to-end instead of inferred from debug prints.

ACMI-1's struct layout is already fixed and pinned with `static_asserts`
(Sprint 17).

### ⭐ Sprint 20 — `tools/acmi_dump.py`: the tape is now a time series

Reads a `.vhs` and emits the flight as numbers — no display, no game needed.
Validated on two real PO tapes (the 919 KB landing tape and the 7.3 MB
`TAPE0006`, 150 entities / 45 755 positions); both pass the four-way 32-bit
layout self-check.

**Gold landing tape decoded — and it gives RWY-3 its oracle.** 716 samples,
2003 ft → 28 ft:

```
t(s)      alt(ft)    x(ft)      y(ft)    pitch   roll
32523.1      812     760942    1313739   0.153   0.004
32546.7      401     766494    1311961   0.178  -0.010
32565.9      135     770936    1310336   0.179   0.002
32574.5       28     772911    1309677   0.233   0.006   <-- TOUCHDOWN
32595.9       28     776118    1308574   0.229  -0.144
32612.9       28     776126    1308571   0.229  -0.144   <-- stopped
```

Smooth descent, wings level (roll ≈ 0 throughout), and — the decisive part —
**ground altitude pinned at exactly 28 ft for the whole rollout, never varying.**

That is the direct counterpart to RWY-3: on Windows the airfield surface is a
**stable** elevation, while our runway posts step 0.0 → −22.2 → −26.0 as terrain
LOD refines. The gold's 28 ft agrees with our converged ~26 ft, so our *final*
value is right and only the *approach to it* is wrong — which is exactly why the
touchdown point appears to retract.

**RWY-3 acceptance criterion, now objective:** fly the same mission, dump our
tape, and require ground altitude to be constant through the rollout as the gold
is. No screenshots, no judgement calls.

### ⭐⭐ ACMI END-TO-END WORKS — a Windows tape loads in the player

`TAPE0006.vhs` (7.3 MB, Windows-recorded, 150 entities) copied into `acmibin/`,
selected in *LOAD ACMI TAPE*, loaded. The playback UI comes up with transport
controls, timeline slider and camera/focus readouts, and the clock reads
**`05:04:03:10`** — matching the `startTime = 18243.1 s = 05:04:03` that
`tools/acmi_dump.py` decodes independently. **Two separate implementations, one
in C++ and one in Python, agree on the contents of a 7.3 MB binary.**

Getting there needed three more fixes in `SetupSimTapeEntities`, all the same
family as ACMI-1 and all found from the crash backtrace:

1. **`ACMI_Callsigns[e->uniqueID]` indexed with no bound.** `uniqueID` is a VU
   entity id — **1..477** on this tape — used directly as an index into an array
   sized by the callsign count. That was the SIGSEGV. Two sites.
2. **`ACMI_CallRec` was another 32-bit on-disk struct declared with `long`**
   (`teamColor`), making the record 24 bytes instead of 20, so the `memcpy`
   stride was wrong and every callsign after the first was garbage. Missed by the
   Sprint-17 sweep because it lives in `acmirec.h`, not `acmitape.h`. Now
   `int32_t` and `static_assert`-pinned at 20.
3. **`delete` on `new[]` arrays** at two callsign free sites — the
   alloc/dealloc-mismatch class this port has been clearing for months.

Fix 2 arguably caused fix 1's severity: with the wrong stride, the label and
colour read were garbage regardless.

**Crash attribution came straight from the backtrace** (`C_Button::Process` →
`ACMI_LoadACMICB` → `ACMIView::LoadTape` → `ACMITape::ACMITape` →
`SetupSimTapeEntities`), which is what running under gdb bought.

**Next:** dump our own landing tape and diff the approach against the gold's —
RWY-3's acceptance criterion (ground altitude constant through rollout) is then
measurable end-to-end.

## Sprint 21 (2026-08-09) — ACMI capture chain: hooks + a real gap

Trying to produce our own tape for a gold diff exposed a chain of gaps. The
gold half of the diff is already done (28 ft pinned through rollout); this is
about getting *our* half.

### ACMI-3 (defect): a recorded flight can never become a loadable tape

`ACMIRecorder` writes only a raw **`acmibin/acmi*.flt`**. Converting that into a
loadable `TAPEnnnn.vhs` is `ACMI_ImportFile()` — and **nothing calls it**. Both
call sites in `acmiui.cpp` (1278, 1381) are commented out, and the only live
caller is a multiplayer chat command (`ui_comms.cpp:594`, the `.dofile` handler).
So in normal single-player use a recording is written and then never converted.

That matters more than it looks: per PO direction ACMI is the project's
**quantitative instrument** for sim/pilot performance. An instrument whose output
cannot be loaded records nothing usable.

`FindFirstFileA` was checked and is **not** at fault — the Linux compat version
converts backslashes, resolves the directory case-insensitively and uses
`fnmatch(FNM_CASEFOLD)`. The enumeration works; it is simply never reached.

### Also learned: the tape is flushed by StopRecording, not by recording

A tape only materialises when recording **stops**. Any flight cut short — crash,
harness kill, hang — leaves the recording live and **nothing on disk**. That is
very likely why the PO's hand-flown landing produced no tape despite a clean
exit.

### New automation hooks

| hook | effect |
|---|---|
| `FF_AP_MODE=<0\|1\|2>` | forces the autopilot mode **at the dispatch site** (`SimToggleAutopilot`), which is where the `APNormal` + realistic-avionics branch diverts to `SimRightAPSwitch`. The earlier override inside `AircraftClass::ToggleAutopilot` could never fire. |
| `FF_DEBUG_AP=1` | reports the mode in force and traces `ToggleAutopilot` at **function entry** |
| `FF_ACMI_RECORD=1` | starts an ACMI recording as the mission begins |
| `FF_ACMI_STOP=<sec>` | stops recording N seconds in — this is what **flushes the tape** |
| `FF_ACMI_IMPORT=1` | runs `ACMI_ImportFile()` at UI start, converting `acmi*.flt` → next free `TAPEnnnn.vhs` |

**Sprint 18's diagnosis is retro-validated.** With `FF_AP_MODE=0` the log shows:

```
[AP] SimToggleAutopilot: mode forced to 0 (was 2), realisticAvionics=1
[AP] ToggleAutopilot ENTER (autopilotType=4, option=2)
```

`ToggleAutopilot` had **never** been entered in any previous run. Forcing the
mode at the dispatch reaches it, confirming both the `SimRightAPSwitch` diversion
and that the first override was placed one level too deep.

### Honest notes

- **`SDL_VIDEODRIVER=dummy` does not work for FreeFalcon** — it dies during init;
  this game needs real GL. Headless ACMI conversion is not an option.
- **The recorded `.flt` from the test flight was lost to my own mistake:** a
  `pkill -f` whose pattern matched its own command line, killing the shell and the
  queued run. That is the self-match trap already recorded in the project notes
  (`pgrep -x`, not `pgrep -f "<string in my own argv>"`). Cost: one flight.

## Sprint 22 (2026-08-09) — the PO's landing, measured against the gold

The PO hand-flew TE "09 Landing Final Approach" and landed, with ACMI recording
armed. **544 samples, 210.8 s, 2013 ft → 28 ft** — a complete approach. Both
profiles are committed as `docs/acmi/landing_gold.csv` and
`docs/acmi/landing_ours.csv`.

### ⭐ Our landing is quantitatively equivalent to the Windows landing

| | gold (Wine) | ours (native) |
|---|---|---|
| touchdown | t+156.6 s, **36 ft** | t+160.9 s, **36 ft** |
| touchdown position | (772728, 1309737) | (773345, 1309997) |
| rollout min altitude | **28.3 ft** | **28.3 ft** |
| rollout spread | 8.8 ft | **7.6 ft** |
| rollout samples | 148 | 133 |

Identical touchdown altitude, identical minimum ground altitude to a tenth of a
foot, touchdown points ~670 ft apart (two hand-flown approaches), and our spread
is marginally *tighter*. **This is the first hard evidence that the port's flight
and ground-collision behaviour matches the oracle**, and it is exactly the kind
of tracked, optimisable measure ACMI was prioritised for.

### ⚠️ CORRECTION: "the gold is pinned at 28 ft" was a sampling artifact

Sprint 20 recorded that the gold's rollout altitude was *"pinned at exactly 28 ft
for the whole rollout, never varying"*, and RWY-3's acceptance criterion was
built on that contrast. **It is false.** The dump printed every *N*th sample
(`step = len(sel)//40`), which happened to land on 28s. The full series shows the
gold varying **28.3 → 37.1 ft**, a spread of 8.8 ft — slightly *more* than ours.

Both builds vary through rollout by a similar amount. There is no
stable-vs-stepping contrast, and the criterion derived from it was wrong.

### RWY-3 reproduces — but ACMI is the wrong instrument for it

The same flight's `FF_DEBUG_RUNWAY` log shows **12 of 31 runway posts changing
elevation mid-approach** (e.g. `-19.6 → -25.1 → -26.0`), identical to the
automated flights. The PO's description — *"the runway pulled away from me, like
a carpet being pulled out from under me"* — is real and reproducible.

But the tape does **not** capture it, and cannot: **ACMI records aircraft state,
not rendered scenery.** The jet lands correctly because collision uses the
converged elevation; what moves is the *rendered runway surface* while terrain
LOD refines. That is why the touchdown point became terrain visually while the
altitude series looks normal.

**So RWY-3 is a rendering/geometry defect, not a flight-dynamics one.** Its
acceptance criterion is therefore **not** an ACMI measure but the runway-post
series: *no runway post may change elevation during an approach.* Currently
**12 of 31 do**.

Two lessons, both mine: a criterion was built on a subsampled series without
checking the full data, and an instrument was chosen before establishing that it
measures the quantity in question. The tape answers "did the aircraft behave?" —
it never could answer "did the scenery move?".

## Sprint 22 (2026-08-10) — OUR LANDING vs THE GOLD: the first quantitative parity result

The PO hand-flew TE "09 Landing Final Approach" twice under gdb with ACMI
recording. The second flight captured cleanly: **544 samples, 210.8 s,
2013 ft → 28 ft**, read straight out of the raw `.flt` (bypassing the broken
import, ACMI-4).

### ✅ Result: our landing is quantitatively equivalent to the Windows landing

| | GOLD (Wine) | OURS (native) |
|---|---|---|
| samples | 716 | 544 |
| duration | 195.8 s | 210.8 s |
| touchdown | t+156.6 s, **36 ft** | t+160.9 s, **36 ft** |
| touchdown position | (772728, 1309737) | (773345, 1309997) |
| rollout min altitude | **28.3 ft** | **28.3 ft** |
| rollout spread | 8.8 ft | **7.6 ft** |

Identical touchdown altitude, identical rollout minimum to a tenth of a foot,
and our spread is marginally *tighter*. Touchdown points are ~670 ft apart,
which for two independently hand-flown approaches is close.

**This is the first hard evidence that the port's flight and collision
behaviour matches the oracle** — and it is exactly the kind of tracked,
optimisable measure the PO asked ACMI for.

### ⚠️ Correction: the "pinned at 28 ft" claim was a sampling artifact

Sprint 20 recorded that the gold's ground altitude was *"pinned at exactly 28 ft
for the whole rollout, never varying"*, and RWY-3's acceptance criterion was
built on that contrast. **It is not true.** That reading came from
`tools/acmi_dump.py --approach`, which prints every *N*th sample
(`step = len(sel)//40`); the subsample happened to land on 28s. The full series
shows the gold varying **28.3 → 37.1 ft, a spread of 8.8 ft** — comparable to
ours. There was no stable-vs-stepping contrast to build on.

Same failure mode as the truncated greps earlier in this run: a display
convenience silently dropped data, and the conclusion was drawn from what
survived. **Print the aggregate (min/max/spread/distinct) alongside any
subsampled series** — the aggregates are what caught it here.

### RWY-3 reframed: a RENDERING defect, and ACMI is the wrong instrument

The PO's report stands — *"the runway pulled away from me, like a carpet being
pulled out from under me"* — and `FF_DEBUG_RUNWAY` from that very flight
reproduces it exactly: **12 of 31 runway posts change elevation mid-approach**
(e.g. `-19.6 → -25.1 → -26.0`), identical to the automated runs.

But the tape shows the aircraft landing normally. Both are true because they
measure different things:

- **ACMI records aircraft state** — where the jet was. Collision uses the
  converged elevation, so the landing is correct and matches the gold.
- **The defect is in the rendered runway surface**, which moves while terrain
  LOD refines. That is what looked like a carpet being pulled, and why the
  chosen touchdown point turned into terrain.

So RWY-3 is a **rendering/geometry defect, not a flight-dynamics one**, and the
earlier acceptance criterion was wrong twice: wrong in its premise (the
artifact above) and wrong in kind (measuring the aircraft when the defect is in
the scenery).

**Correct RWY-3 acceptance criterion: no runway post may change elevation
during an approach.** Currently 12 of 31 do. Instrument: `FF_DEBUG_RUNWAY=1`,
post-elevation series per position.

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

## EPIC TE2 — TE "02 Takeoff" playable to rotation (opened 2026-08-15, PO-raised)

**PO report (2026-08-15):** flying TE "02 Takeoff", the aircraft sits bogged in
terrain with no runway visible, the throttle does not move it, and it explodes.

**Gold standard (PO-supplied, Wine, 2026-08-15).** Two shots of the same mission
start from the Windows build in the same Wine prefix
(`~/sgl/SAT/freeFalcon/WP`, same game data our build loads):
- *Cockpit:* runway stretching ahead, taxiway/apron, hangars and tower on the
  horizon, HUD live.
- *Orbit camera:* **two F-16s line astern**, both with gear down **on the
  tarmac**, runway edge markings clear.

**Acceptance:** our TE 2 start matches those two shots — flight staggered on a
visible runway, gear on the surface, throttle produces a takeoff roll, nothing
explodes.

### TE2-1 — whole flight spawned on one coordinate (fixed 2026-08-15)

`AircraftClass::FindBestSpawnPoint`'s `START_RUNWAY` branch (added by `207de9d7`
to stop aircraft spawning ~30 ft off the runway) called
`TranslatePointData(obj, initData->ptIndex, ...)` for **every** aircraft. Every
flight member arrives with the *same* `ptIndex`, so the whole flight was moved
onto a single point — two jets inside each other. Measured with
`FF_DEBUG_SPAWN=1`: natural positions `(1044551.1,1270545.3)` and
`(1044692.4,1270449.5)` — ~150 ft apart — both rewritten to
`(1044660.3,1270538.1)`.

That matches the PO's symptoms exactly: at idle the two coexist; the moment the
throttle advances they move into each other, and `[DEATH]` shows **six aircraft
dying on one timestamp** (`t=25114858`) via the `pctStrength <= -1.0` bleed in
`SimVehicleClass::Exec` → `SetExploding` → `AircraftClass::Exec`
(`aircraft.cpp:1867`) → `SetDead`.

Unlike the RAMP/TAXI branch this path has no `FindDesiredTaxiPoint` handing out
distinct points and no `PT_OCCUPIED` reservation, so that mechanism cannot just
be copied. **Fix:** project each aircraft onto the runway centre line while
keeping its own along-runway distance — still on the runway (what `207de9d7`
wanted) but preserving the line-astern spacing the TE data already provides,
which is what the gold orbit shot shows.

**Sprint 1 (2026-08-15) — closed.** TE2-1 fixed and verified numerically; TE2-2
root-caused and a fix landed, awaiting visual acceptance. Also cleared four
defects found while instrumenting: the ~30s 3D SIGSEGV (signed 16-bit start
vertex in `CDXEngine::DrawSurface`), and the `objectiv.cpp` /`unit.cpp` /
`ctree.cpp` memory defects. Regression: 200s IA ASAN soak clean — 3D at 62 FPS,
zero ASAN errors, zero `tex.cpp` assertions, `rc=124`.

### TE2-2 — aircraft buried in terrain, runway not rendered (fix landed, needs eyes)

Physics places the aircraft correctly: `[GROUND]` reports `groundZ=-26.00` with
the aircraft reference at `-31.99`, a gap of exactly `CheckHeight` (5.99) — wheels
on the collision surface. But the PO's screenshot shows the jet sunk to
mid-fuselage in grass with no runway at all, so the **rendered** terrain sits
above the collision surface that `GetGroundLevel` reports.

Meanwhile the runway feature carries `initData->z = -5.00`, **21 ft below** the
collision ground — so the runway is buried under both surfaces.

Three elevation sources disagree at the same x/y: runway feature (−5.00),
collision query (−26.00), rendered terrain (higher still). The gold shows all
three coincident.

**SUPERSEDED — see "container re-pick" below.** The decal analysis that follows
was reasoned about a code path that was not executing at the player's airbase at
all (no flat surfaces were being inserted there), so its conclusion could not be
observed and was wrong. Kept for the record.

**Root cause found and fixed (75bc6056).** `FF_DEBUG_RUNWAY` gave it directly:

```
[RUNWAY] flat GetGroundLevel=0.0 decal=3.0 -> z=-3.0
```

Flat runway/tarmac surfaces were drawn at `GetGroundLevel` **minus a 3 ft
decal** (negative z is up), so the drawn surface floats 3 ft above the surface
the wheels rest on — every parked aircraft sunk 3 ft into it. The decal existed
to stop the runway z-fighting the terrain mesh, but that job moved to the
slope-scaled `glPolygonOffset` applied at flush time to this exact batch
(`9ed8f3b2`, `FF_SetRunwayDepthBias`). Redundant, and harmful. Defaulted to 0;
`FF_RUNWAY_ZLIFT=<ft>` restores a lift.

Confirmed non-accumulating: the decal recomputes from `GetGroundLevel` each
frame rather than from the stored z, so it never drifted (179 samples all
`z=-3.0`, never `-6.0`).

**Sprint 2 correction — the aircraft is NOT buried, and this is not an elevation
bug.** Captured our own sim frames (`FF_VIEW_SCRIPT` + ImageMagick; note
`CLAUDE.md`'s "agent can't capture sim frames" is stale, fixed by `ed3e1274`) and
extended `FF_DEBUG_GROUND` to sample all three elevation sources at the player:

```
[GROUND] acZ=-31.99 groundZ=-26.00 aboveGround=5.99 vpAccurate=-26.00 vpApprox=-26.00 onGround=1
```

All three agree exactly, and the 2D-pit capture matches the gold closely —
instruments, live HUD, hangars left and tower right on the horizon in the same
places, **horizon at normal eye height**. The "bogged in terrain" appearance
comes from the *external* cameras (chase/orbit) sitting low enough that terrain
occludes the lower fuselage; it is not where the aircraft is.

The decal removal stands on its own merits (drawn surface now coincides with the
collision surface) but was not the cause.

**Real remaining defect: the runway/tarmac is never rendered at the player's
airbase.** With `FF_DEBUG_RUNWAY=1` over a full TE 2 run:

- exactly **6** `InsertStaticSurface` calls in the whole run, all belonging to one
  platform at `(1085660,1362831)` — a base ~19 miles away;
- all 25 `DrawablePlatform::Draw(OTW)` traces are that same distant platform;
- our airbase (aircraft at `(1044566,1270503)`) inserts **no** flat surfaces at
  all, while its buildings clearly render in the capture;
- **no** `flat surface SKIPPED (prio > BuildingDeaggLevel)` traces, so they are
  not being priority-filtered — they never reach that code path.

**Root cause narrowed to one flag (2026-08-15).** A trace at the container
dispatch (`addobj.cpp`, `FF_DEBUG_RUNWAY`) logs every feature that reaches it
with its position and the two flags that decide its fate. Over a full TE 2 run,
644 checks:

```
190x  alreadyBuilt=0 ELEV=0 FLAT=0  pos=(1041393,1267712)   <- the PLAYER's airbase
 34x  alreadyBuilt=1 ELEV=0 FLAT=1  pos=(1084875,1362831)   <- the distant one, works
 59x  alreadyBuilt=1 ELEV=1 FLAT=0  pos=(998784,1369793)    <- a bridge (ELEV)
```

`(1041393,1267712)` is the player's airbase — it matches the `objPos` in the
`FindBestSpawnPoint` trace. It reaches the dispatch 190 times and **never has
`FEAT_FLAT_CONTAINER` set**, so it never becomes a `DrawablePlatform`, never
gets `InsertStaticSurface` called, and its runway is never drawn. The base 19
miles away does have the flag and renders its 6 flat surfaces correctly.

So this is not a rendering bug at all — the airbase is never classified as a
flat container. `campaignFlags` is populated from `initData->flags`
(`simbase.cpp:310`), which for features comes from the feature class data
`Flags`. Since the PO's Wine gold shows a runway at this very base, the flag
should be set and we are failing to derive it.

**Next step:** compare the feature-class `Flags` our loader produces for the
objective at `(1041393,1267712)` against what the data actually holds — this
port has a long history of 32/64-bit field-width bugs in exactly this kind of
decode (`fourbyte`, `unit_flags`, the DIRTY_* decoders), and a shifted flags word
would present exactly like this.

This also re-frames the older `CLAUDE.md` entry "runways/airstrips invisible +
can't land": that is this defect, and it is about surface *insertion*, not
elevation.

**This contradicts the RWY-3 reframing above** ("RWY-3 is a rendering defect;
the collision elevation is correct, proven by Sprint 22's landing parity").
Sprint 22 measured an *airborne approach*, where the aircraft never touches the
ground and interpenetration cannot arise, so it could not have caught a
ground-start placement error. Both records are right about different things.

### TE2-2 RESOLVED — the runway now renders (container re-pick + 3ft decal restored)

Two changes, in this order, and the order matters:

1. **`2101bfa2` — find the real container.** `CreateDrawable` took the objective's
   container from `GetComponentLead()`, and the comment claims the container is
   "stored in the lead element". It is not: the lead is just
   `components->GetFirst()` of a `TailInsertList`, i.e. whichever feature was
   created first. Per-feature tracing of the player's airbase:

   ```
   f=0   pos=(1041393,1267712) classID=2370 flags=0x1    FLAT=0   <- picked as lead
   f=20  pos=(1041393,1267712) classID=1918 flags=0x115  FLAT=1   <- same spot, IS a container
   ```

   Two features sit at the objective centre and we picked the one without the
   flag. Across a whole TE 2 run exactly one objective had a container as its
   `f=0` — and that is precisely the one base whose runway rendered. Now, if the
   lead carries neither container flag, the components are scanned for one that
   does. Platforms built 1 -> 2, flat surface inserts **6 -> 63**.

2. **Decal restored to 3ft.** With the surfaces finally being inserted, the decal
   could be measured for the first time: at 0 the tarmac loses the depth test and
   vanishes (cockpit view is plain grass), at 1ft it is still gone, at **3ft** it
   renders. So the slope-scaled `glPolygonOffset` from RWY-2 is *not* sufficient
   alone and the geometric lift is doing real work. This reverts the default set
   in `75bc6056`, which had been reasoned about a path that was not executing.

**Verified visually:** orbit capture shows two F-16s line astern on grey tarmac
with the treeline and tower behind — matching the shape of the PO's Wine gold,
where before there was only grass.

**Remaining gaps against the gold** (tracked as TE2-5 and below): the parked jets
are sunk ~3ft so the gear is hidden, the tarmac has no runway markings, and the
2D-pit view still shows grass further ahead, suggesting the flat surfaces are not
drawn out to the distance the gold shows.

### SESS-5 — SIGINT/SIGTERM tore down GL from the signal handler (fixed)

A repro run left a process alive for 9+ minutes after both SIGINT and SIGTERM, 19
threads parked in `futex_do_wait`. `ptrace_scope` forbids attaching to a running
process, so it was reproduced with gdb as the *parent*: deliver SIGINT during a
live TE 2 flight, then interrupt again to capture the state.

The handler was doing this:

```c
static void signal_handler(int sig) {
    ...
    SDL_GL_DeleteContext(g_GLContext);   // sim thread is mid-draw with this
    SDL_DestroyWindow(g_SDLWindow);
    SDL_Quit();
```

It runs on the main thread while the **sim thread is still rendering** with that
context. gdb caught the consequence directly:

```
SimulationLoopControl::Loop -> OTWDriverClass::RenderFrame
  -> ContextMPR::FlushPolyLists -> RenderPolyList
    -> D3D7Dev_DrawPrimitiveVB -> D3D7Device::DrawVertices
      -> libnvidia-glcore   <-- SIGSEGV
```

So the "hang" was really two failure modes from one cause: the context deleted
under a drawing thread (segfault), and `SDL_Quit()` deadlocking against the
still-running campaign/sim threads (the futex parking). None of those three SDL
calls is async-signal-safe in the first place.

Fixed by making the handler `_exit(128 + sig)` — async-signal-safe, and the
kernel reclaims window, context and threads without racing them. This matches
what `main()` already does on the normal path (`_exit(0)` after "Goodbye!"), and
the orderly Exit-button shutdown never reaches this handler.

Verified: two TE 2 runs SIGINT'd mid-flight now give 0 crashes and 0 leftover
processes (previously SIGSEGV/SIGABRT plus a surviving process); the Exit button
path still ends `rc=0` with "Goodbye!" and no crash.

### FARTEX-1 — far textures above id 31128 were never released (16-bit truncation)

`FarTexDB::Release` bounded the texture id with a **`(WORD)`** cast:

```c
ShiAssert(texID < (WORD) texCount);
```

`texCount` is an `int` and is **96664** for the Korea theater, so the cast
truncates the bound to `96664 & 0xFFFF = 31128`. `TextureID` is a `DWORD`, and
`Request()` correctly compares against `(DWORD) texCount` — so ids above 31128
are loaded and refCounted on request, while `Release()` judged them out of range
and returned early. **Their refCount was incremented and never decremented: those
far textures were never freed.**

Found by instrumenting the long-ignored `[Failed: texID < (WORD) texCount]`
assertion instead of silencing it. The values gave it away immediately — every
reported id was *smaller* than texCount, e.g.:

```
[FARTEX] Release out-of-range texID=33770 texCount=96664 (over by -62894)
[FARTEX] Release out-of-range texID=42846 texCount=96664 (over by -53818)
```

A negative overrun is impossible unless the comparison bound is not what it
appears to be.

This is very likely also the long-recorded *"far texture loading errors (42xxx
IDs not found) — non-fatal"*: 42xxx sits squarely in the wrongly-rejected range.

Fixed by using `(DWORD)`, matching `Request()`. Verified on a 200s IA ASAN soak:
assertions 2 → 0, out-of-range rejections 10 → 0, zero ASAN errors, 3D reached.

### SESS-2 — ctree use-after-free (fixed, exit path now verified)

`C_TreeList::DeleteItem` read `item->Parent->Child` through a freed `TREELIST`
during ATO teardown. Fixed by re-parenting the whole child list to the
grandparent before the node is deleted (children hold a `Parent` back-pointer and
deleting a node does not delete its children, so a parent freed first left every
child dangling; the `F4IsBadReadPtr` guard above cannot detect a freed heap
block).

Verification needed a *real* in-game exit — the soak's timeout SIGINT never
reaches `~C_Hash`. Driven with `FF_UI_CLICK`, using a screenshot of the exit
dialog to find its OK button at (660,501) rather than guessing:

- main menu -> Exit -> OK: `rc=0`, `Goodbye!`, **0 ASAN errors**
- main menu -> Tactical -> mission 02 -> RESCUE (back) -> Exit -> OK: `rc=0`,
  `Goodbye!`, **0 ASAN errors**

Caveat: neither run logs ATO-tree activity, so this corroborates the fix on the
teardown path rather than proving the exact populated-tree case from the original
report (which followed a full TE flight).

Handy for future automation: **RESCUE (back) and the main-menu Exit are both at
(49,750)**, and the exit confirmation's OK is at (660,501), CANCEL at (362,500).

### TE2-6 — runway invisible at distance (FIXED: depth bias default too weak)

After the container re-pick, the player's airbase inserts 63 flat surfaces (all
of them: `flat surface SKIPPED (prio > BuildingDeaggLevel)` count is 0) and the
orbit capture clearly shows the jets standing on grey tarmac. But both the 2D pit
and the 3D virtual pit still show **grass ahead**, where the PO's gold shows the
runway stretching away from the aircraft.

Not a heading problem: the ATC data gives runway ends 020/200, and the aircraft
sits at −159.9° ≡ 200.1°, i.e. correctly lined up with runway 20 — matching the
TE screen's own SitRep, *"You are lined up on the runway, ready for take off."*

**Measured — the surfaces ARE there, they are just invisible at distance.**
Logging each inserted surface's position and transforming into the aircraft's
frame (along = ahead, cross = right) shows an unmistakable runway chain running
away from the aircraft:

```
along=  176  cross= -493
along=  514  cross=    8
along= 1038  cross= -498
along= 2438  cross= -500
along= 3585  cross=    3
along= 5458  cross= -505      <- over a mile of runway, dead ahead
```

24 surfaces lie ahead of the aircraft, 21 within 3000 ft, the nearest 167 ft
away. So insertion, position and heading are all correct — and the tarmac
directly under the jet does render (visible in the orbit capture). What fails is
visibility with **distance**.

That points squarely at depth precision: the 3 ft geometric lift wins the depth
test against the terrain mesh close up and loses it further out, so the runway
dissolves into terrain as it recedes. This is the same family as RWY-2/RWY-3 and
matches the PO's older description of the runway "pulling away like a carpet".

**FIXED — the depth bias default was far too weak.** It *is* being applied (746
`depth bias ACTIVE` activations in a run), just not strongly enough:
`glPolygonOffset(-3, -64)` only wins the depth test within a few hundred feet.
Measured by sampling the ground band of a 2D-pit capture:

| `FF_RUNWAY_BIAS` | far band | mid band |
|---|---|---|
| `-3,-64` (old default) | grass | grass |
| `-8,-1024` | tarmac | grass |
| **`-16,-2048` (new default)** | **tarmac** | **tarmac** |

With the new default the cockpit view shows a continuous runway stretching ahead
with the aircraft lined up on it, and matches the PO's Wine gold: runway, apron,
hangars left, tower right. Verified on the default build — pit far, pit mid and
the orbit view under the aircraft all read tarmac.

### ZBIAS-1 — D3DRENDERSTATE_ZBIAS was mistranslated (fixed); it is NOT the runway lever

The engine carries a per-surface `dwzBias` (`dxdefines.h:180`) and
`CDXEngine::DrawSurface` pushes it as `D3DRENDERSTATE_ZBIAS` before drawing —
the mechanism by which the original game draws decals that stay coplanar with
what they sit on. The compat layer translated it as:

```c
glPolygonOffset(0.0f, -(float)value);
```

Two things wrong: **factor 0**, so no slope term at all — exactly the case that
matters for a long flat surface at a glancing angle — and D3D7 `ZBIAS` is a 0..16
enumeration rather than GL depth units. Fixed by adding the slope term.

**Hypothesis that this explained the runway: DISPROVEN.** Tracing the actual
values shows the data asks for `dwzBias=1` (34223 draws) and `dwzBias=16` (519),
but the flat runway/tarmac surfaces carry **0** — they never take this path.
Confirmed directly: with `FF_RUNWAY_ZLIFT=0 FF_RUNWAY_NOBIAS=1`, relying on the
repaired ZBIAS alone, the runway does not render (bands read grass). So the
Windows build must keep the runway visible by some other means, and *why* remains
open — it is not this render state.

Scale left deliberately conservative (1.0 factor and 1.0 units per step, i.e. the
original unit magnitude plus the missing slope term). An earlier revision scaled
units 128x per step, anchored on the -2048 that works for the runway batch — but
that anchor is invalid precisely because the runway does not use ZBIAS, and it
would have applied a large speculative offset to 34k surfaces per frame.
`FF_ZBIAS_SCALE="factor,units"` tunes it if a reference is ever available.

Verified no regression: TE 2 pit bands all read tarmac with defaults.

### TE2-5 — runway drawn 3ft above where aircraft stand (FIXED, visual lift)

**Fixed by lifting the DRAWABLE, not by touching physics.** Flat surfaces are
drawn `FF_RunwayDecal()` (3 ft) above the terrain so they win the depth test;
aircraft are placed with their wheels at `GetGroundLevel`, i.e. the terrain
height. `OTWDriverClass::ObjectSetData` now applies the same offset to the
drawable while the aircraft is on the ground.

Deliberately visual only — `obj->ZPos()` is untouched, so collision, the flight
model, ACMI recording and the Sprint-22 landing parity all see exactly what they
saw before. `FF_NO_GEAR_LIFT=1` disables.

Verified: the orbit capture now shows the jet standing on the tarmac with its
underside and gear visible, where before it was cut off at mid-fuselage.

Two candidate fixes were ruled out by measurement first, and are recorded so they
are not retried: shrinking the lift (0/1/2 ft all make the runway vanish
entirely) and the `D3DRENDERSTATE_ZBIAS` path (the runway surfaces carry
`dwzBias=0` and never use it — see ZBIAS-1).



`drawbldg.cpp` places flat surfaces at `GetGroundLevel - 3ft` so they win the
depth test, but ground aircraft are placed with their wheels *at*
`GetGroundLevel`. A parked jet is therefore sunk 3ft into the tarmac and its gear
is invisible — visible in every external capture, and the origin of the PO's
"bogged down in terrain" description.

**Mechanism confirmed from the PO's flight (2026-08-15).** Two separate things
were being conflated, and `FF_DEBUG_CAM` separated them:

1. *The camera is NOT going underground.* The clamp at `otwloop.cpp:2483` runs
   every frame and pins the camera 5 ft above **terrain** — the trace prints
   `viewPos` pre-clamp, which is why raw samples look negative. But the tarmac is
   drawn 3 ft above terrain, so the camera ends up only **2 ft above the visible
   surface**.
2. *The "submerging" is depth-bias occlusion.* Pulled hard enough to beat the
   terrain, the polygon offset also makes the tarmac beat the **aircraft**, so the
   surface draws over the jet's lower fuselage. Raising the view angle exposes
   more of that plane, hence "any elevation of the view causes the aircraft to
   start submerging".

That puts coverage and occlusion in direct tension: a larger bias fixes the near
tarmac gap and worsens the occlusion. Measured, decal 0 with a very large bias
(`-128,-65536`) renders every band but still occludes the aircraft, so
coplanar-plus-bias is not an escape either.

**Therefore the only real fix is the one this item has always named: ground
contact must use the surface that is drawn.** With the aircraft sitting *on* the
tarmac rather than 3 ft inside it, the bias can drop back to a modest value that
beats terrain without beating aircraft. That means touching the per-frame ground
settle in the airframe, so it is left for explicit PO sign-off rather than done
opportunistically.

Measured by sampling the ground band of an orbit capture (grey =
tarmac visible, green = terrain only):

| lift | ground band avgRGB | verdict |
|---|---|---|
| 0 ft, even with `FF_RUNWAY_BIAS="-8,-512"` | green | invisible |
| 1 ft | green | invisible |
| 2 ft | 64,72,54 green | invisible |
| **3 ft (default)** | **94,94,90 grey** | **visible** |

Note the 0ft row: raising the slope-scaled `glPolygonOffset` far beyond its
`-3,-64` default does *not* rescue a coplanar surface, so the depth bias is not
the lever here — these surfaces go through the deferred flush path. The
geometric lift is load-bearing and 3 ft is the minimum that works.

That rules out the cheap fixes and leaves the real one: ground contact must use
the drawn surface. Not attempted yet — it means touching aircraft ground physics,
and the spawn-time placement is re-settled by the physics each frame, so a
placement-only offset would be undone.

### TE2-3 — popup MFDs in HUD view (WITHDRAWN as diagnosed, symptom still open)

**The diagnosis below was wrong and the fix has been reverted.** `VirtualMFD[]`
*is* rebuilt for the real resolution at `otwdrive.cpp:2491-2508` — `MfdSize` is
rescaled `154 * DispHeight / 480` and all five rects are recomputed from
`DispWidth`/`DispHeight`. The static 640x480 initializer at `:157` is only a
placeholder that never survives `OTWDriver::Enter`. A second rebuild added after
it double-scaled the value (`side=352` where the correct figure is 246) and made
the panels bigger, not right.

**What stands:** our HUD-view capture does show four large MFD panels in the
corners, and the PO reports a stray small panel at bottom centre in "1 view".
The corner panels are the *designed* size (246px squares at 1024x768), so the
open question is not their geometry but whether popup MFDs should be drawn in
that view mode at all — which needs a Wine side-by-side in the same view, not
more code reading.

<details><summary>Original (incorrect) diagnosis, kept for the record</summary>



PO report: *"Try 1 view, a small '2' view appears incorrectly at bottom centre."*
Our HUD-view capture shows the same family of defect from the other end — four
**oversized** MFD panels pinned to the screen corners.

`VirtualMFD[]` (`otwdrive.cpp:157`) is a static table of pixel rects written for
a **640x480** screen: 154px squares placed with literal `640 -` and `480 -`
offsets. `MFDClass` then normalises those rects by
`DisplayOptions.DispWidth/DispHeight` (`mfd.cpp:182-185`, `:943-946`), which are
**1024x768** here. Two different pixel spaces, so every popup MFD is placed and
sized wrongly.

Fixed by rebuilding the table against the real display size in
`OTWDriver::Enter` before the MFDs are constructed, keeping the original
proportions (a square 154/640 of the width, one per corner). `FF_DEBUG_MFD=1`
logs the rebuilt geometry.

Note this is resolution-dependent, which is why it never showed on the original
640x480 target and why the PO sees it at 1024x768.

</details>

### TE2-4 — the player's TE 2 flight is killed BY deaggregation (release build)

**This retracts the TE-02 retirement below.** That retirement was based only on
ASAN runs and the PO's gdb session. On the **release** build — the one actually
played — TE 2 still fails, and the cause is now identified.

Controlled comparison, identical click script and mission, differing only in
binary:

| build | at the deagg wait | outcome |
|---|---|---|
| `build-asan` | `IsAggregate=0` (already deaggregated) | reaches 3D, 95 `[GROUND]` samples |
| `build` (release) | `IsAggregate=128 IsDead=1` | never reaches 3D |

The wait exits immediately — `delayCounter` is still 120, untouched — because
`ddd20274` added `and not flight->IsDead()` to the loop. So this never was a
deaggregation *race*: the flight is already dead when the wait begins.

A backtrace from `UnitClass::SetDead` (new `FF_DEBUG_DEATH` trace in
`UnitClass::SetDead`, resolved against the release symbol table) gives the chain:

```
UnitClass::Deaggregate(FalconSessionEntity*)
  -> RegroupFlight(FlightClass*)
     -> UnitClass::KillUnit()
        -> UnitClass::SetDead(int)
```

**Deaggregating the flight is what kills it.** `RegroupFlight`
(`camptask/flight.cpp:4894`) is the *disband* routine — it releases the
callsign, returns pilots to the squadron with `PILOT_AVAILABLE`, resupplies
squadron stores and ends with `flight->KillUnit()`. It belongs at mission end,
not at deaggregation.

**ROOT CAUSE FOUND AND FIXED — an out-of-bounds array read in the runway scan.**

A `build-relg` variant (Release + `-g`, same `-O3` codegen) resolved the call
site exactly: `unit.cpp:1667`, `CancelFlight((Flight)this)`, reached because
`GetDeaggregationPoint` returned `DPT_ERROR_CANT_PLACE`. That value has one
source — `FindTaxiPt`'s `if (not rwindex) return DPT_ERROR_CANT_PLACE; // runway
is toast` — so `ATCBrain::FindBestTakeoffRunway` was returning 0.

Instrumenting the scan showed the inputs were *fine* — `numRwys=2`, both runways
`state=0` (usable), indexes `{1,2}` and `{4,5}`, headings 020/200 — yet `best`
came out still at its initial 91, meaning nothing was ever accepted. Recording
the loop's own iterations (into arrays, no I/O inside the loop, which perturbs
the timing) gave it away:

```
loop iterations=2
ITER i=0 j=0 idx=1 data=20 delta=171
ITER i=1 j=0 idx=4 data=20 delta=171
```

**Only `j==0` ever ran.** The scan never looked at the second runway end. With
wind at 191°, only the 020 ends were considered (delta 171, rejected against
`best=91`) while the 200 ends (delta 9, easily accepted) were never examined.

The loop was written

```c
for (j = 0; runwayStats[i].rwIndexes[j] and j < 2; j++)
```

which evaluates `rwIndexes[j]` **before** the bound, so at `j==2` it reads one
past a 2-element `int` array (`atcbrain.h:145`). That is undefined behaviour, and
`-O3` transformed the loop accordingly. Fixed by testing the bound first,
`for (j = 0; j < 2 and runwayStats[i].rwIndexes[j]; j++)`, at both sites in
`atcbrain.cpp` (`:2393`, `:3043`).

Measured on the uninstrumented release build:

| | before | after |
|---|---|---|
| runs | 5 | 5 |
| result | **5/5 failed** (`IsDead=1`, never reached 3D) | **5/5 reached 3D** (79–101 `[GROUND]` samples) |

Two notes worth keeping. This is why the bug looked build-dependent: `-O2` +
ASAN did not miscompile the UB the same way, so `build-asan` always worked and
the release build always failed. And it is why the loop-level `fprintf` made it
disappear — printing inside the loop changed what the optimiser did.

`CLAUDE.md` lists `[Failed: numRwys > 0]` under "Known Issues (Non-blocking)" as
*"non-fatal assertions that don't crash the game"*. They don't crash it — a
no-usable-runway answer **cancels the player's flight**, which is exactly how TE
2 died.

### TE-02 — TE runway ground start never deaggregates (SUPERSEDED by EPIC TE2)

~~TE "02 Takeoff" with a runway ground start: the StartLoop deaggregation wait
expires (`IsAggregate=128, delayCounter=120`) and bails gracefully to the menu.~~

~~**Retired 2026-08-15 — no longer reproduces.**~~ **That retirement was wrong**
— it generalised from ASAN-build runs only. It does still reproduce on the
release build; see TE2-4 above for the real cause. What holds from those runs is
narrower: under `build-asan` the flight deaggregates cleanly and reaches 3D, so
the failure is build/timing dependent rather than universal.

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

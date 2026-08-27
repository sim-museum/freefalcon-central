# FreeFalcon Linux Port — Current Status

_Last updated: 2026-08-17. Branch `develop`, all commits pushed to origin._
_Latest session: `docs/STATUS_2026-08-17_session.md`._

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
| 23 PO crash-test + autonomous scrum | ✅ done | **One heap corruptor was producing six unrelated crash signatures.** The radio voice chain had never worked (`FRAG_/EVAL_/COMM_FILE_INFO` and `.tlk` are on-disk 32-bit layouts declared `long`); fixing it reached a stubbed speech codec whose `PMSIZE`/`CODESIZE` are never initialised, so `AddNoise` **wrote** past an 80 KB buffer and the damage surfaced in the campaign UI, map refresher, a hash table and the sound streamer. Campaign crash rate ~1 in 3 → **0 in 8**; ASAN heap-buffer-overflow → 0 errors. Also fixed: Setup `strcat(NULL)` crash, **RES-1** (resolution list empty *and* never loaded — six defects), **CRASH-4** AI targeting OOB, **CRASH-5** A/G-with-bombs cast (reproduced, causality both ways), **SAVE-2** (saves stored 120 bytes of uninitialised tail), and the **WARN-1** backlog. Open: PIT-1 unmeasured (its new probe does not work — failure mode recorded), TEX-1 unreproduced |

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

## Sprint log — overnight 2026-08-15/16 (autonomous)

**EPIC TE2 accepted by the PO:** *"takeoff works ... I took off, turned around and
landed successfully ... which makes ff playable."*

Closed this session: TE2-1 (coincident spawn), TE2-2 (runway never rendered),
TE2-4 (flight cancelled by the runway scan), TE2-5 (aircraft drawn 3ft inside the
tarmac), TE2-6 (runway invisible at distance / near-field gap), SESS-1, SESS-2,
SESS-3, SESS-5, FARTEX-1, D3DX-1.

Two themes worth carrying forward:

1. **Chasing long-ignored assertions found real bugs.** The `tex.cpp` asserts hid
   an image leak; `fartex.cpp`'s hid a 16-bit truncation that leaked every far
   texture above id 31128; `context.cpp:1919`'s hid two unimplemented D3DX stubs
   that disabled the loading splash and the in-sim cursor. None were noise.
2. **Measure before believing a mechanism.** Several confident diagnoses this
   session were wrong and were retracted in-tree rather than left standing:
   TE2-3 (MFD geometry), the ZBIAS hypothesis for the runway, "shrink the decal",
   and a premature `decal=0` default. Each is recorded with the measurement that
   killed it.

Regression state at sprint close: 200s and 300s IA ASAN soaks clean (0 errors),
TE 2 reaches 3D with 0 crashes and 0 assertions, dogfight reaches 3D clean under
ASAN, SIGINT mid-flight terminates with no crash and no leftover process, and the
Exit button still exits `rc=0`.

## Sprint log — 2026-08-16, "scrum all might" (autonomous, continuing)

Theme: **finish the `sizeof(long)` sweep by testing the round-trip, not the code.**

Closed: SAVE-1 (now including the `Encode` header bug that made every campaign
save we wrote unloadable — see the entry for the before/after trace), SND-1
(`LoadRiffFormat` returned 0 for every WAV).

The lesson that generalises: the earlier SAVE-1 field-width fixes were all
correct and all verified the same way — "TE 2 still reaches 3D, no crashes" —
which proved only that *loading shipped Windows saves* still worked. None of
them exercised a file **we** had written. The moment the actual round-trip ran
(in-game SAVE dialog → reload from the SAVED tab) the real blocker appeared in
one run, four layers above the fields that had been under inspection. Same shape
as the SND-1 find: audio "works", so the RIFF reader looked fine, but the reader
that was broken serves a *different* set of sounds than the one that was heard.

**Where a fix is verified matters as much as whether it is verified.** Prefer the
test that exercises the artefact the change produces.

**The single highest-leverage change of the session was a build flag.** The build
passed `-w`, so the compiler had been silent for the entire port. `FF_WARN=ON`
(see the new CMake option) turns on a hand-picked diagnostic set, and the *first*
run found work that three careful manual sweeps had walked straight past —
including a live stack buffer overflow. See **WARN-1**. Anything statically
checkable should go through the compiler before it goes through a regex.

Also this session: **SESS-4** (POV hat edge-vs-level), **CRLF-1** (audit closed,
two fixes), **SND-1** (`LoadRiffFormat` returned 0 for every WAV),
**MSG-1** (20 `new[]`/scalar-`delete` mismatches), **STUB-1** (compat stub audit,
closed clean), and **TE2-7** taken as far as it can go from this side — five
more candidate causes eliminated by measurement, the symptom restated twice, and
finally shown to be the boundary between two atlas cells rather than a defect.

Two of those came from working the "keep sweeping" notes left in `CLAUDE.md`.
Both times the *named* suspects were already fixed and the real defects were
adjacent code the note did not mention — the send buffers for MSG-1, nothing at
all for STUB-1. A stale to-do list is still a useful pointer at a *class*; it is
not a work queue.

Regression state at sprint close, all on the ASAN build with every fix in:

| check | result |
|---|---|
| TE 2 → 3D soak | 88 samples, **0 ASAN errors** |
| Instant Action soak | **0 ASAN errors** |
| campaign flow soak | **0 ASAN errors** |
| campaign **save → reload** round-trip | works end to end; `LZSS_Expand` exact at 5881, sane `CurrentTime`, live map |
| TE 2 / campaign / logbook on release build | 0 crashes; pilot roster, callsign and squadron patch all render |
| **extended TE-02 flight soak** (~10 min in 3D) | 211 samples, **0 ASAN errors**, 0 crashes, 0 assertions |

The extended soak matters because the short runs cannot reach the time-dependent
machinery: the texture bank's 10-second release sweep, the campaign aggregation
cycle, ATC re-planning, and particle/trail lifetimes. Ten minutes of continuous
flight with zero findings is a much stronger statement than the 100-second runs.

Incidental observation from its `FF_DEBUG_GROUND` trace, relevant to the
**runway-elevation decoupling** note: at the start `groundZ = 0.00` under a
stationary aircraft, and by the end `groundZ = -26.00` with the aircraft settled
at `acZ = -31.99` — i.e. `aboveGround` stayed sane (2.39 → 5.99) and the terrain
elevation resolved upward once the fine terrain finished loading. Worth keeping in
mind: some of the "airfield is flat at z=0" behaviour is a *load-time transient*,
not a permanent state.

**A false alarm worth recording, because it cost twenty minutes and looked
exactly like a regression.** After the FMT-1 session-decode change, the campaign
SAVE flow stopped producing a file — mtime unchanged, no error. The decode change
was the obvious suspect. It was not the cause: the save name is derived
deterministically from the campaign clock (`Save-Day 1 09 00 05`), earlier test
runs had already created that exact file, and the game will not silently
overwrite an existing save. Clearing the prior test saves made it work first try.
**When a scripted UI flow stops producing output, check for state the earlier
runs left behind before suspecting the code.**

Note on running these: the ASAN build is enough slower that `FF_UI_CLICK`
schedules tuned on the release build drift and later clicks miss their targets.
Budget roughly double the delays, and confirm the flow actually reached the step
under test rather than trusting a "0 errors" line — a run that never got there
also reports zero.

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

### D3DX-1 — Render2DBitmap was a no-op: two unimplemented D3DX stubs (fixed)

`ContextMPR::Render2DBitmap` converts a pixel buffer, creates a temp texture with
`D3DXCreateTexture`, uploads it with `D3DXLoadTextureFromMemory` and draws a quad.
Both D3DX calls were **unimplemented stubs** in `win_only_stubs.cpp` that set the
out-param to NULL and returned `E_FAIL`, so the function threw before drawing.

That silently disabled **both** of its callers:
- the loading splash bitmap (`otwdrive/splash.cpp`), and
- the in-sim mouse cursor (`siminput/sicursor.cpp`).

The only symptom was a recurring assertion, `[Failed: FALSE ==
F4IsBadReadPtr(pDDSTex, ...)]` at `context.cpp:1919` — which is merely
`F4IsBadReadPtr`'s Linux definition of "this pointer is NULL". Exactly the
"stub returns default" class `CLAUDE.md` warns about.

Implemented against the existing `D3D7Surface` machinery: a 32-bit ARGB texture
surface plus a row copy into its pixel buffer, marked `isDirty` so the normal
upload path pushes it to GL.

**Trap worth recording:** the declaration in `compat/d3dxcore.h` named parameters
2-4 `pdwWidth, pdwHeight, pdwMipMapCount`, but the real D3DX7 signature is
`(device, pdwFlags, pdwWidth, pdwHeight, ...)` and `Render2DBitmap` passes
`&dwFlags, &dwActualWidth, &dwActualHeight`. Implementing to the *declared* names
sized the texture from `D3DX_TEXTURE_NOMIPMAP` and the row copy ran off the
source buffer — an immediate SIGSEGV in `SplashScreenUpdate` on the first test.
The header has been corrected so the next reader is not misled the same way.

Verified: TE 2 reaches 3D with 0 crashes and 0 assertions, and a 200s IA ASAN
soak is clean (0 errors) — the row copy is memory-safe.

### CRASH-1/2 — the PO's SIGSEGV, and what finding it uncovered (fixed 2026-08-16)

PO crash flying a campaign OCA strike on autopilot at 8× time compression:

```
SimObjectType::Reference()
BombClass::Start(...)  <- SMSClass::DropBomb  <- AircraftClass::DoWeapons
```

They had released nothing — `DoWeapons()` runs for every aircraft the sim execs,
so this was an AI aircraft releasing on the OCA target. `Reference()` only touches
`mutex` and `refCount`, so crashing inside it means the object was already freed.

**CRASH-1 — `SimObjectType::Release` tested the refcount outside the lock:**

```c
{ F4ScopeLock l(mutex); --refCount; }   // lock scope ENDS here
if (refCount == 0) delete this;         // unsynchronised
```

Two threads releasing the same object can both observe zero and both delete it.
Fixed by deciding under the lock. A sweep for the same shape (locked decrement,
unlocked zero-test) across the built tree found **no other instances**.

**CRASH-2 — then ASAN was pointed at the right flow.** TE 2 had been soaking clean
for hours, but it does not exercise the campaign AI. Driving a *campaign* flight
into 3D under ASAN immediately produced four `heap-use-after-free`s, all in
`ATCBrain::ProcessQueue`, all on the sim thread:

```
READ of VU_ID at ProcessQueue:593 / 659 / 707 / 720
freed by RemoveTraffic(VU_ID, int) at atcbrain.cpp:4119, two frames earlier
in the same call
```

`ProcessQueue` queries `nextTakeoff`/`nextLand` **once** before its loop, and
`RemoveTraffic` inside that loop can free exactly those nodes — so every later
`nextTakeoff->aircraftID` reads freed memory. This is the takeoff/landing path.
Fixed by capturing the two fields actually read afterwards; semantics unchanged,
since those values were computed before the loop either way.

Clearing those exposed five `alloc-dealloc-mismatch`es underneath, in two places:
`related_events[]` in `RegisterEvent` (three sites), and the `loadout` chain —
`SetLoadout` frees the previous loadout with `delete[]`, but three producers
(`FlightClass::LoadWeapons` and two in `iaction.cpp`) allocated a **scalar**
`LoadoutStruct` and handed it over. All made 1-element arrays.

**Result: campaign flight into 3D is now 0 ASAN errors**, from 4 use-after-frees
plus 5 mismatches.

**The lesson is about coverage, not about any one bug.** These had been sitting
under a suite that reported clean all day, because every soak used TE 2 and
Instant Action — neither of which runs the campaign ATC queue or the AI weapons
path. `related_events` and one `loadout` site had even appeared as candidates in
the earlier static sweep and were not followed through, because that sweep's line
numbers came from comment-stripped text. **A sanitiser only finds what you
actually execute; pick the flow that matches the report.**

### PIT-1 — the 3-view (virtual pit) renders no tarmac (open, 2026-08-16)

PO report, with shots: taking off and landing, the **2-view shows the runway and
the 3-view shows grass**. Reproducible headlessly, which makes this cheap to
iterate on:

```
FF_VIEW_SCRIPT="1@62;s@70"   ground band avgRGB (90,91,86)  tarmac
FF_VIEW_SCRIPT="4@62;s@70"   ground band avgRGB (63,82,50)  GRASS
```

Eliminated by measurement, so they are not retried:

| candidate | result |
|---|---|
| runway depth bias too weak in the pit pass | no — `-64,-32768` and even `-256,-131072` change nothing |
| the bias being applied at all | no — `FF_RUNWAY_NOBIAS=1` changes nothing either |
| pit stencil masking discarding the surfaces | no — `FF_PIT_NO_STENCIL=1` changes nothing |
| the geometry not being submitted | no — `FF_DEBUG_FLUSHES` reports **identical** counts in both views, `plain=746 tex=3699` |
| the bias code not running in the pit pass | no — `FF_DEBUG_RUNWAY` shows it active in both (167 vs 161 traces) |

`FF_PROBE_PIXEL="512,300"` attributes the ground pixel in 3-view: the final
writer is a large **terrain** batch (`fvf=0x1d2 nVerts=4380 tex=54 light=1
blend=1`), and the runway atlas never paints that pixel at all. In 2-view the
runway does. All the probed draws report `pit=0`, i.e. the world is drawn before
the pit pass, so this is not the pit geometry overdrawing it.

So the surfaces are submitted, the bias is applied, nothing masks them — and the
terrain still paints over them **in this pass only**.

**Then the measurement method turned out to be the problem, twice.** Recording
this at length because it invalidated two intermediate conclusions:

1. *Sampling one screen band is not enough.* The original "3-view shows grass"
   came from a single band at `H*0.42`. Scanning a **strip** down the screen shows
   the pit view does render tarmac — just only close to the aircraft:
   `......TTTTTTTTTT` versus 2-view's `TTT.TTTTTTTTTTTT`.
2. *Single captures are not always reproducible.* One run of the default bias in
   3-view gave a patchy `......T.......TT`, which read as "more bias helps".
   Running the identical configuration three times gives `......TTTTTTTTTT` every
   time — that run had captured before the scene reached steady state.

With a metric that is actually reproducible, the finding reverses: **in the pit
view the runway's visible extent does not depend on the bias at all.** Default
`-32,-8192`, `-4,-256` and `-256,-131072` all produce the identical strip. In the
HUD and 2D-pit views it depends on bias strongly (`-4,-256` collapses view 0 to
`.......T........`). So whatever limits the runway in the pit view is *not* the
depth fight.

One further caveat that invalidates the original framing: comparing the **same
screen row** between two views is meaningless, because the cameras differ, so a
given row corresponds to a different ground distance in each. Any future
comparison here has to be against ground distance, not screen position.

Next: establish where the pit view's runway actually ends in world terms, and
whether that distance matches the 2D pit's. Until that is measured, "3-view shows
no tarmac" is not established as a rendering defect at all — it may be the same
draw distance seen through a different camera.

**Attempt at that measurement (2026-08-17): the instrument does not work yet.**
Added `FF_PROBE_DEPTH="x,y0,y1,step"`, which reads the depth buffer down a strip
and unprojects each sample with the modelview/projection captured during the
world pass, to state the runway's extent in feet from the eye instead of screen
rows. It runs, but its output is not usable and must not be quoted:

* every sample reads `depth=0.999999` (the far plane), i.e. at capture time the
  depth buffer no longer holds the world pass;
* the recovered eye is `(-1.0, -5.3, -0.9)` and distances come out ~240,000 —
  so the matrices captured are a cockpit-local pass, not the world camera.

Two things were learned that the next attempt should start from:

1. The capture point is wrong. `SaveGLFramebufferAsBMP` runs at end of frame,
   after the cockpit pass; the world depth and matrices have to be grabbed
   *during* the terrain draw, not at swap.
2. Batch-size filtering to find "the terrain draw" is unreliable: with the hooks
   on `DrawIndexedPrimitive`, `DrawIndexedPrimitiveVB` and `DrawVertices`, the
   largest batch seen for a whole frame was **6 vertices** at the timings first
   tried, because the mission was still on the loading screen — the campaign
   click script does not reach a rendered cockpit until ~340 s after sim entry,
   far later than the 55–70 s the earlier PIT-1 runs used.

The probe is left in (env-gated, off by default) but PIT-1 stays open and
unmeasured. Its blocking question is unchanged: where does the runway end, in
ground distance, in each view.


### GEAR-1 — "landing gear is not visible": three causes eliminated (2026-08-16)

The PO reports the jet parked belly-down with no gear at both takeoff and
landing, and appearing to sink further as the view angle rises. Chased it from
the extended soak's ground trace; three plausible causes are now ruled out by
measurement, and the diagnostic is left in place.

1. **Aircraft placed at the wrong elevation — no.** The ~10-minute soak shows the
   engine correcting itself: at spawn `acZ = -2.39` against `groundZ = 0.00`
   (fine terrain not yet loaded), and once it resolves the aircraft settles to
   `acZ = -31.99` over `groundZ = -26.00`, holding `aboveGround = 5.99` stably for
   the whole flight. That is the right standoff for an F-16 on its gear. **Some of
   the "airfield is flat at z=0" behaviour is a load-time transient, not a
   permanent state** — worth knowing for the runway-elevation item too.

2. **Gear geometry never enabled — no.** `AircraftClass::Wake()` is the *only*
   live place that pushes switch state to the drawable (`SetSwitchMask(1..4,
   14..19)`), and it is gated on `OnGround()`. That looked like a race: nothing
   consumes `SimMoverClass::switchChange`, so if the aircraft were woken before
   the ground handler set `ON_GROUND` the gear would never be drawn for the rest
   of the sortie. Measured with the new `FF_DEBUG_GEAR=1`: **`OnGround=1
   gearPos=1.00 -> gear masks APPLIED`** for all four aircraft. Hypothesis dead.

3. **The TE2-5 lift not compensating — no.** The 3 ft runway decal and the
   matching aircraft-drawable lift are both active and both keyed off the same
   `FF_RunwayDecal()`, so they cancel.

Found on the way, and fixed: **SWITCH-1**, a heap overflow where
`SimMoverClass`'s `FILE*` constructor sized `switchData` and `switchChange` by
`numDofs` while reading and writing `numSwitches` elements into them. The stream
constructor does it correctly, which is what marks the other as a slip.

**Still open, and it needs the PO's eyes.** An `FF_RUNWAY_ZLIFT=0` vs default
A/B does change how much fuselage is visible, but the orbit-view stills are not
trustworthy enough to conclude from — the camera distance may also be selecting a
lower LOD without gear geometry. This wants the Wine side-by-side that TE2-7 is
already waiting on.

### TE2-7 — tarmac has no runway markings (open; three causes ruled out)

The PO's Wine gold shows painted runway markings; ours is flat grey. Three
plausible causes have been eliminated by measurement, recorded so they are not
retried:

1. **Texture binding** — `TextureBankClass::Select` is an empty stub and
   `FF_TEXFIX=1` implements it. No visible change. (`CLAUDE.md` already recorded
   "no effect", but that test predates the runway rendering at all, so it was
   worth repeating under the corrected precondition.)
2. **Decal depth** — markings painted on a runway would be classic ZBIAS decals,
   and `dwzBias=16` does occur in the data. Running with a much larger ZBIAS scale
   (`FF_ZBIAS_SCALE="2,128"`) produced no markings.
3. **Missing texture on the surfaces** — disproven directly. The flat runway batch
   carries real, varied textures: `texFlag=1` with `m_TexID` values 1544, 747,
   5031, 50, 187, 746, 734, 177 across surfaces.

4. **Missing / unresolvable marking art — disproven, 2026-08-16.** In DDS mode
   (`SyncDDSTextures`) every object texture id is a loose file
   `terrdata/objects/KoreaOBJ/<id>.dds`, so the ids from (3) can be inspected
   with no instrumentation at all. They resolve, and they contain exactly the
   markings the PO's Wine shot shows:

   | id | size | content |
   |---|---|---|
   | 747 | 131200 (512² DXT1) | runway with **dashed white centreline** |
   | 50 | 174904 | **yellow threshold chevrons** on tarmac |
   | 734 | 131200 | runway **number "8"** + threshold bar |
   | 1544 | 174904 | chain-link perimeter fence (not runway) |

So the art exists, resolves, and is bound to the right surfaces. **The markings
are lost in rendering, not missing from the data** — which retires the whole
"content" line of enquiry that (3) pointed at.

5. **Mip-level blur — disproven by measurement, 2026-08-16.** The leading theory
   was that `d2836c60`'s `glGenerateMipmap` + `LINEAR_MIPMAP_LINEAR` + 8× aniso
   averaged thin white lines into grey. Added `FF_NO_MIPMAP=1` (d3d_gl.cpp) to
   pin sampling to mip 0 and captured the identical HUD-view frame both ways:

   | | median lum | stdev | bright pixels (marking proxy) |
   |---|---|---|---|
   | mipmaps on | 96.0 | 12.4 | 1644 (0.88%) |
   | mip 0 only | 96.0 | 12.5 | 1656 (0.88%) |

   No difference. Filtering is not the cause.

**The symptom was mis-stated, and the corrected statement points somewhere else.**
The tarmac is *not* uniformly unmarked. Capturing the HUD view on the runway
(`FF_VIEW_SCRIPT="0@62;s@68"`, noting the script clock runs from **program
start**, so 3D entry is ~60s in for TE 2) shows the dashed centreline and the
tyre-rubber streaks rendering correctly in the middle distance. Scanning bright
-pixel fraction down the central runway column gives a **hard cutoff**:

```
  rows 390-420    4.53%   <- markings render
  rows 420-450    3.72%   <- markings render
  rows 450-480    0.00%   <- and stop, abruptly
  ... every band down to row 780: 0.00%
```

So markings draw beyond a threshold distance and are *completely* absent nearer
than it — with no fade. A clean distance discontinuity like that is an **LOD /
representation switch**, not a texture, filtering, binding or depth problem, all
of which would degrade smoothly or fail everywhere.

**What paints the near field (2026-08-16).** `FF_PROBE_PIXEL="512,650"` attributes
the grey pixel to a single lit indexed draw: `DIPVB fvf=0x1d2 idxCount=12
tex=604`, drawn over the terrain (`tex=535`). Dumping that GL texture
(`FF_DUMP_GLTEX=604` → `/tmp/gltex_604.bmp`) shows a **1024² airfield atlas**
carrying edge lines, threshold bars, yellow taxiway paint and tyre streaks. So
the near-field surface is bound to marked art, and the "LOD switch" framing above
is not quite right either — the near field is a textured airfield slab, not a
plain substitute.

**Full geometry (per-vertex dump, `FF_PROBE_UV_VERTS=1`).** The slab is two quads
sharing a 270-unit width, and the mapping is clean and deliberate:

| quad | x (length) | u (across width) | v (along length) | atlas cell |
|---|---|---|---|---|
| far | −1106 → −606 (500u) | 0.0 → 0.5 | 1.0 → 0.5 | left/bottom |
| near | −606 → +593 (1199u) | 0.0 → 0.5 | 0.5 → 0.0 | left/top |

`u` runs across the runway width, `v` along its length, and the two quads take
the two vertical halves of the atlas's left column. The values land exactly on
`0.0 / 0.5 / 1.0` — cell boundaries of a 2×2 atlas, not drift. Wrap mode is
`GL_REPEAT` on both axes, so tiling is available and simply not requested.

**And the render matches the art.** Cropping the two sampled cells:

- the **near** cell is a runway that is *itself* mostly grey concrete — faint
  dashed centreline, tyre streaks, white edge lines — stretched over 1199 units;
- the **far** cell is the taxiway junction, dense with yellow curves.

That is exactly what the frame shows: white edge lines and streaks near the
aircraft, the yellow-rich band further out. The "markings stop at a cutoff"
measurement was reading the boundary between two *different atlas cells*, not a
rendering failure. Nothing in the near-field pipeline is demonstrably wrong.

**Status: blocked on a Wine side-by-side.** Every mechanism that could be checked
from this side has been checked and cleared (binding, depth bias, surface
textures, art presence, mip filtering, UVs, wrap mode, and now the atlas mapping
itself). The remaining question is whether Wine draws this same airbase with the
same two cells and simply looks richer, or selects a different/higher-detail
representation — and that can only be answered against the gold standard. What
would settle it: the PO's Wine shot **from the TE-02 start position in HUD view**,
so the same slab is in frame.

⚠️ **A retracted intermediate result, kept as a warning.** The first UV numbers
this probe produced were `u=[0..0] v=[0..0]` — an apparently perfect smoking gun
for "samples one texel, renders flat". It was an instrument bug: the probe was
handed `vb->data` (the vertex-buffer **base**) plus an index *count*, so it read
`dwIndexCount` vertices from the head of the buffer, which are not in the draw at
all. It was caught only because the control probe on a pixel where markings *do*
render returned byte-identical output — the same draw, the same zeros. A single
measurement would have shipped a wrong root cause. `FF_ProbePixel` now takes an
`FF_ProbeIndexed` (base + indices + count + startVertex) and decodes the vertices
actually drawn. **Always run the control.**

Two dead ends worth not repeating, found while chasing this:
- `Texture::DumpImageToFile()` cannot be used to inspect texture content here.
  It ends in `Texture::SaveDDS_DXTn()`, whose **entire body** is inside
  `#if _MSC_VER >= 1300` and which `return true`s unconditionally — on Linux it
  reports success and writes no file.
- `TexturePool[id].tex.dimensions` is not a side length at runtime; it read
  131072 for every id sampled, which is the *byte size* of a 512² DXT1. Do not
  trust the bank's palettized-path metadata in DDS mode.

`FF_DUMP_OBJTEX="id,id,..."` (texbank.cpp) dumps the palettized-bank content of
given ids to `/tmp/ff_objtex_<id>.bmp`; it is the right tool if the bank is ever
run in palette mode, but in DDS mode read the `.dds` files directly as above.

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

### SAVE-1 — squadron saves were written 4 bytes wider than they are read

`SquadronClass` reads `fuel` as `int32_t` and `schedule[]` as `uint32_t` — both
already corrected for the 32-bit Windows on-disk format — but still **wrote and
sized** them with `sizeof(long)`, which is 8 here:

| field | write / size | read |
|---|---|---|
| `fuel` (file, `Save`/`SaveSize`) | `sizeof(long)` = 8 | `int32_t` = 4 |
| `schedule[]` (file) | `sizeof(long)` × N = 8N | `uint32_t` × N = 4N |
| `fuel` (dirty data, `WriteDirty`) | `sizeof(long)` = 8 | `int32_t` = 4 in `ReadDirty` |

So our own writer and our own reader disagreed: 4 bytes of drift for `fuel` plus
4 per schedule slot, corrupting every field after them. Exactly the AUTOSAVE-1
class ("Encode wrote 32-byte event nodes while Decode read the 20-byte layout"),
which `CLAUDE.md` explicitly flagged as still outstanding on the Encode/SaveSize
side.

Fixed all four write/size sites to match the readers. Only the write path
changed, so loading existing Windows-format campaigns is unaffected; what changes
is that campaign saves we produce are now readable back.

Verified: TE 2 reaches 3D (111 samples) and Instant Action reaches 3D, both with
zero crashes and zero assertions.

**Round-trip now verified — and it found the real blocker (2026-08-16).**

Driving the flow the PO would (Campaign → COMMIT → START CAMPAIGN → bottom-bar
**SAVE** → name → SAVE, then reload it from the **SAVED** tab) showed the save was
still unloadable, for a bigger reason than any of the field-width fixes above:

`CampaignClass::Encode` prefixes the compressed campaign block with its
uncompressed size written as `sizeof(long)` — 8 bytes here, 4 on the 32-bit
Windows the format comes from — and returns `newsize + sizeof(long)`.
`CampaignClass::Decode` reads that header as `int32_t`. So every save this build
wrote sat 4 bytes out of phase. Decode still recovered the correct `datasize`
(it is the low half of the little-endian 8-byte field), which is why the failure
looked like data corruption rather than a header bug:

| | before | after |
|---|---|---|
| `datasize` | 25128 | 25128 |
| `LZSS_Expand(srcSize)` | 5932 → returned **5950** (over-consumed) | 5882 → returned **5881** (exact) |
| `CurrentTime` | 1852768256 (garbage) | 32410619 (Day 1 09:00:10) |
| outcome | `InvalidBufferException: Trying to write 28271 bytes to 27082 buffer` | `NumAvailSquadrons=112 Tempo=255`, live map |

Shipped saves (save0/1/2) always loaded because Windows wrote 4 bytes — which is
why this survived every previous campaign test. Only a save *we* wrote exposes it.

The `.cam` container is self-describing enough to check without running anything:
each member's length must equal its 4-byte prefix + 4. Post-fix the `.cmp` member
is 5889 = 4 + 5885, and its inner Encode header reads 25128.

Also hardened the path that surfaced it: the throw came out of
`Decode` ← `LoadScenarioStats` ← `LoadCampaignFileCB` — the preview that runs
**inside the UI event handler the moment the user clicks a save row**, which is
outside the `try/catch` previously added to `FM_LOAD_CAMPAIGN`. An incompatible
save therefore killed the game on a list click. Now caught the same way, draining
the recursively-held `campCritical` the unwind skips, and failing the preload.

Remaining `sizeof(long)` sites audited and cleared: `vusessn` `domainMask_`
(writer and reader agree; local-only file), the UI95 `ccontrol`/`ooutput`/
`cfontres` binary `.scf` path (dead — the `C_Base(FILE*)` ctors have no
instantiation site; the UI parses text `.scf`), and `src/tools` (not built).

### SESS-4 — POV hat was edge-driven; a held hat panned for one frame (fixed 2026-08-16)

The sim was written against **polled** DirectInput: `IO.povHatAngle` held its
value for as long as the hat was pressed, and `ProcessJoyButtonAndPOVHat`
re-fires the mapped view function every cycle off that held value. SDL reports a
hat only on **change**, so the Linux path wrote `povHatAngle` once per edge and
`IO.ResetAllInputs()` cleared it again. Holding the hat therefore produced at most
a single frame of movement — which reads to a player as "the hat does nothing".

Fixed by re-asserting the hat from live SDL state once per main-loop iteration,
restoring the polled semantics the consumer expects.

Also fixed on the way: `g_JoystickIndex` was the **device index** (0) while SDL
joystick events carry the **instance id** in `.which`. They coincide for the first
stick on a fresh run, so it worked — but after any hot-plug the ids diverge and
every joystick event is silently filtered out.

**New harness — a hat can now be driven with no hardware:**

| env | effect |
|---|---|
| `FF_VIRTUAL_JOYSTICK=1` | attaches an SDL virtual joystick (4 axes, 8 buttons, 1 hat) and opens it in preference to a real stick |
| `FF_SIM_HAT="dir@sec[+holdms];..."` | `c/u/d/l/r/ul/ur/dl/dr`; `sec` from **sim entry** (like `FF_SIM_KEY`), `holdms` default 1000 |

SDL feeds the virtual hat through the ordinary `SDL_JOYHATMOTION` path, so this
exercises the real code rather than bypassing it. The instrumentation prints
`hatEvents`/`accepted` counters, which is what made the diagnosis unambiguous:

```
before:  hatEvents=1 accepted=1   IO.povHatAngle[0]=-1   <- event DID arrive, value gone
after:   hatEvents=1 accepted=1   IO.povHatAngle[0]=0    <- POV north, still held
         NumberOfPOVs=1                                  <- dispatch loop iterates
```

Distinguishing "the event never arrived" from "it arrived and was cleared" was
the whole diagnosis; a single counter separated them.

**Still open in SESS-4:** HUD-view MFD panel placement, which needs the PO's Wine
side-by-side.

### MSG-1 — 20 `new[]` / scalar-`delete` mismatches (fixed 2026-08-16)

`CLAUDE.md` flags this as a class to keep sweeping, naming the msgsrc message
destructors (FlightPlan, SendImage, SendEval, SimDirtyData, sendvc,
requestcampaigndata) as latent. **Those are all already `delete[]`** — the only
`delete dataBlock.data` still scalar is `updateailist.cpp`, which is not in the
build (fixed anyway).

The live defects were elsewhere, and the list above would never have found them.

**The campaign send buffers (6).** `EncodeUnitData` and `EncodeObjectiveDeltas`
both return `new VU_BYTE[size + 1]` through an out-param, and its consumers freed
it with scalar `delete`:

    falcsess.cpp:200,205            unitDataSendBuffer / objDataSendBuffer  (session dtor)
    requestcampaigndata.cpp:244,262
    sendunitdata.cpp:223

`falcsess.cpp` is the one that matters: it runs on session teardown, so it is
reached in single-player, not just multiplayer.

**Repo-wide (14).** Then swept the whole built tree with a static pairing check —
a name assigned from `new T[...]` and later freed by a scalar `delete name;` in
the same file — verifying each candidate against its real allocation and
discarding the case-aliased directory symlinks (`src/Falcsnd` vs `src/falcsnd`)
and unbuilt trees:

| file | buffer |
|---|---|
| `falcsnd/voicemapper.cpp` | `voiceflags = new unsigned int[n]` |
| `falclib/entity.cpp` | `vhc = new uchar[NumVehicleEntries * MOVEMENT_TYPES]` |
| `campaign/campupd/cmpclass.cpp` | `CampaignSquadronData = new SquadUIInfoClass[NumAvailSquadrons]` |
| `campaign/camptask/flight.cpp` ×2 | `loadout = new LoadoutStruct[loadouts]` |
| `graphics/terrain/tlevel.cpp` | `postArray = new Tpost[POSTS_PER_BLOCK]` |
| `graphics/texture/fartex.cpp` | `pBuf = new BYTE[ddsd.dwLinearSize]` |
| `vu2/src/vuevent.cpp` | `callsign_ = new char[len + 1]` |
| `ui/src/campaign/general.cpp` ×3 | `WordWrap = new _TCHAR[len + 1]` |
| `ui/src/campaign/campaign.cpp` | `filedata = new char[count * sizeof(UnitHistoryType)]` |
| `ui95_ext/chistory.cpp` | `Data_ = new O_Output[Count_]` |
| `acmi/src/acmihash.cpp` | `Table_ = new ACMI_HASHROOT[TableSize_]` |

Several are hot, not teardown-only: `FlightClass::RemoveLoadout` runs whenever a
flight drops stores, the `DIRTY_STORES` decode branch frees and reallocates
`loadout` every time that dirty bit arrives during campaign play, and
`postArray` is per terrain block.

This is the class behind the campaign-exit crash (31cc565e). Scalar `delete` on
an array allocation is UB that corrupts allocator metadata, so it never fails
where it is written — it surfaces as a crash somewhere else, much later. Worth
sweeping statically rather than waiting for ASAN to happen to walk the path.

### WARN-1 — the build had `-w`; turning it off found real bugs (2026-08-16)

`add_compile_options(... -w)` disabled **every** compiler warning, so for the
whole life of the port GCC has been unable to say anything. That is a large
missed lever, because several of the diagnostics it offers are precisely the bug
classes this session had been chasing by hand:

| warning | the bug it is |
|---|---|
| `-Wsizeof-pointer-memaccess` | BOMB-1, exactly |
| `-Wmismatched-new-delete` | MSG-1, all 21 of them |
| `-Wuninitialized` | the SND-1 "WAVEFORMATEX from a failed parse" shape |
| `-Wstringop-overflow` | the classic sprintf-into-a-fixed-buffer overflow |

New CMake option, off by default (17k warnings, ~8k of them template-body noise,
is not a useful default for 800k lines of legacy code):

```
cmake -DFF_WARN=ON -B build-warn && ninja -C build-warn 2>&1 | tee /tmp/warn.log
```

**The first run found five defects the manual sweeps had missed:**

1. **Stack buffer overflow** — `drawparticlesys.cpp:5522` wrote `PS_NAMESIZE` (64)
   bytes into `char FileName[PARTICLE_NAMES_LEN]` (32). Driven by how long a name
   is in `particlesys.ini`.
2. `package.cpp:950` — `memset(targetf, 255, 5)` on an `int[5]` set **five bytes**,
   leaving `targetf[1..4]` uninitialised before `GetFeatureID()` read them.
3. `cmpclass.cpp:1290` — one more `new[]` freed with scalar `delete`. The
   repo-wide regex sweep missed it because the allocation is a *chained*
   assignment (`bufhead = buffer = new uchar[size]`) and the pattern bound only
   one name. The compiler had no such difficulty.
4. `ui_lgbk.cpp:686` — `%s` given an `IMAGE_RSC*`, which has no string member at
   all, making `_stprintf` walk the object hunting for a NUL into a 260-byte stack
   buffer.
5. compat `ExitProcess` was not `noreturn` though it is just `exit()`, so every
   caller ending in it looked like it fell off a non-void function — burying the
   real `-Wreturn-type` signal under a false positive.

**The biggest find: `#ifdef DEBUG` is LIVE in this build.**

`-Wnonnull` ("'this' pointer is null") pointed at two null dereferences, and the
warning is itself the proof the branch compiles — GCC only emits it when it can
see the pointer is provably null at the call. Following that up turned into a
seven-site class.

CMake defines `_DEBUG`, and `shi/assert.h` contains a *"make defining of DEBUG and
_DEBUG automagic"* block that promotes either one to both. So **every
`#ifdef DEBUG` branch in the codebase is the branch that compiles.** They read as
dead debug-only code; they are not.

The damage follows a single history. `SimObjectType` once took a 3-arg `OBJ_TAG`
constructor. When that signature disappeared, each call was commented out under
`#ifdef DEBUG` — but the *dereference below it* was left in place:

```c
#ifdef DEBUG
    //airtargetPtr = new SimObjectType( OBJ_TAG, self, (FalconEntity*) airtarget );
#else
    airtargetPtr = new SimObjectType((FalconEntity*) airtarget);
#endif
    airtargetPtr->Reference();          // <-- NULL in this build
```

Seven sites, all fixed with `#if defined(DEBUG) && !defined(FF_LINUX)`:

| site | pointer | path |
|---|---|---|
| `campweaponfiremsg.cpp:1255` | `tmpTargetPtr` | campaign unit fires a missile (deref'd twice) |
| `lgbfcc.cpp:869` | `tmpTarget` | LGB fire control — *never assigned at all* |
| `dlogic.cpp:104` | `airtargetPtr` | AI pilot acquires an air target |
| `fccmain.cpp:1137` | `retObject` | FCC targets a ground feature |
| `tankbrn.cpp:1280` | `tankingPtr` | tanker logic |
| `h_dlogic.cpp:293` | `targetPtr` | helicopter digital pilot |
| `target.cpp:79` | `newTarget` | missile shared its target instead of copying it |

Found the first two with the compiler; found the remaining five with a structural
search (the `#else` assigns a pointer, the `DEBUG` branch does not, the pointer is
dereferenced after `#endif`) over all 195 `#ifdef DEBUG` blocks in the built tree.
The compiler can only prove the ones it can see locally — worth remembering that
a warning is a *starting point* for a class sweep, not the end of one.

**The sweep is closed.** All 195 `.cpp` blocks plus the 4 in headers (a gap in the
first pass — it only covered `.cpp`) have been examined. Beyond the seven fixed,
the remaining candidates were checked individually and are harmless: the two
`atcbrain` blocks and `team.cpp`'s are DEBUG-gated extra validation, and
`atm.cpp`'s `airbase = NULL` is deliberate error handling immediately followed by
`if (airbase) … else …`.

**Triage note, to save the next session the work:** the ~38 int/pointer-cast
warnings look alarming and are mostly *not* defects. They are misplaced casts like
`(SimBaseClass*)entity->IsDead()`, where the cast binds to the **call result**
rather than the object — but `OnGround()` and `IsDead()` are virtual in
`SimBaseClass`, so dispatch is unaffected and the boolean survives as truthiness.
Left alone deliberately.

**A third pattern, found by fixing the first two:** *a guard path returns a
failure code without writing its out-parameters, and the caller ignores the
return.* Three live instances, each fixed at the correct end:

| function | fix | why there |
|---|---|---|
| `RadarDopplerClass::TargetToXY` | seeded the **callers'** locals | its early-out is deliberate — the comment is *"cursor position gets messed up"*, so leaving the caller's values alone is the contract |
| `ObjectiveClass::GetFeatureOffset` | zeroed the outputs **in the function** | pure garbage-checks, no such intent; features were landing at `XPos() + stack garbage`, and one call site is the objective *decode* path where the index comes from the save file |
| `CSoundMgr::LoadRiffFormat` | checked every read | see SND-1 |

Then swept the class properly: 78 functions in the built tree can return before
writing an out-param, but the return value is discarded by callers in only two,
and **both are false positives** — `CheckIfBlockingRunway`'s `info` and
`SetSeekerPos`'s `az`/`el` are caller-owned in/out structs that are read as well
as written, so leaving them untouched on an early return is correct. No further
live defects in this class.

**Second pass over the diagnostics (2026-08-16, later).** After the first round of
fixes the counts fell: `-Wmaybe-uninitialized` 95 → 71, and
`mismatched-new-delete`, `memset-elt-size`, `return-type`, `nonnull` and
`stringop-overflow` all went to **zero**. Working the rest turned up four more,
two of them memory-safety rather than cosmetic:

| site | finding |
|---|---|
| `displays/helpers.cpp` ×3 | bullseye bearing/range assigned only inside `if (theRadar)` with no `else`, then `sprintf`'d into `char str[12]` — a garbage float in `"%03.0f"` smashes the stack. Seeded, and all 11 `sprintf(str, …)` in the file bounded. |
| `ui95/imagersc.cpp` ×2 | `count` is deliberately carried across loop iterations, so an else-branch on the **first** iteration used it uninitialised — as a `memcpy` **length**. |
| `simlib/math.cpp` | `TwodInterp` clamps its inputs only inside a guard that can fail, then interpolates on stack garbage. This is the 2-D table interpolator behind the **aero and engine tables**. |
| `rwr/advancedhts.cpp` | EXP offsets assigned in conditional blocks, subtracted outside them — HTS symbology offset by garbage. |

Plus `setupinp.cpp` (`buttonId`/`mouseSide` from another return-without-writing
`GetFunction`, used to index the cockpit button table) and `simvudrv.cpp`
(`sessionD2` gating network sends).

Skipped with reasons, so they are not re-examined: `harmpod`'s `trig` (`mlSinCos`
writes both fields unconditionally on the non-MSVC path — false positive),
`radardigi`'s `ret` (`#ifdef SAMDEBUG`, not compiled), and `modes.cpp`'s `elhack`
(declared and used inside the same guarded block).

Remaining queues in `/tmp/warn.log`: 95 `-Wmaybe-uninitialized`, 137 `-Wformat`,
79 `-Wformat-security`, 13 `-Wimplicit-function-declaration` (an undeclared
function is assumed to return `int`, which truncates a returned pointer on 64-bit
— worth a look), 7 `-Wnonnull`.

### FMT-1 — unbounded / format-parsed string copies (fixed 2026-08-16)

`-Wformat-security` flags 79 calls of the shape `sprintf(dest, src)` — a
non-literal format with no arguments, i.e. `sprintf` used as `strcpy`. Two
hazards at once: a `%` anywhere in the source makes printf consume garbage
varargs, and the copy has no length limit.

**The pilot identity path was the worst of it.** `FalconSessionEntity` holds
`name[21]` and `callSign[13]`, and five paths wrote them unsafely:

| path | problem |
|---|---|
| stream `Decode` | `size` is a **uchar read straight off the wire or out of a save** (0–255), and `memcpychk` bounds the *source*, not the destination — so up to 255 bytes into a 21-byte buffer, then `name[size] = 0` up to 234 bytes past the end |
| `_stprintf(name, LogBook.NameWRank())` | user-entered pilot name: unbounded **and** format-parsed |
| `_stprintf(callSign, LogBook.Callsign())` | same |
| `_tcscpy(name, pname)` | unbounded; the `name[_NAME_LEN_] = 0` truncation on the next line only ran *after* the overflow |
| `_tcscpy(callSign, pcallsign)` | same |

So a pilot whose callsign contained a `%` could crash the game, and a campaign
save could overflow the session buffers. All bounded and terminated; the decode
clamps to capacity while still consuming the full field so the stream stays in
sync. Verified the logbook still shows callsign *Viper* / pilot *Joe Pilot*.

**Also bounded**, from the 27 definite `-Wformat-overflow` cases (the *"may write
a terminating nul past the end"* subset — the other 630 are GCC assuming an
arbitrary-length `%s` and were left alone deliberately):

- `icp/icpstpt.cpp` ×3 — `char hoursStr/minutesStr/secsStr[3]` written with
  `"%2d"`; two digits plus a nul exactly fills them, so any three-digit or
  negative value runs one past the end, and `FormatTime` derives minutes from a
  `long`.
- `campui/misseval.cpp` ×2 — AI pilot name/callsign built as `"%s%d"` from the
  flight name into `_TCHAR[30]`, unbounded.

### STUB-1 — "stub returns default" audit: CLEAN (closed 2026-08-16)

`CLAUDE.md` has carried an open action since June — *"audit remaining stubs in
compat_winbase.h"* — left over from the `GetPrivateProfileInt/String` find, where
default-returning stubs zeroed every `.ini` tuning value in the game and caused
the campaign aggregation flap. Closing it, with the method recorded so nobody
repeats the sweep.

Two passes over `src/compat`:

1. **Constant-returning functions.** Parsed every compat function whose body is
   nothing but `return <constant>;` (after stripping comments and `(void)x;`
   discards): **312 matches, 58 unique** once the case-aliased header pairs
   (`winuser.h` ↔ `compat_winuser.h`, etc.) are collapsed.
2. **Success with an unfilled out-parameter** — the actually dangerous shape,
   since the caller gets uninitialised data and no error: **16 matches**.

Then ranked by call sites *outside* `src/compat`. Every candidate that looked
alarming turned out to be dead:

| candidate | why it is not a live defect |
|---|---|
| `OpenCampFile` → FALSE (43 callers!) | different function — `BOOL OpenCampFile(HWND)` is a **camptool dialog**; the live one is `FILE* OpenCampFile(char*,char*,char*)` in campaign.cpp. Name-matching conflated them. |
| GDI set (`GetDC`, `SelectObject`, `BitBlt`, `CreateCompatibleDC`, …) | all callers in `camptool`, `src/tools`, `src/movie`, `src/dxutil` — **none built** |
| `ContextMPR::TextOut` (surface `GetDC`) | only caller is `bspview` — not built |
| `ProcessVertices`, `ProcessVerticesStrided`, `GetClipStatus`, `SetClipStatus`, `GetClipPlane`, `EvaluateMode`, `Optimize` | **0 live callers each** |
| `MultiplyTransform` (`// TODO: Implement matrix multiplication`) | **0 live callers** |
| `RegisterClass`, `InvalidateRect`, `TranslateMessage`, `DispatchMessage`, `EndPaint` | their pointer arguments are *inputs*, not out-params |

`camptool` is `if(WIN32)`-gated in `src/campaign/CMakeLists.txt`; `dxutil`,
`movie` and `src/tools` are absent from the build graph entirely. Checking a
candidate against `ninja -t targets all` before investigating it is what made
this quick.

**Result: no remaining live instance of this bug class.** Worth stating plainly
because a negative result here is genuinely useful — it means a future "silently
wrong value" symptom should be chased somewhere other than the compat stubs.

### SND-1 — `LoadRiffFormat` returned 0 for every WAV (fixed 2026-08-16)

Found by continuing the SAVE-1 sweep past the campaign code into every other
`sizeof(long)` binary reader. `CSoundMgr::LoadRiffFormat` reads the RIFF size
field as `long`. At 8 bytes that single read consumes the size field *and* the
`"WAVE"` tag, so the `strncmp(buffer, "WAVE", 4)` immediately below it compares
against `"fmt "` and the function returns 0 — for every file, always.

A/B'd against real game WAVs rather than argued from the code:

| file | old (`long`, 8b) | new (`int32_t`) |
|---|---|---|
| `BoomA1.wav` | 0 (WAVE check failed) | 115200 |
| `biggun1.wav` | 0 | 152320 |
| `LSpdTone.wav` | 0 | 45072 |

It is live: `F4StartStream` (streamed audio) and `cmusic.cpp` both call it — and
`F4StartStream` ignores the return value and uses its stack `WAVEFORMATEX`
regardless, so every streamed sound was configured from an **uninitialized**
header. That the in-flight engine audio works is not evidence against this; those
go through `LoadRiff`/`LoadWaveFile`, which were corrected in an earlier session.

Same defect in `FillRiffInfo`'s chunk walk (reads 8 bytes, advances 4) and in
both `SkipRiffHeader` overloads. The latter two are currently unreferenced, but
they feed a garbage `size` straight into `fread(buffer, size, 1, fp)` over a
`char[256]` — a stack smash waiting for a caller, so fixed rather than left.

**Bug class, restated:** `sizeof(long)` against any format defined by 32-bit
Windows. This session found it in three unrelated subsystems (campaign save
headers, campaign field widths, RIFF parsing) after it had already been fixed a
dozen times elsewhere. When a binary reader misbehaves, check the width first.

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

### VOICE-1/2/3 — the radio voice chain, and the crash family it was causing (2026-08-17)

Three defects in a chain, each one hiding the next. Recorded together because
the sequence is the interesting part.

1. **VOICE-1.** `FRAG_FILE_INFO` / `EVAL_FILE_INFO` / `COMM_FILE_INFO` are
   documented on-disk layouts, written by 32-bit Windows, and declared their
   offset as `long` — 16/16/18 bytes here against 8/8/14. `GetFragInfo()` strides
   by `sizeof()`, so every record after index 0 was read at the wrong offset, and
   `maxfrags = fragOffset / sizeof(*f1)` came out at **half** its true value.
   Measured: 4 voicefilter assertions per campaign flight → 0.

2. **VOICE-2.** Fixing that let the voice system reach `.tlk`, which had the same
   defect three ways over (index stride, an 8-byte read of a 4-byte field, and
   `TlkBlock.data` at offset 16 instead of 8). Twelve *new* assertions appeared
   the moment VOICE-1 landed, in code that had never been reachable. → 0.

3. **VOICE-3.** With both fixed, voice streams played for the first time on this
   port — and reached `LHSP::ReadLHSPFile`, whose `PMSIZE`/`CODESIZE` are never
   initialised because the ST80 codec is stubbed out. Garbage `PMSIZE` was
   accumulated into the returned decode length, stored as `dataInWaveBuffer`, and
   `AddNoise()` then **wrote** `*pos = level` across ~81,600 bytes of an
   80,960-byte buffer.

**What that write did, and the wrong turn it caused.** A stray write past an 80KB
heap block corrupts whatever allocation follows it. The crashes therefore
appeared in six unrelated places — `O_Output::Cleanup`, `O_Output::SetText`,
`C_MapIcon::UpdateInfo`, `UI_Refresher::RemoveMission`, `C_Hash::Find`,
`CSoundMgr::ProcessStream` — three of them under one callback. I built a
confident theory from that clustering: `UI_Refresher` holding dangling pointers
to destroyed UI items. **It was wrong.** The pointers were fine; their memory was
being overwritten. Six corroborating sites and a plausible mechanism were not
enough, and only ASAN settled it.

Two process lessons worth keeping:

* *Fixing a data-format bug exposes code that has never run.* This happened three
  times in one sprint. Expect the next layer to be untested, not merely unfixed.
* *A cluster of crash sites is evidence of a corruptor, not of a bug at those
  sites.* When symbolised backtraces point at several structurally unrelated
  places, reach for the sanitiser before building a theory about any one of them.

Evidence: ASAN on the identical command went heap-buffer-overflow/rc=139 →
0 errors/rc=124; release crash rate went ~1 in 3 (2/6, 1/5, 1/3) → 0 in 8.

---

## Sprint 24 — WHITE-1: A/G master mode whites out the screen (`ef4497ed`)

The PO's report: pick the CCIP tactical engagement (or campaign CCIP), press
A/G on the UFC, and about a second later the whole frame goes white except the
MFDs and HUD, permanently. Instant Action was fine.

**Root cause.** Selecting A/G is the only thing that puts the FCR into ground
map, so `RenderGMComposite` is the one render pass in this port that runs
*nested inside another one* — it draws during the cockpit/MFD pass. It finishes
with `RenderGMRadar::FlushDrawnTargets` → `ContextMPR::FlushPolyLists`, which is
written as **frame-level teardown**: it flushes the global DX engine, resets the
shared polygon arena, and rewrites global Z/stencil state. The part that bites is
`TheDXEngine.FlushBuffers()`, which brackets itself with
`CreateStateBlock(D3DSBT_ALL)`/`ApplyStateBlock` over the shared device. Run from
inside another pass, that bracket leaves global state altered such that the 2D
cockpit panel — a full-screen chroma-keyed quad — samples white from then on.

One flush is enough, and it is pure loss: `TheVbManager.TotalDraws` is the
pending-item count and it is **0** there, so the call draws nothing at all. The
fix guards exactly that: a nested flush with an empty draw list returns before
the state-block bracket. `FF_DX_NESTED_FLUSH=1` restores the old behaviour.

**Evidence.** Unattended repro (TE "20 Bombs with CCIP", scripted A/G click,
frame captured 19 s later), causality shown both ways:

| run | white % |
|---|---|
| fixed | **0.0** |
| `FF_DX_NESTED_FLUSH=1` | 96.4 |
| fixed, repeated ×3 | **0.0** |

and the captured frame shows a correct cockpit with A/G engaged — ground-map MFD
sweeping, A/G HUD symbology, `airGroundBearing` feeding the MFD offset.

**How it was found, and two theories that died.** Entirely by measurement. The
ground-map render stages were disabled one at a time (`FF_GM_SKIP`, kept as a
kill switch for this newly-exercised path): that isolated the target-return draw,
then the flush inside it, then the DX engine call inside that. Both of the
theories I formed on the way were killed by measurement rather than argument, and
they are recorded so nobody re-tries them:

* **Not** `AllocResetPool` recycling the shared polygon arena out from under the
  main scene. Plausible, and I shipped a guard for it — the arena measured
  **0 KB** at that point and the guard changed nothing. Reverted.
* **Not** the ground-map "heart of darkness" branch that renders straight into
  the primary surface. `bRender2Texture` measured **1**, so the private
  render-target branch is taken.

Also ruled out by measurement, in the order they were tried: the mode predicate
(`IsAGMasterMode` and `GetMainMasterMode` were each made to lie — the whiteout
survived both, exonerating every reader that goes through them), and texture
damage (`FF_DUMP_GLTEX` now reports the whole mip chain; level 0 and level 1 of
the panel texture are clean and the chain is complete, 1600×1200 down to 1×1).

**Bug class to expect again.** *A function written as end-of-frame teardown is
not safe to call from a render pass nested inside another one.* `FlushPolyLists`
is reached from every instrument context, not just the main renderer. Open issue
#10 (terrain painting over the 3D-pit MFD screens as the last writer) has the
same "two flushes per frame, one carries all the polys" shape and is worth
re-examining in this light — tracked as **NEST-1**.

### NEST-1 — audit of the other nested render passes

WHITE-1 raised the obvious question: `ContextMPR::FlushPolyLists` is frame-level
teardown, and the ground-map radar is not the only pass that reaches it from
inside another pass. Every caller was enumerated:

| caller | nested? | flushes real work? |
|---|---|---|
| `OTWDriverClass::RenderFrame` ×2 | no — the main frame | yes (one of the two is routinely empty) |
| `RenderGMRadar::FlushDrawnTargets` | **yes** | **no** — nothing queued |
| `mavdisp` (Maverick), `lantirn`, `lantmfd`, `laserpod`, `cpmirror`, `padefov` | yes | yes — each `DrawScene()`s first |
| `acmiloop`, `c3dview` | no — their own frames | yes |

Two things fell out of measuring rather than assuming:

* **An empty draw list is not by itself the problem.** The main frame flushes
  with `TotalDraws == 0` constantly — 59 of the 60 sampled flushes in an A/G run
  — and it is harmless. The damage needs the *nested* context as well, which is
  why the guard is keyed on both and why a general "skip when empty" rule would
  have been wrong.
* **The ground-map radar is the only nested pass that flushes nothing.** All the
  other nested passes draw a scene first, so the guard cannot fire for them and
  must not: their flush has real work to do.

Scenario coverage: in the CCIP TE repro, in both the 2D pit and the 3-D virtual
pit, `RenderGMRadar::FlushDrawnTargets` is the *only* nested flush exercised, and
both views render correctly with A/G engaged after the fix.

**What is not settled.** Whether a nested flush that *does* draw also perturbs
global state is untested — those sites need sensor-specific scenarios (a Maverick
loadout, a LANTIRN or laser pod, the padlock view). No symptom is known for them
today, so nothing was changed there. If a "displays go strange in A/G" report
turns up with a Maverick or pod aboard, this table is where to start.

---

## Sprint 25 — PIT-1: the 3-view renders no tarmac

PIT-1 has been on the board as *unmeasured* since it was raised, with the note
that every attempt needed the PO's eyes because the agent could not capture
sim-mode frames. That is no longer true — it now has an **unattended repro and a
numeric metric**, and six candidate causes have been eliminated by measurement.

**Repro.** TE "02 Takeoff", parked on the runway, switch view, capture:

```
FF_UI_CLICK="677,748@12;140,128@18;824,750@24;973,750@30" \
FF_VIEW_SCRIPT="<view>@62" FF_SIM_SCREENSHOT="70:/tmp/x.bmp"
```

Metric: sample rows y = 280/320/360 across the frame and count grey (tarmac) vs
green (grass) pixels.

| view | | tarmac |
|---|---|---|
| 0 — HUD only, no pit model | grey ✓ (runway fills the frame) | **renders** |
| 1 — 2D pit | grey=42 green=1 | **renders** |
| 2 — chase | grey=63 green=0 | **renders** |
| 4 — virtual pit | grey=**0** green=22 | **missing** |

View 0 is the same eye position as view 4 with no pit model, and it renders the
runway perfectly. **So this is the pit pass, not the camera, the LOD, or the
aircraft's position.**

**The decals are submitted.** A per-draw trace (`FF_DEBUG_ORDER`) counts 63
runway/tarmac draws per frame in the 3-view — identical to the 2D pit. Both views
have the same per-frame order: an empty flush, the big terrain flush
(plain≈770 tex≈3716), then the 63 decal draws. Ordering is not the difference.

**Eliminated by measurement** (each with the switch that tested it, all left in
place and defaulting to the original behaviour):

| hypothesis | switch | result |
|---|---|---|
| depth bias loses the decal | `FF_RUNWAY_NOBIAS=1` | no change |
| stencil rejects it at pit entry | `FF_PIT_NO_STENCIL=1` | no change |
| stencil rejects it at pit exit | `FF_PIT_EXIT_STENCIL=0` | no change |
| pit-exit depth clear makes draw order paint order | `FF_PIT_EXIT_ZCLEAR=0` | no change |
| pit-exit `DrawSolidSurfaces` drains world surfaces under the pit projection | `FF_PIT_EXIT_SOLID=0` | no change (the 16 surfaces there are genuinely the pit's) |
| the decal is buried under terrain | `FF_RUNWAY_ZLIFT=60` | **no change in the 3-view** — while the same switch moves the tarmac right out of the sampled band in the 2D pit (grey 42 → 2) |

That last one is the sharpest result: the lift switch demonstrably moves the
tarmac in a working view and does *nothing* in the 3-view, so in the pit pass the
decals contribute **nothing to the image at all** — they are not merely hidden.

**Best remaining clue.** Probing one ground pixel in each view:

* view 1: `tex=573`, 12 verts, paints **grey** — that is the decal.
* view 4: `tex=573` never appears. Instead a `tex=54` batch of **4380** verts
  paints green last, where view 1's `tex=54` batch has **14** verts.

So the pit pass submits a materially different world-geometry set. **Next step:**
trace which drawable/LOD supplies `tex=573` in view 1 and why it is not submitted
in view 4 — that is now the whole question.

**Harness knowledge worth keeping.** The TE list is not scrolled: rows run 01 at
y≈110 to 34 at y≈672, ~17 px apart, and row 20 ("Bombs with CCIP") is y=433. But
the list has a **dead band in x**: at x=205 the rows around y≈118–134 hit no
control at all, while x=110 or x=140 hit them fine. Row 02 ("Takeoff") is
reachable at **(140,128)**, not (205,128) — that cost several runs to find.

### PIT-1 continued — correction, and the model that occludes

**Correction to the table above.** I wrote that `FF_RUNWAY_ZLIFT=60` produced "no
change in the 3-view" and concluded the decals contribute nothing to that pass.
That conclusion was wrong, and the reason is worth recording: the metric samples
three rows across the middle of the frame, and lifting the decals 60 ft moves
them *above the horizon* — looking at the actual image, they are plainly there,
forming a grey roof over the canopy. **The decals do render in the 3-view.** A
numeric metric that only samples where you expect the answer will confirm
whatever you already believe; the image disproved it in one glance.

With that corrected, the lift sweep says something sharper. At lifts of 3 (the
default), 8, 20, 30, 45 and 60 ft the ground band is byte-identical
(grey=6 green=106 of 588) — the tarmac never reappears at ground level at *any*
lift. So this is not a near-coincident z-fight that a few feet of decal offset
would win.

**What the probe shows.** At the same ground pixel, in the 3-view the decal
(`lod=156`, `tex=573`) never paints at all, while three draws do:

```
lod=4105  192 verts  tex=30
lod=2647 1248 verts  tex=610
lod=2647 4380 verts  tex=54    <- green, last world draw
```

In the 2D pit at the same pixel, `lod=2647` submits only its 1248-vert `tex=610`
batch, the decal paints grey before it, and it survives. **Model 2647 submits a
4380-vert ground batch in the virtual-pit pass that it does not submit in the 2D
pit pass**, and the decal is depth-rejected against it.

Also measured: `lod=156` is drawn 300/300 times with `pit=0` in *both* views, so
the decal is submitted identically and outside pit mode — the difference is
entirely on the occluder's side. The per-frame distinct-LOD sets are 122/123
identical between the two views; view 1 uniquely draws `153`, view 4 uniquely
draws `4105`.

**Next step:** identify model 2647 and why the virtual-pit pass selects a LOD of
it that includes a ground surface — `TheStateStack.SetLODBias` / the pit FOV and
`resRelativeScaler` are the obvious levers. This is very likely the same family
as the known runway-elevation decoupling (the airfield rendering as a ~20 ft
plateau over flat z=0 collision data).

### PIT-1 — a second correction, and the honest state

Two more results, one of which invalidates part of the analysis above.

* `FF_LOD_BIAS_CAP=5.441342` (clamping the pit pass to the highest bias the 2D
  pit ever uses — measured with `FF_DEBUG_LODBIAS`, which shows the pit pass
  additionally running at 7.584) produces **no change**. So the LOD bias is not
  the lever either.
* **The cross-view pixel comparison is confounded.** Views 1 and 4 do not share a
  field of view, so pixel (960,330) is *not the same world point* in both. The
  "model 2647 submits a ground batch in the pit pass that it does not submit in
  the 2D pit" conclusion recorded above does not follow from that data and should
  not be treated as established. Comparing two renders pixel-for-pixel requires
  first showing the pixel means the same thing in both.

**What is actually established for PIT-1:**

1. Unattended repro and metric (above), replacing "needs the PO's eyes".
2. The tarmac renders in views 0, 1 and 2 and is missing in view 4.
3. The decals *are* submitted in view 4 — 300/300 draws, all with `pit=0`,
   identical to the working view — and they *do* render there: lifted 60 ft they
   appear plainly, as a roof above the horizon.
4. At ground level they never appear, at any lift from 3 to 60 ft.
5. Not the depth bias, not stencil (entry or exit), not the pit-exit depth clear,
   not the pit-exit solid-surface drain, not the LOD bias.

So: drawn, in the right pass, outside pit mode, and invisible only at their real
height. **Next step:** compare the two views at the same *world* point rather
than the same pixel — anchor on a feature (the runway threshold) and probe where
it projects in each view, or match the FOVs before comparing.

### AGTE-1 — WHITE-1 regression-tested across three A/G missions

The WHITE-1 fix was found and verified on one mission (TE 20, "Bombs with
CCIP"). It has now been exercised on the two other TEs that drive the same
machinery, each flown unattended with the A/G button clicked in the sim:

| mission | pre-A/G | post-A/G | crashes |
|---|---|---|---|
| TE 20 — Bombs with CCIP | 0.0% white | **0.0%** | 0 |
| TE 18 — A-G Radar Modes | 0.0% white | **0.0%** | 0 |
| TE 24 — Mavericks | 0.0% white | **0.0%** | 0 |

TE 18 is the mission built around the ground-map radar that WHITE-1 was in.
TE 24 carries Mavericks, which drives `mavdisp` — one of the nested render
passes NEST-1 flagged as untested — and is the code path CRASH-5 was found in.
Both render correctly with A/G engaged: ground-map MFD sweeping, A/G HUD
symbology, stores page populated.

Caveat: flying a mission with Mavericks aboard is not the same as putting the
Maverick *video page* up, which needs the weapon selected and its page called.
So `mavdisp`'s nested flush is exercised but not proven to be under load.

### PIT-1 — SOLVED: the pit pass restored a cull mode the world pass never used

The 3-view showed grass where the runway is because, on leaving pit mode, the DX
engine put back a **hard-coded** cull mode:

```c
m_pD3DD->SetRenderState(D3DRENDERSTATE_CULLMODE, (m_bCullEnable) ? D3DCULL_CW : D3DCULL_NONE);
```

That is not what the world pass was using. Every object drawn after pit exit
inherited `D3DCULL_CW`, and the runway/tarmac decals are single-sided and wound
the other way — so they were culled when viewed from above. Views 0, 1 and 2 run
no pit pass, nothing overrides the cull mode, and the tarmac renders correctly:
that is the whole of the view-dependence.

**The fix**: save the cull mode actually in force at pit entry
(`GetRenderState(D3DRENDERSTATE_CULLMODE, &m_ffPrePitCullMode)`) and restore
exactly that at pit exit, instead of assuming a value.

| run | ground band |
|---|---|
| before | gray=6 green=106 of 588 |
| `FF_PIT_EXIT_CULL=0` (NONE) | gray=428 green=2 |
| `FF_PIT_EXIT_CULL=2` (CCW) | gray=428 green=2 |
| **fix (restore what was saved)** | **gray=428 green=2** |
| 2D pit, fix vs old behaviour forced | gray=291 both — unchanged |

**The clue that cracked it was one I had already collected and misread.** The
z-lift sweep showed the decals invisible at ground level but plainly visible when
lifted 60 ft. I first recorded that as "no change" (a metric that sampled the
wrong rows), then as "drawn but occluded". Neither fit: what it actually means is
that the surface is only drawn when seen **from below** — the signature of a
back-face cull with the wrong winding, not of an occluder. Every hypothesis about
*something covering it* (stencil, depth clear, solid-surface drain, LOD, a
stronger depth bias up to 32×) failed because nothing was covering it.

Note this also explains why the earlier `-32,-8192` depth-bias tuning was needed
to make the runway survive in the 2D pit and yet did nothing here: two different
problems on the same geometry.

### PITSTATE-1 — audit of the pit pass's other restores: only the cull mode was wrong

PIT-1 was a restore-to-a-guessed-constant bug, and the same block has three more
of that shape. Rather than "fix" them on the strength of the pattern, the
pre-pit state was measured (`FF_DEBUG_PITSTATE`, read at the very top of the pit
entry branch, before anything is overwritten):

```
[PITSTATE] pre-pit cull=1 fogStart=0.00 fogEnd=80000.00 fogEnable=1 zwrite=1 lighting=1
```

| pit exit does | in force before the pit | verdict |
|---|---|---|
| `CULLMODE = (m_bCullEnable ? CW : NONE)` → CW (2) | **NONE (1)** | **wrong — fixed under PIT-1** |
| `FOGSTART = 0.0f` | 0.0 | matches, leave alone |
| `glEnable(GL_LIGHTING)` | lighting on | matches, leave alone |
| `SetStencilMode(STENCIL_CHECK)` | — | by design: the world pass must reject over pit pixels, and `FF_PIT_EXIT_STENCIL=0` showed no visual difference |

That `cull=1` reading is also a direct, independent confirmation of the PIT-1
diagnosis: the world pass really was running with `D3DCULL_NONE`, and pit exit
really was changing it to `CW` behind everything drawn afterwards.

So: one real bug in four candidates, and the other three left untouched with the
measurement that says why. The pattern was a good lead; it was not evidence.

---

## Sprint 26 — MFD-THRU-1: terrain through the 3D-pit MFD screens (CLAUDE.md issue #10)

Re-opened first to check whether the PIT-1 cull fix had changed it. It had not:
cropping the same MFD region from a pre-fix and a post-fix capture gives
structurally identical images. The only difference is the colour of what bleeds
through — grass before, tarmac now, because the tarmac is finally rendering.

**It is genuinely the outside world.** Flown airborne, the left MFD shows real
terrain — green/brown ground texture with a hard diagonal edge — and 13.9% of the
screen's pixels change between two captures 20 s apart. World-locked, not a stale
canvas.

**It is pit-pass specific.** The same instruments in the 2D pit are clean. Only
the left MFD is affected; the right MFD is correct in the same frame.

**What the probe establishes:**

* The instrument content is composited **additively** — `src=GL_SRC_ALPHA
  dst=GL_ONE`, with an alpha test at `GEQUAL 0.5`. That is right for
  self-luminous symbology drawn *onto an opaque black screen*: the black
  background contributes nothing, so whatever is behind shows through.
* **Nothing draws an opaque backdrop in the screen hole.** The frame order at an
  MFD pixel is sky → world objects → symbology. This corrects the note in
  CLAUDE.md issue #10, which said "the MFD black-background quad draws EARLY":
  backtracing that early black draw gives `RenderOTW::DrawSun` ←
  `DrawSkyNoRoof` ← `DrawSky` ← `DrawScene`. It is the sky, not an MFD backdrop.
* Not the pit-exit solid-surface drain (`FF_PIT_EXIT_SOLID=0` — no change), and
  not solid surfaces at all: `FF_NO_SOLID=1` makes the wash **larger**, so those
  surfaces were partly covering it, not causing it.

So the pit model's MFD screen area is a transparent hole, the world renders
through it, and only the additive symbology lands on top.

**Next step:** decide where the opacity belongs — either the pit model's screen
polygon should be drawn opaque (it is presumably chroma-keyed so the RTT canvas
can show through) or the 3D-pit canvas blit should be an opaque copy rather than
an additive one. The 2D pit must be re-checked against whichever is changed,
since additive-over-an-opaque-panel is correct there and a blanket change would
break it.

### MFD-THRU-1 — where the content actually lives

Dumping a whole frame's draw calls (`FF_DRAWLIST`) in the 3D pit separates the
two halves of the pipeline:

* **The instrument canvases do render, into an RTT.** 136 draws go to `fbo=1`,
  at viewports `550,568,200x199` and `550,318,200x200` inside a 768×768 target —
  the two MFDs. They include untextured `blend=0 atest=0` fills, i.e. the opaque
  background *is* being drawn, into the canvas.
* **The symbology seen on screen does not come from there.** The draw that paints
  green at the left-MFD pixel is `tex=4, n=30, fvf=0x2c4, fbo=0` with a full
  `1920x1080` viewport and scissor off — screen space, straight to the window,
  additively blended, with nothing opaque drawn under it.

So the content and its black background exist in the canvas; what is missing is
the step that puts that canvas onto the pit's screen opaquely. `Canvas3D` is not
the culprit: its `ClearDraw()` is empty in the original code too, so Windows does
not clear there either — the opacity has to come from the composite, not a clear.

**Next step is now specific:** find where the 3D-pit screen polygon samples the
MFD RTT texture, and why the visible symbology instead arrives as a screen-space
additive overlay to `fbo=0`. One of those two paths is being used when the other
should be.

### MFD-THRU-1 — root cause found: the canvas background is written with alpha 0

The chain, each step measured:

1. **The pit model has no screen face behind the MFDs.** Skipping the canvas
   composite entirely (`FF_NO_RTT_QUAD=1`) leaves the bleed unchanged
   (331 vs 329) — there is nothing behind the canvas but the world. So the
   canvas is what has to fill the hole.
2. **The composite is chroma-keyed, by data.** `3Dckpit.dat` lines 35–36 end in
   `c`, which `VirtualDisplay::SetRttCanvas` maps to
   `STATE_CHROMA_TEXTURE_GOURAUD2`. The texture rects in those lines
   (`550 1 750 200`, `550 250 750 450`) match the RTT viewports seen in a frame
   dump exactly, Y-flipped.
3. **The canvas background carries alpha 0.** Reading the canvas texture back
   (`FF_DUMP_RTT=1`) — one shared 768×768 target, GL texture 4:

   | canvas | rect | alpha zero | alpha full |
   |---|---|---|---|
   | HUD | (1,1)–(430,430) | 98% | 0% |
   | DED | (1,500)–(200,580) | 95% | 4% |
   | PFL | (1,600)–(200,680) | 100% | 0% |
   | RWR | (250,500)–(430,680) | 100% | 0% |
   | **MFD left** | (550,1)–(750,200) | **95%** | 4% |
   | **MFD right** | (550,250)–(750,450) | **94%** | 5% |

   Only the symbology has alpha. The black background does not, so a keyed
   composite discards it and the screen is a hole.

**Why a blend change is the wrong fix.** `FF_RTT_BLEND=g` makes the MFDs render
perfectly — an opaque screen with full symbology, bleed 329 → 3 — but it is
applied to every canvas, and the **HUD then becomes an opaque black slab** with
the world no longer visible through it. The HUD *must* stay keyed. The engine
distinguishes the two by canvas content, not by blend mode: on Windows the MFD
canvas's background evidently survives the key and the HUD's does not.

So the fix belongs where the alpha is written, not in the composite. **Next
step:** make the MFD canvas's background opaque in the RTT. `Canvas3D::ClearDraw()`
is an empty stub and is the natural candidate — but it is shared with the HUD,
so anything done there has to leave the HUD transparent. That is the constraint
that decides the shape of the fix, and it is why nothing was changed here yet
rather than shipping something that trades a cosmetic MFD defect for an unusable
HUD.

Ruled out along the way: the pit alpha test (`FF_PIT_NO_ATEST=1`, no change), the
pit-exit solid drain, and solid surfaces generally.

---

## Sprint 27 — SOAK-1 and HANDOFF-1

**SOAK-1.** This session changed global render state in two places every frame
touches — the nested DX-engine flush guard (WHITE-1) and the pit-exit cull
restore (PIT-1) — so both were put under a soak before the PO tests A/G. Six
unattended flights (TE 02 takeoff in the 3-view and the 2D pit, TE 18 A-G radar,
TE 20 CCIP, TE 24 Mavericks, TE 20 in the 3-view), each with the A/G click or a
view switch: **zero crashes, zero whiteouts, and only two distinct assertion
sites across all six**.

**HANDOFF-1 — found by that assertion inventory, and it is a real mis-target.**
`SimCampHandoff(..., HANDOFF_RADAR)` converts a radar/HARM lock between the
campaign and sim representations of a unit. It reads the unit's radar type and
asserts it is non-zero — "make sure we didn't get asked to find a radar in a
non-radar unit" — and then searches the unit's components for the vehicle whose
radar type matches.

`ShiAssert` does not halt this build. With `campRadarType == 0` the search
compares `GetRadarType() == 0`, which matches **the first component that also has
no radar** — so the lock is handed to an arbitrary truck instead of failing.

Measured, not argued: instrumenting the guard to run the old search and report
what it would have returned gives, in one CCIP flight,

```
20 [HANDOFF] radar handoff asked of a non-radar unit;
             unguarded search would have returned a NON-RADAR vehicle
```

Twenty mis-targets in a single flight. The fix returns NULL, which is this
function's own documented contract — *"If the target is no longer valid and no
handoff is possible, NULL will be returned"* — and all three live callers
(`fccmain`, `sensclas`, `beamrider`) already handle NULL. Applied at both
`HANDOFF_RADAR` sites (the sim→camp and camp→sim searches), and the assertion is
left in place as the diagnostic.

Soak re-run with the guard in: six flights, zero crashes, zero whiteouts.

---

## Sprint 28 — PO verification against the Wine gold standard

The PO flew the CCIP TE on both builds and recorded them. **The whiteout is gone**
(confirmed by the PO on the A/G press), and leaving the SMS on CCRP the Linux run
is "almost identical to gold standard". Two things came out of it.

### CRASH-7 — bomb release dereferenced NULL (fixed, `a7017798`)

Found in the PO's session log, not by the soak. `BombClass::SetTarget` released
the old target, set `targetPtr = NULL`, then called `targetPtr->Reference()` —
because the only two lines that assigned `targetPtr` are commented-out
`Copy(OBJ_TAG, this)` calls whose signature no longer exists. `targetPtr` is a
`SimMoverClass` member initialised to NULL, so it was **provably NULL** there:
every release against a designated target, deterministically.

Same defect and cause as one already fixed in `MissileClass::SetTarget`; the bomb
was missed. All nine `::SetTarget(SimObjectType*)` implementations were audited —
the bomb was the only one lacking the assignment. The soak missed it because none
of those six flights dropped a bomb; *a soak only covers what it actually does*.

Also corrected: a comment in `simobj.cpp` claimed this exact backtrace was the
refcount race fixed as CRASH-1/2. It was not, and leaving that in place would
have sent the next reader hunting a race that is not producing this signature.

### BLUE-1 narrowed by the PO to the CCIP sub-mode

On **CCRP the run is clean**; the blue screen only appears after switching to
CCIP. That halves the search, and it means the release path itself is not at
fault — CCRP releases bombs too.

### BOMB-1 — bomb impact produces no explosion

Measured from the two recordings rather than by eye. Both 1920×1080/60. The Linux
capture is windowed, so the game region (`crop=1030:810:444:120`) has to be
isolated before any metric means anything:

* **Wine**, t≈130.5 s: a large fireball with smoke plumes on the tarmac —
  1.21% fire-coloured pixels, 9.9% bright.
* **Linux**, t=256 s: the bomb model is plainly visible falling toward the
  runway threshold. By t=260 s it is simply gone. No fireball, no smoke, no
  crater flash — and **no fire-coloured pixels anywhere in the game region across
  the entire 290 s run**.

Also worth noting from the same frames: the PILOT OPTIONS dialog at t=266 s
renders its text correctly. So the "text as white blocks" the PO saw earlier is
*not* a general font defect — it is part of the corrupted state that follows a
BLUE-1 event, which makes it a symptom of BLUE-1 rather than a separate bug.

Code path traced: `BombClass::DoExplosion` sends a `FalconMissileEndMessage` with
`endCode = BombImpact` and `SetParticleEffectName(auxData->psBombImpact)`.
`Process()` calls `AddParticleEffect(name)` when the name is non-empty, and
otherwise falls through to a legacy branch that should still spawn
`SFX_GROUND_EXPLOSION` for a high-explosive bomb. So *something* should draw
either way, which is what makes this interesting. `FF_DEBUG_MSLEND=1` reports the
end code, position, ground type, effect name and spawn result — that trace on one
real impact should settle it.

**Harness status for the repro:** `FF_SIM_KEY` does deliver the pickle
(`kbdPickle=1` confirmed in the log), so the input path works; the bomb did not
release because master arm and sub-mode were not set up. Master arm is
`Shift+M` (`0x32`, modifier 1) and no code change is needed to script it — a held
`0x2A` with `0x32` pressed inside it does the job with the existing syntax.

### BOMB-1 — most likely a symptom of the runway-elevation decoupling

The bomb's detonation test is, in `BombClass::Exec`:

```c
terrainHeight = OTWDriver.GetGroundLevel(x, y);
...
else if (z >= terrainHeight)   // z is positive-down; at or below the ground
{
    z = terrainHeight;
    SetExploding(TRUE);
}
```

and `DoExplosion()` — which is what sends the `FalconMissileEndMessage` that
spawns the effect — only runs once `IsExploding()` is set.

CLAUDE.md already documents that **at an airfield `GetGroundLevel` returns 0
while the airfield renders as a ~20 ft plateau** (the airfield post falls outside
the loaded fine-terrain radius and falls back to coarse 0). That is the same
defect that makes the jet collide 20 ft *under* the visible runway.

Applied to a bomb: it falls through the visible runway surface and detonates
~20 ft below it, so the fireball spawns **inside the terrain** and is occluded.
That is exactly what the PO's recording shows — the bomb reaches the runway and
simply disappears, with no effect at all.

Supporting evidence against the alternatives, all checked:

* The bomb data **does** define impact effects (`_mk82`, `_mk83`, `_mk84`,
  `$CLUSTER_BOMB`, …) — not an empty-name fallback case.
* Those names **do** exist in `terrdata/particlesys.ini`.
* Named-effect spawning demonstrably works — `FF_TEST_EXPLOSION` renders both the
  direct and named paths.

So the particle system is not at fault, and this is not a separate bug: **fixing
the elevation decoupling should restore bomb explosions as well as landings.**
That raises its priority considerably — it is not just a landing annoyance, it
also silently eats every bomb impact on an airfield.

**A stale blocker worth deleting.** The CLAUDE.md entry for that issue says
*"Blocker: agent can't capture sim-mode frames (glReadPixels=white,
import-window=black) so every attempt needs the user's eyes."* That is no longer
true — `FF_SIM_SCREENSHOT` has been capturing sim frames reliably throughout this
session, and the whole PIT-1 investigation was run on them. The issue is now
tractable unattended. It also references `memory/runway-elevation-decoupling.md`,
which no longer exists; the content survives only in CLAUDE.md.

Still to verify: a trace of `z` vs `terrainHeight` at an actual detonation. That
needs an automated release, which needs master arm plus a CCRP pickle held
through the solution — the PO's correction (no dive required in CCRP) means
HARNESS-1 does **not** need axis injection after all.

### BOMB-1 confirmed from the PO's ACMI tape

The PO dropped a bomb on the airstrip, saw no explosion, and reported one detail
that decides the case: **the explosion sound played *after* the bomb disappeared
into the airstrip.** A bomb that detonated on the visible surface would sound at
the moment it vanished; a delay means it kept falling below that surface first.

They also saved an ACMI tape, which turns the argument into numbers. Parsing
`TAPE0012.vhs` directly (header 80 bytes, entity records 36, position records 41
— the layout is pinned by static_asserts in `AcmiTape.h`):

```
entities=3  feats=892  positions=2467
uid=1, uid=2   flags=0x04 (aircraft)   z from -12362 to -12249
uid=393        type=377                the bomb, 197 samples
```

* The **bomb** is recorded from z = −12140 down to z = −729 ft (z is
  positive-down, so those are altitudes) and the tape stops before impact.
* **All 892 features are recorded at exactly z = 0.00** — minimum, maximum and
  mean all zero, not one non-zero value in the whole theater.

Two controls, because "every value is zero" is exactly the shape of a recording
artifact rather than a finding:

1. **Aircraft z records correctly** in the same tape (−12362 … −12249), so the
   position pipeline is not simply dropping z.
2. **The recorder writes the real field** — `featPos.data.z = theObject->ZPos()`
   in `simdrive.cpp:2189`. It is not writing a constant.

So the zero is the sim's own belief: every feature, the whole airfield included,
sits at `ZPos() == 0`. Across 892 features spanning a theater that cannot be real
elevation data. This is the same flat-z=0 world CLAUDE.md describes for the
runway-elevation decoupling, now measured from a live tape instead of inferred.

Applied to the bomb: it detonates when `z >= GetGroundLevel(x,y)` = 0, i.e. at
sea level, roughly 20 ft *below* the rendered runway — so the fireball spawns
inside the terrain and is occluded, and the bang arrives late because the bomb
had further to fall. That is precisely what the PO saw and heard.

**BOMB-1 needs no fix of its own.** Fixing the elevation decoupling should
restore bomb explosions, landings, and the jet's collision height together.

### BOMB-1 — RETRACTION: it is not airfield-specific

I closed BOMB-1 as a symptom of the *airfield* elevation decoupling, on the
strength of the ACMI tape showing all 892 features at `ZPos() == 0`. **That
conclusion was wrong, and the PO's next test is what killed it.**

Asked whether a bomb explodes anywhere that is not an airfield, they dropped one
on **open terrain in actual CCIP** and got identical behaviour: the terrain
swallows the bomb, a delay, the explosion *sound*, and no visible explosion.
Frame-scanning that recording's game region (same windowed crop) across all 172 s
gives a peak "fire" measure of 1.51%, and the frame responsible is the **credits
screen** — there is no explosion anywhere in the run.

So the airfield's flat-z=0 features cannot be the explanation: ordinary terrain
behaves the same. What survives from the tape analysis is only that features are
recorded at z=0; what does *not* survive is the inference that this is why bombs
vanish.

**What the two runs actually establish, together:**

* The bomb sinks *into* the rendered terrain rather than stopping at its surface,
  on both an airfield and open ground.
* It does detonate — the sound plays — but the effect is never visible.
* The particle system is not at fault (`FF_TEST_EXPLOSION` renders, the effect
  names exist in `particlesys.ini`, the bomb data defines them).

That still points at a rendered-vs-collision elevation mismatch, but a **general**
one rather than an airfield artifact — `GetGroundLevel` returning a lower surface
than the one being drawn, everywhere. The next measurement is therefore to
compare `GetGroundLevel(x,y)` against the rendered terrain height at the same
point over ordinary terrain, which needs no user input.

**Process note.** I built a confident, well-evidenced story from one tape and one
audio cue, and a single control the PO ran demolished the part that mattered. The
sound-delay reasoning was also weaker than I presented it: sound propagation
delay over a few thousand feet is seconds on its own, so the late bang never
needed a buried detonation to explain it. Worth remembering the next time a
measurement and a plausible mechanism seem to agree.

### BOMB-1 — third tape, and a reading I nearly got wrong

The PO's bridge run (`TAPE0013.vhs`) missed the bridge, so it is a pure ground
impact. Parsing it:

```
tape startTime=32742.6 totPlayTime=213.3  -> ends 32956.0
AIRCRAFT uid=1    800 samples, t 32742.6 -> 32955.8   final z=-7852
weapon   uid=591  148 samples, t 32918.7 -> 32938.4   final z=-1235
```

The asymmetry looks damning: the aircraft records to the end of the tape while
the bomb stops 17 s early at z = −1235 (1235 ft above **sea level**), still
descending ~640 ft/s. I checked the recorder for distance culling — there is
none, it walks the live object list — so the obvious conclusion is that the bomb
leaves the sim about 2 s before it reaches the ground, which would be a real
finding.

**It is not a safe conclusion, and the reason is worth writing down.** That
arithmetic assumes the ground beneath the bomb is at sea level. Nothing
establishes that — this is inland terrain, not the coastal airfield of the
previous tape. If the terrain there is ~1100 ft, the bomb stopped being recorded
essentially *at* impact, which is exactly what should happen. The tape gives z
relative to sea level and gives no terrain elevation at all (feature z is
uniformly 0 and therefore useless for this).

So this tape does **not** settle it either way. Recording it because the
"aircraft records to the end, bomb stops early" comparison is genuinely
compelling and will look like evidence to the next reader — it is not, without a
terrain height to subtract.

**The measurement that does settle it needs no user input:** instrument
`OTWDriver.GetGroundLevel(x, y)` and compare it against the terrain height the
renderer draws at the same point, over ordinary terrain. If those disagree, the
bomb sinking into the visible ground is explained and the fix is in the elevation
query; if they agree, then the ground level is right and the missing explosion is
somewhere in the effect path after all. That is the next thing to do, and it can
be run unattended.

---

## Sprint 29 — ELEV-1 measured: the elevation query is NOT broken

The PO asked for the measurement that would decide whether bombs sink because
`GetGroundLevel` disagrees with the drawn terrain. It has been run, and the
answer is **no — the query is correct where it matters.**

`OTWDriverClass::GetGroundLevel` delegates to `viewPoint->GetGroundLevel(x, y,
normal, &lod)` and the `lod` says which terrain detail level answered.
Instrumented with `FF_DEBUG_GROUND=1` (a per-second sample of the query, the
player's own position, and a probe at increasing range ahead of the aircraft).

**Parked on the runway, TE 02** — the aircraft's z *is* the rendered surface
there, so this is a direct read of any split:

```
[GROUND] query=0.00   lod=5  player z=-2.39   ground=0.00   split=-2.39
[GROUND] query=-26.00 lod=0  player z=-31.99  ground=-26.00 split=-5.99
```

Once the fine terrain has loaded the query answers at **lod=0 with −26.00 ft**,
and the aircraft rests 5.99 ft above it — which is just gear height. No 20 ft
split. The first sample *does* show the documented failure (0.00 at coarse
lod=5), but it is a **startup transient that resolves within a second or two**,
not a standing condition.

**Airborne over open terrain, probing ahead of the aircraft:**

| range | ground | lod |
|---|---|---|
| 0 ft | −1560 | **0** |
| 3 000 ft | −1695 | **0** |
| 6 000 ft | −1550 | **0** |
| 12 000 ft | −1344 | **0** |
| 24 000 ft | −1288 | 1 |
| 48 000 ft | −1425 | 2 |
| 96 000 ft | −746 | 3 |

Real elevations (this terrain is 1300–1700 ft), finest LOD out to 12 000 ft
(2 nm), coarsening gracefully beyond, and **not a single zero**. A bomb lands
well inside the lod=0 radius, so it gets a correct ground height.

**Consequences.**

1. **The "flat z=0 collision world" in CLAUDE.md does not reproduce.** What
   reproduces is a transient at terrain-load time. That entry should not be
   taken at face value; whatever it described has either been fixed since or was
   always the transient.
2. **BOMB-1 is not an elevation problem.** The bomb gets the right ground height,
   so the missing explosion lives somewhere in the impact/effect path after all —
   back to where the code reading pointed before I let the airfield theory pull
   me away.
3. The same applies to the landing complaint, which was attributed to the same
   flat-z=0 story and now needs re-measuring on its own terms rather than
   inheriting a conclusion.

**Harness status (HARNESS-1).** To instrument a real impact, `FF_TEST_BOMB` was
added: it raises the FCC's bomb pickle at given times so an automated flight can
release without a human. It is not working yet — `FCC->GetTheBomb()` stays NULL
because it requires `Sms->CurHardpoint()` to be a *bomb* station, and forcing the
master mode bypasses the station selection the real A/G press performs.
`curHardpoint=1` is selected but its weapon is not a bomb. Left in, default off.

---

## Sprint 30 — ASAN-1: a global-buffer-overflow in the campaign package code

The sanitiser build had not been rebuilt since 2026-08-17, and this session
changed global render state, the bomb target path and the radar handoff. Rebuilt
and soaked over the TE flights that exercise those paths.

Three A/G flights came back clean. The flight that reaches the **3-D pit** did
not:

```
ERROR: AddressSanitizer: global-buffer-overflow
READ of size 1 at 0x... thread T14
  #0 PackageClass::GetFACFlight()      package.cpp:2182
  #1 FlightClass::GetFACFlight()       flight.cpp:4284
  #2 FlightClass::GetFlightController() flight.cpp:4294
  #3 VoiceManager::AddNoise(...)       voicemanager.cpp:1704
  #4 VoiceManagementThread(void*)      voicemanager.cpp:662
```

`GetFACFlight` opens with `MissionData[mis_request.mission].skill`.
`mis_request.mission` is a **`uchar`** — 0…255 — and `MissionData` has
`AMIS_OTHER` = **41** entries. Nothing bounds it. A package whose request is
unset or stale therefore reads past the end of a global table, and it is the
**voice thread** doing it, mid-flight, on every session that gets there.

Fixed by returning NULL for an out-of-range mission, which is this function's
normal "no FAC flight" answer. Measured on the identical command and mission:
**1 error → 0**, with the flight still reaching the sim.

This is the same shape as CRASH-4 (an unchecked index into a fixed table read
from a thread that has no idea the value is stale) and the same thread that
carried VOICE-3. Note the other `MissionData[...]` index sites — `misseval.cpp`
1834/1836/3967 and `mission.cpp` 256/257 — take the same unchecked `uchar`; they
were left alone because nothing has shown them going out of range, but they are
where to look if this recurs.

**Method note:** "0 errors" and "the tool never ran" produce identical output, so
both were checked — the binary carries 39 `__asan` symbols and the runs were
confirmed to reach `RunningGraphics` before the result was believed. The first
pit run had in fact *not* reached the sim and had to be re-run with a longer
timeout, which is exactly how a clean-looking null result gets manufactured.

---

## Sprint 31 — BLUE-1 identified: it is the NVG path, not the bomb release

The PO reported the screen going blue after switching to CCIP and releasing a
bomb, and reasonably attributed it to the release. It is not the release.

Reasoning from the screenshot rather than the story: green cockpit art on a
**pure blue** field with the world absent is exactly what a green-tinted render
over an unkeyed chroma background looks like, and the engine has exactly one
mode that tints everything green — night vision (`SetGreenMode` plus
`ColorBankClass::GreenMode`). `ToggleNVGMode` is bound to **N (0x31)**.

Tested directly on an automated flight — press N mid-flight and measure:

| | pure blue | green |
|---|---|---|
| before | 0.0% | 0.1% |
| **after N** | **80.2%** | 5.6% |

The captured frame is a match for the PO's screenshot: green cockpit, pure blue
everywhere else, no outside world. **BLUE-1 is broken NVG rendering**, and it now
has a one-keystroke unattended repro instead of a bombing run.

Two things follow. The visible defect is that NVG renders the 2D pit green over
a chroma background that is no longer being keyed out, and drops the world
entirely — the blue is the panel's chroma colour showing through. Separately,
something in the PO's session enabled NVG without them pressing N; that is worth
finding, but it is a second question and the rendering is broken either way.

Also cleared this sprint: the "no visible explosion" hunt gained nothing from a
420 s flight — zero weapon impacts of any kind, so waiting for AI bombs is not a
route to an instrumented impact either.

### BLUE-1 bisected to `ContextMPR::SetNVGmode` — with a contradiction left standing

Measured with a metric that does not depend on where the aircraft happens to be
pointing. The first attempt used "percentage of blue pixels", which varies with
attitude between runs and produced misleading numbers; counting the **draws that
paint the probed pixel blue** does not:

| configuration | blue-painting draws | all from tex 29 |
|---|---|---|
| NVG on (baseline) | 865 | 865 |
| `FF_NVG_NOVTX=1` — suppress the NVG vertex tint | 790 | 790 |
| `FF_NVG_SKIP=ctx` — skip `context.SetNVGmode(TRUE)` | **0** | 0 |

So the culprit is `renderer->context.SetNVGmode(TRUE)`, and the blue is texture
29 — the 2D cockpit panel — painting its **pure-blue chroma background** because
the alpha test stops discarding it. The world renders correctly underneath
(green terrain is visible in the probe trace right before the panel covers it).

**The contradiction, recorded rather than papered over.** `SetNVGmode` does
nothing but assign the flag:

```c
void ContextMPR::SetNVGmode(BOOL state) { NVGmode = state; }
```

and that flag has exactly **two** readers in the tree — the NVG vertex tint at
`context.cpp` 3252 and 3369, confirmed by searching for the tint constants
(`0xFF00FF00` / `0x0000B400`) across every source file. Yet suppressing both
readers leaves the bug (790 draws) while clearing the flag removes it (0). One of
those two facts has to be wrong, and finding out which is the next step. The
guard was verified to be compiled in (the env string is present in the binary and
the code is at both sites), so the likeliest explanations are that the panel is
drawn through a path that reads the flag some other way, or that the two runs
differ in something not yet controlled.

Everything here is env-gated and off by default; the defaults regression is
clean (WHITE-1 0.0% white, 0.0% blue).

### BLUE-1 FIXED — the contradiction resolved, and it was my grep

The contradiction recorded above ("the flag's only readers are the tint sites,
yet suppressing them leaves the bug") had a boring cause: **my grep was filtered.**
Searching again with nothing excluded turned up a third reader:

```
otwloop.cpp:2600:  if (renderer->context.NVGmode) TheDXEngine.SetState(DX_NVG);
```

That is the one that matters, and the exclusion pattern I used to tidy the first
search is exactly what hid it. Worth remembering: a filtered grep that returns a
tidy answer is a filtered answer.

**Root cause.** `DX_NVG` builds night vision from a four-stage texture pipeline —
`D3DTOP_ADDSMOOTH` on stage 0, `ADDSIGNED` on 1 and 3, `DOTPRODUCT3` on 2 — and
moves the alpha source to stage 3 via `CDXEngine::m_AlphaTextureStage`, where
chroma keying is then configured (`ALPHAARG1=TEXTURE, ALPHAOP=SELECTARG1`).

**This layer implements none of those three operations.** They are absent from the
`COLOROP` switch entirely and fall through to its `default`. The state therefore
leaves stage 0 set up for a pipeline that never executes, and the 2D cockpit
panel drawn afterwards through the MPR path inherits it, loses its chroma key,
and covers the screen with its pure-blue background.

**Fix:** skip `SetState(DX_NVG)` until those operations exist. Nothing real is
lost — it configures a pipeline the layer cannot run — and NVG then renders
properly: green world, green cockpit, legible HUD and MFDs. The green comes from
`SetGreenMode` and `ColorBankClass::GreenMode`, which are independent of this.
`FF_NVG_DXSTATE=1` restores the old behaviour.

| | blue-painting draws |
|---|---|
| fixed (default) | **0** |
| `FF_NVG_DXSTATE=1` | 724 |

Six-flight regression soak clean after the change.

**A speculative fix that was tried and reverted.** Before finding the third
reader I redirected the alpha stage states to texture unit 0 when the requested
stage had no texture bound — reasoning that `m_AlphaTextureStage = 3` sent the
chroma config to an empty unit. It reduced the blue draws (~860 → 673) without
fixing it, and in the form that fired at all it redirected *every* stage, which
would disturb genuine multitexturing. Reverted: a change that neither fixes the
bug nor has evidence behind it is worse than none, however plausible its story.

---

## Sprint 32 — MFD-THRU-1 fixed: the 3-D pit MFDs are opaque again

This was parked because the obvious fix broke something worse: forcing a
non-chroma composite made the MFDs perfect *and* turned the HUD into an opaque
black slab. That was a **global** override, though — `FF_RTT_BLEND` applied to
every canvas. The MFD call sites can be told apart from the HUD's.

`SetRttCanvas` now takes an `opaqueCanvas` flag, defaulted false, and only the
two MFD sites in `mfd.cpp` pass true. Those canvases then composite opaquely
instead of chroma-keyed. The HUD is not marked, so it stays keyed and remains
see-through.

Why this is the right shape: the MFD screens are *holes* in the pit model,
filled only by this composite (verified earlier — removing the composite leaves
the bleed unchanged at 331 vs 329). `3Dckpit.dat` asks for chroma, but the
canvas background is rendered with alpha 0, so a keyed composite discards the
background and the outside world shows through the instrument. An MFD is an
opaque display; it should not be keyed.

| | left-MFD bleed | dark |
|---|---|---|
| fixed (default) | **34** | 668 of 810 |
| `FF_NO_OPAQUE_MFD=1` | 328 | 321 |

And the constraint that blocked this is satisfied — the HUD region is
**identical** in both runs (47 of 1911 dark), so it is untouched and still
transparent. Six-flight regression soak clean.

**Measurement note:** the captures came back 1024×768 rather than the 1920×1080
of earlier sessions, and the fixed pixel crop I had been using silently indexed
out of range. The metric is now expressed as fractions of the frame, which is
what it should have been from the start — a crop tuned to one resolution is a
measurement that quietly stops meaning anything when the resolution changes.

---

## Sprint 33 — BOMB-1 root cause: the explosion branch excluded bombs

The PO's "bomb hits, no explosion" is now explained end to end, and the chain
only became visible once an automated release existed.

**Getting a release.** `FF_TEST_BOMB=<sec>` raises the FCC's bomb pickle at a
given time. It stalled for several attempts because `FCC->GetTheBomb()` needs
`Sms->CurHardpoint()` to be a bomb station, and neither `SetCurHardpoint` (leaves
`curWeapon` stale, which `DropBomb` early-outs on) nor
`SelectWeapon(wtMk82, wdGround)` (selects station 1, which holds no bomb) does
the job. `SMSClass::SetCurrentWeapon(station, weapon)` is the SMS's own entry
point and sets both. With that, bombs release on demand.

**What the release showed.** A live Mk-82 impact produces:

```
Process: endCode=11 (BombImpact) type=2 stype=3 TYPE_MISSILE=6 psName=''
```

Two independent things are wrong, and each alone would have been enough:

1. **The named effect is empty.** `dataIdx` is **0** for both Mk-82 and Mk-84,
   and bomb dataset 0 is `default`, whose `.dat` defines no `psBombImpact`. The
   named-effect path is therefore correctly skipped. (The weapon table itself
   reads fine — mnemonic `M82`, class 2, domain 2, type 5 — so this is the data's
   own index, not a 32/64-bit layout bug.)
2. **The fallback that should then draw it is unreachable.** The `endCode` switch
   holding `case BombImpact` and `case FeatureImpact` is gated on
   `if (type == TYPE_MISSILE)`. A bomb is `TYPE_BOMB` (2), not `TYPE_MISSILE`
   (6). Proven rather than inferred: a trace placed *inside* `case BombImpact`
   never executed while a `BombImpact` message was being processed.

So no path drew a bomb impact at all. Fixed by letting bombs into the switch —
its cases are selected by `endCode`, so a bomb only ever reaches the bomb ones.
`FF_NO_BOMB_IMPACT_FX=1` reverts.

| | impacts | legacy branch reached | effect spawned |
|---|---|---|---|
| fixed | 1 | **1** | `SFX_GROUND_EXPLOSION` at the impact point |
| `FF_NO_BOMB_IMPACT_FX=1` | 1 | **0** | none |

with `DamageType=2` (`HighExplosiveDam`) and `BlastRadius=293` reaching the
spawn. Six-flight regression soak clean.

**Not yet visually confirmed.** The trace proves the effect is spawned at the
impact point, and `FF_TEST_EXPLOSION` separately proves the particle system
renders — but I have not caught the fireball in a screenshot, because the impact
lands roughly 2 nm ahead of and below an aircraft flying straight and level, and
the cockpit view does not necessarily contain it. The PO seeing a fireball is
still the test that closes this.

**A measurement error worth recording.** The first attempt at reading the weapon
type printed `type=-1 stype=-1`, which looked like an unresolved entity class and
would have sent me hunting a class-table bug. The trace was simply placed *above*
the assignment — it was printing the variables' initial values. Moving it below
the lookup gave `type=2 stype=3`, the real answer. A trace is a measurement, and
where you put it is part of the measurement.

### Assertion inventory and ASAN coverage after the fixes

The six-flight soak's assertion inventory is now down to two sites, and **both
are explained**:

* `handoff.cpp:86` — HANDOFF-1's own assertion, deliberately left in place as the
  diagnostic. The guard below it returns NULL; the assertion still reports the
  condition.
* `atm.cpp:792` — `ShiAssert(sq->GetRating(j) == 0 or uc->Scores[j] > 0)`.
  **Checked and benign.** The loop runs `j < ARO_OTHER` (16) over
  `Scores[MAXIMUM_ROLES]` (16), so it is in bounds — this is not the ASAN-1
  shape. It fires while a squadron's rating decays in a role its unit class
  scores 0 for: `(rating*2 + 0)/3` takes a few campaign cycles to reach zero, and
  the assertion is over-strict during that decay, exactly like the already
  documented `atm.cpp:2134`. Recorded so it is not chased again.

ASAN coverage was also extended past the TE flights that the first soak used:
a **campaign** run (commit → priorities → START CAMPAIGN → pick a flight → slot
in → TAKEOFF) reaches the sim with **0 errors**. That matters because the
campaign path is where VOICE-3 and CAMPUI-1 lived, and the TE-only soak would
never have touched it.

### NVG-2 sized: exactly three texture ops are missing, and only NVG/TV want them

`FF_DEBUG_TEXOP=1` reports every D3D texture op that falls through the `COLOROP`
switch's `default` and silently becomes `MODULATE`.

* **A normal A/G flight reports none.** The gap does not touch ordinary
  rendering.
* With the NVG state restored (`FF_NVG_DXSTATE=1`) it reports exactly three:

```
[TEXOP] unimplemented COLOROP value=11 on stage 0   (D3DTOP_ADDSMOOTH)
[TEXOP] unimplemented COLOROP value=8  on stage 1   (D3DTOP_ADDSIGNED)
[TEXOP] unimplemented COLOROP value=24 on stage 2   (D3DTOP_DOTPRODUCT3)
```

which is precisely the `DX_NVG` stage setup. The diagnosis behind the BLUE-1 fix
is therefore confirmed by direct measurement, not just by reading the code.

Two of the three map exactly onto GL combine modes (`GL_ADD_SIGNED`,
`GL_DOT3_RGB`); `ADDSMOOTH` (a + b − ab) has no direct equivalent and would need
approximating. **Deliberately not implemented yet.** `DX_TV` shares the same
branch and drives the Maverick and laser-pod displays, so implementing these
changes how those render — and there is no way to tell better from worse without
the Wine reference to compare against. Shipping an unvalidatable rendering change
is how the earlier speculative alpha-stage redirect happened. The gap is now
measured and named; implementing it is a task for when a reference is at hand.

### A third stale note: the landing missions load again

CLAUDE.md records that TE "09 Landing Final Approach" and "10 Instrument
Landing" **cannot load** — their unit data was said to overrun the decode buffer
at the second unit, and the note concludes the missions "still won't LOAD
(incompatible unit data — a deep format issue not worth the risk)".

**Both load and fly now.** Tested directly:

```
StartReadCampFile: filename='09 Landing Final Approach'  -> reachedSim=1
StartReadCampFile: filename='10 Instrument Landing'      -> reachedSim=1, joinFailed=0, crash=0
```

and a capture from TE 09 shows a normal cockpit on approach over water with the
coastline ahead. Whatever fixed it came from the intervening decode work; the
note was never revisited.

This matters beyond tidiness: the PO's standing "can't land" complaint was
logged against a build where the two landing *training* missions could not even
be started. They can be flown now, which is the natural way to test landing.

That is three stale claims in this file corrected in one session — the flat-z=0
collision world, "the agent can't capture sim frames", and now the unloadable
landing missions. **Notes describing a defect are only true of the build that
observed it**; each one here was disproved by simply running the thing again.

---

## TESWEEP-1 — all 34 Tactical Engagement missions swept

Every stock TE mission was launched, driven to the 3D world, held there, and
exited, one process per mission, logs kept at `/tmp/te-<row>.log`.

**Result: 34 of 34 load, reach the sim, and exit clean.** No segfaults, no
aborts, no join failures, no mission that fails to load. That includes rows 33
and 34 (the two F-18 carrier missions), which a note in this file previously
claimed were unloadable — the third stale claim corrected this session.

The value of the sweep is less the pass/fail than the **assertion inventory** it
produced, since recurring assertions have twice this project turned out to be
unfixed bugs rather than noise. Across the 34 runs, 68 assertion *lines* were
logged at 8 distinct sites; 7 missions were assertion-free.

> **Read the counts below as coverage, not frequency.** See "What an assertion
> count actually counts" further down: `ShiAssert` suppresses a site after its
> first hit and prints two lines per hit, so every number in this section is
> `2 × (sites that fired at least once)`. The 68 lines are 34 site-firings, and
> a mission listed with "6" hit three distinct sites, not six times.

| count | site | assessment |
|---|---|---|
| 20 | `team.cpp:1800` | guarded — `AttachChild` already rejects `slotNumber >= nSlots` |
| 18 | `texbank.cpp:314` | guarded |
| 10 | `handoff.cpp:86` | **upstream assert is over-strict.** A HARM asking to hand off inside a unit with no radar is a normal request, not a defect; HANDOFF-1 now answers it with NULL, which every caller handles. The assert fires on the healthy path |
| 10 | `atm.cpp:792` | benign decay clamp |
| 4 | `objectiv.cpp:3663` | guarded — the loop's `else` resets `count = 0` |
| 4 | `drawbsp.cpp:108` | guarded |
| 2 | `seeker.cpp:350` | degenerate but safe — see below |
| 2 | `drawbsp.cpp:107` | guarded |

Missions touching the most distinct sites are the weapon-employment ones
(Rockets and 20mm Cannon at three sites each; HARMs, CCRP Bombs, AIM-7 and AIM-9
at two), which is where the `handoff.cpp` and `seeker.cpp` sites live.

**`seeker.cpp:350` — an ARH missile going active with `GetRadarType() == 0`.**
Memory-safe: `RadarClass::RadarClass` does `radarData = &RadarDataTable[type]`,
and index 0 is a real entry, with the follow-on `RDRDataInd` lookup bounds-checked
against `NumRadarDatFileTable`. So nothing is read out of range — the missile just
goes active carrying a no-radar radar and cannot guide. Recorded rather than
fixed, because whether that is wrong depends on whether the missile *should* have
had a radar type, which is a data question needing the Wine reference.

Worth flagging: this is the **same "radar type 0" condition as HANDOFF-1**, now
seen at a second site. Two independent code paths reaching a zero radar type
suggests the value may be under-populated at its source rather than each
consumer being individually at fault. That source is the thing to look at next
if a guidance defect shows up.

### Method note — the same measurement error, twice more

Row 33 first read `sim=0` with no mission name and looked like the sweep's one
genuine failure. It was a **mid-write sample**: re-read after the run completed,
it showed `RunningGraphics: 1` and the correct mission name. This is the third
time this session a metric was read before the thing it measures had finished
(the others: "0 ASAN errors" from a run that never reached the sim, and a pixel
metric sampling rows the geometry had already left). *A number harvested from a
log while the process is still writing it is not a measurement.* Check the
process has exited before believing the count.

---

## RADARTYPE-1 — the zero radar type, and two real bugs behind it

The TE sweep left one open thread: two independent sites reaching
`GetRadarType() == 0`, at `handoff.cpp:86` (10 hits) and `seeker.cpp:350` (2).
Both consumers were already safe, so the question was whether the *value* was
wrong. It is not. The value is correct everywhere, and each site was asking the
wrong question about it.

### The data is fine — this is not another VOICE-1

The first suspicion was a struct-layout mismatch, since `RadarType` is read by a
raw block `fread` straight into `WeaponClassDataType`, and `LoadWeaponData`
**skips its size sanity check entirely on the `g_bFFDBC` path** — the exact shape
of VOICE-1/VOICE-2. Measured instead of assumed:

| table | `sizeof` here | file | arithmetic |
|---|---|---|---|
| `WeaponClassDataType` | 60 | 47 822 / 47 824 | `2 + 797×60` and `2 + 797×60 + 2` |
| `UnitClassDataType` | 336 | 252 002 | `2 + 750×336` |
| `VehicleClassDataType` | 160 | 110 562 | `2 + 691×160` |

Every table divides exactly, in both the plain (count leading) and DBC (count
trailing) copies of each file, and parsing at those offsets yields identical
field distributions from the two independently-formatted copies. The 64-bit
build's layout matches the 32-bit on-disk format for this whole family.

### `seeker.cpp` — SEEKER-1, an operator-precedence bug that disarms Sparrows

Reading the weapon table directly settles what a zero means:

```
AIM-120B  51    AIM-7M   0     AA-10A  0
AIM-120C-4 148  AIM-7E   0     AA-10B  0
AIM-120C-5 67   AIM-7E-2 0     AA-10C  0
AIM-120C-7 66   AIM-7F   0     AA-10D  0
AA-12     63
```

Only 15 of 797 weapons carry a radar, and the split is exactly right: active-radar
missiles have one, semi-active ones do not. A Sparrow has no radar of its own —
it rides the launching aircraft's illumination. `RadarType 0` is correct data.

The missile parameter files agree independently. `sim/misdata/*.dat` is parsed
positionally by `readin.cpp`, and its "Time To Go Active (sec)" field lands
exactly where `mslActiveTtg` is read (between Autopilot Bandwidth and Seeker
Type, in both files' comment labels):

```
aim120B.dat   15.0      aim7.dat    -1
aim120c.dat   15.0      aim7e.dat   -1
aa12.dat      15.0      aim7f.dat   -1
```

So the two tables encode the same fact twice: the AMRAAMs and the AA-12 go
active, the Sparrows never do.

The bug is the branch that decides to go active, in the live (`NEW_RUNSEEKER`)
`RunSeeker`:

```c
if (
    inputData->mslActiveTtg > 0 and
    ( timpct * factor < inputData->mslActiveTtg and ...Type() not_eq Radar ) or
    ( launchState == InFlight and ...Type() not_eq Radar and (not isSlave or not targetPtr) )
)
```

`and` binds tighter than `or`, so the `mslActiveTtg > 0` test at the top gates
**only the first disjunct**. Any in-flight missile with a non-radar seeker and no
slaved target took the second one — including every semi-active weapon in the
game. And `GoActive()` is destructive: it deletes the working passive seeker
*before* it ever looks at `GetRadarType()`, then installs
`RadarMissileClass(0)`. The missile is left with a no-radar radar and cannot
guide.

The dead `#else` copy of this identical condition repeats the `mslActiveTtg`
test inside the second disjunct. The live copy lost it.

Fixed both halves: the condition now gates the second disjunct as the legacy copy
does, and `GoActive()` refuses the transition before destroying anything.
`FF_NO_SEEKER_TTG_FIX=1` restores the old behaviour.

### `handoff.cpp` — HANDOFF-2, and a correction to my own HANDOFF-1

The handoff assertions are **not** in the HARM mission. They are in rows 15, 17,
19, 20 and 22 — Sidewinder, Sparrow, CCRP, CCIP and 20mm A-G, two apiece. The
caller is `GroundListElement::HandoffBaseObject`, the FCC's ground target list,
which asks `SimCampHandoff(baseObject, HANDOFF_RADAR)` — "find the emitter
vehicle inside this unit."

That is the right question for the HTS/RWR and HARM callers. It is the wrong one
here: an FCC ground target is a bomb or gun target. **266 of 750 unit classes
(35%) have no radar vehicle at all** — Supply, Armored, Corps Arty, SCUD,
Airlift — so for a third of the target set the question has no answer.

This also corrects HANDOFF-1 from earlier in this session. Before that fix the
mismatch was invisible: the search fell through and matched the first component
whose radar type was also 0, i.e. an arbitrary live vehicle. That is genuinely
wrong for the emitter callers, which is what HANDOFF-1 fixed by returning NULL —
but for the FCC ground list it was roughly the right answer, and returning NULL
made it *drop* the target instead of following it into the deaggregated unit.
`HANDOFF_LEADER` asks what this caller means and is the deterministic form of
what the old fall-through did. `FF_FCC_HANDOFF_RADAR=1` restores the old style.

### Measured, both fixes, both directions

The assertion count alone would not have been enough: an assert that stops firing
proves the question changed, not that the answer got better. So `[FCCHANDOFF]`
(under `FF_DEBUG_HANDOFF=1`) reports what actually happened to the target —
`followed` into the deaggregated unit, or `dropped` to NULL.

TE row 22 "20mm Cannon (A-G)", two runs each way, identical numbers each time:

| | dropped | followed | handoff asserts |
|---|---|---|---|
| fixed | **0** | 131 | 0 |
| `FF_FCC_HANDOFF_RADAR=1` | **40** | 91 | 2 |

40 + 91 = 131. The old style lost the FCC's ground target on **31% of handoffs**,
which tracks the 35% of unit classes that have no radar vehicle. The fix follows
all 131.

TE row 29 "Offensive BFM" and row 22, assertion counts, two runs each way:

| | row 29 seeker | row 22 handoff |
|---|---|---|
| both fixes in | 0, 0 | 0, 0 |
| both reverted | 2, 2 | 2, 2 |

The reverted counts reproduce the sweep exactly. The cross terms are the useful
part: the seeker fix never moves the handoff count and the handoff fix never
moves the seeker count, so each metric responds only to its own change.

**One fix's correctness depended on which caller you looked at.** HANDOFF-1 was
right about the sensor callers and wrong about the FCC, because a single style
enum was serving two different questions. Worth checking the other callers when
a shared helper is changed on the strength of one of them.

---

## WPAREN-1 — adding `-Wparentheses`, and what it found

SEEKER-1 was an operator-precedence bug, so the obvious next question was how
many more of those exist. `-Wparentheses` was added to the `FF_WARN` build for
exactly that. It yields **295 warnings**:

| count | class |
|---|---|
| 154 | assignment used as truth value |
| 126 | `&&` within `\|\|` — the SEEKER-1 shape |
| 15 | arithmetic in operand of a bit-op |

This is a queue to triage, not a bug list: most instances are correct as written.
`seeker.cpp:29` is a good example — `not sensorArray or not sensorArray[0] or
(PreLaunch and parent and IsAirplane and MasterArm == Safe)` parses exactly as
intended. Two genuine defects came out of the first pass.

### GNDATK-1 — a null dereference in AI ground attack

`gndattck.cpp:4408` read:

```c
if (groundTargetPtr and (A) or (groundTargetPtr->BaseData()->IsCampaign() and ...))
```

which parses as `(groundTargetPtr and A) or (groundTargetPtr->…)`. With no ground
target selected the first disjunct short-circuits false and the second
dereferences the null pointer. The commented-out original directly above it
(`// if (groundTargetPtr and groundTargetPtr->BaseData()->IsMover())`) shows the
null check was meant to cover the lot. `GetCampaignObject()` was also being
dereferenced without a check, which `handoff.cpp` does check before use.

Same file as CRASH-4, which came from one of the PO's gdb backtraces.

**Status: fixed by inspection, reachability not demonstrated.** `FF_DEBUG_GNDATK=1`
reports whenever the pre-fix expression *would* have dereferenced null, and over
four runs (Mavericks ×2, 20mm A-G, CCIP bombs) it fired **zero** times. The parse
is unambiguous and the fix is safe, but this is the same status as CRASH-4: a
correct guard on a path I could not make the automation reach. Not "verified".

### Twenty `=` where `==` was meant

```c
if (theRadar->digiRadarMode = RadarClass::DigiSTT)   // assigns, always true
```

Twenty sites across `actions.cpp`, `bvrengage.cpp`, `wvrengage.cpp` and
`wingactions.cpp` — the same copy-pasted block. Each *assigns* the AI's radar
mode while drawing a debug label, then branches on whether the constant is
non-zero, so the label always reads " STT" and the act of displaying the radar
mode changes it. Only live when `g_nShowDebugLabels & 0x40`, so this is not in
the normal AI path — but it means the one facility for observing AI radar
behaviour both lies and perturbs what it measures. Worth knowing before anyone
turns those labels on to diagnose something. Fixed to `==`, plus two
`strcat(tmpchr, "%s OFF")` that printed the format string literally.

### A hypothesis of mine that the measurement killed

After HANDOFF-2, TE row 22 grew a new assertion site, `texbank.cpp:407`, that was
not in the sweep baseline. The tidy explanation was ready: the FCC now retains
its target instead of dropping it, so a model that used to be abandoned is now
released, and `Release()` hits the same guarded sentinel-id check that
`Reference()` (line 314) already did.

Running it both ways killed that:

```
fixed    handoff86=0  texbank314=2  texbank407=2
revert   handoff86=2  texbank314=2  texbank407=2
```

`texbank:407` appears identically with the fix reverted, so it has nothing to do
with HANDOFF-2. The handoff numbers in the same table re-confirm HANDOFF-2 for
free.

My second explanation was that it was a run-length artifact, since these runs
were 180 s against the sweep's 150 s. **That is also wrong** — a later 150 s run
of the same mission shows it too (`314 ×2, 407 ×2, 792 ×2`, with `86` gone).

So: the site is guarded, it is not caused by HANDOFF-2, and its absence from the
sweep baseline is **unexplained**. It may be run-to-run variation in a stochastic
sim, or another change in this commit range. Recorded as open rather than
resolved, because two tidy explanations have already failed.

Both explanations were plausible and mechanistic, and the first would have gone
into this file as fact if the revert arm had been skipped on the grounds that the
story hung together. **A new symptom appearing next to a change is not evidence
the change caused it** — and the follow-up explanation deserves the same
scepticism as the one it replaced. Cost of checking each: one run.

### WPAREN-1 triage, second pass — the crash shape, searched properly

GCC prints only the *first* line of a multi-line condition, so grepping the
warning text finds only single-line instances. Re-running the search against the
source — reading each flagged condition through to its closing paren — turns 126
warnings into 8 candidates for the GNDATK-1 shape (a pointer null-guarded before
an `or` and dereferenced after it). Of those:

* **3 are false positives.** The `harmPod` chains in `gndhud.cpp:781`,
  `mislhud.cpp:643` and `navhud.cpp:1940` re-guard `harmPod` in every disjunct.
* **1 is `dlogic.cpp:877`,** where the `sensorArray` guard covers only the first
  disjunct. Grouping restored — but it is a tidy, not a fix: the lines directly
  above it already dereference `sensorArray[0]` unguarded, so a NULL array would
  have crashed before reaching it.
* **4 are in `navhud.cpp` (945, 1004, 1327, 1519) and are left alone deliberately.**

The navhud four are worth stating precisely, because the interesting problem
there is *not* the null dereference my search was looking for:

```c
if (g_bRealisticAvionics and g_bINS and ownship and ownship->INSState(INS_PowerOff) or
    not ownship->INSState(INS_HUD_STUFF))
```

`ownship` is already dereferenced unguarded in the surrounding lines
(`((AircraftClass*)ownship)->af->gearPos`), so it cannot be NULL here and there
is no crash to fix. What the precedence actually does is put
`g_bRealisticAvionics and g_bINS` on the **first disjunct only** — so with
realistic avionics switched *off*, INS state can still move the heading tape.
At `navhud.cpp:1327` an enclosing `if (g_bRealisticAvionics and g_bINS)` makes
that moot; at 1004 there is no such enclosure.

That is a change to what the HUD draws, and the correct grouping is a judgement
about intended avionics behaviour, not about C. **Deferred to a Wine
comparison**, same as NVG-2: the gold-standard build can show whether the heading
tape moves with realistic avionics off, and that answer decides the grouping.
Guessing here would change what the PO sees on the HUD on the strength of an
operator-precedence argument alone.

---

## TEXBANK-1 closed, and what an assertion count actually counts

TEXBANK-1 asked two things about `texbank.cpp`'s invalid-index guard: are the ids
the `-1` sentinel the guard assumes, or genuine miscomputed indices; and why did
`Release()` (line 407) start appearing when `Reference()` (line 314) always had.
`FF_DEBUG_TEXBANK=1` reports every rejected id. One TE row 22 run:

```
16 [TEXBANK] REF invalid id=-1
16 [TEXBANK] REL invalid id=-1
```

**Every** rejected id is exactly `-1` — never an out-of-range value — so the guard's
premise is right and the condition is by design. Reference and release are
perfectly paired at 16 each, so skipping both sides leaves no refcount skew.
TEXBANK-1 is benign; closed.

### The part that matters more

That same run logged **2** assertion lines at line 314 and **0** at 407, while the
trace shows the condition occurred **16 times at each**. So assertion output is
nothing like a frequency count. From `shierror.h`:

```c
#define ShiAssert( expr ) \
 if (shiAssertsOn && !(expr)) { \
     static int skipThisOne = FALSE; \
     if (!skipThisOne) { ... choice = MessageBox(...); \
     ... else if (choice == IDIGNORE) { skipThisOne = TRUE; } ...
```

`skipThisOne` is a **per-site static**, and the Linux `MessageBoxA` stub returns
`IDIGNORE` unconditionally for `MB_ABORTRETRYIGNORE`. So every site fires **once
per process** and is suppressed forever after. It also prints twice per hit —
once through `OutputDebugString`, once through the stub's `fprintf` — which is
why every count in the inventory is even.

**An assertion count is `2 × (distinct sites that fired at least once)`.** It
answers "was this site reached in this run", never "how often". Every number in
the TESWEEP-1 inventory above has been reframed accordingly: 68 lines are 34
site-firings, `team.cpp:1800` at "20×" means it was reached in 10 missions, and
the missions "asserting most" are the ones touching the most distinct sites.

This also dissolves the rest of TEXBANK-1. Whether line 407 appears in a given
log is a one-bit fact — did `Release()` see a `-1` before the log window closed —
so its absence from the sweep baseline was never the anomaly it looked like. Both
of my earlier explanations for it were wrong, and so was the premise of the
question.

Worth carrying forward: the inventory is still a good work queue — it is how
VOICE-1, VOICE-2, SAVE-2 and now SEEKER-1 were found — but only for *which* sites
are reached. Ranking by count ranks nothing. To measure how often a condition
actually occurs, add a trace like `FF_DEBUG_TEXBANK`; the assertion will not tell
you.

---

## WINE-1 — the gold standard is agent-drivable after all

Two open items (**NVG-2** #41 and **NAVHUD-1** #47) were parked on "needs the Wine
reference", where "the Wine reference" meant *the PO running the Windows build and
recording video*. That framing was wrong: the Windows build runs here, under this
agent, and can be captured the same way the Linux build can.

The one thing that made it look impossible:

```
$ wine FFViper.exe
wine: '.../WP' is a 32-bit installation, it cannot support 64-bit applications.
$ WINEARCH=win32 wine FFViper.exe
wine: WINEARCH is set to 'win32' but this is not supported in wow64 mode.
```

Neither message is about FreeFalcon: the prefix is 32-bit, system `wine` defaults
to wow64, and `WINEARCH` cannot override it. System `wine32` *will* boot it — but
**that is not the supported path, and using it was my mistake.** The PO already
has a launcher, `~/sgl/SAT/freeFalcon/freeFalcon.sh`, which pins a specific
runner and explains why in a comment:

> system wine 10 (wow64) can't boot this 32-bit prefix at all; lutris-9.22-staging
> detected the joystick but the setup screen locked up. Wine-GE 8 boots the prefix
> and the game enumerates the stick via DirectInput8

and launches inside a **Wine virtual desktop** sized to match `display.dsp`:

```sh
wine explorer /desktop=FreeFalcon,1024x768 'C:\FreeFalcon6\FFViper.exe'
```

That detail is the whole answer to capture (below). The script also documents why
the executable needs a full Windows path, why the desktop must be exactly
1024×768 (exclusive fullscreen mode-set fails on a multi-head screen otherwise),
and why the window has to be pinned to 0,0 with `xdotool` on this two-monitor
setup. **Read the launcher before driving the Wine build**; it encodes several
hours of someone else's debugging.

First capture is the main menu, rendering correctly — FreeFalcon 6.0 / FFViper
2.3.3.44, full menu bar. The useful part is that **the Windows build's UI lands on
the same coordinates the Linux harness already uses**: the menu bar sits at
y≈750 with TACTICAL ENGAGEMENT at x≈673, which is what `FF_UI_CLICK`'s
`677,748` was written against. So the existing click scripts translate to
`xdotool` against the Wine window with essentially the same numbers.

### The catch, found by trying it

`scripts/wine-drive.sh` drives the Wine window with window-relative `xdotool`
clicks, and **the clicks land**: a test click at `673,750` came back with
TACTICAL ENGAGEMENT visibly highlighted, at exactly those coordinates.

But the rest of that same capture is **black**. The Windows build renders through
DirectDraw, so an `import` of the window returns only what X has been given —
freshly blitted damage regions — not the DirectDraw surface. The one region that
had just been redrawn (the hover highlight) is the one region that appeared. The
earlier full menu capture worked because it happened to land right after a full
redraw.

Both `import -window` and `ffmpeg x11grab` returned black. That looked like the
familiar wall — DirectDraw/GL surfaces are not X-readable, which is exactly why
the *Linux* build needed in-process `glReadPixels` rather than an X grab.

It was not that wall. It was that I was launching the game **wrong**: system
`wine32` running `FFViper.exe` directly gives a 1920×1080 exclusive-fullscreen
window on the second monitor, and that surface is not capturable. Launched the
supported way — `freeFalcon.sh`, GE-Proton runner, inside
`wine explorer /desktop=FreeFalcon,1024x768` — the game renders into an ordinary
X window inside the virtual desktop, and a plain `import -window` returns **a
clean, complete 1024×768 frame**. Verified: the main menu captured in full.

So both halves work:

| | mechanism | status |
|---|---|---|
| drive | `xdotool mousemove --window W x y click 1` | works; a test click at `673,750` highlighted TACTICAL ENGAGEMENT |
| capture | `import -window <Falcon 4 - FreeFalcon>` inside the virtual desktop | works; full 1024×768 frame |

And the coordinates line up with the Linux harness: the Wine window is exactly
1024×768 with the menu bar at y≈750 and TACTICAL ENGAGEMENT at x≈673 — the same
numbers `FF_UI_CLICK`'s `677,748` was written against.

**Status of the two items.** NVG-2 and NAVHUD-1 are genuinely unblocked. A matched
side-by-side of the same TE mission from both builds is now a scripting job.

**A second oracle worth remembering:** the PO's screen recordings in
`~/Videos/`. `ffmpeg -ss <t> -i <video> -frames:v 1` pulls a reference frame
straight out of them. The frame at t=130 s of `260822_wine_ff_TE_CCIP.mp4` shows
the Wine build's bomb impact — a large fireball on the airstrip under a WEAPON
CAMERA label — which is the gold standard for BOMB-1, the defect the PO reported
as "no visible explosion" on Linux.

**Config safety.** The Wine build shares the game data directory. Measured: a run
rewrites only `config/Viper.plc`; `display.dsp` and `registry.ini` are untouched.
`config/` and `ffviper.cfg` were backed up before the first Wine launch and
restored after the last, with `diff -rq` confirming no differences remain. NVG-2 additionally
needs the D3D7 semantics for the three missing ops (`ADDSIGNED` = `a1 + a2 - 0.5`,
`ADDSMOOTH` = `a1 + a2 - a1·a2`, `DOTPRODUCT3`), which GL's texture combiners
cover directly for two of the three. Retitled from "blocked" to "ready".

---

## NVG-2 — three ops implemented, and the rest of the gap measured

The three texture operations `DX_NVG`/`DX_TV` need have exact fixed-function
equivalents, so no approximation is involved:

| D3D | semantics | GL |
|---|---|---|
| `D3DTOP_ADDSIGNED` (8) | `A1 + A2 - 0.5` | `GL_ADD_SIGNED` |
| `D3DTOP_ADDSIGNED2X` (9) | same, doubled | `GL_ADD_SIGNED` + `GL_RGB_SCALE 2` |
| `D3DTOP_DOTPRODUCT3` (24) | `4·dot(A1-½, A2-½)` | `GL_DOT3_RGB` |

Those are in, defaulting on, `FF_NO_TEXOP_FIX=1` reverts. Sources are left alone
deliberately: `D3DTSS_COLORARG1/2` arrive as their own calls and program
`SOURCE0/SOURCE1` themselves.

**New harness: `FF_TEST_NVG="<sec>[,<sec>…]"`** toggles night vision at those
times. `NVGToggle()` must run on the sim thread, so the main thread raises a flag
that `otwloop` consumes next to the existing view-mode request. Verified working:
two toggles, on the sim thread, no crash.

### Why the implementation is not yet verified

With NVG toggled on, the `[TEXOP]` trace reports **nothing** — and, decisively,
it reports nothing **with `FF_NO_TEXOP_FIX=1` as well**, which restores the old
fallback. If the ops were being requested, the reverted arm would have logged
them. They are not being requested at all, so "no unimplemented ops remain" was a
vacuous result and the new code is currently **unexercised**.

The reason is **BLUE-1, my own earlier fix**. Its comment says it outright —
*"So skip it until those operations exist"* — and it skips the only call site:

```c
if (renderer->context.NVGmode and ffDxState) TheDXEngine.SetState(DX_NVG);
```

`ffDxState` defaults to 0. `SetState(DX_NVG)` is what issues all four stages, so
with BLUE-1 in place the ops can never appear. Exercising them requires
`FF_NVG_DXSTATE=1`.

### Two more pieces the gap actually contains

Reading `dxengine.cpp:715-745`, the NVG pipeline is four stages, and the ops are
only part of it:

```
stage 0  ARG1=DIFFUSE ARG2=DIFFUSE  ADDSMOOTH
stage 1  ARG1=TEXTURE ARG2=CURRENT  ADDSIGNED
stage 2  ARG1=CURRENT ARG2=DIFFUSE  DOTPRODUCT3
stage 3  ARG1=CURRENT ARG2=TFACTOR  ADDSIGNED
```

1. **`D3DTOP_ADDSMOOTH` (11) is deliberately still unimplemented.** It is
   `A1 + A2 - A1·A2`, which is exactly `GL_INTERPOLATE` (`S0·S2 + S1·(1-S2)`)
   with `S2=A1`, `S1=A2` and **`S0` forced to white**. The only GL source that
   can carry a constant is `GL_CONSTANT` — which is also where `D3DTA_TFACTOR`
   has to live. They collide unless the per-unit `GL_TEXTURE_ENV_COLOR` is
   managed per stage. Guessing an arrangement here would produce something that
   renders but is not the D3D result, so it is left out until it can be checked
   against Wine.

2. **`D3DRENDERSTATE_TEXTUREFACTOR` is a no-op in this layer** — the case exists
   and does nothing but `break`. Stage 3 adds `D3DTA_TFACTOR`, which is the NVG
   green tint (`0x0000a000`), so at present that stage would add the default
   env colour (black) instead. Implementing the ops without this would still not
   give NVG its colour.

### Both implemented, and then actually run

`D3DRENDERSTATE_TEXTUREFACTOR` now stores the factor (ARGB, so `0x0000a000` is
green at 0.63) and `ADDSMOOTH` is implemented as `GL_INTERPOLATE` with `S2=A1`,
`S1=A2` and `S0` held at white. The white/TFACTOR collision is resolved by
setting `GL_TEXTURE_ENV_COLOR` **per unit**: a unit gets the texture factor only
if one of its args actually names `D3DTA_TFACTOR`, and ADDSMOOTH sets white on
its own unit. A stage cannot be both, and the trace says so if one ever is.

Run with `FF_NVG_DXSTATE=1` so `SetState(DX_NVG)` is issued: **no `[TEXOP]`
fallbacks**, and this time the result is meaningful because the call site really
executed. No crash, sim reached, NVG toggled.

**And the captured frame shows night vision working.** The cockpit renders in
proper NVG green — the four-stage pipeline executes and produces the right look.

**But the outside world is pure blue.** BLUE-1's exact symptom, unchanged. So
implementing the operations was necessary and not sufficient, and the assumption
written into BLUE-1's comment — that the blue screen was a consequence of the
pipeline being unexecutable — is **wrong**.

### The other half of BLUE-1, now identified

`DX_NVG` does not only set colour ops. At `dxengine.cpp:748` it also does:

```c
m_AlphaTextureStage = 3;
```

and the chroma-key setup that follows is applied to *that* stage:

```c
m_pD3DD->SetTextureStageState(m_AlphaTextureStage, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
m_pD3DD->SetTextureStageState(m_AlphaTextureStage, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
```

The chroma key in this port is an alpha test. In the 2D panel path only stage 0
has a texture bound, so alpha selected from stage 3 is meaningless, the key stops
discarding the panel's background, and its pure blue fills everything behind the
cockpit. That is the mechanism behind the PO's "screen goes blue", stated
exactly rather than inferred.

**Default is unchanged and safe:** `DX_NVG` is still skipped, so the PO sees no
difference. The colour ops are implemented and dormant until the alpha half
lands.

Completing NVG-2 now means honouring `m_AlphaTextureStage` in the compat layer's
alpha path, then re-enabling `DX_NVG` and confirming green NVG *with* a keyed
cockpit — and finally a side-by-side against Wine, which WINE-1/WINE-2 made
possible.

### NVG-3, first attempt — a negative result

The obvious fix for the surviving blue was to make the moved chroma-key stage
reach the unit that actually produces the fragment: when `ALPHAOP=SELECTARG1`
arrives for a stage whose unit has no texture bound, mirror
`COMBINE_ALPHA=REPLACE` / `SOURCE0_ALPHA=GL_TEXTURE` onto unit 0.

The mirror **fired** — `[TEXOP] alpha stage 3 has no texture; mirroring chroma
key onto unit 0` — and changed nothing. Frame mean before the attempt
`srgb(0.0015%, 23.16%, 27.21%)`, after it `srgb(0%, 23.16%, 27.21%)`, and the
capture is pixel-for-pixel the same blue.

So the mechanism is **not** simply "the alpha combiner was programmed on the
wrong unit", or the mirrored state is being overwritten before the draw — the
`COLOROP=DISABLE` arm sets `SOURCE0_ALPHA` on unit 0 itself, and `ApplyStateBlock`
re-applies stage state per polygon, so either could undo it. Reverted rather than
left in: it does not work, and a dormant non-working hack is worse than nothing.

Next attempt should establish *what actually samples the key* before changing
state again — read `GL_ALPHA_TEST`/`GL_ALPHA_TEST_REF` and the unit-0 combiner
immediately before one of the blue draws, rather than reasoning forward from the
D3D calls. What is confirmed so far: the four colour ops are right (the cockpit
renders in correct NVG green), and the blue is not caused by them.

### NVG-3, second attempt — also negative, but the blue is now identified exactly

The surviving blue measures **`srgb(0, 32, 127)`**, uniform. That number is not
arbitrary: it is pure-blue chroma key `(0, 0, 255)` run through stage 3's
`ADDSIGNED` against the NVG texture factor `(0, 0.627, 0)`:

```
R: 0.0 + 0.0   - 0.5 -> clamps to 0     ->   0
G: 0.0 + 0.627 - 0.5 -> 0.127           ->  32
B: 1.0 + 0.0   - 0.5 -> 0.5             -> 127
```

Two things follow. The blue **is** the panel's chroma key, arriving un-discarded —
not a cleared background and not a colour-op error. And `TEXTUREFACTOR` and
`ADDSIGNED` are demonstrably correct: the green term lands on 32/255 to the pixel.

Two fixes tried against that, both reverted:

1. **Mirror the key onto unit 0** when the alpha stage has no texture. No effect —
   and the reason is now clear: `DrawIndexedPrimitiveVB` *already* forces
   `COMBINE_ALPHA=REPLACE`/`SOURCE0_ALPHA=GL_TEXTURE`/`glAlphaFunc(GEQUAL, 0.5)`
   onto unit 0 at draw time. The mirror was redundant.
2. **Make a textureless alpha stage pass `GL_PREVIOUS` through** instead of
   sampling a texture it does not have. Also no effect; the corner stayed exactly
   `srgb(0,32,127)`.

And the probe (`FF_DEBUG_NVGALPHA=1`, kept) shows the draw-time gate's inputs are
**identically distributed with `DX_NVG` applied and skipped**:

```
tex0=NO  alphaTest=OFF func=0x207 ref=0.000 blend=0   SKIPPED
tex0=NO  alphaTest=on  func=0x206 ref=0.003 blend=1   SKIPPED
tex0=yes alphaTest=OFF func=0x207 ref=0.000 blend=0   SKIPPED
tex0=yes alphaTest=on  func=0x206 ref=0.500 blend=1   RUNS
```

So the gate is not what differs when the screen goes blue. (Counts are capped per
category, so this establishes which cases occur, not their volume — a real
frequency comparison would need the cap lifted.)

**Where that leaves NVG-3.** The failing surface is drawn by something the
instrumented `XYZRHW` paths do not cover, or its texture simply has no alpha=0
key pixels to discard under NVG. The next step is to identify *the draw itself*
rather than the state around it — capture the bound texture's alpha channel for
the draw that paints `(0,32,127)` (the `FF_DUMP_GLTEX` alpha reporting already
exists), which distinguishes "key present but not tested" from "key never
generated for this texture".

Two plausible mechanisms, two measurements, both refuted. Recording the refutations
because the next attempt should not re-run them.

---

## THEATER-1 — Balkans generated no missions: a suspend/resume race, fixed

**Symptom (PO, gold-standard verified):** switching theater to Balkans under
Wine works completely; on Linux the campaign loads, the clock runs, but the
frag order stays empty forever, TASK/TOT/TGT read "Not available", and Balkans
units are drawn over Korea terrain.

**Ruled out first, each by measurement:** theater path resolution (all eight
globals correct after the switch — `FF_DEBUG_THEATER=1`), mixed-case Balkans
terrain filenames (`Theater.map` opens via the case-insensitive shim), missing
campaign data (6888 objectives load from `Theaters/Balkans/campaign/save0.cam`),
and my own harness (it wasn't clicking START CAMPAIGN — fixed and validated on
Korea, where teams 2/4/5 then task with 49/4/13 squadrons).

**The differential that broke it open:** same binary, same click sequence —
Korea runs `DoCampaignLoop`; Balkans never does, *while the campaign clock still
advances* (`CurrentTime += deltatime` sits outside the gate). The suspend traces
show the mechanism exactly:

```
KOREA    thread: Got CAMP_SUSPEND_REQUEST, setting CAMP_SUSPENDED   <- ack in time
         LoadCampaign: Suspend -> already suspended
         LoadCampaign: Resume  -> clears CAMP_SUSPENDED             <- runs

BALKANS  LoadCampaign: Suspend - TIMEOUT after 1 second             <- no ack; request left set
         LoadCampaign: Resume  -> IsSuspended()==false -> NO-OP
         thread: Got CAMP_SUSPEND_REQUEST, setting CAMP_SUSPENDED   <- stale ack, after Resume
         (nothing ever clears it; the campaign thread idles forever)
```

The 1-second timeout in `CampaignClass::Suspend` is this port's own earlier
anti-hang patch. When it fires, `CAMP_SUSPEND_REQUEST` is left pending with
nobody waiting; upstream `Resume()` starts with `if (not IsSuspended()) return;`
so it cancels nothing; the campaign thread then acknowledges the stale request
and suspends permanently. Theater-dependence is pure timing: a Balkans switch
leaves the campaign thread busy loading fresh theater data at exactly the moment
Suspend waits, so the ack misses the window; Korea acks in time. Not a Balkans
bug — a race any slow load could trigger, Balkans just triggers it reliably.

**Fix (`cmpclass.cpp`, `Resume()`):** on FF_LINUX, Resume clears *both*
`CAMP_SUSPEND_REQUEST` and `CAMP_SUSPENDED` with and-not. Resume means "the
campaign should run", so a not-yet-acknowledged suspend request must not
survive it; idempotent clears mean the ack-races-Resume window ends in
"running", not "stuck". Every other Suspend/Resume pair (UI start/end in
`main_linux.cpp`) is healed by the same change.

**Verified:** identical Balkans run, fix in — `DoCampaignLoop` runs, all 8
teams have ATMs, team 2 tasks with 69 squadrons and team 6 with 23,
`missionsFilled` reaches 19/58, and the screenshot shows a populated frag order
(BAI/Escort/Strat Bomb/Strike, one flight Ingress), TASK "BAI — west of
Podgorica", a drawn flight plan, and Balkans event text (Kukes). That was the
PO's headline complaint.

**Still open (split to THEATER-2):** the campaign map imagery/labels are still
Korea's ("EAST SEA" overlay, Korea minimap, stale menu background) — UI art and
map resources loaded once are not re-read after a theater switch. Cosmetic-to-
serious (unit icons sit on wrong-looking terrain) but functionally the campaign
now runs. Also noted in passing: inactive teams print garbage `missionsFilled`
values (uninitialised fields, harmless today); and the Balkans `artdir` tree is
nested one level deeper than its .tdf declares (`art/art/`), which Windows
tolerates via recursive resource attach — check `ResAddPath(..., recurse)` on
Linux before trusting theater-specific art.

---

## TERRAIN-Z, sprint 1 — features baked at transient ground height; convergent re-snap

The PO's epic: takeoff, landing and bombing must match the Wine gold standard,
solving "the physics engine terrain seems to be a few meters below the graphics
engine terrain".

### The measurement chain, including two falsified fixes

**Correction (same session):** these runs were labelled TE-02/Korea but actually
ran in the **Balkans** theater — `curTheater` had been left on Balkans by the
THEATER-1 work, and the harness loaded "002 Eurofighter training flight" (the
screenshot even shows Eurofighters, which should have given it away). The
mechanism and fix are theater-independent and the verification stands; only the
theater label was wrong. Korea's own runway numbers from ELEV-1 (−26 ft, gear
5.99) differ from the −14 below, which is a Balkans airbase.

Parked on the runway, all three numbers at one spot (z negative = up):

| what | z (negative = up) |
|---|---|
| terrain posts, lod 0, at the parked aircraft | **−14.0 ft** |
| two platform-child drawables, elsewhere on the field | −3.0 ft |
| parked player (physics ground + gear) | −20.4 ft |

**Correction (2026-08-24), and it matters.** The middle row was originally
written up as "features sit 11 ft below the terrain around them — the PO's few
metres, measured". **That subtraction is invalid.** The −3.0 drawables are at
`(2067027,34439)` and `(2061970,29379)`; the −14.0 ground reading is at
`(2068116,28081)` — different places on a field whose elevation varies. Same
error in kind as the retracted "model 2647" pixel comparison earlier in this
project: two numbers from two locations subtracted as though they shared one.

RWY-2 independently contradicts the 11 ft story too: it *measured* the flat
runway surfaces as **coplanar** with the terrain mesh — they z-fight it at
approach range, which is why a slope-scaled polygon offset was added. Coplanar
surfaces are not 11 ft below anything.

**What survives, and it is what the fix rests on:** the same query at the same
place answers `0.00 at lod=5` during streaming and `−14.00 at lod=0` once fine
terrain arrives. The LOD is the signal, not a cross-location subtraction. Feature
Wake() sampling inside that window bakes 0; that is real, and the re-snap
addresses exactly it. The size of any residual visible offset is **not**
established by this section and should not be quoted from it.

Getting from that number to the mechanism took three instrumented runs, each of
which killed the previous theory:

1. *"Features bake the coarse-LOD transient at Wake"* — partially wrong: a
   re-check one second later returned the SAME value for all 500 features
   (`moved=0 settled=500`), so the wake answer looked stable.
2. *"Wake's SetPosition never reaches the drawable"* — the drawable IS created
   from the entity's z, and syncing it at Wake changed nothing.
3. The creation/wake probes then pinned it: the drawable is created (asleep) at
   z=0, Wake's `GetApproxGroundLevel` **returns 0.00 — the streaming transient**
   — and my re-snap's "settled when two answers agree" test was satisfied *by
   the transient itself*: at one second in, the terrain has still not streamed,
   so "0.00 twice" looked like convergence. **Two equal answers can be the
   transient twice.**

### The fix

`SimFeatureClass::Wake()` still snaps as before, but every woken feature is
queued for **convergent re-snapping** (`FF_QueueFeatureResnap`), serviced about
once a second from the sim loop. The settle test is not value stability but
**answer provenance**: an entry is only retired when `GetGroundLevel` answers at
`lod <= 1` (fine terrain streamed in at that spot). Each re-snap moves the entity
*and* its drawable (statics have no per-frame draw sync). Entries whose entity
vanishes (bubble shrink) are dropped and simply re-queue on their next Wake.
`FF_NO_FEATURE_RESNAP=1` reverts; `FF_DEBUG_RESNAP=1` reports.

Verified by counter: first service tick after sim entry **moved all 500 queued
features** off the transient; 233 near the viewer settled at fine LOD
immediately; the remainder left the bubble and will re-queue when re-woken.
Player physics unchanged and correct (gear height above lod-0 ground).

### Still open in this epic

- The **aircraft spawn** uses the same transient (`[SPAWN] groundZ@pos=0.00` at
  placement, real ground −14) — physics re-settles it, but the first seconds and
  any takeoff roll started inside them are wrong. Same fix shape applies.
- The residual **−3.0 vs 0.0** discrepancy: drawables were created at 0.00 yet
  drew at −3.0, so something applies a −3 offset after creation. Unidentified;
  small next to the 11 ft, but it will still be there after the re-snap.
- The PO's acceptance test: takeoff, landing, bombing side-by-side against Wine.

---

## THEATER-2 — stale Korea imagery after a theater switch: found and fixed

The residual from THEATER-1: with the Balkans campaign running correctly, every
piece of UI imagery — menu background, campaign map, minimap, map labels —
stayed Korea's.

**Root cause: an earlier port fix with a baked layout assumption.**
`C_Resmgr::OpenResFile` (cresmgr.cpp) strips a leading `art/` from resource
names whenever `FalconUIArtThrDirectory` ends in `/art`:

- Korea: artdir `<data>/art` IS the art level — stripped name
  `<data>/art/resource/main.idx` exists. Works.
- Balkans: artdir `Theaters/Balkans/art` *also* ends in `/art`, but its tree
  keeps the `art/` level inside (`art/art/resource/...`). The stripped open
  fails **silently** and falls through to `FalconUIArtDirectory` — Korea's art.

So the theater's own `mainbg.rsc`, `campmap.rsc`, `intel.rsc` etc. (all shipped
by Balkans) were never even attempted at their real paths. On Windows this works
because the resource manager registers the artdir tree recursively and resolves
by basename; the Linux path goes through this direct-open helper instead.

**Fix:** try the *unstripped* name against the theater dir first (Balkans
layout), then the stripped one (Korea layout), then the base-art fallback as
before. Verified visually: the game now boots into the Balkans theater with the
Balkans main-menu background (F-16 at Aviano under the Alps) where Korea's art
used to persist.

Remaining for the PO to confirm in play: campaign map imagery and the strategic
map labels, which ship in the same `.rsc` family and should follow.

---

## NVG-3, third pass — the surviving blue is an alpha-0 key-blue quad

The vertex-colour probe (extension of `FF_DEBUG_NVGALPHA`) caught it exactly:

```
12x  KEY-BLUE untextured XYZRHW draw: n=24  diffuse=000000ff  alphaTest=0
```

An **untextured** 24-index quad whose every sampled vertex is diffuse ARGB
`000000ff` — pure chroma-key blue with **vertex alpha 0** — drawn with the alpha
test disabled. The NVG frame corner still measures `srgb(0,32,127)`, which is
exactly this quad's blue after stage 3's ADDSIGNED against the green texture
factor. This is the 2D pit's panel-background fill.

Why it shows on GL and not on D3D: the quad is meant to be invisible via its
alpha (0). In our path nothing ever consumes that alpha —

- the chroma-key combine sources `ALPHA` from `GL_TEXTURE`, and sampling an
  *unbound* texture returns alpha = 1;
- with `GL_BLEND` off and the alpha test off, the RGB paints opaque regardless.

D3D7's fixed function disables a stage that references an unbound texture and
falls back so fragment alpha comes from **diffuse** — 0 — which the pipeline
then discards (keyed or blended away).

**Next step (precise):** make the untextured-draw path source alpha from
`GL_PRIMARY_COLOR` when no texture is bound (matching D3D's stage fallback), and
check which of blend/alpha-test D3D uses to discard it — the existing
null-texture check in `DrawIndexedPrimitiveVB` already disables `GL_TEXTURE_2D`,
so the remaining gap is only that neither blending nor testing is active for
this draw on GL. Verify by the corner pixel: `(0,32,127)` must leave, the NVG
cockpit must stay green, and the normal (non-NVG) pit must not regress —
that same quad is drawn outside NVG too and is currently covered by other
geometry rather than discarded.

---

## TERRAIN-Z sprint 2 — automated bombing works end to end; visual framing still manual

The bombing acceptance test can now run without a human. On Korea TE row 20
("Bombs with CCIP"), with `FF_TEST_BOMB=20 FF_TEST_SUBMODE=ccip`, a bomb was
released through the production path and:

```
[MSLEND] Process: endCode=11 ... groundType=7 type=2 (BOMB)
[MSLEND] BombImpact legacy branch: DamageType=2 BlastRadius=293
[MSLEND] spawning SFX_GROUND_EXPLOSION at (1724929,1204013,-1697)
```

Release → fall → ground impact at real terrain elevation (−1697 ft, lod-0
data, not the transient) → **BombImpact fires → SFX_GROUND_EXPLOSION spawns**.
That is the BOMB-1 fix and the TERRAIN-Z elevation chain working together on the
full production path, reproducible in one command. Two independent runs, same
result.

Caveats, stated plainly:

* `FCC->GetTheBomb()` still reads NULL in the harness trace while the release
  demonstrably happens — the release path instantiates the weapon object as part
  of its own sequence, so HARNESS-1's remaining gap is *only* in the pre-release
  introspection, not in the release. The KNOWN-INCOMPLETE note in doweapon.cpp
  stands corrected to that narrower claim.
* The weapon-camera capture (`FF_VIEW_SCRIPT "5@88;s@95..."`) produced clean
  in-sim frames (forest under the weapon camera) but did not frame the fireball
  itself — the shots landed seconds after the ~2s effect. Getting the fireball
  into an automated frame needs impact-triggered capture rather than scripted
  times; noted as a harness improvement, not a defect.
* **PO acceptance remains the decisive test** for the epic: fly takeoff, landing
  and a bombing pass and compare against the Wine gold standard
  (`260822_wine_ff_TE_CCIP.mp4` t≈130 s for the fireball reference). The feature
  re-snap fix means the airbase deck now coincides with the terrain, so the
  previous "bomb swallowed by the airstrip / explosion hidden under the slab"
  mechanism should be gone.

### Session state at handoff

Committed and pushed this stretch: THEATER-1 (campaign suspend/resume race —
Balkans generates missions), MISSEVAL-1 (aircraft_name overflow abort),
TERRAIN-Z-1 (feature ground re-snap), THEATER-2 (theater art resolution),
NVG-3 probes (blue pinned to the alpha-0 key-blue quad, fix path documented).
`curTheater` is restored to **Balkans** (the PO's last manual choice). Open:
NVG-3 fix, NAVHUD-1 (needs Wine HUD comparison), TERRAIN-Z PO acceptance, the
spawn transient (self-correcting, deliberately left), and the −3 ft residual
feature offset.

---

## NAVHUD-1 closed — the precedence bug is real but unreachable, no Wine run needed

NAVHUD-1 was parked on "needs a Wine HUD comparison": at `navhud.cpp` 945/1004/
1327/1519, `and` binds tighter than `or`, so `g_bRealisticAvionics and g_bINS`
gates only the first disjunct and `not ownship->INSState(INS_HUD_STUFF)` can
move the heading tape on its own. The open question was whether that changes
what the HUD draws with realistic avionics off.

It cannot, and following the flag settles it without flying anything:

* `INS_HUD_STUFF` is only ever set or cleared inside `AircraftClass::RunINS()`.
* `RunINS()` is called from exactly one place, `aircraft.cpp:1710`, under
  `if (g_bINS and not isDigital)`.
* `g_bINS` is a compile-time `true` (`f4config.cpp:284`) with no assignment
  anywhere else, and `INSFlags` initialises to 0.

So `INSState(INS_HUD_STUFF)` is driven purely by INS alignment status, entirely
independently of `SimAvionicsType`/`g_bRealisticAvionics`. Turning realistic
avionics off (difficulty `ATEasy`) does not stop the INS from running or from
maintaining that flag — it only stops *other* avionics code paths consulting it.
The ungated disjunct therefore evaluates the same INS state the gated one would,
and the grouping cannot change what is drawn.

**Left unchanged deliberately.** The parse is wrong but the behaviour is not, and
"fix it to match intent" would be a speculative change to HUD drawing with no
observable defect behind it — exactly the kind of change this project has been
burned by. Recorded here so the next `-Wparentheses` sweep does not re-open it.

Cost of closing it this way: three greps. Cost of the Wine comparison it was
waiting for: a driven side-by-side flight in two builds.

### NVG-3, fourth pass — the alpha theory is dead; the blue is a *backdrop*, not a leak

The vertex-alpha fix (source alpha from `GL_PRIMARY_COLOR` when untextured,
matching D3D's unbound-stage fallback) was implemented and **had no effect**.
The probe says why, and it kills the whole theory:

```
KEY-BLUE untextured XYZRHW draw: n=24 diffuse=000000ff alphaTest=0 blend=0
```

`alphaTest=0` **and** `blend=0`. Nothing in the pipeline consumes fragment alpha
for this draw, so it cannot matter what alpha we compute — zero or one, the RGB
paints opaque either way. Both `D3DRENDERSTATE_ALPHATESTENABLE` and
`ALPHABLENDENABLE` *are* implemented in the layer, so this is the application's
genuine intent, not a missing state. The fix was reverted rather than left in:
it is semantically right but observably dead, and it would have changed alpha for
every untextured diffuse draw in the game — real regression surface for zero
measured benefit.

**What this reframes.** The key-blue quad is not "leaking through a broken key" —
it is an opaque **backdrop** that the pit is *supposed* to draw over. It is drawn
in normal (non-NVG) mode too, where we never see it, because the cockpit panel
covers it. So the defect is not the backdrop appearing; it is **the covering draw
failing in NVG mode**. Every theory so far (alpha stage, chroma key, alpha
source) has been aimed at the wrong object.

**Next step:** identify the draw that covers this backdrop in non-NVG mode and
find why it does not cover it under `DX_NVG` — compare the same frame's draw
list with `FF_NVG_DXSTATE=1` and without, looking for a draw that is present in
one and absent (or degenerate) in the other. The instrument for that is a
per-draw dump keyed on the frame after the NVG toggle, not more state probing.

Four theories, four measurements, four refutations — but the object under
investigation is now the right one.

---

## TESWEEP-2 — full regression sweep after this session's changes: 34/34 clean

This session changed campaign threading (THEATER-1), a campaign UI struct
(MISSEVAL-1), sim feature placement plus a new per-second sim-loop service
(TERRAIN-Z), and UI resource path resolution (THEATER-2). All 34 stock TE
missions were re-run on `build-relg` to check none of that regressed.

**Result: 34 of 34 load, reach the sim and exit clean. Zero crashes, zero
aborts.** (Worth noting explicitly given MISSEVAL-1 was a `_FORTIFY_SOURCE`
abort in the TE mission-select path — the sweep exercises exactly that path 34
times.)

### Assertion inventory vs the TESWEEP-1 baseline

| | TESWEEP-1 | TESWEEP-2 |
|---|---|---|
| assertion lines | 68 | **64** |
| `handoff.cpp:86` | 10 lines (5 missions) | **0** |
| `seeker.cpp:350` | 2 lines (Offensive BFM) | **0** |
| `atm.cpp` | 792 (×10) | 835 (×10) |

The two sites this session set out to remove are gone, confirmed per mission:

* **`handoff.cpp:86` — 0 occurrences across all 34 logs.** Mission 17 is the
  cleanest single proof: baseline `3663 ×2` + `86 ×2`, now `3663 ×2` alone.
  Mission 22 dropped 6 lines → 4 the same way. That is HANDOFF-2.
* **`seeker.cpp:350` gone**, and Offensive BFM (its only baseline source) now
  reports **0** assertions, with Defensive and Head-on BFM at 0 as well. That is
  SEEKER-1.

`atm.cpp` moved from line 792 to 835 because the file gained the FF_DEBUG_ATM
gate trace — same rating-vs-scores assertion, shifted, not a new one.

### Three sites not in the baseline — measured, not mine

`tviewpnt.cpp:358/359` (ground-type post bounds) and `drawbsp.cpp:180`
(`AttachChild` slot number) appear here and did not in TESWEEP-1. Two of them sit
in terrain-query code that TERRAIN-Z now calls once a second, so they were the
obvious suspects. **Tested rather than assumed:** the same mission with
`FF_NO_FEATURE_RESNAP=1` produces the *identical* site set, and 358/359 did not
reproduce at all on a re-run. They are run-to-run variation in which sites a
stochastic sim reaches — exactly the one-bit-per-site behaviour TEXBANK-1
established. TERRAIN-Z is not implicated.

### Harness note

The sweep's row→y mapping was off by one on the first batch (row 1 loaded
mission "02"). Corrected to `y = 94 + N*17` (verified: mission 22 → y 468) and
mission 01 was run separately, so the coverage is a genuine 34, not 33 with a
mislabelled edge. `scratchpad/te-sweep.sh` forces the Korea theater first,
because TE row coordinates index the *current* theater's mission list and
`curTheater` persists across runs — a trap that produced two wasted measurements
earlier in this session.

### NVG-3, fifth pass — the structural finding: four D3D stages collapse onto one GL unit

`FF_DEBUG_2DCENSUS` (three revisions — the first two saturated their signature
cap and would have shown truncation as absence) finally produced a discriminating
comparison of the same frame with and without `FF_NVG_DXSTATE=1`.

**Draw classes are byte-for-byte identical between the two modes:**

```
units=0 aFunc=0x207 aRef=0.500  n=10   tex=NO  diffuse=000000ff aTest=0 blend=0 zTest=0
units=0 aFunc=0x207 aRef=0.500  n=24   tex=NO  diffuse=000000ff aTest=0 blend=0 zTest=0
units=1 aFunc=0x206 aRef=0.003  n=6    tex=yes diffuse=00000000 aTest=1 blend=1 zTest=1
units=1 aFunc=0x206 aRef=0.500  n=264  tex=yes diffuse=00000000 aTest=1 blend=1 zTest=1
units=1 aFunc=0x207 aRef=0.000  n=6    tex=yes diffuse=00000000 aTest=0 blend=0 zTest=1
```

So the covering-draw theory fails as a presence question: nothing is missing, and
no alpha/blend/depth state differs. What differs is only the combine applied.

**`units=1` looked like the structural finding — and the generalisation was
wrong.** It was measured only on `isXYZRHW` (2D) draws, then written up as "this
layer only ever enables one GL texture unit". The NVG world pipeline runs on *3D*
draws, which that census never sampled. Censusing those too gives the opposite
answer:

| | 3D draws | 2D (XYZRHW) draws |
|---|---|---|
| `FF_NVG_DXSTATE=1` | **units=3**, stage 0 *and* stage 1 textured | units=1 |
| DX_NVG skipped | units=1 | units=1 |

So the layer supports multi-unit rendering perfectly well, and DX_NVG genuinely
drives a multi-stage chain — but **only for the 3D world**. Three units appear
with NVG on and never with it off, so that half is working.

**The real gap is the 2D pit path.** Pre-transformed draws stay on a single unit
in both modes, so the pit panel is rendered through stage 0 alone — and stage 0
under DX_NVG is `ADDSMOOTH(DIFFUSE, DIFFUSE)`, which never samples a texture.
That is a far narrower target than "the layer cannot do multitexture", which is
what the previous entry claimed.

**Positive confirmation that the NVG-2 ops do run**, from the pixel:

| stage-0 op | corner pixel |
|---|---|
| `ADDSMOOTH` implemented (default) | `srgb(0, 32, 127)` — key blue through ADDSIGNED + green factor |
| `FF_NO_TEXOP_FIX=1` → `MODULATE` | `srgb(0, 0, 255)` — raw key blue |

The transform is visible in the output, so `ADDSIGNED` and `TEXTUREFACTOR` are
genuinely executing. That is worth having: NVG-2's implementations are verified
live, not just compiled.

**Also refuted:** the hypothesis that stage-0 ADDSMOOTH starves the panel of its
texture (both its args are DIFFUSE; the texture only arrives at stage 1). Forcing
stage 0 back to MODULATE — which *does* sample the texture — leaves the backdrop
just as visible. So the panel's failure to cover is not explained by the stage-0
op either.

**Standing tally for NVG-3: five theories, five measurements, five refutations** —
alpha stage, chroma key, alpha source, missing covering draw, and stage-0
starvation. Each was killed by the run meant to confirm it. What is now known
precisely: the blue is an untextured alpha-0 key-blue quad with no alpha test or
blend; the draw list and per-draw state are identical in both modes; the NVG ops
execute and transform pixels; and the four-stage pipeline is collapsed to one
texture unit.

**The next attempt should start from multi-unit support**, not from another
single-stage theory: bind the same texture to the units that reference
`D3DTA_TEXTURE` and enable the stages DX_NVG configures, so the chain can
actually run. That is a substantial piece of compat-layer work and should be
scoped as its own item rather than bolted onto NVG-3.

`DX_NVG` remains skipped by default (BLUE-1), so none of this reaches the PO.

---

## ASAN-2 — campaign flight soak against this session's changes: clean

The ASAN build was a day stale while this session changed campaign threading
(THEATER-1's `Resume` now clears two flags), added a **mutex-protected
cross-thread queue** serviced from the sim loop (TERRAIN-Z's feature re-snap),
enlarged a struct and rewrote its copies (MISSEVAL-1), and changed a UI resource
path helper (THEATER-2). Rebuilt (0 errors) and soaked.

**Result: 0 AddressSanitizer errors over a 420 s campaign flight, and the run
reached 3D** (`RunningGraphics` present, `rc=124` = ran to its timeout). Both
halves matter — this script exists precisely because Instant Action and TE-02
reported clean for a whole session while nine memory errors sat in the campaign
path neither could reach. A clean soak that never enters the sim proves nothing.

**Confirmed exercised, not just clean.** A second soak with
`FF_DEBUG_RESNAP=1` shows the new queue actually running under ASAN, at campaign
scale:

```
[RESNAP] moved=1208 settled=231 dropped=0 pending=987
[RESNAP] moved=0    settled=0   dropped=987 pending=0
```

1208 features re-snapped off the streaming transient in one campaign flight
(TE-02 moved 500), 231 settling at fine LOD immediately and the rest dropping out
as they left the bubble — the designed behaviour. 0 ASAN errors on that run too.
Without this, "0 errors" would not have distinguished *clean* from *never ran*.

**Broadened to three paths**, since the build had been stale a full day and one
soak covers one path:

| soak | ASAN errors | reached 3D |
|---|---|---|
| campaign flight (420 s) | 0 | yes |
| dogfight UI path (200 s) | 0 | yes |
| Instant Action (200 s) | 0 | yes |

All three ran to their timeouts (`rc=124`) rather than exiting early.

**What this does and does not cover.** ASAN checks memory safety, not data
races: it does *not* validate the re-snap queue's locking. That queue is written
from the sim thread (`Wake`) and drained from the sim loop, with a
`std::mutex` held across both, but "no ASAN findings" is not evidence the
locking is right — only ThreadSanitizer would speak to that. Recorded as a known
gap rather than implied coverage.

### NVG-4 scoped, and where it blocks

Comparing the two state setups makes the mechanism exact. `DX_OTW` explicitly
disables the extra stages; `DX_NVG` configures them:

```c
DX_OTW:  stage 0 MODULATE(TEXTURE, DIFFUSE);  stages 1,2,3 COLOROP = DISABLE
DX_NVG:  stage 0 ADDSMOOTH(DIFFUSE, DIFFUSE); stage 1 ADDSIGNED(TEXTURE, CURRENT)
         stage 2 DOTPRODUCT3(CURRENT, DIFFUSE); stage 3 ADDSIGNED(CURRENT, TFACTOR)
```

For **3D** draws the app binds a texture at stage 1 (terrain multitexture), so
our `SetTexture(1, …)` enables that GL unit — hence `units=3`. For **2D pit**
draws nothing is ever bound at stage 1, so the unit stays disabled even though
the stage carries a live COLOROP. In D3D a stage is active by virtue of its
COLOROP, texture or no texture; in this layer a unit is active only if a texture
was bound to it. That difference is the whole of NVG-4.

**Where it blocks, and why I am not guessing past it.** Making the 2D path
enable those units requires deciding what `D3DTA_TEXTURE` yields on a stage with
no texture bound. D3D7 documents this as undefined, so any choice — white, black,
the stage-0 texture, opaque — is a guess, and it feeds straight into
`ADDSIGNED(TEXTURE, CURRENT)` where each choice produces a visibly different
image (white gives `current + 0.5`, i.e. a wash, not identity). Picking one and
shipping it is precisely the speculative-rendering-change pattern this project
has had to revert repeatedly.

**The oracle settles it.** WINE-1/WINE-2 established that the Windows build can
be driven and captured here. A single Wine NVG frame answers what the pipeline is
supposed to produce, and the choice follows from that rather than from a guess.
That is the next step for NVG-4, and it needs the Wine build driven into the sim
with NVG toggled — a larger automation job than the menu capture already done.

`DX_NVG` stays skipped by default meanwhile, so none of this reaches the PO.

### TERRAIN-Z — how far the transient family actually reaches: one member

The transient is a *class* of bug (sample ground height while terrain is still
streaming, bake the 0), so the obvious follow-up was to find its other victims.
Auditing every `GetGroundLevel`/`GetApproxGroundLevel` caller that runs at
creation time:

| caller | pattern | exposed? |
|---|---|---|
| `SimFeatureClass::Wake` | snap once, never again | **yes — fixed by the re-snap queue** |
| `GroundClass::Init` | snap at init… | no |
| `GroundClass::Exec` | …but re-queries **every frame** (grndmain.cpp 416, 592) | self-correcting |
| `AirframeClass::Init` | snap at placement… | no |
| aircraft physics | …re-settles within seconds | self-correcting |
| `HeloClass`, `gndhndl` | init-time clamps, then per-frame physics | self-correcting |

**Every mover re-queries; only statics bake.** Statics have no per-frame Exec —
that is exactly why they needed a queue rather than a one-line fix. So the family
has one member and it is closed, rather than the open-ended hunt the pattern
first suggested.

Recorded because the natural instinct after finding a bug class is to assume more
instances exist; here the audit says otherwise, and that is worth knowing before
someone spends a sprint looking.

---

## TERRAIN-Z — the PO's sea-level experiment: no constant offset, the gap is slope-dependent

The epic was framed as "the physics engine terrain seems to be a few meters below
the graphics engine terrain". The PO designed the experiment that settles it:
**use water as the reference**, because sea level is a known truth of exactly 0 —
unlike land, where neither model can be checked against anything absolute.

Two flights, instrumented with `FF_DEBUG_GROUND`:

| terrain | `vpAccurate` vs `vpApprox` | visible behaviour |
|---|---|---|
| **water** (flat, truth = 0) | **0.00 ft — every one of 23 samples** | aircraft flown into the sea explodes **at the surface** |
| **land** (sloped) | mean **7.0 ft**, max **59.7 ft**, 10% of samples >20 ft | bomb sinks in, pause, explosion heard but not seen |

`vpAccurate` is the interpolated triangle height physics uses; `vpApprox` is the
nearest-post height. Over water both read 0.00 and agree with the known truth.

**This kills the constant-offset theory.** If the physics model sat uniformly
below the graphics model, water would show the same offset — and water is exact
to the last decimal. The disagreement appears *only* where terrain has relief.
That is the signature of interpolation across post spacing (a triangle mid-face
sits below the posts that bound it on a ridge, above them in a bowl), not a
systematic z error in either model.

It also explains the PO's original symptom without any appeal to a global offset:
a bomb landing on sloped ground detonates at the interpolated height while the
eye sees the drawn mesh, so the fireball spawns inside the hillside. On flat
water there is nothing to interpolate, so the explosion is visible — which is
exactly what the PO observed.

**Corrections this supersedes.** The retracted "11 ft" figure earlier in this file
was a cross-location subtraction and should be ignored; these numbers are
same-point, same-sample comparisons from live flights. And the feature re-snap
(TERRAIN-Z sprint 1) fixed a real but *different* bug — statics baking the
streaming transient — which is why the PO's bombing symptom was unchanged by it.
Both facts were established by the PO flying, not by inference.

**Where this points next.** The remaining question is which height the *renderer*
draws at a given point versus which the physics query returns, at the same LOD.
`FF_DEBUG_LODZ` is in place at bomb impact for exactly that, and has not yet
caught an impact. That measurement, on sloped ground, should size the gap the PO
sees.

### TERRAIN-Z — the airstrip case: runway surfaces are drawn at a fixed sea-level height

The PO pushed back on the slope-dependent conclusion with a case it cannot
explain: on the landing TE the aircraft sits half-submerged in an airstrip that
is **flat and barely above sea level**. Measuring that case directly (TE 02
ground start, `FF_DEBUG_GROUND` + `FF_DEBUG_RUNWAY`):

```
aircraft   acZ = -31.99   groundZ = -26.00   aboveGround = 5.99 (gear)
           vpAccurate = vpApprox = -26.00     <- terrain query agrees with itself
runway drawables:  z = -3.0   at EVERY position sampled
distinct z across ALL runway drawables in the mission:  -3.0
```

**One constant value for every runway piece at every location.** Pieces 1,559 ft
from the player report the same −3.0 as pieces at another airbase 90,000 ft away.
That is not terrain-following placement; it is a fixed height near sea level.

**This supersedes the slope-dependent conclusion for the airstrip case.** The
`vpAccurate`/`vpApprox` spread measured earlier is terrain-vs-terrain and says
nothing about objects drawn *on* terrain. The airstrip never samples terrain at
all, which is why a flat field shows the fault just as strongly as a hillside —
exactly the PO's objection, and it was right.

**It also predicts the magnitude.** With the surface pinned near sea level, the
error simply *is* the field elevation: ~23 ft here (terrain −26 vs drawable −3),
and the 2–3 m the PO reports on a Balkans strip that is barely above the sea.

**Prime suspect**, consistent with the constant: `DrawablePlatform`'s constructor
sets `position.z = 0.0f` ("to ensure we're in the lowest object list" — a sort
key, not a placement), and `InsertStaticSurface` recomputes `position.x` and
`position.y` from its children's extents while never touching `z`. Nothing in
that class ever assigns a real elevation.

**Why Wine is unaffected** is now a sharp question rather than a vague one: same
terrain files, same airbase, same platform data — so the divergence is in how
this port places or draws those surfaces, with a working oracle beside it. The
next step is the same ground start captured in both builds.

### TERRAIN-Z — RETRACTION of the "constant sea-level runway" finding, and why the decal stays

**The previous entry's headline finding is wrong and is retracted.** I reported
that every runway drawable sits at a constant `z = -3.0` regardless of position,
and blamed `DrawablePlatform`'s `position.z = 0.0f` sort key. Both halves are
false.

**What `-3.0` actually was.** `drawbldg.cpp` computes `position.z = gl - decal`
for flat surfaces, with `decal = FF_RunwayDecal() = 3.0`. So `z = -3.0` means
`gl = 0.0` — the ground query answering zero, not a sea-level placement. Running
TE-02 again with the full trace:

```
[RUNWAY] flat GetGroundLevel=-26.0 approx=-26.0 delta=0.0 decal=3.0 -> z=-29.0 pos=(1043420,1270614)
[RUNWAY] flat GetGroundLevel=-26.0 approx=-26.0 delta=0.0 decal=3.0 -> z=-29.0 pos=(1044231,1270905)
[RUNWAY] flat GetGroundLevel=  0.0 approx=  0.0 delta=0.0 decal=3.0 -> z= -3.0 pos=(1087165,1362720)
```

The player's airbase reads `-26.0 -> z=-29.0`. Only the **far** airbase, ~90,000 ft
away and outside streamed terrain, reads `gl=0 -> z=-3.0`. My earlier probe had
sampled only those far-field rows and I read their shared value as a global
constant. Same error class as the retracted "11 ft" figure: a conclusion drawn
from an unrepresentative subset.

**`DrawablePlatform` was never the placement path.** It is a *container* ("large
flat objects which can lie beneath other objects"), holding `flatStaticObjects` /
`tallStaticObjects` / `dynamicObjects`. Its `position.z = 0` is a display-list
sort key that never reaches the screen. The drawn surface is the child
`DrawableBuilding`, which under `FF_LINUX` re-fetches the **accurate** ground
level *every frame*. Runway surfaces are terrain-following, contrary to what I
reported.

**What actually differs from Wine** is a three-part compensation stack, every
part `#ifdef FF_LINUX`, i.e. absent from the Windows/Wine build:
1. `drawbldg.cpp` — flat surfaces drawn `FF_RunwayDecal()` = **3 ft** above terrain
   so they win the depth test against the terrain mesh;
2. `otwdrive.cpp` — the aircraft *drawable* lifted the same 3 ft (visual only;
   `ZPos()` untouched) because aircraft are *placed* with wheels at terrain height;
3. `otwdrive.cpp` — a further empirical **2 ft** `FF_GEAR_LIFT`.

That stack, not a placement bug, is the answer to "why Linux and not Wine".

**Tested the retirement condition the code names, and it fails.** The comment at
`drawbldg.cpp:121` says the workaround is obsolete if the coarse approximation
ever agrees with the accurate value. It now does — `delta=0.0` on **every**
sample at the player's airfield. But disabling the stack
(`FF_RUNWAY_ZLIFT=0 FF_NO_GEAR_LIFT=1`) makes TE-02 **worse**, not equal to Wine:
the tarmac disappears (grass in the foreground) and the jet sinks with its gear
hidden. Side-by-side orbit captures vs the Wine gold standard confirm the ON
configuration is the closer of the two.

**So the prediction in that comment is falsified**: the decal's job is winning the
depth test, not correcting placement, and fixing placement does not retire it.
`delta=0` is necessary but not sufficient. The stack stays until the depth-test
problem is solved on its own terms.

Gold standard for this comparison: Wine, Korea, TE-02, orbit view — captured
under Wayland via XSendEvent input + gpu-screen-recorder (see the session memory
note; XTEST and X11 grabs both fail silently here).

**Still open**: the PO's half-submerged report was on the **landing** TE, not
takeoff. TE-02 parked geometry now looks close to Wine, so the landing case is
the next measurement, not a re-run of this one.

### TERRAIN-Z — the landing airfield: coarse and accurate ground level disagree by up to 9.5 ft

TE-02 (takeoff) showed `delta = 0.0` between `GetGroundLevel` (accurate,
interpolated) and `GetGroundLevelApproximation` (coarse, nearest post) on every
sample. **TE-09 "Landing Final Approach" does not.** Over 2230 flat-surface
samples:

```
delta   count        delta   count
 0.0     1478         6.3      48
 2.2       50         7.6      48
 1.6       49         8.1      48
 0.5       49         9.5      48
 5.1       48         4.9      44
```

The non-zero rows cluster at a *second* airfield (pos ~779000,1308000), distinct
from the 772-773k cluster which reads a clean `-26.0 / -26.0 / delta=0.0`:

```
GetGroundLevel=-16.4  approx=-26.0  delta=9.5  -> z=-19.4  pos=(780494,1310476)
GetGroundLevel=-17.8  approx=-26.0  delta=8.1  -> z=-20.8  pos=(780036,1305443)
GetGroundLevel=-20.8  approx=-26.0  delta=5.1  -> z=-23.8  pos=(779057,1308036)
GetGroundLevel=-25.3  approx=-26.0  delta=0.6  -> z=-28.3  pos=(777575,1311983)
```

**`approx` is pinned at exactly `-26.0` across the whole field while the accurate
value ranges `-16.4 .. -25.3`.** Negative z is up, so the accurate surface sits up
to 9.5 ft *below* the coarse one. A constant coarse value over a wide area is what
a flattened airbase plateau looks like; the accurate interpolation is not flat
there.

**Magnitude matches the PO's report.** 9.5 ft = 2.9 m, against a reported "2-3 m",
on the landing TE where the report was made — while the takeoff TE, which reads
delta=0.0 throughout, looks close to Wine in side-by-side orbit captures. The
symptom tracks the disagreement, not the theater or the aircraft.

Runway surfaces are drawn at `gl - decal`, i.e. against the **accurate** value, so
at this field the drawn strip is warped across a ~9 ft range instead of sitting on
one plane -- on an airstrip that should be flat.

**Not yet established**, and deliberately not claimed: which of the two heights
the *rendered terrain mesh* uses, and therefore whether the visible gap is the
runway following the accurate surface while the mesh draws the coarse one. That
needs either a mesh-height probe at a fixed point or an observed touchdown.

**Blocked on the PO for the touchdown case**: TE-09 starts airborne and the
aircraft does not land itself, so the half-submerged-on-landing geometry cannot
be captured by the scripted harness. Four scripted approach frames show the jet
still airborne over water at t=78..108s.

### NVG-4 unblocked: the Wine NVG reference finally exists — and it rescopes the ticket

NVG-4 was blocked on one thing: *"it needs the Wine build driven into the sim with
NVG toggled — a larger automation job than the menu capture already done."* That
automation now exists (see the Wayland driving note: XSendEvent input,
gpu-screen-recorder capture, window repositioning after the 3D transition), so the
oracle frame was captured instead of guessed.

NVG toggle is **N** — `keystrokes.key` binds `ToggleNVGMode` to `0x31`, which is
the DirectInput scancode `DIK_N`, not the ASCII '1'.

**What Wine actually produces** (Korea, TE-02, 2D pit, NVG on): the whole frame is
green — terrain, sky, cockpit panels, MFDs and instruments alike — with **full
detail preserved**. Frame mean drops 0.319 -> 0.088 on toggle.

**This kills the wash-out branch outright.** The open design question was what
`D3DTA_TEXTURE` yields on a stage with no texture bound, feeding
`ADDSIGNED(TEXTURE, CURRENT)`. If it yielded white, the result would be
`current + 0.5` — a bright wash. The reference frame is dark and detailed, so
whatever the semantics are, the pipeline's net effect is a contrast-preserving
green tint, not a wash. That was the specific guess the ticket refused to make,
and it no longer has to be made.

**And the ticket's framing does not survive the comparison.** NVG-4 assumed the
2D pit fails to participate in the NVG pipeline. Side by side, the Linux pit
*is* green, with panel and MFD detail comparable to Wine. The visible gap runs the
other way: on Linux the **runway/tarmac surface is not tinted** — it renders grey
against a green world — while Wine tints it with everything else.

Worth being exact about what the Linux side of that comparison is: `DX_NVG` is
**skipped by default** (BLUE-1, `otwloop.cpp:2654`, `FF_NVG_DXSTATE=1` restores
it), and the green comes from `SetGreenMode` / `ColorBankClass::GreenMode`, which
are independent of the DX state. So the capture is Linux-*without*-the-multistage-
pipeline against Wine-*with*-it — and it still lands close. The BLUE-1 comment's
claim of a "green world" is right for the world and wrong for the tarmac.

**Rescoped**: the remaining NVG gap is flat runway/tarmac surfaces missing the
green tint, not the 2D pit path. Those are the same surfaces that carry the
Linux-only decal/depth handling in TERRAIN-Z, which is where to look first.

### NVG-4 correction: the previous entry's rescoping was measured in the wrong configuration

The entry above rescoped NVG-4 away from the 2D pit on the strength of a Linux/Wine
side-by-side. **That comparison was run with `DX_NVG` skipped**, i.e. with the
multi-stage pipeline never executing — so it could not exercise the 2D pit path the
ticket is about, and the conclusion drawn from it does not hold. NVG-4 stands as
originally scoped.

Re-running TE-02 with `FF_NVG_DXSTATE=1` (the pipeline restored) shows the BLUE-1
symptom is **still present**: the world is replaced by flat blue and blue patches
appear across the MFDs. NVG-2 (ADDSMOOTH / ADDSIGNED / DOTPRODUCT3) and NVG-3
(`m_AlphaTextureStage`) were each necessary but together are **not sufficient** —
the 2D cockpit panel still inherits a stage configuration it cannot execute and
loses its chroma key. The skip at `otwloop.cpp:2654` stays.

| configuration | frame mean | result |
|---|---|---|
| Linux, `DX_NVG` skipped (default) | 0.234 | green pit + world, tarmac untinted grey |
| Linux, `FF_NVG_DXSTATE=1` | 0.168 | **blue fill returns** — world and MFD patches |
| Wine (gold) | 0.088 | green throughout, detail preserved |

**A methodological note against myself**: I read the 0.234 -> 0.168 drop as movement
*toward* Wine's 0.088 before looking at the frame. It is nothing of the kind — the
mean fell because flat blue is dark. Frame mean is not a similarity metric, and
using it as a proxy for "closer to the reference" is exactly the kind of shortcut
that produced the retracted TERRAIN-Z figures. The image decided it, not the number.

**What does survive** from the previous entry, because it was measured in the
default configuration the PO actually runs:
- The Wine reference frame itself — green throughout with detail preserved, which
  still rules out the wash-out branch (`D3DTA_TEXTURE` yielding white).
- NVG toggle is `N` (`DIK_N` = 0x31).
- With `DX_NVG` skipped, `GreenMode` tints via the colour-bank palette swap
  (`ColorPool = GreenTVBuffer`), which reaches colour-indexed geometry but not
  textured surfaces — consistent with the tarmac staying grey.

### TERRAIN-Z — measured: the runway is drawn against an interpolated COARSE surface that loses the airbase plateau

The previous entry left one thing explicitly unclaimed: which height the drawn
mesh corresponds to. Instrumented it (`FF_DEBUG_MESHZ=1`, `drawbldg.cpp`) to print
the terrain post at every LOD beside both queries, and ran the two missions.

**TE-09, airfield seen from the approach — fine LODs absent:**
```
pos=(779855,1307200) accurate=-18.3 approx=-26.0 | L0=-99999 L1=-99999 L2=-26.0 L3=-26.0 L4=-0.0
pos=(781101,1307747) accurate=-24.1 approx=-26.0 | L0=-99999 L1=-99999 L2=-99999 L3=-26.0 L4=-0.0
pos=(779451,1311341) accurate=-19.6 approx=-26.0 | L0=-99999 L1=-99999 L2=-26.0 L3=-26.0 L4=-0.0
```

**TE-02, player parked on the field — fine LODs present:**
```
pos=(1044231,1270905) accurate=-26.0 approx=-26.0 | L0=-26.0 L1=-26.0 L2=-26.0 L3=-26.0 L4=-0.0
```
**Zero** of 707 TE-02 samples have `accurate != approx`.

**The pattern is exact.** When `L0`/`L1` are streamed in, every source agrees at
`-26.0`. When they are not, `GetGroundLevel` returns a value that matches **no
available post** — `L2` and `L3` both read `-26.0` while it answers `-18.3`. It is
interpolating between widely-spaced coarse posts, and that interpolation does not
preserve the flattened airbase plateau, which at L2 spacing is only a post or two
across. The nearest-post approximation returns `-26.0` and is, at those moments,
the *more* faithful answer.

Flat surfaces are drawn at `gl - decal` against the **accurate** value, so on an
approach the strip is drawn warped across a ~9 ft range and only snaps flat once
the fine LOD streams in. Magnitude (9.5 ft = 2.9 m) and mission (the landing TE)
both match the PO's report.

**Note the irony**: the original non-`FF_LINUX` path used
`GetGroundLevelApproximation`. The Linux change to a per-frame *accurate* refetch
fixed the frozen-value bug it was written for and introduced this one, because
"accurate" is only accurate where fine posts exist.

**Correction to a previously recorded conclusion.** The streaming-transient family
was recorded as *bounded at exactly one member* (statics baking z at `Wake`). That
is wrong: this is a second, distinct member — a live per-frame query returning
coarse-interpolated values that misrepresent flattened terrain. The audit that
bounded the family only asked "does this caller re-query?", and a caller that
re-queries every frame still gets a wrong answer while the fine LOD is missing.
Re-querying is not sufficient; the *answer's provenance* has to be checked too,
which is the rule that came out of the first member and was not applied here.

**Not yet claimed**: that this is *the* cause of the half-submerged airstrip. The
mechanism, magnitude and mission all line up, but the touchdown itself has not
been observed — that still needs the PO to fly TE-09.

**Candidate fix, not yet implemented**: gate the accurate refetch on fine-LOD
availability and fall back to the nearest-post approximation otherwise. To be built
opt-in behind an env var and measured before anything changes by default.

### TERRAIN-Z — FF_RUNWAY_LODGATE: gate the accurate refetch on answer provenance

Built the candidate fix from the previous entry, opt-in behind
`FF_RUNWAY_LODGATE=1`. `GetGroundLevel` already reports which LOD answered via its
`lod` out-param, so the gate uses that rather than probing LODs separately: when a
**coarse** LOD answered (`lod > 1`), prefer `GetGroundLevelApproximation`, which
returns the flattened airbase plateau; otherwise keep the accurate value. This is
the provenance rule from the first streaming-transient member, applied to the
second.

**TE-09 (landing approach), divergence of the drawn surface from the plateau:**

| configuration | samples | mean | max |
|---|---|---|---|
| gate off (default) | 883 | 1.75 ft | **9.60 ft** |
| `FF_RUNWAY_LODGATE=1` | 709 | **0.00 ft** | **0.00 ft** |

The row that previously read `accurate=-18.3` at pos=(779855,1307200) now reads
`used=-26.0`. 603 of the gated run's samples were coarse-answered, so the gate is
doing real work rather than being a no-op.

**TE-02 regression check — the case that already worked.** Player-area rows all
answer at `glLod=0` with `used = approx = -26.0`, so the gate does not touch them;
only 74 of 773 rows are coarse-answered, and those are the far airbase 90,000 ft
away. Zero assertions. Orbit captures with and without the gate are visually
indistinguishable in the runway and aircraft; whole-frame RMSE is 0.08, accounted
for by cloud/sky and camera phase, not by the surface. So the fix is inert exactly
where the existing behaviour was already correct.

**Deliberately left OFF by default.** The measurement shows the drawn surface now
tracks the plateau, but the symptom this is meant to cure — the half-submerged
airstrip on landing — has still never been observed by the harness, because TE-09
starts airborne and the aircraft does not land itself. Flipping the default on a
mechanism argument plus a geometry metric, without once seeing the symptom clear,
is the pattern that produced the reverts earlier in this project.

**For the PO**: fly TE-09 with `FF_RUNWAY_LODGATE=1` and compare against a run
without it. That single observation is what the default flip is waiting on.

### TERRAIN-Z — the gate is confirmed live in the PO's own flight

During the PO's TE-09 flight with `FF_RUNWAY_LODGATE=1`, **399 of 399** `[MESHZ]`
samples were coarse-answered (`glLod=2`) with `used == approx` on every one. So the
gate is not a scripted-harness artefact: in the real approach the fine LODs are
never present at the airfield, the gate fires continuously, and the drawn surface
takes the nearest-post value throughout.

Worth noting the approximation is not a single plateau value everywhere —
e.g. `pos=(780980,1309143)` reads `L2=-16.0` while `L3=-26.0`, and the gate used
`-16.0`. The gate makes the drawn surface agree with the **nearest post at the
answering LOD**; it does not impose a constant.

Also fixed on review: the `FF_DEBUG_MESHZ` probe called `getenv` on every
flat-surface draw, every frame, in the runway rendering path, while the same
function caches its other env lookups in statics (`FF_RunwayDecal`, the gate
itself). Now cached the same way. Not a correctness bug and invisible to every
measurement taken — the kind of thing only a reread catches.

**Still open, and deliberately so**: whether the *symptom* clears. The geometry
metric went to zero and the gate demonstrably fires, but neither of those is the
PO seeing the jet sit on the strip. Awaiting that report before any default flip.

### TERRAIN-Z — LOD gate REFUTED as the fix; the real split is the physics settle height

**PO flew TE-09 with `FF_RUNWAY_LODGATE=1` and the jet is still buried 2-3 m.**
The gate is not the fix. Recorded plainly because the geometry metric it was built
against went to zero while the symptom did not move.

**Why it could not have worked.** At touchdown the PO's own log shows terrain
already fully resolved and self-consistent:
```
acZ=-28.33 groundZ=-26.00 aboveGround=2.33 vpAccurate=-26.00 vpApprox=-26.00 lod=0
[LIFT] zPos=-28.33 drawZ=-33.33      (fixed 5ft visual lift: 3 decal + 2 gear)
```
`lod=0`, both queries agree at `-26.00`. There is **no coarse-LOD disagreement at
the touchdown point at all**, so the gate had nothing to bite on. The 9.5 ft
divergence it removes is real but happens *elsewhere* — on the approach, at range —
and is simply not this symptom. A mechanism can be genuine, measurable, and
irrelevant; this one is.

**What the numbers actually say.** Comparing the working case against the broken one:

| | runway drawn | aircraft drawn | origin above surface | `aboveGround` |
|---|---|---|---|---|
| TE-02 parked (looks right) | -29.0 | -36.99 | 7.99 ft | **5.99** |
| TE-09 landed (buried) | -29.0 | -33.33 | 4.33 ft | **2.33** |

Same airframe, same model, same gear, drawn **3.66 ft lower** — and 3.66 is exactly
the `aboveGround` difference. The aircraft's *physics resting height above terrain*
is not constant between a ground start and a landing, while the visual compensation
that hides the runway decal **is** a fixed 5 ft, empirically A/B-tuned against the
5.99 case (`FF_GEAR_LIFT` default 2, see `otwdrive.cpp`). A constant correction
cannot cover a variable error.

The PO's video (`260825_TE9_ON_still_half_buried_on_runway.mp4`, t=209s) confirms it
visually: the runway surface cuts through the jet at the gear line, struts and
wheels entirely below the tarmac.

**This also re-reads the PO's original words correctly.** "The physics engine
terrain seems to be a few meters below the graphics engine terrain" — the physics
is letting the aircraft settle ~3.7 ft too low relative to where the same aircraft
rests when parked. The terrain queries agree with each other; the *settle* does not
agree with itself across mission types.

**Next, and specifically**: the ground-contact settle in `src/sim/airframe/eom.cpp`
(`gearHt = GetAeroData(AeroDataSet::NosGearZ) - radius`, lines 243 and 1966) is what
determines resting height. The question is why it lands on 2.33 ft after a touchdown
and 5.99 ft at a ground start. Chasing the *drawn* surface any further is chasing
the wrong half of the split.

**Gate disposition**: `FF_RUNWAY_LODGATE` stays opt-in and OFF. It corrects a real
approach-range inaccuracy and is inert at touchdown; it is not this bug and must not
be sold as one.

### TERRAIN-Z — correction: the settle is not varying arbitrarily, the aircraft is GEAR-UP

The previous entry concluded the aircraft's "physics resting height is not constant
between a ground start and a landing" and pointed at the settle as the bug. That
framing was wrong, and the aero data plus the PO's own video say so.

**Both readings are exact contact points, not drift.** From `sim/ACDATA/f16cbk40.dat`:
`Gear Z = 6.1`, `Fus Radius = 2.5`, and `GROUND_TOLERANCE = 0.1` (`eom.cpp:54`):

| observed `aboveGround` | = | contact point |
|---|---|---|
| **5.99** (TE-02 parked) | 6.1 - 0.1 | **landing gear** |
| **2.33** (TE-09 landed) | 2.5 - 0.1 (residual from pitch) | **fuselage belly** |

`CheckHeight()` takes the max of the nose/wing/gear/body contact terms. Nothing is
drifting: in one case the gear term wins, in the other the gear term is absent and
the body term wins. That is the code behaving exactly as written.

**The PO's video shows why: there is no landing gear on the aircraft.** At t=175 and
t=190 the jet is on the runway with no struts and no wheels visible — fuselage and
engine nozzle directly on the tarmac. It is gear-up, in the visuals *and* in the
physics, consistently. So this is **not** a physics/graphics split, which is what the
whole epic has been chasing.

**Two candidate readings, and I cannot yet separate them:**
1. The PO never lowered the gear, and this flight is simply a belly landing. Then
   `2.33` is correct, the geometry is correct, and there is no bug in this run at all.
2. The gear failed to deploy or animate — the DOF (`ComplexGearDOF`, driven from
   `surface.cpp:1614`) never reaching its commanded value. That would be a real bug,
   and it would corrupt both the visual and the contact term together, exactly as
   observed. It would also line up with the PO's *earlier, separate* report on a
   different flight: **"Gear never went up."**

**Loose end, recorded rather than smoothed over**: with `zPos=-28.33` drawn at
`-33.33` and the runway surface at `-29.0`, the aircraft origin is 4.33 ft above the
drawn surface and a 2.5 ft fuselage radius puts the belly ~1.8 ft *above* the tarmac,
i.e. slightly floating — yet the PO reports buried. So the drawn model's belly sits
further below its origin than `FusRadius`, or the lift stack is not applying as
computed. That discrepancy is unexplained and must not be glossed.

**Decisive next step is one question to the PO, not more instrumentation**: was the
gear down on that approach? The answer selects between "no bug here" and "the gear
DOF is broken", and no amount of further measurement on my side substitutes for it.

### TERRAIN-Z — TE-09 starts 10 nm out on final, so gear-down was a pilot action

Static check, no run needed. `campaign/SAVE/09 Landing Final Approach.trn` describes
itself as **"Landing From 10 nm out on Final"** — the aircraft starts airborne at a
range where an F-16 is normally still clean. Combined with the PO's video showing no
struts or wheels at t=175, t=190 *or* t=209, the likeliest reading is that the gear
was never lowered and this was a belly landing.

If so, `aboveGround = 2.33` is the correct fuselage contact for a gear-up aircraft,
and **that run contains no bug at all** — the epic would have spent two sprints
investigating a correctly-behaving simulator. Worth stating in those terms rather
than softening it.

Supporting evidence that the gear system is *not* globally broken: on TE-02 the
ground start reads `aboveGround = 5.99`, which is the gear contact term at full
extension (`Gear Z 6.1 - GROUND_TOLERANCE 0.1`). So `ComplexGearDOF` does reach its
commanded value there. A universally dead gear DOF is inconsistent with that.

**What this still does NOT explain**, and the reason the question stays open: with
the drawn origin 4.33 ft above the drawn runway and a 2.5 ft fuselage radius, a
gear-up jet should sit ~1.8 ft *above* the tarmac. The PO sees it buried. A belly
landing accounts for the physics number but not for the ~1.8 ft the visual is low.
So even under the innocent reading there is a residual visual discrepancy in the
buried direction, and it is unexplained.

Still gated on one PO answer: was the gear down? That selects between "no bug in this
run, plus a residual visual offset to chase" and "gear DOF failed on this approach".

### TERRAIN-Z — ROOT CAUSE for the TE-09 flight: hard-landing gear collapse, not terrain

PO confirmed the gear **was** down on the approach ("the landing marker in the HUD
only appears if the gear is down" — `hud.cpp:1605` keys the marker on
`gearPos > 0.5F`, so that is independent corroboration from the avionics side).
That rules out the belly-landing-by-omission reading in the previous entry.

**Everything observed comes from one code path**, `gndhndl.cpp:336`:

```c
else if (af->vt * impactAngle < sinkRate * 3.0F * (...) and af->gearPos > 0.8F)
{   //we hit too hard for the landing gear, crunch
    af->SetFlag(AirframeClass::EngineOff);
    mFaults->SetFault(FaultClass::eng_fault, ...);
    af->gearPos = 0.2F;
    for (int i = 0; i < af->NumGear(); i++) {
        af->gear[i].flags or_eq GearData::GearProblem;
        SetDOF(ComplexGearDOF[i], 0.0F);       // gear DOF explicitly zeroed
    }
```

Each symptom follows directly:
- `SetDOF(..., 0.0F)` -> gear not drawn (PO video: no struts or wheels).
- DOF 0 -> `CheckHeight()`'s gear term collapses to `FusRadius`, giving
  `aboveGround = 2.33` instead of the gear term's 5.99.
- `gearPos = 0.2` -> HUD landing marker extinguishes *after* touchdown, having been
  lit throughout the approach, exactly as the PO described.

**The trigger is not inverted** — checked, because this project has a history of
precedence and comparison bugs. It is an escalating ladder of `<` tests: the earlier
branch (`< sinkRate * 1.75`) is the normal landing and returns FALSE; this one
catches the band above it. With `sinkRate 15` (`f16cbk40.dat`) and the on-runway
factor of 1.0, the crunch band is **26.25 to 45 ft/s vertical, i.e. ~1600-2700
ft/min**. That is a genuinely hard arrival, and the PO's `aboveGround` trace shows a
bounce (3.40 -> 8.34 -> 14.75 -> 4.88) consistent with one.

**So for this flight the simulator is behaving correctly**: gear down, hard touchdown,
gear collapses, aircraft ends up on its belly. Not a terrain bug, not a rendering
bug, and not the physics/graphics split the epic was named after.

**What remains genuinely open:**
1. Whether landings in this port are *unduly* hard — i.e. whether the flight model or
   ground handling makes a normally-flyable approach exceed 26 ft/s here but not under
   Wine. The PO has reported the buried-airstrip symptom repeatedly, and "every landing
   collapses the gear" would be a real defect even with correct collapse logic.
2. The unexplained ~1.8 ft: with the origin drawn 4.33 ft above the drawn runway and a
   2.5 ft fuselage radius, a belly-resting jet should sit slightly *above* the tarmac,
   not buried. The collapse explains the physics number, not the visual offset.

**Next**: instrument the touchdown decision — print `vt * impactAngle`, the computed
threshold and which branch is taken — so the PO's next landing says whether the sink
rate was genuinely out of limits or the band is being entered on a reasonable approach.

### GEAR-2 — the gear command chain, mapped end to end, with a falsifiable prediction

PO supplied a second screenshot, timestamped **2:48, before touchdown**: landing
marker lit, no gear visible. That **kills the hard-landing-collapse root cause** from
two entries ago — the collapse happens *at* touchdown and sets `gearPos = 0.2`, which
would extinguish the marker, not light it. An effect cannot precede its cause. The
timestamp did the work my reasoning did not.

**The chain, verified by reading every link:**

| step | file | effect |
|---|---|---|
| pilot presses G | `commands.cpp:2355` `AFGearToggle` | `gearHandle = 1.0F` |
| per frame, if local | `surface.cpp:1560` | `gearHandle > 0 or OnGround()` -> `SetAcStatusBits(ACSTATUS_GEAR_DOWN)` |
| per frame | `airframe.cpp:873` | status set -> `gearPos += 0.3F * SimLibMinorFrameTime` |
| per frame | `surface.cpp:1608` | `DOF = (gearPos - 0.5) * 2 * NosGearRng` |

Note `gearHandle` and `ACSTATUS_GEAR_DOWN` are mutually reinforcing once set; the
pilot command is what breaks into the loop.

**The two thresholds do not agree, and that is the whole shape of the bug.** The HUD
element the PO reads as the landing marker is the **AOA bracket** (`hud.cpp:1605`),
gated on `gearPos > 0.5F`. The gear DOF is `(gearPos - 0.5) * 2`, i.e. **zero at 0.5**
and only fully extended at `gearPos == 1.0`. So across the entire band
`0.5 < gearPos < 1.0` the aircraft displays "configured to land" while the gear is
anywhere from stowed to partly out. Doors animate over `0..0.5`
(`surface.cpp:1594`, `gearPos * 2`), gear over `0.5..1.0` — a two-phase animation
whose *annunciation* fires at the phase boundary rather than at completion.

**Prediction, recorded before the measurement so it can fail:** the probe will show
`gearPos` sitting just above **0.5** and not advancing to 1.0. Marker on, DOF ~0, no
gear drawn, contact term collapsed to `FusRadius` -> `aboveGround = 2.33`. Every
observed number falls out of that single value.

If instead `gearPos` reaches 1.0, the prediction is wrong: the gear would be deploying
in the model and failing to be *drawn*, which is a different bug in a different place.

At 0.3/sec the travel should complete in ~3.3 s, so a stall requires the increment to
stop — either `ACSTATUS_GEAR_DOWN` being cleared each frame (note the `IsLocal()`
guard on the setter) or `SimLibMinorFrameTime` being wrong. `FF_DEBUG_GEAR=1` prints
`gearPos`, `gearHandle`, the DOF argument and the stuck/broken flags twice a second.

Instrumented build is running and waiting on the PO. TESWEEP-3 paused at **21/34, all
clean**, to free the machine; it resumes afterwards.

### GEAR-2 — prediction REFUTED by its own probe; the real fault is deltzGear == 0

The previous entry predicted `gearPos` would stall just above 0.5. **Wrong**, and the
probe I wrote to test it says so:

```
gearPos=0.000 handle=-1.00 DOF=0.000
gearPos=0.100 handle= 1.00 DOF=0.000     <- PO commands gear down
gearPos=0.254 handle= 1.00 DOF=0.000
gearPos=0.405 handle= 1.00 DOF=0.000
gearPos=0.559 handle= 1.00 DOF=0.186
gearPos=0.714 handle= 1.00 DOF=0.675
gearPos=0.869 handle= 1.00 DOF=1.161
```

`gearPos` advances 0.154 per half-second = **0.308/sec**, exactly the coded 0.3 rate,
and the DOF follows it correctly once past 0.5. Deployment works. The two-threshold
mismatch is real but is a ~1.7 s transient, not the bug.

**What the probe caught instead is decisive:**

```
[CHKHT] radius=2.50 gearHt=3.59 | nose=-3.20 wing=0.00 gear=0.00 body=2.44
        -> deltz=2.44 minHeight=2.34 complex=1
```

**`deltzGear = 0.00` while the gear is 87% deployed.** `CheckHeight()` takes the max
of the contact terms, so the body term (2.44) wins and `minHeight = 2.34` — which is
the `aboveGround = 2.33` measured at the TE-09 touchdown. The aircraft rests on its
belly *irrespective of gear position*, because the gear simply does not participate in
ground contact on this path.

That single fact explains the whole epic's symptom without any terrain involvement:
the jet sits `FusRadius` above the ground instead of `NosGearZ`, i.e. ~3.6 ft too low,
while the runway is drawn 3 ft *above* terrain — so it appears sunk into the tarmac.

**Where it goes wrong**: the complex branch (`eom.cpp:1981`) is gated on
`NumGear() > 1 and platform->drawPointer`, seeds `float best = 0.0F`, computes each
gear's contact point, transforms it by the orientation matrix, and keeps
`if (PtWorldPos.z > best) best = PtWorldPos.z`. A `best` of exactly 0.0 means **no
gear produced a positive transformed z** — either the loop did not run (null
`drawPointer`) or every `PtWorldPos.z` came out <= 0. The non-complex fallback
(`eom.cpp:2023`) uses `gearHt * gearPos + radius` and would have given ~5.6 here, so
the two paths disagree by the entire gear length.

**Also observed**: the PO reports the gear "got stuck" part-way, and the probe stopped
emitting at `gearPos = 0.869` while the game kept running — `RunGearSurfaces` ceased
being called for the player aircraft. That is a second, separate anomaly on the same
flight and must not be conflated with the contact-term fault.

**Next**: instrument the complex loop itself — `NumGear()`, `drawPointer`, and each
gear's `PtRelPos.z` and `PtWorldPos.z` — to establish whether the loop runs at all and
which term collapses. No more predictions ahead of that.

### GEAR-2 ROOT CAUSE — gearPos is animated only in RemoteUpdate(), never for the local player

PO: "the gear gets stuck partially deployed" (video `260825_gear.mp4`), and the probe
froze at `gearPos = 0.869` with the game still running. Traced to a single structural
fault.

**Verified by exhaustive grep of every write to `gearPos`:**

| site | file | what it does |
|---|---|---|
| `gearPos = 0.0F` | `airframe.cpp:147` | construction |
| `gearPos = 1.0F / 0.0F` | `airframe.cpp:470,573,593` | discrete set at init / in-air / on-ground |
| `gearPos = 1.0f` | `gndhndl.cpp:54` | ground start |
| `gearPos = 0.2F` | `gndhndl.cpp:354,419` | hard-landing collapse |
| **`gearPos += / -= 0.3F * SimLibMinorFrameTime`** | **`airframe.cpp:876,881`** | **the only continuous animation** |

Lines 876/881 are inside **`AirframeClass::RemoteUpdate()`** (function begins at 717).
And `RemoteUpdate()` has exactly two callers:

```c
if ( not IsLocal()) { ShowDamage(); af->RemoteUpdate(); return FALSE; }  // aircraft.cpp:2213
void AircraftClass::MakeLocal(void) { ...; af->RemoteUpdate(); ... }     // virtuals.cpp:810 (one-shot)
```

**So the per-frame gear animation runs only for aircraft that are NOT local.** For the
player's own aircraft it is never driven. `Exec()` (the local path) calls
`RunLandingGear()`, but that function only handles wheel spin and strut compression
(`gear.cpp:49`) — it does not touch `gearPos`.

That is why the gear freezes part-way: it advances only while the aircraft is still
being updated on the remote path, then stops permanently the moment it is not.

**Everything else follows from the frozen value.** With `gearPos` stuck below 1.0 the
gear DOF `(gearPos - 0.5) * 2` never reaches full, the gear is drawn partly stowed, and
`CheckHeight()`'s gear contact term does not win — leaving `deltzGear = 0.00` and the
body term at 2.44, i.e. `minHeight = 2.34`, the `aboveGround = 2.33` measured at
touchdown. One frozen float explains the partly-deployed gear, the belly contact, and
the "aircraft half-buried in the runway" symptom that named this entire epic.

**Confidence**: the code facts are verified by grep and by reading both call sites. The
causal link from "aircraft stops being remote" to the observed freeze is inference
consistent with the trace, and the fix must be validated against a flown landing rather
than assumed.

**Fix direction (not yet implemented)**: drive the gear animation from the local path
too — the honest options are to move the increment into `RunLandingGear()` (called from
both `Exec()` and the remote path) or to add the equivalent step to `Exec()`. To be
built opt-in and measured before any default change, per the pattern that has repeatedly
saved this project from speculative rendering/physics edits.

### GEAR-3 — all three gears read GearBroken, which is why the gear never touches the ground

The GEAR-2 animation fix (gearPos not driven on the local path) is real but is **not**
the cause of the PO's symptom. With it applied and the gear extended to `DOF = 1.326`
(84% of the 90 deg range), `CheckHeight()` still reports `gear = 0.00` and
`minHeight = 2.33` — the belly. Measured, not assumed.

**The reason, from a probe inside the contact loop:**

```
[GEARZ] numGear=3 drawPtr=1 complex=1 brk=1,1,1 best=0.00 -> deltzGear=0.00
```

`brk=1,1,1` — **all three gears carry `GearData::GearBroken`**, from the earliest
sample. The loop body is guarded by `if (not (gear[i].flags bitand GearBroken))`, so it
never executes for any gear; `best` stays at its 0.0F seed and `deltzGear` is 0. The
fuselage term (2.44) then wins `CheckHeight()` and the aircraft rests on its belly
regardless of gear position. Zero per-gear probe lines were emitted, confirming the
body never runs rather than running and computing zero.

**A latent bug found while tracing it** (`airframe.h:527`):

```c
GearStuck = 0x01, GearBroken = 0x02, DoorStuck = 0x04, DoorBroken = 0x08,
GearProblem = 0x0F,      // a MASK of all four, not a distinct flag
```

`GearProblem` is every bit, so `flags or_eq GearData::GearProblem` (`gndhndl.cpp:358`
and `423`) sets **GearBroken** as a side effect of flagging a "problem". Reads use it
correctly as a mask (`cblights.cpp`), so the enum is doing double duty as flag and
mask. Both write sites are landing-time, so this does not explain gears already broken
at mission start — but it will break the gear on any hard landing and is worth fixing
on its own.

**Not yet identified: what sets GearBroken before the first sample.** Ruled out by
measurement: the water/river branch (`eom.cpp:1332`) — a one-shot probe placed at that
exact site **never fired**. Initialisation is correct (`readin.cpp:151-158` allocates
`NumGear` entries and zeroes `flags`, and it is the only `new GearData` in the tree).
Remaining candidates: `eom.cpp:1505`, and whether `readin.cpp`'s init actually runs for
the player's airframe instance. That is the next measurement.

**Status of the GEAR-2 animation fix**: switched to **opt-in** (`FF_GEAR_ANIM_FIX=1`).
It is correct in direction but measured at 0.62/sec against the coded 0.3 — as
`RunLandingGear()` has three call sites and is not once-per-frame — and it does not
cure the symptom. It will not go default-on until it has a single-call-site home and a
flown validation.

### GEAR-4 — the gear DOF loop stops running; gearPos is fine and the GEAR-2 fix is REMOVED

Two of my own conclusions die here, both to measurement.

**1. "gearPos is only animated in RemoteUpdate, never for the local player" — REFUTED.**
A/B with the fix off and on:

```
fix OFF : gearPos 0.000 -> 0.245 -> 0.399 -> 0.550 -> 0.700 -> 0.852   brk=0,0,0
fix ON  : gearPos 0.000 -> 0.306 -> 0.613                              brk=0,0,0
```

`gearPos` already advances at the correct 0.3/sec **without** the fix; the fix merely
double-stepped it to 0.6/sec. And a probe placed outside the animation shows it reaching
**`gearPos=1.000`**. The fix addressed a problem that did not exist and has been
**removed from the tree**, not left as dead opt-in code.

**2. "All three gears are GearBroken" — a SAMPLING ARTEFACT.** `[GEARZ]` lives in
`CheckHeight()`, which only runs near the ground, so every `brk=1,1,1` sample was taken
*after* ground contact. The continuously-running probe reports `brk=0,0,0` throughout
the approach and deployment. Gears are not broken in flight. This is the same
unrepresentative-subset error as the retracted runway-z and "11 ft" figures — the third
time this session, and the tell each time was reading a value without first asking when
and where the probe fires.

**What is actually happening**, from probes on both sides of the gate:

```
[GEAR4] MoveSurfaces: IsComplex=1 drawPtr=1 gearPos=1.000    <- outside RunGearSurfaces
[GEAR2] gearPos=0.871 DOF=1.168 brk=0,0,0 stk=0,0,0          <- inside, then silence
```

`gearPos` reaches 1.000 and `MoveSurfaces` keeps running with `IsComplex=1` and a valid
`drawPointer` — but the **gear DOF loop inside `RunGearSurfaces` stops executing**. The
DOF freezes at 1.168 of the 1.571 rad (90 deg) range: **74% deployed, permanently**.
That is exactly the PO's "gear comes out part way and gets stuck".

Since that same loop is the only writer of `ComplexGearDOF`, and the DOF is what both
the visual and `CheckHeight()`'s complex contact term read, one stalled loop produces
the stuck gear, the belly contact and the half-buried appearance.

**Next, and narrow**: `RunGearSurfaces` runs its per-gear work inside
`if (af->auxaeroData->animWheelRadius[0])` (`surface.cpp:1571`) after an `IsLocal()`
block. Establish which condition stops holding — probe each gate separately rather than
inferring, given three theories have already died this way.

### GEAR-5 ROOT CAUSE — gear overspeed trip breaks a random gear and parks its DOF at 60%

**The PO called this one**: "what is the max speed at which gear can be safely deployed?
If we're deploying at too high a speed, the result is stuck gear."

`eom.cpp:1608`, the airborne branch:

```c
// FRB - gear damage when flying too fast with gear down
gearLimitSpeed = minVcas * KNOTS_TO_FTPSEC * 1.1f;
if (minVcas < 220.0f) gearLimitSpeed = 220.0f * KNOTS_TO_FTPSEC;

if (gearPos >= 0.9F and vt > gearLimitSpeed)
{
    int which = rand() % NumGear();
    ...
    gear[which].flags or_eq (DoorStuck bitor GearStuck bitor DoorBroken bitor GearBroken);
    mFaults->SetFault(FaultClass::gear_fault, ldgr, fail, TRUE);
}
```

`MinVcas` for the F-16 is **250** (`f16cbk40.dat`), so the limit is
**250 x 1.1 = 275 knots**. The PO reported being "under 300" — which is *over* 275.

**The full causal chain, every link measured or read directly:**

1. Gear commanded down; `gearPos` animates correctly at 0.3/sec (measured) and reaches
   `0.9`.
2. At `gearPos >= 0.9`, if `vt` exceeds 275 kt, **one random gear** (`rand() % NumGear()`)
   is flagged `DoorStuck|GearStuck|DoorBroken|GearBroken`.
3. `RunGearSurfaces` guards its DOF write on `not GearStuck and not GearBroken`
   (`surface.cpp:1606`). For the flagged gear it takes the `else` and parks the DOF at
   **`NosGearRng * 0.6f * DTR`** — 60% of travel, frozen, no longer tracking `gearPos`.
   That is the "gear comes out part way and gets stuck".
4. `CheckHeight()`'s complex loop skips broken gears entirely, so that gear contributes
   no contact point; with the contact term lost the fuselage term (2.44) wins and
   `minHeight` becomes 2.33 instead of the gear's 5.99.
5. The aircraft rests ~3.6 ft too low, on a runway itself drawn 3 ft above terrain —
   the "aircraft half-buried in the airstrip" that named this epic.

**Two design faults worth separating:**
- **The limit is low.** 275 kt against a real F-16 gear limit of ~300 KIAS.
- **The comparison uses `vt`, TRUE airspeed**, against a limit derived from a *calibrated*
  speed (`MinVcas`). TAS exceeds CAS with altitude, so the effective indicated limit is
  lower still, and lower the higher you are.

**Not yet established, and it matters**: this code is **not** `FF_LINUX`-gated, so the
Wine build runs the same rule. Either the PO flies the approach slower under Wine, or
`vt` differs between the builds. Before changing the limit, that has to be measured —
raising a threshold to mask a wrong airspeed would be the same mistake as the LOD gate.

`FF_DEBUG_GEAR=1` now logs `[GEAROVR]` at the trip with vt in knots, the limit, MinVcas,
gearPos and altitude, so the next flight records the actual numbers rather than inviting
another inference.

### GEAR-5 — trip confirmed on Linux, and Wine does NOT trip on the same mission

**Linux, reproduced three times** (TE-09, gear commanded down by `FF_TEST_GEARDOWN`):

```
[GEAROVR] gear[2] broken: vt=316.1 kt (533.5 ft/s) limit=274.9 kt minVcas=250.0 gearPos=0.90 alt=1984
[GEAROVR] gear[2] broken: vt=315.9 kt ...
[GEAROVR] gear[1] broken: vt=315.7 kt ...
```

Fires at exactly `gearPos = 0.90`, at ~316 kt true against the 274.9 kt limit, breaking a
**different random gear each run** (`rand() % NumGear()`) — matching the code and
matching the PO's asymmetric, tilting aircraft.

**Wine, same mission, same gear command: the gear deploys FULLY.** Drove the Wine build
to TE-09, pressed G (`DIK_G` = 0x22), and captured the orbit view: all three gears down
and locked, nose and both mains clearly extended. So the Linux build breaks a gear where
the gold standard does not — even though `eom.cpp:1608` is **not** `FF_LINUX`-gated and
both builds read the same `MinVcas = 250` from the same `f16cbk40.dat`.

**Two candidate explanations, neither yet established:**
1. `vt` is inflated in the Linux build, so the same mission state reports a higher speed
   and crosses a limit Wine stays under.
2. The Linux aircraft genuinely starts or holds the approach faster (flight-model or
   drag difference), so it is really at 316 kt where Wine is slower.

**Not measured, and I am not guessing it**: Wine's airspeed at that moment. The Wine
capture path is H.264 video and the HUD digits are too blurred to read honestly; the
Linux side kept returning a fly-by camera instead of the HUD view, so the matched
comparison failed. That comparison is the next step and needs a sharper Wine capture or
a direct `vt`/`vcas` log rather than a screenshot.

**Why this matters before touching the threshold**: raising the 275 kt limit would make
the symptom disappear on Linux without establishing whether the airspeed feeding it is
correct. That is precisely the LOD-gate error — driving a metric to zero without
confirming it was the cause. The limit is also compared against `vt` (TRUE airspeed)
while being derived from `MinVcas` (a calibrated speed), which is wrong on its own terms
and worth fixing independently of the magnitude question.

### GEAR-5 — `vt` is NOT inflated; the aircraft really is at 307 KIAS

Measured at the trip:

```
[GEAROVR] gear[2] broken: vt=315.5 kt vcas=307.1 kt limit=274.9 kt minVcas=250.0
          gearPos=0.90 alt=1984 mach=0.48
```

`vt / vcas = 1.027`, which is exactly the true-to-calibrated ratio for ~2000 ft. **So the
Linux airspeed is not inflated** — candidate (1) from the previous entry is dead. The
aircraft is genuinely flying the approach at **307 knots indicated**.

**That reframes the item.** A real F-16's landing-gear limit is ~300 KIAS, so lowering
the gear at 307 KIAS *should* damage it. The simulator is behaving approximately
correctly, and the code's 275 kt limit (MinVcas 250 x 1.1) is merely ~25 kt harsher than
the real airframe rather than being the cause. Raising 275 -> 300 would **not** have
saved this landing, which is exactly why it was worth measuring before changing.

**What remains genuinely unexplained** is the Wine divergence: the same mission, driven
the same way, deploys the gear fully under Wine. Since `vt` is correct on Linux, the
remaining possibilities are that the Wine aircraft is slower at the moment the gear
passes `gearPos 0.9` (different drag, throttle state, or simply more elapsed time
decelerating before the command), or that something else suppresses the trip there.
Measuring Wine's airspeed is still the open task and still needs a capture path that can
resolve HUD digits.

**Practical consequence for the PO, and the likeliest whole story**: the approach is
being flown at ~300+ KIAS with the gear coming down. Slowing below ~250 KIAS before
lowering the gear should let it deploy fully, and would confirm the entire chain from the
PO's side in one flight. The `FF_RUNWAY_LODGATE`, hard-landing-collapse and
gearPos-animation theories are all dead; this one is measured end to end except for the
Wine leg.

**Left unchanged deliberately**: the 275 kt threshold, and the `vt`-vs-`MinVcas`
true/calibrated mismatch. The mismatch is a genuine defect (a limit derived from a
calibrated speed compared against a true one) but at 2000 ft it accounts for only ~8 kt
of the 32 kt exceedance, so fixing it would not change this outcome either. Both are
worth doing on their own merits, neither is the cause, and neither should be presented as
a fix for the PO's landing.

### GEAR-5 CONFIRMED — gear deploys fully when lowered below 250 KIAS

PO flew TE-09 slowing below 250 KIAS before dropping the gear. From the PO's own
flight log:

```
[GEAR2] gearPos=1.000 handle=1.00 DOF=1.570 brk=0,0,0 stk=0,0,0 onGnd=0 agl=-1332
[GEAROVR]  -- did not fire
```

`gearPos` reaches **1.000**, the DOF reaches **1.570 rad** (the full 90 deg
`NosGearRng`), no gear is broken or stuck, and the overspeed trip never fires. Against
the earlier runs at ~307 KIAS, where a random gear broke at exactly `gearPos 0.90` and
its DOF parked at 60%, this is a clean controlled comparison with one variable changed.

**The chain is now confirmed end to end, every link measured:**

1. Gear lowered above ~275 kt -> `eom.cpp:1608` flags one **random** gear
   `DoorStuck|GearStuck|DoorBroken|GearBroken`.
2. `RunGearSurfaces` guards its DOF write on those flags, so that gear's DOF parks at
   `NosGearRng * 0.6` — permanently part-deployed, asymmetric (hence the PO's tilt).
3. `CheckHeight()` skips broken gears, losing that contact point; the fuselage term
   (2.44) wins and `minHeight` becomes 2.33 instead of the gear's 5.99.
4. The aircraft rests ~3.6 ft too low on a runway itself drawn 3 ft above terrain — the
   "physics terrain a few meters below graphics terrain" that named this epic.

Lower the gear below 250 KIAS and none of it happens.

**What this retires.** The TERRAIN-Z epic was misnamed from the start: no terrain defect
was ever involved. Refuted along the way, each by a measurement rather than an argument:
the feature re-snap, the coarse/accurate LOD gate (`FF_RUNWAY_LODGATE`, driven to 0.00
divergence and still not the cause), the hard-landing collapse, the gearPos animation
path, and "all three gears broken at mission start". The surviving explanation is a
single `rand()` call behind a speed threshold.

**What remains open, and is now small:**
- The limit is **275 kt** (`MinVcas 250 * 1.1`) against a real F-16 gear limit of ~300
  KIAS — ~25 kt harsh.
- It compares `vt` (**true** airspeed) against a limit derived from `MinVcas` (a
  **calibrated** speed). Wrong on its own terms; worth ~8 kt at 2000 ft.
- Wine did not trip on the same mission. With Linux's `vt` now proven correct
  (`vt/vcas = 1.027`), the likely answer is simply that the Wine approach was flown
  slower — but it is unmeasured, and the improved capture path (`-q ultra`) exists to
  settle it.

Neither threshold change would have saved the 307 KIAS landing, so neither is a fix for
the reported symptom and neither will be presented as one.

### GEAR-5 CLOSED, and a separate runway-surface defect isolated

**PO flight, gear dropped at 250 KIAS.** Gear deployed correctly; at rest
`aboveGround` settles at **5.89-6.03**, i.e. the **gear** contact term
(`NosGearZ 6.1 - GROUND_TOLERANCE 0.1`), not the 2.33 fuselage term. The aircraft is on
its wheels — PO confirms all three visible. GEAR-5 is closed.

**The remaining symptom is different and was masked by the gear bug.** PO: "the jet went
up and down sinking below the surface but then coming out again, like it does when it
takes off (due to an early attempt to fix the can't-see-the-landing-gear-on-the-runway
problem). Finally comes to a stop past the runway in terrain, all 3 wheels clearly
visible."

**Physics is not what is moving.** From the same flight:
- `onGround` makes **one** transition (156 airborne samples, then 53 on-ground) — the
  `OnGround()`-gated 5 ft visual lift is *not* flickering, which was my first guess.
- `groundZ` ramps smoothly `-17.08 -> -14.60 -> -12.39 -> ... -> -3.63` as the aircraft
  rolls up a slope, with `acZ` tracking it and `aboveGround` steady near 6.

So the aircraft is moving smoothly over smoothly-varying ground. What oscillates must be
the **drawn runway surface** — which is exactly the warping measured under TERRAIN-Z:
flat surfaces are drawn at `gl - decal` against a `GetGroundLevel` that, where it answers
from coarse posts, does not preserve the flattened airbase plateau, spanning up to 9.5 ft
across one field.

**This re-opens `FF_RUNWAY_LODGATE` for the symptom it actually fits.** The gate was
built to make the drawn surface agree with the nearest post at the answering LOD, drove
the divergence from mean 1.75 ft / max 9.60 ft to **0.00/0.00**, and was correctly
rejected as the cause of the belly landing — the terrain at *touchdown* was already
self-consistent (`lod=0`, both queries -26.00). But a warped drawn strip under a
smoothly-rolling aircraft is precisely "sinking below the surface then coming out again".
Refuting it for one symptom did not refute it for this one, and I nearly left it buried
because of that.

**Test, cheap and decisive**: fly the same landing with `FF_RUNWAY_LODGATE=1`. If the
sinking stops, the gate earns its default; if not, it stays off and the drawn-surface
fault is elsewhere.

**Minor, logged for later**: `stk=0,1,1` appears during rollout — gears 1 and 2 pick up
`GearStuck` (not broken) from the ground-roll damage path (`eom.cpp:1505`). `GearStuck`
also gates the DOF write, so those gears stop tracking `gearPos` and park at
`NosGearRng * 0.6`. Cosmetic here since they were already fully extended, but it is the
same flag-gates-animation coupling as GEAR-5 and should be looked at.

### FF_RUNWAY_LODGATE — DEFAULT ON. PO-confirmed fix for the sinking runway surface

**PO flew TE-09 with the gate on: "no, the jet did not sink in and come back out."** The
symptom the gate was re-scoped against is gone.

The same flight also ended with a hard landing that broke the rear gear, and the log
reads correctly once the probe's own guard is accounted for: the last `[GEAR2]` line is
`gearPos=1.000 DOF=1.570 brk=0,0,0 stk=0,0,0 agl=-6` — healthy, fully-extended gear six
feet up — and then the probe **goes silent** while `[GROUND]` continues to the end. The
probe sits inside `if (not GearStuck and not GearBroken)`, so its silence *is* the flag
being set at touchdown. Final rest at `aboveGround = 2.33` is fuselage contact, i.e. the
collapsed gear. That is the hard-landing path behaving correctly, not a regression, and
not the GEAR-5 bug returning: no `[GEAROVR]` fired and the gear was fully deployed on
approach.

**The near-miss worth recording.** This gate was refuted by a PO flight earlier and I
recorded it as refuted *without qualification*. It had only ever been tested against the
belly landing, where terrain at touchdown was already self-consistent (`lod=0`, both
queries `-26.00`) so the gate had nothing to act on. The sinking-and-emerging symptom is
a **warped drawn surface under a smoothly-rolling aircraft** — exactly what the gate
corrects. Generalising one negative result across every symptom buried a working fix for
hours. **A refutation is scoped to the symptom it was tested against.**

**Why it is safe to default on:**
- Measured divergence of the drawn surface from the plateau: mean 1.75 ft / max 9.60 ft
  -> **0.00 / 0.00**.
- **Inert where fine LODs are present**: on TE-02 every player-area row answers at
  `glLod=0`, so the gate never fires there; only the far airbase 90,000 ft away is
  affected.
- No tarmac disappearance reported, which was the live failure mode when the whole
  compensation stack was disabled earlier.

`FF_NO_RUNWAY_LODGATE=1` reverts. Full 34-mission regression sweep running against the
new default; the flag comes back off if anything regresses.

### GEAR-5 follow-ups — analysis only, NOT applied (these are PO decisions)

Two known defects remain in the gear overspeed trip. Both are real; **neither would have
saved the 307 KIAS landing**, so neither is a fix for the reported symptom and both are
balance decisions rather than corrections. Recording the analysis so the choice is
informed, and deliberately changing nothing.

**1. There is no gear-limit constant in the data.** `f16cbk40.dat` carries only
`Min Vcas 250`, `Max Vcas 850`, `Corner Vcas 420` — no landing-gear speed limit. So
`eom.cpp:1608` improvises one from the *minimum* comfortable speed:

```c
gearLimitSpeed = minVcas * KNOTS_TO_FTPSEC * 1.1f;   // 250 * 1.1 = 275 kt
```

Using a *minimum* speed scaled by 1.1 as a *structural* limit is a heuristic, not a
modelled value. Options, in increasing order of intrusiveness:
- leave 275 (harsh by ~25 kt against the commonly-cited F-16 gear limit of 300 KIAS);
- change the multiplier `1.1 -> 1.2`, which yields **exactly 300** for the F-16 and
  scales sensibly for other airframes since each carries its own `MinVcas`;
- add a real per-aircraft gear-limit field to the aero data — most correct, most
  invasive, touches every `.dat`.

The middle option is attractive precisely because it is data-derived rather than a magic
number, but it *is* a flight-model change and raises the speed at which players can
safely drop gear across every aircraft.

**2. The comparison mixes airspeed types.** `gearLimitSpeed` is derived from `MinVcas`, a
**calibrated** speed, but is compared against `vt`, **true** airspeed:

```c
if (gearPos >= 0.9F and vt > gearLimitSpeed)
```

Measured at the trip: `vt=315.5 kt` vs `vcas=307.1 kt` — a ratio of 1.027, correct for
~2000 ft. So the effective indicated limit is ~8 kt lower than intended at that altitude,
and falls further the higher the aircraft is. Comparing `vcas` would be correct on its
own terms and is a one-token change, but it does slightly relax the limit.

**Recommendation**: fix (2) on correctness grounds — it is unambiguously wrong to compare
true against calibrated — and leave (1) alone unless the PO wants gear-down speeds
loosened. Not applied pending that decision.

### TESWEEP-4 triage — row 12's assertion delta is NOT the LOD gate

The gate-on sweep matches the gate-off baseline row for row, with one exception: **row 12
"Nav and Timing"** reads `asserts=8` against `asserts=4`. ShiAssert fires once per site
per process and prints two lines, so that is **2 sites -> 4 sites**, and it deserved
checking before the default flip was allowed to stand.

The two extra sites are:

```
tviewpnt.cpp:358  [Failed: (xPos >= -0.5f) and (xPos <= LEVEL_POST_TO_WORLD(1, LOD) + 0.5f)]
tviewpnt.cpp:359  [Failed: (yPos >= -0.5f) and (yPos <= ...)]
```

**Both are inside `TViewPoint::GetGroundType()` (line 333), not
`GetGroundLevelApproximation()` (line 515).** The LOD gate calls the latter and never the
former, so it is not the direct cause. `GetGroundType` is called only from
`drawparticlesys.cpp` — explosion/smoke particles sampling the ground type under
themselves.

**Most likely explanation**: TE-12 flies unpiloted to timeout and crashes somewhere; the
impact spawns particles which query ground type at the impact point. A slightly different
flight path puts that impact somewhere else, lighting different assertion sites. These are
position-dependent one-shot warnings, so run-to-run variance moves them.

**Also worth noting the assertion is over-eager**: it fires *before* the availability
check, and the function then returns 0 safely without dereferencing the post. So it warns
about querying an unstreamed position rather than reporting a bad read.

**Not concluded** — an indirect path (the gate changes drawn surface heights, which could
shift where other code samples) cannot be excluded from a single sample. Planned check:
re-run row 12 twice with the gate on and twice with it off once the sweep finishes. If the
count varies within either configuration, it is variance; if it tracks the flag, it is the
gate and the default comes back off.

### TESWEEP-4 RESULT — 34/34 clean with the LOD gate default ON, and row 12 settled

**Full sweep: 34/34 reaching sim, 0 crashes, 62 assertion lines** (TESWEEP-2 baseline was
64 — slightly *fewer*, not more).

**Row 12's assertion delta was variance, proven by the right experiment.** Re-ran row 12
twice per configuration:

| config | run 1 | run 2 | sites |
|---|---|---|---|
| gate ON | 8 | 8 | 180, 314, 358, 359 |
| gate OFF | **12** | **4** | 180,258,314,358,359,367 / 180,314 |

The count varies **4 -> 12 within the gate-off configuration alone**, so it does not track
the flag. `GetGroundType`'s bounds assertions are position-dependent one-shots and TE-12
flies unpiloted to timeout, crashing somewhere slightly different each run; the impact
particles then sample ground type wherever that was.

**The method point is the reusable part.** Comparing a single gate-on run against a single
gate-off run would have "confirmed" a regression that does not exist. **Two runs per
configuration is what separates a real effect from noise**, and it is cheap. The same
discipline applied earlier would have caught that the LOD gate's 0.00 divergence metric
was not evidence about the belly landing.

`FF_RUNWAY_LODGATE` stays **default ON**: PO-confirmed against the symptom, 34/34 clean,
assertion total no worse than baseline, and the one anomaly disproved.

**Probe cleanup built and verified** — the five probes for refuted theories are gone and
the tree builds clean.

### GEAR-6 — unreachable gear-break branch in the ground-roll damage path

`eom.cpp:1485`:

```c
if (gear[which].strength < 50.0F)
{
    mFaults->SetFault(gear_fault, ldgr, fail, FALSE);
    gear[which].flags or_eq GearData::GearStuck;
    ... SetDOF(ComplexGearDOF[which], newpos);   // random, up to 50 deg
}
else if (gear[which].strength < 0.0F)            // <-- UNREACHABLE
{
    platform->SetDOF(ComplexGearDOF[which], 0.0F);
    gear[which].flags or_eq GearData::GearBroken bitor GearData::DoorBroken;
    ... sndWheelBrakes
}
```

**Any `strength < 0.0F` also satisfies `strength < 50.0F`, so the first branch always
wins and the second can never execute.** Gear damaged during ground roll can therefore
only ever become *stuck*, never *broken*, and the "gear snaps off" sound and full DOF
collapse are dead code. The tests are simply ordered wrong: `< 0.0F` must be checked
first.

**Observed consequence**, from the PO's successful landing: `stk=0,1,1` — two gears
stuck, none broken, exactly as this ordering forces. Once `GearStuck` is set,
`RunGearSurfaces` stops tracking `gearPos` for that gear (it is the same guard as GEAR-5)
and the DOF is left at the random `newpos`, up to 50 deg — a gear frozen part-deployed.

**Deliberately NOT applied**, because the correct fix makes the sim *harsher*: swapping
the order means heavily-damaged gear breaks off instead of merely sticking. That is what
the code intends and what the sound effect and DOF-to-zero clearly expect, but it is a
balance change and belongs to the PO alongside the other two gear decisions.

**Also reconsidered and NOT called a bug**: `GearProblem = 0x0F` being a mask of all four
flags. Both write sites (`gndhndl.cpp:358,423`) are in the hard-landing collapse path,
which already sets `gearPos = 0.2` and zeroes the DOF — marking every gear flag there is
plausibly deliberate shorthand for "everything is wrong", not an accident. Reads use it
correctly as a mask. Flagging it as a bug earlier was premature.

### BOMB-1 — first scripted bomb impact captured; FX path confirmed executing

The bombing half of the epic had never been exercised: both PO attempts ended in the
INPUT-1 and CRASH-8 crashes, and the impact probe never caught a detonation. Driven
scripted at last (TE-20 "Bombs with CCIP", `FF_TEST_BOMB=35`).

**A release and an impact both happened:**

```
[TESTBOMB] bomb at station 3 weaponId=5 -> SetCurrentWeapon: hardpoint=3
[TESTBOMB] mode->AirGroundBomb, pickle raised at t=35.0
[MSLEND] Process: endCode=11 pos=(1730125,1209373,-1851) groundType=7 type=2 stype=3
[MSLEND] BombImpact legacy branch: DamageType=2 BlastRadius=293
[MSLEND] spawning SFX_GROUND_EXPLOSION at (1730125,1209373,-1851)
```

**The impact-FX fix works at code level.** The earlier repair — bombs were excluded from
the impact switch by `type == TYPE_MISSILE`, so nothing drew a bomb impact at all — now
reaches `spawning SFX_GROUND_EXPLOSION`. That had been committed but never observed
executing; it now has been.

**Height at impact, first measurement of the bombing case:**

```
[LODZ] impact physicsZ=-1851.1 | lod0=-1765.0 lod1=-1753.0 lod2=-1763.0 lod3=-1601.0 lod4=-1607.0
```

Negative z is up, so the detonation is **86 ft above the finest terrain post**, and the
posts themselves span **250 ft across LODs** at that spot.

**Not concluded, and specifically not claimed as the PO's symptom.** `endCode=11` with
`groundType=7` suggests this bomb may have struck a *structure* rather than bare terrain,
in which case detonating above the terrain post is correct and the 86 ft means nothing.
A single impact on unknown geometry cannot distinguish that from a genuine
physics-vs-drawn-terrain gap. Needs impacts on known flat bare ground, repeated.

**Harness gap found**: `FF_SHOT_ON_IMPACT=1` did not produce a sim frame — the only
capture written was a stale loadout screen. So there is no visual of the fireball, and
"does the explosion render where the PO can see it" remains unanswered. Fixing that
capture is a prerequisite for closing the bombing item, since the PO's report is
explicitly visual ("no fireball ... then the sound of an explosion").

### BOMB-1 correction — the impact capture DOES work; the camera is the problem

The previous entry claimed `FF_SHOT_ON_IMPACT` "did not produce a sim frame". **Wrong** —
it writes `/tmp/ff_impact_<N>.bmp`, not `/tmp/ff_view_<N>.bmp`, and I checked the wrong
filename. `/tmp/ff_impact_0.bmp` exists, 1024x768, `mean=0.438`, with the log line
`[IMPACTSHOT] requested /tmp/ff_impact_0.bmp` sitting in plain view in the same log I had
already grepped. A file-not-found conclusion drawn without checking what the code
actually writes.

**What the frame shows**: the aircraft in level flight over terrain, viewed by the orbit
camera pointed *at the jet*. The impact is on the ground some distance away and simply
out of frame — so the fireball's visibility is still unverified, but for a camera-framing
reason, not a broken capture.

**The remaining question is narrow and unchanged**: does the bomb explosion render where
the pilot can see it? Answering it needs the camera looking at the impact point, not at
the aircraft — either a weapon/target view during the fall, or a release from low
altitude so the detonation is in frame. The FX spawn call itself is confirmed executing
(`[MSLEND] spawning SFX_GROUND_EXPLOSION`), so what is untested is purely whether it is
drawn and visible.

### BOMB-1 — fireball visibility still INCONCLUSIVE; six frames prove nothing yet

Captured six frames spanning the detonation (`FF_SIM_SCREENSHOT` at 92/95/98/101/104/107 s,
forward HUD view, `[MSLEND] spawning SFX_GROUND_EXPLOSION` confirmed in the same run). **No
fireball appears in any of them.**

**That is not evidence the effect fails to render**, and it must not be logged as such. The
impact is at (1730134, 1209380) while the aircraft continues in level flight at altitude;
by the time of these captures the detonation is plausibly far behind and below the nose,
outside the field of view entirely. A frame that could not have shown the effect says
nothing about whether the effect exists.

This is the same trap as the earlier sampling errors — reading a value without first
establishing that the instrument could have observed the thing being measured.

**What would actually settle it**, in order of preference:
1. A camera that follows the weapon or looks at the impact point, so the detonation is
   guaranteed in frame.
2. Failing that, log the aircraft position and heading at each capture and compute whether
   the impact point lay within the view frustum — only then does an empty frame count as a
   negative result.
3. A low-altitude release, so the aircraft is close enough that the impact stays in view.

**Confirmed so far and not in doubt**: release works, impact occurs, and the FX spawn call
executes (which is the fix that had never been observed running). Only "is it drawn where
the pilot can see it" is open.

### BOMB-1 RESOLVED at code level — the explosion effect is created and correctly mapped

Three attempts to photograph the fireball all failed for framing reasons, and the third
made the reason quantitative: at capture the aircraft was **11,800 ft horizontally and
8,160 ft above the impact, flying away from it**, so no forward view could contain the
detonation. Rather than keep chasing camera angles, measured the thing directly.

```
[MSLEND] spawning SFX_GROUND_EXPLOSION at (1730138,1209379,-1852)
[PSMAP]  table built: paramSets=658 maxEffectId=657 dropped=0  PPN[6](GROUND_EXPLOSION)=210
[PSMAP]  PS_AddParticleEx id=6 -> PPN[6]=210
```

**Every link in the chain is now verified:**
1. Release happens (`[TESTBOMB] pickle raised`, weapon selected on station 3).
2. Impact happens (`[MSLEND] BombImpact legacy branch`, BlastRadius 293).
3. The bomb reaches the impact-FX switch — the original defect was `type == TYPE_MISSILE`
   excluding bombs entirely, so **nothing** drew a bomb impact. That fix now demonstrably
   executes.
4. `PS_AddParticleEx(SFX_GROUND_EXPLOSION + 1)` resolves to a **valid** particle set
   (`PPN[6] = 210`), from a table built with **0 dropped** effects.

So the PO's "no fireball — the bomb disappears into terrain, then a pause, then the sound
of an explosion" has its cause repaired and the repair is confirmed running. What remains
is visual confirmation by the PO on a normal bombing run, which is now a low-risk check
rather than an open defect.

**Method note**: the useful move was abandoning the camera. Three runs were spent trying
to frame a 1,900-ft-elevation impact from an aircraft at 10,000 ft; one run measuring the
effect-ID mapping answered the question outright. When an observation is hard to stage,
check whether the thing itself can be measured instead of photographed.

**Open, and small**: the LODZ spread at impact (`physicsZ=-1852` vs `lod0=-1765`, 250 ft
across LODs) is still uninterpreted — `endCode=11 groundType=7` suggests a structure hit
rather than bare terrain, so it may be meaningless. Needs impacts on known flat ground
before it can be called anything.

### BOMB-1 CLOSED — bomb impacts land within 0.3 ft of the ground; the "87 ft gap" was my probe

```
[LODZ] impact (1730136,1209380) physicsZ=-1852.3 interp=-1851.9 delta=-0.3
       | lod0=-1765.0 lod1=-1753.0 lod2=-1763.0 lod3=-1601.0 lod4=-1607.0
groundType=7 = COVERAGE_THICKFOREST   (bare terrain, NOT a structure)
```

**The detonation is 0.3 ft from the interpolated ground level.** There is no
physics-vs-terrain gap at bomb impact.

**Two of my own errors, both corrected by this one measurement:**
1. I hypothesised the impact hit a *structure* (`endCode=11 groundType=7`). Decoding the
   enum shows `7 = COVERAGE_THICKFOREST` — bare terrain. The hypothesis was wrong and was
   never checked before being written down.
2. The "86-87 ft above terrain" figure came from comparing `physicsZ` against
   `FFPostZAtLOD`, which returns the **nearest post**. Physics collides against the
   **interpolated** surface. On terrain steep enough that the LOD posts span 164 ft, the
   nearest post and the interpolated height legitimately differ by tens of feet. The probe
   was measuring the wrong quantity, and the "gap" was an artefact of the instrument.

That is the fourth instance this session of a conclusion drawn from a measurement that
could not support it. The pattern is consistent enough to state as a rule: **before
believing a delta, confirm both sides of it are the same kind of quantity.**

**BOMB-1 is closed.** Release, impact, the impact-FX switch, and a valid particle system
(`PPN[6]=210`, 0 dropped) are all verified, and impacts land on the ground to within a
foot. The PO's original "no fireball" defect — bombs excluded from the impact switch
entirely — is repaired and the repair confirmed executing.

**EPIC TERRAIN-Z is now fully accounted for:**
- *takeoff/landing sinking into the runway* — fixed, `FF_RUNWAY_LODGATE` default ON, PO-confirmed
- *aircraft half-buried in the airstrip* — the gear overspeed trip; workaround confirmed by the PO (gear below 250 KIAS); three tuning decisions left with the PO
- *bombing, no fireball* — repaired and verified

### ASAN-3 — LOD gate is clean; found a pre-existing new[]/delete mismatch in the UI image loader

**The gate itself is memory-clean.** TE-09 "Landing Final Approach" under ASAN — the
mission where `FF_RUNWAY_LODGATE` fires hardest (603 coarse-answered samples measured
earlier) — reached sim with **0 ASAN errors**. The default flip is safe on the path that
exercises it most.

**But a second soak on TE-20 reported 5245 errors**, all the same defect and none of it
mine:

```
ERROR: AddressSanitizer: alloc-dealloc-mismatch (operator new [] vs operator delete)
    #1 C_Image::LoadImage(long, char*, short, short)  src/ui95/cimagerc.cpp:604
    #2 SelectTheater  src/ui/src/ui_main.cpp:2368
    #3 LoadAllTheaters / TheaterButtonCB
```

`LoadTargaFile` allocates with `data = new char[bytesToRead]` (`targa.cpp:92`), and both
call sites freed it with **scalar `delete`** (`cimagerc.cpp:604` and `:698`). That is
undefined behaviour once per image loaded, which is why a single visit to the theater
screen produces thousands of reports. Fixed to `delete[]` at both sites.

Checked the third scalar delete in the tree (`theaterdef.cpp:179`): it frees a single
`TheaterDef` from a linked list and is correct as-is. Left alone.

**Why it only appeared now, which is the uncomfortable part.** The TE-09 soak walks
straight into the mission; the TE-20 click script happens to route through the THEATER
screen first. ASAN-2 earlier used a campaign path and never touched it either. This was
not thoroughness — soak coverage is entirely determined by which UI path the click script
walks, and this one had never been covered. Worth a deliberate pass over the UI screens
rather than relying on incidental routing.

**Possible connection to THEATER-1/THEATER-2**: heap corruption during theater image
loading is consistent with the PO's original reports of stale landing-page art and map
imagery not updating after a theater switch. Not claimed — those were separately
diagnosed and fixed — but if theater art misbehaves again, this is now a known prior.

**Verification pending**: ASAN tree rebuilding; both soaks to be re-run to confirm the
count drops to 0. Committing the fix now with that status stated rather than after.

### ASAN-3 — four distinct allocator mismatches, all on the theater-switch teardown path

The soak that was meant to validate the LOD gate found four pre-existing memory-safety
defects instead. The gate itself is clean (TE-09, 0 errors). Every one of these fires
through `TheaterList::SetNewTheater` -> `FreeAllMissileData` / `FreeAllAirframeData` /
`FreeManeuverData`, i.e. **on every theater switch**.

| site | mismatch | count | fix |
|---|---|---|---|
| `cimagerc.cpp:604,698` | `new[]` vs `delete` | ~2 | `delete[]` — **verified gone** |
| `airframe.h` (AeroData, RollData), `missile.h` (Missile{Aero,Range,Engine}Data) — 18 sites | `new[]` vs `delete` | ~2500 | `delete[]` |
| `MissileAuxData` — 6 `SAFE_DELETE` sites | **`malloc` vs `delete`** | 1218 | `free()` |
| `digimain.cpp:899` FreeManeuverData — 3 sites | `new[]` vs `delete` | 215 | `delete[]` |

**The MissileAuxData one nearly got the wrong fix.** Having just corrected eighteen
`new[]`/`delete` sites, the obvious move was `delete[]` here too. Reading what ASAN
actually reported — `alloc-dealloc-mismatch (malloc vs operator delete)` — shows those are
`ID_STRING` fields allocated with `malloc()` (`datafile.cpp:58`). `delete[]` would have
replaced one undefined behaviour with another *and changed the error text*, so the fix
would have looked like it worked. Pattern-matching a fix across similar-looking sites is
how that happens.

**Verification discipline applied**: each member was confirmed array-allocated before its
`delete` was changed — `clift`, `times`, `thrust`, `velBreakpoints` were each checked
individually, and `theaterdef.cpp:179` was examined and **left alone** because it frees a
single linked-list node correctly. `Missile.h` is a 9-byte symlink to `missile.h`, so the
18 sites are 18, not 28.

**Possible bearing on THEATER-1/THEATER-2**: thousands of mismatched-allocator frees during
theater teardown is a plausible mechanism for the PO's original reports of missing
missions, stale landing-page art and Korea terrain under Balkans unit placement. Not
claimed — those were separately diagnosed and fixed — but recorded as a prior.

**How it was found is not a credit to method**: TE-09 and the earlier campaign soak walk
straight into the mission; only the TE-20 click script happens to route through the THEATER
screen. Soak coverage is decided by whichever UI path the script walks, and this one had
never been walked. A deliberate UI-screen pass is warranted.

**Verification pending**: ASAN tree rebuilding with all four fixes; the theater-path soak
will be re-run and the count reported as measured, zero or not.

### ASAN-3 VERIFIED — theater-switch teardown is now clean: 5243 -> 0

Re-ran the theater-path soak (TE-20, click script routing through the THEATER screen) on a
rebuilt ASAN tree with all five fixes:

```
mission: 20 Bombs with CCIP   reached sim: 1   ALL ASAN errors: 0
```

| stage | errors |
|---|---|
| baseline | 5243 |
| after cimagerc + airframe/missile headers + MissileAuxData + FreeManeuverData | 356 |
| after `EngineData::~EngineData` | **0** |

**Five defects, ~5200 undefined-behaviour events per theater switch, all pre-existing.**
Every one fired through `TheaterList::SetNewTheater`.

**`EngineData` is the one worth remembering.** Its destructor lives in `readin.cpp`, not
in the header, so a header-only search missed it — and it was *internally inconsistent*,
with `thrust[]`/`fuelflow[]` already using `delete[]` while `mach`/`alt` used scalar
`delete` three lines above. Someone had fixed half of it previously. The lesson is to
search implementation files for destructors too, which is what finally caught it.

**Proactive scan done rather than assuming five was the set**: enumerated every `ID_STRING`
field in the tree (these are `malloc`'d by `datafile.cpp:58`) and checked each for a
`delete`/`SAFE_DELETE`. Only the `MissileAuxData` group was affected, and it is fixed. The
`malloc`-vs-`delete` class is closed.

**Noted, not chased**: `TheaterDef`'s ~16 `ID_STRING` fields appear never to be freed at
all — a leak rather than a mismatch, invisible here because the repro runs with
`detect_leaks=0`.

**Bearing on the PO's theater reports**: this does not retroactively explain THEATER-1/2,
which were separately diagnosed and fixed. But ~5200 mismatched frees on every theater
switch is a real corruption source removed from exactly the operation the PO reported
misbehaving.

### ASAN-4 — deliberate UI-screen pass: found a sixth defect, now clean

ASAN-3's five defects were found by accident: one soak's click script happened to route
through the THEATER screen. That is not coverage, so this walks **every** main-menu screen
in one run — LOGBOOK, TACTICAL REFERENCE, ACMI, SETUP, COMMS, THEATER, TACTICAL
ENGAGEMENT, INSTANT ACTION, DOGFIGHT — returning toward the menu between visits.

**It found a sixth defect on the first pass:**

```
ERROR: AddressSanitizer: strncpy-param-overlap
  ranges [0x70a1e6c2d590,0x70a1e6c2d596) and [0x70a1e6c2d590,0x70a1e6c2d596) overlap
  #2 O_Output::SetText     ui95/ooutput.cpp:217
  #3 C_EditBox::SetText    ui95/ceditbox.cpp:629
  #4 SaveControlValues     ui/src/logbook/ui_lgbk.cpp:2042
```

`ui_lgbk.cpp:2042` calls `ebox->SetText(callsign)` where `callsign` is that same edit
box's own buffer, so `_tcsncpy` runs with `src == dst` — undefined behaviour — every time
the logbook is saved.

Fixed with a `txt not_eq Label_` guard **inside `O_Output::SetText`** rather than at the
call site, so every caller is covered; handing an object its own string back is unlikely
to be unique to the logbook.

**Severity is low and worth saying so**: a self-copy is effectively a no-op in practice.
It is still UB, and the fix is one comparison.

**Re-ran the full pass after the fix: 0 errors.**

**The finding that matters is about method, not this bug.** Six memory-safety defects have
now surfaced in two sessions, and *every one* was in code that no soak had deliberately
exercised. "We ran a soak" was being treated as "we have coverage" when the two are
unrelated — the soak's reach was whatever path its click script happened to walk. A screen
list is a weak form of coverage, but it is a stated one, and it paid immediately.

### Self-review of this session's changes — one hot-path getenv missed by my own cleanup

Reviewed the session's ten changed source files, separating behaviour changes from
debug-only additions. One real finding, in code I had already been editing:

`drawbldg.cpp:104` gated the **entire per-frame accurate-refetch block** on an uncached
`getenv("FF_RUNWAY_OLD")`:

```c
if (g_ffRunwayDbg && !getenv("FF_RUNWAY_OLD"))   // every flat surface, every frame
```

Earlier in the session I cached three `FF_DEBUG_RUNWAY` `getenv` calls in this same file
and walked straight past the one guarding the whole block — the most expensive of the four,
and on the exact path `FF_RUNWAY_LODGATE` now uses by default. Now cached like the rest.

**The pattern is worth naming**: I fixed instances of a defect while missing the instance
that mattered most, because I was matching on the *string* (`FF_DEBUG_RUNWAY`) rather than
on the *property* (uncached `getenv` in a per-frame path). Searching for the property found
it immediately.

Split of the session's source changes, for the record:
- **Behaviour**: `drawbldg.cpp` (LOD gate default ON + getenv caching), `cimagerc.cpp`,
  `airframe.h`, `missile.h`, `digimain.cpp`, `readin.cpp`, `ooutput.cpp` (six memory-safety
  fixes).
- **Debug/test only, all behind cached env flags**: `surface.cpp` (`FF_DEBUG_GEAR`,
  `FF_TEST_GEARDOWN`), `eom.cpp` (`FF_DEBUG_CHKHT`, `[GEAROVR]`), `missileendmsg.cpp`
  (`[LODZ]` interpolated ground).

`missileendmsg.cpp:351,379` still use uncached `getenv`, deliberately left: they run once per
missile impact, not per frame.

### ASAN-5 — sim-mode pass: 7/8 clean, and a heap-buffer-overflow READ in the weapon path

TESWEEP-4 covered all 34 missions in the **release** build (functional only). This is the
memory-safety half, over a spread chosen for distinct subsystems rather than for count.

```
row  1 basic-handling   asan=0      row 15 aim9         asan=0
row  2 takeoff          asan=0      row 19 bombs-ccrp   asan=0
row  9 landing          asan=0      row 22 guns-ag      asan=0
row 11 flameout-landing asan=0      row 26 harms        asan=1   <--
```

**TE-26 HARMs: heap-buffer-overflow READ of size 8**, `DrawableBSP::AttachChild`
(`drawbsp.cpp:108`), via `SMSBaseClass::AddWeaponGraphics` -> `GroundClass::Wake` ->
`UnitClass::Deaggregate`.

**The cause is sequencing, not a missing check:**

```c
ShiAssert(slotNumber < instance.ParentObject->nSlots);
ShiAssert((instance.SlotChildren) and (instance.SlotChildren[slotNumber] == NULL)); // reads here
if ( not instance.SlotChildren) return;
if (slotNumber >= instance.ParentObject->nSlots) return;   // guard, too late
```

The bounds test already existed — it was simply placed *after* the dereference, so the
assertion meant to catch the bad state performed the illegal read itself. `sms.cpp:727`
passes a hardpoint index the model has no slot for, which HARM stations exceed. Fixed by
moving the guards ahead of the indexing assertion; the check is preserved, just sequenced
correctly. **Re-ran TE-26: 0 errors.**

**Severity is higher than tonight's other finds and worth distinguishing.** The five
allocator mismatches were mismatched *frees* — undefined behaviour, but on memory we
owned. This reads *past the end of a heap buffer*, in a path that runs whenever ground
units deaggregate with HARMs loaded. That is a credible source of hard-to-attribute
instability.

**Seven of eight clean means this is not systemic** — it is one weapon path nothing had
exercised. Third time this session that deliberate coverage found what incidental coverage
walked past.

### ASAN-6 — campaign soak clean under heavy deaggregation (3424 events, 0 errors)

Ran the campaign flight soak on the current build (all seven of this session's memory fixes
in place). Reached 3D, ran to the 420 s timeout.

```
reached sim: 1        ASAN errors: 0
deaggregation/unit-wake trace hits: 3424
```

**The path was genuinely exercised**, which is the check that matters: 3424 deaggregation
events means the entity-lifecycle code — the family the HARMs overflow lived in — ran hard
and stayed clean. A zero from code that never executed would be worthless, and that
distinction has bitten four times this session.

**Prediction was wrong, and that is informative.** I expected finds to cluster in entity
lifecycle, on the reasoning that campaign runs far more deaggregation than any TE. It came
back clean, which supports the HARMs overflow having been a **one-off slot-index case**
(a hardpoint index exceeding the model's `nSlots`) rather than the tip of a systemic
lifecycle problem.

**One of my own checks was worthless and is called out rather than quietly dropped**:
grepping the log for `AddWeaponGraphics` / `CreateVisualObject` returned 0, but those
functions emit no logging, so the grep measured nothing at all. It cannot show the specific
HARM-armed-unit condition was reached. So this run does **not** independently re-verify the
`AttachChild` fix — TE-26 does that, and did.

**Coverage now stated rather than incidental**, four scripts:
- `te-sweep.sh` — 34 TE missions, functional, release build
- UI-screen pass — every main-menu screen, ASAN
- `asan-sim-pass.sh` — 8 missions across distinct subsystems, ASAN
- `camp-fly-asan.sh` — campaign flight with entity churn, ASAN

### STATIC-1 — tree-wide scan for this session's defect classes: no further in-game instances

Scanned the whole tree for the `new[]`-allocated-member-freed-with-scalar-`delete` class
that accounted for six of the seven memory fixes. 288 array-allocated member names, 53
raw candidates, narrowed to 18 where the *same file* allocates that name with `new[]`.

**Triage:**
- 13 are in `src/tools/` (fontmunge, ui_tools) and `src/campaign/camptool/` — build tools,
  not the shipped game. Real, but not in the binary the PO flies.
- **5 were in-game, and all five are false positives.**

**Four (`falcsess.cpp` `name`/`callSign`) are inside comment blocks** — note the `*/`
terminating line 377, and one `// delete callSign;`. The grep matched dead code.

**The fifth (`realweather.cpp:318` `delete metar`) needed real checking.** The `delete` is
live, but its apparent array allocation at line 1676 sits inside a `/* */` opened at 1668
and not closed until after. Enumerating every `metar =` assignment and testing each for
comment enclosure:

```
line 294   live        metar = NULL;
line 1672  COMMENTED   metar = NULL;
line 1676  COMMENTED   metar = new METAR[numMETARS];
```

`metar` is only ever `NULL` in live code, so `delete metar` always deletes a null pointer —
safe, and **not** a defect.

**The method point**: a `grep` for a code pattern will happily match commented-out code, and
this codebase has a great deal of it. Earlier this session I inserted two probes *into* a
comment block for the same reason. Any static candidate list needs a comment-enclosure test
before anything is called a finding — the check is cheap (`awk` tracking the last `/*` vs
`*/` before the line) and it eliminated 5 of 5 here.

**Result: the `new[]`/`delete` class is closed for the shipped game.** The tools instances
are recorded above but not fixed, being outside the game binary.

### STATIC-1 (b) — the assert-before-bounds-check class: one more instance, in the sibling function

Scanned for the shape that made `AttachChild` a heap-buffer-overflow: an assertion that
**indexes an array using a variable whose bounds check appears later in the same function**.

Naive greps were useless here — 195 `ShiAssert` calls index arrays, and filtering for "a
guard within 6 lines" returned ten hits whose guards were null-checks on unrelated things.
The precise test is narrower: extract the *index variable* from the assertion, then look for
a later `if (thatVariable >= limit) return`. That returns **exactly one** candidate across
the whole game tree.

`drawbsp.cpp:137`, `DrawableBSP::DetachChild` — the sibling of the function fixed earlier:

```c
ShiAssert((instance.SlotChildren) and (instance.SlotChildren[slotNumber] == &child->instance));
...
if (slotNumber >= instance.ParentObject->nSlots) { return; }        // 7 lines later
```

Same defect, same file, one function down. **ASAN never caught this one** — nothing in any
run called `DetachChild` with an out-of-range slot — so it is latent rather than observed,
and that is precisely what a static scan is for. Fixed identically: guards first, indexing
assertion after.

**Worth noting what made the scan work.** The first two attempts produced 195 and 10 hits,
both useless. Narrowing from "an assertion containing brackets" to "an assertion whose index
variable is bounds-checked later" cut it to one, and that one was real. A scan that returns
a long list has not found anything; it has just moved the work.

### STATIC-1 (c) — uncached getenv in per-draw paths: four cached, the rest deliberately left

Third defect class from this session: `getenv` called on a hot path, which was found in
`drawbldg.cpp` where one gated the entire per-frame accurate-refetch block.

**Filtering mattered more than searching.** 116 uncached `getenv` calls tree-wide, 71 in
`graphics/` or `sim/` — a list that size is not a finding, it is unreviewed work handed to
the next person. Cost depends on **call frequency**, not on which directory the file lives
in, so narrowing to `getenv` inside `Draw`/`Render`/`Exec`/`Update` functions gives 13, and
flagging those additionally inside a loop gives the genuinely hot ones.

**Cached (per-draw or per-object, every frame):**
- `bspnodes.cpp:323` — `BRoot::Draw`
- `drawplat.cpp:149,212` — both `DrawablePlatform::Draw` paths
- `compat/d3d_gl.cpp:5089` — `DrawVertices`, the hottest path in the renderer

**Deliberately not changed**: the remaining ~100. They sit in per-frame-once paths
(`RenderFrame`, `RenderFirstFrame`), one-shot setup, or non-hot code, where a single
`getenv` per frame is not worth the churn or the risk of touching working code. Listing
them as "candidates" without that judgement would be the same mistake as reporting 195
assertion hits.

This closes STATIC-1. Of the three classes carried over from the ASAN work: the
`new[]`/`delete` class is **closed for the shipped game** (all in-game candidates were false
positives, four of them commented-out code); the assert-before-bounds-check class yielded
**one real latent bug** (`DetachChild`); and the hot-path `getenv` class yielded **four**
worth caching out of 116 candidates.

### STATIC-1 triage verified — `tools` and `camptool` really are out of the Linux build

STATIC-1 dismissed 13 `new[]`/`delete` candidates as "build tools, not the shipped game".
That was an assumption; now checked:

```
src/CMakeLists.txt:37       if(WIN32)  add_subdirectory(tools) ... endif()
src/campaign/CMakeLists.txt:6  if(WIN32)  add_subdirectory(camptool)  endif()
```

Both are Windows-gated, so `src/tools/` (fontmunge, ui_tools) and
`src/campaign/camptool/` are not compiled into the Linux binary. The 13 candidates are
real defects in code that is not built here, and leaving them is correct rather than
merely convenient.

Worth checking because `camptool` sits *under* `campaign`, which **is** built — the path
alone would have suggested it ships.

### NVG-5 — instrument hardened before the run: the census now announces truncation

NVG-5 has resisted five theories, and its own ticket prescribes the one approach never
tried: **measure the per-stage state at the 2D draw** rather than propose a sixth
hypothesis. The instrument for that already exists (`FF_DEBUG_2DCENSUS`), built in an
earlier sprint precisely to "diff cleanly between `FF_NVG_DXSTATE=1` and not", and the
insight it was built on still stands: *the key-blue backdrop is drawn in both modes, so
what differs must be the draw that normally covers it.*

**Before running it, one hazard was worth removing.** The census records up to 256 distinct
draw signatures, and its own comments note it **saturated that cap twice** during
development. A saturated census reports **absence as if it were data** — a draw missing
from the list because the cap filled is indistinguishable from a draw that never happened,
which is exactly the comparison the instrument exists to make. It would have produced a
confident, wrong diff.

Added a one-shot warning when the cap is reached, so a truncated run announces itself:

```
[2DCENSUS] *** SIGNATURE CAP (256) REACHED -- census is TRUNCATED,
                absence from this list proves nothing ***
```

This is the same failure this session hit repeatedly in other forms — a probe that only
runs near the ground, a frame the camera could not have contained, a delta between two
different kinds of quantity. In each case the measurement was silently incapable of
supporting the conclusion drawn from it. An instrument that can saturate should say so.

Run pending: the ASAN sim pass currently holds the machine.

### ASAN-7 — `PilotInfo[-1]` read during campaign load (TE-25), and a prediction corrected

`row 25 Laser-Guided Bombs  asan=1` — the only hit in the resumed pass so far.

**It is not a weapon bug.** The stack runs
`CampaignClass::LoadCampaign` -> `LoadUnits` -> `NewUnit` -> `SquadronClass` ctor ->
`InitPilots` -> `GetAvailablePilot`, i.e. **campaign pilot assignment during load**. My
stated prediction — that remaining finds would cluster in weapon-specific paths, as HARMs
did — is wrong. Mavericks, Rockets, Dive-Toss, CCIP, AMRAAM and Sparrow all came back clean;
the one hit is in campaign data loading that this mission happens to exercise.

**The defect** (`pilot.cpp:246`):

```c
if (best_pilot > -1)
    PilotInfo[best_pilot].usage++;          // guarded

if (PilotInfo[best_pilot].voice_id == 255)  // NOT guarded -> PilotInfo[-1]
    PilotInfo[best_pilot].AssignVoice(owner);
```

`best_pilot` is initialised to -1 and stays there when the loop finds no candidate, so the
`voice_id` test reads one element *before* the array — ASAN: heap-buffer-overflow READ of
size 1. Worse, had the read returned 255, `AssignVoice` would have **written** through the
same out-of-range index. Fixed by extending the existing guard to cover both accesses.

**Third instance this session of the same shape**: a bounds/validity check that exists but
does not extend to the access beside it — `AttachChild`, `DetachChild`, and now
`GetAvailablePilot`. Worth a targeted scan of its own: guarded statement followed by an
unguarded use of the same index.

### Scan for "guard does not cover the adjacent access" — found nothing, and that is weak evidence

Three defects this session shared one shape: a validity check that exists but does not
extend to the access beside it — `AttachChild`, `DetachChild`, `GetAvailablePilot`. Three in
one session warranted a targeted scan.

**Result: 1 candidate, verified a false positive.** `munition.cpp:1839` looked like a
single-statement guard followed by an unguarded use, but the guard's body is itself an `if`
whose braces sit on the next line, so the "unguarded" use is nested two deep inside it.

**This does not close the class, and should not be recorded as if it did.** The heuristic
only matches *single-statement* guards with the access on the very next line — the narrowest
possible form. The three real instances were each different: an assertion ahead of a bounds
check (`AttachChild`), the same in a sibling function (`DetachChild`), and a guarded
statement followed by a separate unguarded `if` block (`GetAvailablePilot`). A pattern that
varies that much in surface form is not reliably greppable.

**Conclusion worth keeping**: for this defect class, ASAN is the better detector and static
scanning is the weaker one — the reverse of the `new[]`/`delete` class, where the static scan
closed the question and ASAN had only ever found what it happened to execute. Neither tool
dominates; they fail in different directions, and a clean result from the weaker one for a
given class means very little.

### ASAN-7 — heap-use-after-free during aircraft death (TE-29), logged not patched

`row 29 Offensive BFM  asan=5` — five heap-use-after-free READs, the most serious class
found this session.

**Reader:**
```
UnitProxFilter::RemoveTest      camplib/camplist.cpp:304
VuGridTree::Move                vu2/vu_grid_tree.cpp:54
VuCollectionManager::HandleMove vu2/vu_collection_manager.cpp:123
VuEntity::SetPosition           vu2/vuentity.cpp:395
SimBaseClass::SetRemoveFlag     simlib/simbase.cpp:533
AircraftClass::SetDead          aircraft/virtuals.cpp:856
AircraftClass::Exec             aircraft/aircraft.cpp:1867
```

An aircraft dying triggers a grid-tree move whose proximity filter reads freed memory.

**What makes it worth care rather than a quick patch**: ASAN says the freed chunk is a
**texture pixel buffer** —

```
allocated: D3D7Surface::AllocatePixelBuffer <- DDS7_Lock <- TextureHandle::Reload <- PaletteHandle::Load
freed:     D3D7Surface::~D3D7Surface <- DDS7_Release <- TextureHandle::~TextureHandle <- CPLight::DiscardLit
```

A campaign proximity filter reading into a freed *texture* buffer is not semantically
sensible. Either a pointer in the VU/campaign layer is dangling into memory that was
recycled, or something upstream is already corrupt and this is a symptom rather than the
cause.

**Deliberately not fixed in this sprint.** Adding a null/validity check at
`camplist.cpp:304` would silence the ASAN report while leaving whatever produces the
dangling pointer intact — converting a detectable fault into an invisible one. That is
strictly worse than the current state.

**Filed as UAF-1** with the next steps stated: reproduce row 29 to confirm it is not a
one-off, then establish which object `RemoveTest` believes it is reading before touching
anything.

**Also worth noting against my earlier prediction**: this is the second find in the resumed
pass, and neither was weapon-specific — TE-25 was campaign pilot loading and TE-29 is entity
death handling. The weapon-path hypothesis from ASAN-5 has now been contradicted twice.

### ASAN-7 COMPLETE — all 34 TE missions now covered under ASAN; UAF-1 is intermittent

The resumed pass finished 20 missions. Combined with the earlier partial (rows 3–8) and
ASAN-5 (rows 1, 2, 9, 11, 15, 19, 22, 26), **every one of the 34 TE missions has now been
run under AddressSanitizer** — a stated coverage claim rather than an incidental one.

**Findings across all 34:**

| mission | finding |
|---|---|
| 26 HARMs | heap-buffer-overflow READ in `DrawableBSP::AttachChild` — **fixed** |
| 25 Laser-Guided Bombs | `PilotInfo[-1]` read in `GetAvailablePilot` — **fixed** |
| 29 Offensive BFM | 5× heap-use-after-free during aircraft death — **UAF-1, open** |

The other 31 are clean.

**UAF-1 does not reproduce on demand.** Re-ran row 29 twice with the *identical* binary —
`build-asan` dates from 04:40, while the `pilot.cpp` and `drawbsp.cpp` fixes are 21:24 and
20:44, so neither is in it and the comparison is clean:

```
original run: asan=5 (heap-use-after-free)
run A:        asan=0
run B:        asan=0
```

**One occurrence in three runs of the same mission with the same binary.** So it is
timing-dependent — consistent with the reader being on thread **T15**, the VU thread, racing
entity teardown. That materially changes how it must be chased: a fix cannot be validated by
"the error went away", because the error is absent two runs in three anyway. Any candidate
fix needs either many runs or a deterministic reproduction first.

Recorded on UAF-1 rather than left as an assumption that it reproduces.

### NVG-5 — the census was watching the wrong draw path, which explains the failed attempts

Ran the prepared census A/B (`FF_DEBUG_2DCENSUS`, `DX_NVG` skipped vs `FF_NVG_DXSTATE=1`,
NVG toggled at 70 s). Result:

```
A (default):        1 census line, no truncation
B (pipeline live):  1 census line, no truncation   -- IDENTICAL
```

**That is not "no difference found".** The symptom demonstrably occurs in run B — captured
frames either side of the toggle show `blue-ish=0` before and `blue-ish=1` after, with frame
mean dropping 0.329 -> 0.168. The screen visibly fills blue while the census reports one
identical signature in both configurations.

**So the instrument is not observing the relevant draws.** The census sits in
`DrawIndexedPrimitiveVB` and only records draws where `dev->textures[0]` is set (or the
key-blue untextured backdrop). One textured 2D draw class in 100 seconds means the cockpit
panel is not passing through that function at all.

The BLUE-1 note said as much and it was read past: *"the 2D cockpit panel drawn afterwards
through the **MPR path**"*. That path is `ContextMPR::FlushPolyLists -> RenderPolyList`
(`3dlib/context.cpp:2649,2659`), a different renderer entirely from the D3D vertex-buffer
entry point the census instruments.

**This is why five theories and this census all failed**: every one of them measured or
reasoned about the D3D stage-state path, while the draw that loses its chroma key goes
through `RenderPolyList`. The measurement has to move to `context.cpp`, not be refined where
it is.

**Recorded as the concrete next step for NVG-5**, replacing "measure the per-stage state at
the 2D draw" — which was right in intent and wrong about *which* draw. No further theories
until the census is on the MPR path and produces a real diff.

### GEAR-5 / GEAR-6 — PO decisions applied

PO ruled on the three open balance questions (2026-08-26):

| decision | ruling |
|---|---|
| 275 kt gear limit vs real ~300 KIAS | **leave 275** — no flight-model change |
| `vt` (true) compared against a `MinVcas`-derived (calibrated) limit | **fix** — correctness, not balance |
| GEAR-6 unreachable break branch | **fix** — accept harsher gear damage |

**Airspeed type** (`eom.cpp:1618`): now compares `vcas * KNOTS_TO_FTPSEC` against
`gearLimitSpeed` instead of `vt`. The threshold itself is untouched, per the PO. Verified
the trip still fires where it should — at 307 KIAS it now reads
`vcas=307.2 kt limit=274.9 kt` rather than testing `vt=315.6`. The practical effect is that
the limit no longer tightens with altitude, which is what the mismatch was doing.

**GEAR-6** (`eom.cpp:1485`): the strength tests were ordered `< 50 (stuck)` then
`else if < 0 (broken)`, making the break branch unreachable — anything below 0 is also below
50. Ground-roll damage could therefore only ever *stick* the gear, and the gear-snaps-off
sound plus the DOF-to-zero collapse were dead code. Reordered most-severe-first. The gear
now breaks when its strength is exhausted, which is plainly what the sound effect and the
DOF collapse were written for.

TE-02 re-run after both: reaches sim, 0 crashes.

**Not changed, deliberately**: the 275 kt threshold, and there is still no gear-limit field
in the aero data — the limit remains improvised from `MinVcas`, the *minimum* comfortable
speed. Recorded so the next person does not rediscover it as a bug.

### NVG-5 — MPR census works, and narrows the fault away from poly-list state

Moved the census onto the path the cockpit panel actually uses
(`ContextMPR::RenderPolyList`, `3dlib/context.cpp:2998`), keyed on distinct
`(renderState, textured)` pairs, with a 512 cap that announces truncation.

```
A (DX_NVG skipped):   2 distinct, no truncation
B (FF_NVG_DXSTATE=1): 2 distinct, no truncation
diff: renderState=37 textured=1  in BOTH  (texID differs, but that is a raw
      pointer value that varies per run and is not part of the dedup key)
```

**The poly-list state is identical between the two configurations.** The panel is submitted
with the same `renderState` and the same textured/untextured classification whether the
`DX_NVG` pipeline is live or skipped — yet the screen fills blue in one and not the other.

**So the fault is not in *which* state the panel is drawn with.** That rules out a whole
family of candidate explanations: it is not that NVG causes a different poly-list state to
be selected, or that the panel loses its texture binding at submission. The difference must
lie downstream — in how `ApplyStateBlock(renderState)` translates that same state into GL
state while the NVG stage configuration is active, or in the texture's alpha content itself.

That is a genuine narrowing rather than another theory: it is a *negative* established by
measurement on the correct renderer, after five theories and one census that were all
measuring the wrong one.

**Next**: instrument `ApplyStateBlock` for `renderState=37` specifically and diff the
resulting GL state (alpha test func/ref, blend, texture env) between the two
configurations. The panel state is the same going in, so the divergence is in the
translation.

### NVG-5 — the `lastState` cache is NOT the cause (sixth theory, refuted in one run)

`ContextMPR::ApplyStateBlock` skips reapplying when the requested state matches the last
one MPR applied. Since `DX_NVG` configures texture stages directly via
`SetTextureStageState` — outside that path — the cache looked like an excellent candidate:
`lastState` would still claim state 37 is current while the stage config had been replaced
underneath, and the panel would draw with NVG's setup and lose its chroma key. That is
precisely the BLUE-1 description.

Added `FF_MPR_NOSTATECACHE=1` to force reapplication and tested it:

```
FF_NVG_DXSTATE=1                          view1 mean=0.168049 blueish=1
FF_NVG_DXSTATE=1 FF_MPR_NOSTATECACHE=1    view1 mean=0.168049 blueish=1
```

**Identical to six decimal places.** The runs are deterministic, so this is a genuine
negative rather than noise. The cache is not the cause.

**And the negative is informative.** Forcing the state block to be reapplied on *every*
poly — which is the strongest possible version of "restore MPR's state" — still leaves the
panel blue. So `StateTable[37]`, the captured D3D state block, **does not contain whatever
`DX_NVG` changes**. Reapplying it cannot undo the NVG stage configuration because that
configuration is not part of what the block captures.

**Next**: determine what `StateTable[]` actually captures (`StateSetupCounter` /
`StateTableInternal` in `context.cpp:80-82`) versus which stage states `DX_NVG` sets
(`dxengine.cpp:708-817`). The question is now specific: which states does NVG set that the
MPR state block does not restore?

Sixth theory refuted, but in one run rather than a sprint — because the measurement was on
the correct renderer and the symptom is deterministic.

### NVG-5 — seventh theory refuted, and the symptom is probably framed wrong

Tested the strongest remaining stage-state theory: MPR's per-state blocks configure **stage
0 only** (`context.cpp:591+`), while `DX_NVG` configures **stage 3** (`COLOROP ADDSIGNED`,
`COLORARG2 TFACTOR`) and moves the alpha source there (`dxengine.cpp:740-760`). Stages 1-7
are disabled once at init (`context.cpp:248`) and never again — so once NVG enables stage 3,
nothing in MPR turns it off. That also explained why forcing reapplication changed nothing:
a stage-0 block cannot undo a stage-3 setup.

`FF_MPR_DISABLE_STAGES=1` disables stages 1-7 before every state-block application:

```
FF_NVG_DXSTATE=1                            mean=0.168049 blueish=1
FF_NVG_DXSTATE=1 FF_MPR_NOSTATECACHE=1      mean=0.168049 blueish=1
FF_NVG_DXSTATE=1 FF_MPR_DISABLE_STAGES=1    mean=0.168134 blueish=1
```

**No effect.** Seventh theory refuted.

**The reframing this forces, which is the useful output.** Every theory so far has assumed
the symptom is *"the 2D panel loses its chroma key and its blue backdrop shows"*. But the
earlier census established that **the key-blue backdrop is drawn in both configurations** —
what differs must be the draw that normally **covers** it. That is the *world*, not the
panel.

So the question may not be "why does the panel lose its key" but **"why does the 3D world
stop covering the backdrop when DX_NVG is live"** — a world-rendering failure showing the
pit's blue through, rather than a panel-transparency failure. The `[3DCENSUS]` data shows 3D
draws *happen* under NVG (`units=3`), but happening is not the same as producing visible
output: if the NVG stage math yields black or fully-transparent fragments, the world would be
invisible and the backdrop would show.

**Next**: verify whether the world renders at all under `DX_NVG` — capture with the pit
hidden, or compare a world-only region of the frame between configurations. That
distinguishes "panel transparent" from "world invisible", which seven theories have not.

Recorded because after seven refutations the problem is more likely in the framing than in
the next hypothesis.

### NVG-5 RE-SCOPED — it is not the panel. The WORLD fails to render under DX_NVG.

Measured the two regions of the frame separately instead of judging the whole image, with
NVG on in both configurations:

| region | `FF_NVG_DXSTATE=1` | default (DX_NVG skipped) |
|---|---|---|
| WORLD (top) | r=0.000 g=0.139 **b=0.472**, **sd=0.059** | r=0.275 g=0.424 b=0.243, **sd=0.192** |
| PIT (bottom) | r=0.000 **g=0.300** b=0.076 | r=0.004 g=0.218 b=0.003 |

**The cockpit panel is GREEN in the "blue screen" frame** — `g=0.300` against `b=0.076`. It
renders correctly, NVG-green, chroma key intact. **The panel is not the problem and never
was.**

**The world region is blue and nearly featureless** — `sd=0.059` versus `0.192` normally.
Low variance means no terrain detail: the world is not drawing, and the pit's key-blue
backdrop shows through where it should be.

**This invalidates the ticket's title and all seven theories.** NVG-5 has been recorded
since the first sprint as *"2D cockpit panel loses its chroma key"*, and every hypothesis —
the alpha stage, the chroma key, the alpha source, the missing covering draw, stage-0
starvation, the `lastState` cache, the stage-3 configuration — targeted the panel or its
state. The panel was rendering correctly the whole time.

The earlier census said as much and was not followed through: *"the key-blue backdrop is
drawn in both modes; what differs must be the draw that COVERS it."* The covering draw is
the world.

**Re-scoped**: why does the 3D world stop producing visible output when `DX_NVG` is live?
`[3DCENSUS]` shows 3D draws still occur (`units=3`), so they execute but yield nothing
visible — consistent with the NVG stage math (`ADDSMOOTH` / `ADDSIGNED` / `DOTPRODUCT3`
implemented in NVG-2) producing black or transparent fragments for world geometry.

**The method lesson, third time this session**: the symptom description was accurate as a
description and wrong as a diagnosis — exactly like "physics terrain sits below graphics
terrain", which turned out to be landing gear. Measuring *regions* rather than the whole
frame took one command and overturned seven sprints of hypotheses.

### NVG-5 — a control saved a false negative: `FF_NVG_NOVTX` does nothing

Next candidate after the re-scope: the world path masks vertex colour to green-only under
NVG (`context.cpp:3364`, `color &= 0xFF00FF00; color |= 0x0000B400`), and `DX_NVG`'s stage 0
is `ADDSMOOTH(DIFFUSE, DIFFUSE)` — so a diffuse with red and blue zeroed, fed through
`ADDSMOOTH` and then `DOTPRODUCT3`, is a plausible way to blank the world. `FF_NVG_NOVTX=1`
exists to suppress that tint.

Tested it with `FF_NVG_DXSTATE=1`: world region **identical to six decimals**
(`sd=0.0590981`, `b=0.472213`). That looked like a clean eighth refutation.

**It was not a refutation. It was a broken test.** Ran the control — the same flag in the
*default* configuration, where the world is green and clearly visible:

```
default, no flag:            r=0.275 g=0.424 b=0.243 sd=0.192
default, FF_NVG_NOVTX=1:     r=0.275 g=0.424 b=0.243 sd=0.192
```

**Identical.** The flag has no effect in the configuration where its effect would be
obvious, so that code path is not reached (or the tint is not what greens the world —
consistent with the earlier finding that `GreenMode` tints via the colour-bank palette swap,
`ColorPool = GreenTVBuffer`, not via vertex colour).

So the vertex-tint hypothesis is **untested**, not refuted, and `FF_NVG_NOVTX` is a dead
lever for this scenario.

**Why this matters more than the theory**: without the control, "no change with the flag"
would have gone into the record as evidence against vertex tinting, and the next person
would have skipped it. A negative result from a lever that does not move is not a negative
result. Cheap rule: **before believing that turning something off changed nothing, prove the
switch is connected** — test it where its effect should be visible.

### NVG-5 — the NVG texture ops are not what blanks the world (verified lever this time)

After `FF_NVG_NOVTX` turned out to be a dead switch, the next lever was checked *before* use.
Added `FF_DEBUG_NVGOPS=1`, a one-shot trace per op arm:

```
with FF_NVG_DXSTATE=1 :  ADDSMOOTH (stage 0), ADDSIGNED (stage 1), DOTPRODUCT3 (stage 1)
without               :  none
```

All three ops execute, and only under NVG — so `FF_NO_TEXOP_FIX=1` (which falls them back to
`MODULATE`) has something real to act on, and the control is clean.

**Result — the lever acted, and the theory is refuted:**

| config | WORLD region |
|---|---|
| `FF_NVG_DXSTATE=1` | sd=0.059  b=0.472  g=0.139 |
| `+ FF_NO_TEXOP_FIX=1` | sd=0.094  **b=0.950**  g=0.013 |
| default (world visible) | sd=0.192  g=0.424 |

Bypassing the ops made the world **more** blue, not less. The output changed materially, so
this is a trustworthy negative rather than another disconnected switch — and the world still
does not come back. If anything the NVG ops are partially *helping*; with them replaced by
`MODULATE` the world region is nearly saturated backdrop blue.

**So the blanking is upstream of the texture-environment stage configuration.** The ops
execute, changing them changes the picture, and no setting of them restores the world. That
points at what is *fed into* the stages — geometry, vertex data, or the draw being culled or
depth-rejected — rather than at how the stages combine it.

**Next**: check whether world geometry is reaching the rasteriser at all under `DX_NVG` — a
depth/blend/cull state difference would blank it regardless of texture-env setup, and
`3DCENSUS` counts draws submitted, not fragments produced.

Eighth NVG theory, but the first refuted with a lever proven connected beforehand.

### NVG-5 — fog ruled out, with a note on what the control could and could not show

`DX_NVG` sets `FOGENABLE = TRUE` (`dxengine.cpp:745`), and fog blends fragments toward a
constant colour — exactly what "uniform, low variance" looks like. Strong candidate.

```
FF_NVG_DXSTATE=1                  WORLD sd=0.0591  b=0.4722
FF_NVG_DXSTATE=1 + FF_NO_FOG=1    WORLD sd=0.0591  b=0.4722   (identical)
default          + FF_NO_FOG=1    WORLD sd=0.1920  g=0.4242   (identical to default)
```

**The control could not demonstrate the lever acting** — `FF_NO_FOG` changed nothing in the
default configuration either. But unlike `FF_NVG_NOVTX`, the implementation was **verified by
reading**: `d3d_gl.cpp:3610` genuinely skips `glEnable(GL_FOG)` when the flag is set. The
absence of visible change in the control is explained by fog being negligible in a clear
daylight scene, not by a disconnected switch.

**So this is a refutation, but on weaker footing than the texture-op one.** The reasoning is:
if fog were blanking the world under `DX_NVG`, disabling fog would have changed that result;
it did not, and the code path that disables it is correct. That is sound, but it rests on code
inspection rather than on an observed control — worth stating plainly rather than filing
alongside evidence of a different quality.

**Two kinds of "nothing changed" now distinguished:**
- `FF_NVG_NOVTX` — switch not connected, result **meaningless** (theory untested)
- `FF_NO_FOG` — switch correct but effect invisible in this scene, result **valid but weaker**
- `FF_NO_TEXOP_FIX` — switch connected *and* demonstrably moved the output, result **strong**

Ninth NVG hypothesis. The blanking is still upstream: not texture-env, not fog.

### NVG-5 — the world is NOT missing: it is rasterised and mis-coloured by the 3-unit combine

Extended `[3DCENSUS]` to report the states that actually reject fragments — depth, blend,
cull, alpha — since the census had only ever counted draws *submitted*.

```
A (world visible):  units=0 ...  |  units=1 ...
B (world blank):    units=0 ...  |  units=1 ...  |  units=3 texStage0=yes texStage1=yes

all classes, both configs:  zTest=1 zFunc=0x203 zWrite=1 blend=0 aTest=0 cull=0
```

**Every fragment-rejection state is identical.** Nothing is being depth-rejected,
alpha-tested away, culled or blended out. And with `blend=0` and `zWrite=1`, whatever those
draws produce is written **opaquely** into the framebuffer.

**So the world is not missing — it is being drawn in the wrong colour.** The sole difference
is the extra `units=3` draw class: under `DX_NVG` the world goes through the three-unit
multitexture combine, and that combine yields a blue-dominant, low-variance result
(`b=0.472`, `sd=0.059` against `0.192`). Low variance is the tell — `DOTPRODUCT3` collapses
RGB to a scalar, which flattens terrain detail exactly as observed.

**This corrects my own re-scope from earlier today.** I moved the ticket from "panel loses
its chroma key" to "the world fails to render", on the reasoning that the blue was the pit's
backdrop showing through. It is not: the world is rendering, over the top, in the wrong
colour. The blue is world geometry rendered blue, not backdrop revealed.

It also fits the texture-op result that looked odd at the time: bypassing the ops to
`MODULATE` pushed the region *further* blue (`b=0.950`) rather than restoring it — consistent
with changing a combine that is producing colour, not with restoring a draw that is absent.

**Re-scoped again, and now narrowly**: which of the three units in the NVG combine produces
the blue-dominant result for world geometry? The Wine reference shows the correct output is
green with detail preserved, so this is now a direct comparison of a colour pipeline against
a known-good image rather than a hunt for a missing draw.

### NVG-5 — not the multi-unit combine: the blue originates at unit 0 / the DIFFUSE input

`FF_NVG_MAXUNITS=<n>` caps how many texture units a world draw uses, disabling the rest
immediately before the draw:

| cap | WORLD |
|---|---|
| 1 | sd=0.065  r=0.0004  g=0.139  b=0.473 |
| 2 | sd=0.062  r=0.0003  g=0.139  b=0.472 |
| 3 | sd=0.062  r=0.0003  g=0.139  b=0.472 |
| uncapped | sd=0.059  r=0.000  g=0.139  b=0.472 |
| **correct (default)** | **sd=0.192  r=0.275  g=0.424  b=0.243** |

The lever acted (r moves off exactly zero, sd shifts) but **one texture unit is just as blue
as three**. So the three-unit combine is not what produces the blue — ruled out.

**Which leaves unit 0 and what feeds it.** In the working configuration the census also shows
`units=1` draws and the world is green; capped to one unit under `DX_NVG` it is blue. The
difference is that unit 0's texture-env is still configured as NVG's
`ADDSMOOTH(DIFFUSE, DIFFUSE)`.

**And this connects to the dead lever from earlier.** `FF_NVG_NOVTX` — which suppresses the
world's green vertex tint (`color &= 0xFF00FF00; color |= 0x0000B400`) — was shown to do
nothing, meaning **that tint path is never reached**. So under `DX_NVG` the world's `DIFFUSE`
is *not* green-tinted, and `ADDSMOOTH(DIFFUSE, DIFFUSE) = D + D - D*D` on an untinted,
sky-and-terrain-coloured diffuse produces exactly a washed blue-dominant result with detail
flattened.

That also explains the otherwise odd texture-op result: replacing the ops with `MODULATE`
pushed it *further* blue (`b=0.950`), which is what squaring an untinted diffuse would do.

**Hypothesis, now specific and testable**: the world's vertex colour is not being NVG-tinted
on the path `DX_NVG` uses, so stage 0 combines an untinted diffuse. **Next**: log the actual
`DIFFUSE` value on world draws in both configurations. If it is green in one and not the
other, that is the defect.

Two threads that each looked like dead ends — a lever that did nothing, and a combine that
was not to blame — turn out to be the same finding from opposite directions.

### NVG-5 — the untinted-DIFFUSE hypothesis is refuted; and a caveat on the sample

Logged the world draw's `DIFFUSE` in both configurations:

```
A (world green, correct):  units=0 diffuse=ffffffff | units=1 diffuse=ffffffff
B (world blue):            units=0 diffuse=ffffffff | units=1 diffuse=ffffffff | units=3 diffuse=ffffffff
```

**`DIFFUSE` is opaque white in both**, including the configuration where the world renders
correctly green. So the world's green does not come from vertex colour at all — consistent
with the earlier finding that `GreenMode` tints via the colour-bank palette swap
(`ColorPool = GreenTVBuffer`), and with `FF_NVG_NOVTX` being a no-op.

**That refutes the hypothesis from the previous entry.** With `D = white`,
`ADDSMOOTH(D, D) = D + D - D*D = 1`, i.e. white — not blue. An untinted diffuse cannot be
squaring into the blue result, because the diffuse is the same white in the working case.

**Caveat, stated rather than glossed**: this probe samples the **first vertex of the first
draw** in each census class. It does not establish that every world vertex carries white.
A per-class single sample is exactly the shape of error that produced the retracted
"every runway drawable is at z=-3.0" figure earlier this session, so the reading is
"the sampled vertex is white in both" — not "all world diffuse is white".

**Where that leaves it.** Unit 0's texture-env is NVG's `ADDSMOOTH` in B and plain
`MODULATE` in A, both fed a white diffuse and the same terrain texture, yet one is green and
one is blue. With the diffuse identical and the fragment-rejection state identical, the
remaining difference is what unit 0 combines the texture *with* — the constant/`TFACTOR`
slot, which `DX_NVG` sets to `0x0000a000` and which `ADDSMOOTH`'s GL implementation also
needs for its white constant (`GL_TEXTURE_ENV_COLOR`).

The NVG-2 implementation notes flag exactly this collision: *"a stage cannot be both
ADDSMOOTH and a TFACTOR consumer"*. `DX_NVG` sets `TEXTUREFACTOR = 0x0000a000` and puts
`ADDSMOOTH` on stage 0. **Next**: check whether stage 0's `GL_TEXTURE_ENV_COLOR` holds
white (what `ADDSMOOTH` needs) or `0x0000a000` (what `TEXTUREFACTOR` set) at the world draw.

### NVG-5 — the ADDSMOOTH/TFACTOR collision does NOT occur; setup is correct at unit 0

Read unit 0's `GL_TEXTURE_ENV_COLOR`, `GL_COMBINE_RGB` and `GL_SOURCE0_RGB` at the world draw:

```
A (green):  units=0/1   env0=(0,0,0,0)  comb0=0x2100 MODULATE     src0=0x1702 GL_TEXTURE
B (blue):   units=0/1   env0=(0,0,0,0)  comb0=0x2100 MODULATE     src0=0x1702 GL_TEXTURE   (identical to A)
            units=3     env0=(1,1,1,1)  comb0=0x8575 INTERPOLATE  src0=0x8576 GL_CONSTANT
```

**The constant slot holds white** — precisely what `ADDSMOOTH` requires for
`GL_INTERPOLATE(S0=CONSTANT, S1=A2, S2=A1)` to compute `A1 + A2 - A1*A2`. It does **not**
hold `TEXTUREFACTOR`'s `0x0000a000` (which would read `(0, 0.627, 0)`). So the collision the
NVG-2 notes warned about — *"a stage cannot be both ADDSMOOTH and a TFACTOR consumer"* — is
**not happening** on the world draw. Hypothesis refuted.

Also worth recording: the `units=0` and `units=1` classes are **identical between
configurations**, down to the env colour and combine mode. Only the extra `units=3` class
differs, which is consistent with everything measured so far.

**So at unit 0 the NVG combine is set up correctly**: right mode, right constant, right
source-0. The blue is produced downstream of a correctly-configured first stage.

**Next, and narrow**: log `GL_SOURCE1_RGB` / `GL_SOURCE2_RGB` and the corresponding
`OPERAND*_RGB` for the `units=3` class. `ADDSMOOTH` requires `S2 = A1` and `S1 = A2`; if the
operand mapping is transposed the arithmetic silently computes something else while every
state read so far still looks right. `DX_NVG` sets stage 0 to `ADDSMOOTH(DIFFUSE, DIFFUSE)`,
so with both args white the correct output is white — anything else means the operands are
not what the implementation assumes.

Eleventh hypothesis. Each of the last six has been refuted in a single run, and each has
narrowed the target rather than merely eliminating a guess.

### NVG-5 — operand mapping is correct, and my DIFFUSE refutation rested on a sample I had flagged

Read the full `ADDSMOOTH` operand mapping on the world draw:

```
env0=(1,1,1,1)  comb0=INTERPOLATE  s0=GL_CONSTANT  s1=GL_PRIMARY_COLOR  s2=GL_PRIMARY_COLOR
op0=op1=op2=GL_SRC_COLOR
```

That is exactly right: `GL_INTERPOLATE(S0=white, S1=A2, S2=A1)` computes `A1 + A2 - A1*A2`,
with both args `PRIMARY_COLOR` as `ADDSMOOTH(DIFFUSE, DIFFUSE)` requires. **Unit 0 is
correct.** Twelfth hypothesis refuted.

**But that creates a contradiction I have to own.** If stage 0 outputs
`A1 + A2 - A1*A2` with both args white, the result is white — so capping to one unit should
have produced a *white* world, not a blue one. Verified the cap is genuinely connected:
with `FF_NVG_MAXUNITS=1` the census reports only `units=0` and `units=1`, no `units=3`. The
lever works.

**So the diffuse is not white on the draws that matter.** Two entries ago I refuted the
untinted-diffuse hypothesis on a reading of `diffuse=ffffffff` — and in the same entry I
wrote that the probe *"samples the first vertex of the first draw in each census class"* and
that the honest reading was "the sampled vertex is white", not "all world diffuse is white".
I then reasoned from it as though it were the latter.

`ADDSMOOTH(D, D) = 2D - D²` on a blue-dominant terrain/sky diffuse produces exactly the
observed result: blue-dominant, brightened, with detail flattened. The hypothesis I discarded
is back, and it was discarded on evidence I had already labelled insufficient.

**Next**: measure the diffuse properly for the `units=3` class — several vertices across
several draws, not one — before treating any diffuse claim as established.

**The lesson is not about NVG.** Writing the caveat is not the same as heeding it. I recorded
the sampling limitation accurately and then drew a conclusion the sample could not support,
which is the same error as the retracted runway-z figure, one entry after warning myself
about it.

### NVG-5 — diffuse confirmed white across all sampled vertices; the contradiction stands

Re-measured the world draw's diffuse across every index of the draw rather than index 0:

```
B (blue),  units=3:  diffN=6 uniform=1 lo=ffffffff hi=ffffffff
A (green), units=1:  diffN=6 uniform=1 lo=ffffffff hi=ffffffff
```

**Uniformly white in both.** The original single-vertex reading was correct — but it is now
established on evidence rather than assumed, which is the difference that matters after the
previous entry.

**So the contradiction is real and remains open.** Unit 0 is verifiably
`INTERPOLATE(S0=CONSTANT white, S1=PRIMARY, S2=PRIMARY)` = `A1 + A2 - A1*A2` with both args
white, which is **white**. Capping to one unit demonstrably removes the `units=3` class from
the census. Yet the world is still blue at one unit. A correctly-configured stage fed white
cannot output blue.

**Sampling limit, stated again because it still applies**: the census reports at most two
draws per unit-count class. "Diffuse is white" holds for the sampled draws — six vertices
each — not for every world draw in the frame. That remains the most likely place for the
contradiction to resolve.

**Next, and it is a small check that should have come earlier**: when `FF_NVG_MAXUNITS=1`
collapses the `units=3` draws into `units=1`, do those draws still show
`comb0=INTERPOLATE` (NVG's ADDSMOOTH) or has unit 0 reverted to `MODULATE`? If the latter,
the capped test never exercised the NVG combine at all, and the "one unit is as blue as
three" result — used to rule out the multi-unit combine — is invalid.

That would make it the third refutation this session built on a lever whose effect was
assumed rather than checked at the point it mattered.

### NVG-5 — WITHDRAWN: "the multi-unit combine is ruled out" was not established

Checked what unit 0's combine actually is under `FF_NVG_MAXUNITS=1`:

```
capped, units=1 draws:   env0=(0,0,0,0)  comb0=0x2100 MODULATE   s0=GL_TEXTURE
uncapped, units=3 draws: env0=(1,1,1,1)  comb0=0x8575 INTERPOLATE s0=GL_CONSTANT
```

The sampled capped draws are ordinary `MODULATE` draws — **not** the NVG combine collapsed to
one unit. The census reports at most two draws per unit-count class, so this does not prove
the NVG draws reverted; it proves only that **I cannot establish the NVG combine was
exercised under the cap**.

**So the earlier conclusion is withdrawn.** "Capping to one unit is as blue as three,
therefore the multi-unit combine is not the cause" required that the capped draws still ran
the NVG combine. They may not have. The multi-unit combine is **back in scope**.

**This is the third time this session** a result rested on a lever whose effect was assumed
at the point it mattered:
1. `FF_NVG_NOVTX` — switch not connected at all (caught by control)
2. `diffuse=ffffffff` — single sample, flagged as insufficient, then reasoned from anyway
3. `FF_NVG_MAXUNITS` — verified it changed the unit *count*, never that it preserved the
   combine *mode*, then drew a conclusion that depended entirely on the mode

Each time the lever *did something*, which is what made it convincing. Verifying that a
switch has an effect is not the same as verifying it has **the effect the conclusion
requires**. That is the sharper form of the rule and it is worth carrying: state what the
conclusion depends on, then check that specific thing.

**Standing NVG-5 position, honestly:** the world renders blue and flat under `DX_NVG`; it is
rasterised, not missing; fragment-rejection state is identical; unit 0's ADDSMOOTH setup and
operands are correct; diffuse is white on all sampled vertices. The multi-unit combine is
un-eliminated. Twelve hypotheses examined, one withdrawn refutation, no fix.

### UAF-1 — the dangling pointer is the entity itself, read during removal

`UnitProxFilter::RemoveTest` (`camplist.cpp:302`) does exactly one thing before returning:

```c
if ( not ent->EntityType()->classInfo_[VU_DOMAIN] or
     ent->EntityType()->classInfo_[VU_CLASS] not_eq CLASS_UNIT)
```

A **1-byte read** through `ent->EntityType()` — matching ASAN's "READ of size 1" precisely.

**So the dangling pointer is `ent`.** Entity *types* are static tables and are not freed;
a type pointer landing inside a freed **texture pixel buffer** is only explicable if `ent`
itself is stale and `EntityType()` is reading recycled memory. That resolves what looked
nonsensical in the original report — a campaign proximity filter appearing to read freed
texture memory. It is not reading texture memory on purpose; it is dereferencing a freed
entity whose storage was recycled into a texture buffer.

**The sequence**, from the stack:

```
AircraftClass::Exec -> SetDead -> SimBaseClass::SetRemoveFlag
  -> VuEntity::SetPosition -> VuCollectionManager::HandleMove
  -> VuGridTree::Move -> UnitProxFilter::RemoveTest(ent)   <-- ent already freed
```

An aircraft dying sets its remove flag, which repositions it, which walks the grid tree and
runs the proximity filter over entities in the cell — and one of those entities has already
been freed while still referenced by the tree. That is a **lifetime/ordering defect in the VU
collection**, not a bug in the filter, which is doing nothing unreasonable.

**Why a fix must not be rushed.** Adding a validity check inside `RemoveTest` would silence
ASAN while leaving a freed entity in the grid tree — the corruption would continue, unreported.
The real question is why an entity is destroyed without being removed from the collection that
still indexes it, and answering that needs the VU ownership rules, not a guard.

**Compounded by intermittency**: measured at 1 occurrence in 3 identical runs, so a candidate
fix cannot be validated by "the error stopped happening". Any attempt needs many runs, or a
deterministic reproduction first.

### UAF-1 — the protective reference is taken AFTER the reads it exists to protect

`VuGridTree::Move` (`vu2/vu_grid_tree.cpp:49`):

```c
VU_ERRCODE VuGridTree::Move(VuEntity *ent, BIG_SCALAR coord1, BIG_SCALAR coord2)
{
    VuScopeLock l(GetMutex());
    VuBiKeyFilter *bkf = GetBiKeyFilter();

    if ((ent not_eq NULL) and (ent->VuState() == VU_MEM_ACTIVE) and bkf->RemoveTest(ent))
    {
        VuEntityBin safe(ent);      // <-- the safety reference, taken here
```

**The protective reference is acquired after the dereferences it is meant to protect.** The
condition already reads `ent->VuState()` and calls `RemoveTest(ent)` — which reads
`ent->EntityType()->classInfo_[...]`, exactly where ASAN reports the use-after-free.

Note the `VuState() == VU_MEM_ACTIVE` test is itself a validity check: `VU_MEM_DELETED` is set
immediately before `delete ent` (`vuentity.cpp:181`). But reading `vuState_` through an
already-freed pointer is undefined behaviour, so the check meant to detect a dead entity is
performed *by dereferencing it*.

**Fourth instance this session of one shape** — a guard or safety measure sequenced after the
access it should protect:

| site | shape |
|---|---|
| `DrawableBSP::AttachChild` | assertion indexes the array before the bounds check below it |
| `DrawableBSP::DetachChild` | same, sibling function (found by static scan, latent) |
| `GetAvailablePilot` | `best_pilot > -1` guards one line, not the adjacent access |
| `VuGridTree::Move` | reference-count protection taken after the reads |

**Why the obvious fix is wrong.** Hoisting `VuEntityBin safe(ent)` above the condition does
not fix it: taking a reference *through an already-freed pointer* is itself undefined
behaviour. The protection cannot be made sound at this site — the entity must not be reachable
from the grid tree after it is freed. That is upstream, in whatever removes entities from
collections relative to `UnRef()` reaching zero.

**Consistent with the intermittency** (1 in 3): the window between an entity being freed and
the tree being walked is a race, which is also why it appears on thread T15 rather than the
main thread.

### UAF-1 ROOT CAUSE — collections index entities by raw pointer while deletion is refcounted

Three facts, each read directly from the source:

1. **Collections hold raw pointers.** `VuGridTree::Insert` (`vu_grid_tree.cpp:152`) inserts
   the entity into a red-black tree row with no `Ref()`. Same for `PrivateInsert`.
2. **Deletion is purely refcount-driven.** `VuDeleteEntity` (`vuentity.cpp:178`):
   `ret = ent->UnRef(); if (ret == 0) { SetVuState(VU_MEM_DELETED); delete ent; }` — no
   collection removal anywhere in that path.
3. **Collection removal is a separate call.** `VuDatabase::ReallyRemove` removes from the
   hash and the collection manager, and does so correctly — `VuBin<VuEntity> safe(entity)`
   is taken **first**, before any removal or state change.

**So if the last reference drops before `ReallyRemove` runs, the entity is freed while still
reachable from the grid tree**, leaving a dangling raw pointer that the next `Move` walks
into. That is the use-after-free, and it explains every observed property: the 1-in-3
intermittency (it is a race on the ordering), thread T15 (the VU thread), and the freed chunk
being recycled into a texture buffer (the storage is simply reused).

**The contrast is instructive.** `ReallyRemove` takes its safety reference *before* the
operations it protects; `VuGridTree::Move` takes its *after* the reads it protects. The same
codebase contains both the correct and incorrect ordering of the identical idiom, twenty lines
apart in different files.

**Fix options, neither small:**
- **(a) Collections take a reference on insert.** Correct by construction — an indexed entity
  could not reach refcount zero. Changes lifetime semantics globally and would keep entities
  alive until every collection drops them, which may expose ordering assumptions elsewhere.
- **(b) Guarantee `ReallyRemove` precedes the final `UnRef`.** Less invasive but relies on
  discipline at every call site, which is what already failed here.

**Not attempting either tonight.** Both are architectural, the defect is intermittent at 1 in
3, and a candidate fix cannot be validated by the error ceasing. Recorded on UAF-1 with the
evidence so the decision can be made deliberately rather than under time pressure.

### ORDER-1 — tree-wide audit: guards sequenced after the access they protect

Four instances of this shape turned up incidentally this session, three of them
live defects, so it got a dedicated scan. `scripts/qa/order-audit.py`, two passes:

* **Pass 1 — guard after access.** `array[idx]` used before the bounds check on
  `idx`, or a pointer dereferenced before its NULL check.
* **Pass 2 — guard too narrow.** A correct validity guard whose body is a single
  unbraced statement, with further accesses to the same name following it at the
  same depth. This is what `GetAvailablePilot` was, and it is the more insidious
  of the two: the guard is *there*, so the code reads as safe.

**The scanner was validated against known positives before its output was
trusted** — run against the pre-fix `drawbsp.cpp` and `pilot.cpp` from git, it
must find `AttachChild`, `DetachChild` and `GetAvailablePilot`. It missed
`pilot.cpp` on the first three attempts, which is what forced pass 2 into
existence and then twice corrected its precedence. An audit tool that finds new
things but would have missed the defects you already know about is not evidence
of coverage.

**Nine defects fixed** (76 raw candidates → 9 real, the rest triaged as false
positives and each FP class fixed in the scanner rather than filtered by hand):

| site | defect |
|---|---|
| `fartex.cpp:755` `FarTexDB::Deactivate` | `ShiAssert(texArray[offset]…)` indexes before `if (texArray == 0) return;` — **identical to the AttachChild overflow** |
| `damage.cpp:1414` | nine dereferences of `orientation` before both `ShiAssert(orientation)` and its NULL guard |
| `modes.cpp:1649` `HitsOnTrack` | `rdrData->rdrDetect` read in an initializer one line above `if (not rdrData) return 0;` |
| `dogfight.cpp:1218` | `pilot->SetPlayer(0)` on a `MakePilot` result whose NULL guard sits after the block |
| `unit.cpp:5819` `NewUnit` | `&Falcon4ClassTable[tid - VU_LAST_ENTITY_TYPE]` formed before `if (not tid)` — UB pointer arithmetic, no deref |
| `squadron.cpp:159` (×2 trees) | `fuel = uc->Fuel…` immediately after an `else` branch that exists *because* `uc` can be NULL |
| `ceditbox.cpp:631` `SetText` | guards `Text_` once, then dereferences it four more times |
| `te_team_victory.cpp` (×3) | `btn->Refresh()` outside the `FindControl` NULL guard |
| `munition.cpp:1272` | `win->RefreshWindow()` outside the `GetParent` NULL guard |

**False-positive classes worth recording**, since each one initially looked like a
finding: adjacent accessors (a flat backward window crosses function boundaries —
fixed with brace-depth tracking); the list walk `p = p->next; if (!p) break;`
(correct by construction); short-circuit guards `if (p and p->x)`; equality tests
`if (a != this)` that are not validity guards at all; and a commented-out line
between an `if` and its body, which made the real body look unguarded.

**Known limit, stated rather than papered over:** neither pass detects the
*safety-reference* shape — `VuGridTree::Move` taking `VuEntityBin safe(ent)`
after the condition already dereferenced `ent`. That is the fourth known
instance and this audit would not have found it. Detecting it needs
reference-semantics knowledge, not textual ordering.

### ORDER-2 — the safety-reference shape, exhaustively enumerated

ORDER-1 recorded that neither of its passes could detect `VuGridTree::Move`'s
"scoped reference taken after the dereferences it protects". Rather than build a
third detector, the population was counted first: **10 textual matches tree-wide,
of which 2 are function declarations in `hardpnt.h` and 8 are real
constructions.** At that size a detector is the wrong tool — all 8 were read.

**Exactly 2 take the reference late, and both are `VuGridTree::Move`** (the file
carries two overloads, `vu_grid_tree.cpp:54/56` using `GetBiKeyFilter()` and
`:216/218` using `filter_`). Both dereference `ent` twice in the `if` condition —
`ent->VuState()` and `RemoveTest(ent)` — before constructing `VuEntityBin safe(ent)`
inside the body.

The other six take the reference **first**, correctly, including the instructive
contrast already noted under UAF-1: `VuDatabase::ReallyRemove` (both overloads)
opens with `VuBin<VuEntity> safe(entity)` before any removal or state change.
`VuCollectionManager::Remove`, `InactivateUnit`, `AttachCamera` and
`RemoveViewpoint` are all correct too — the last two even NULL-check before
binding.

**So the shape is not a spreading pattern; it is confined to one function.** That
converts ORDER-1's "known limit" into a closed question, and it does not add work:
hoisting those two references is *not* a fix, because taking a reference through
an already-freed pointer is itself undefined behaviour. Both sites remain symptoms
of UAF-1's root cause — collections indexing entities by raw pointer while
deletion is refcount-driven — and are fixed only by fixing that.

**No code change.** The value here is the negative result: 8 sites, 6 correct, 2
known, none new.

### SAVE-2 — remaining `sizeof(long)` in the campaign path: checked, no defect

CLAUDE.md says the leftover `sizeof(long)` uses are "primarily in save functions
… won't cause issues when reading existing Windows campaign files". Tested that,
because the *read* side was later changed to 4-byte `int32_t` in places and a
half-applied fix is exactly how a stream desyncs.

**Result: the built path is self-consistent; nothing to fix.**

* Almost every remaining site is in `src/campaign/camptool/`, the tool tree, not
  the game. The built `camptask` copies were already converted and carry comments
  explaining why.
* `flight.cpp`'s `loadoutData` message still writes `lbsfuel` as `sizeof(long)`,
  and this *looked* like a half-applied fix — the sibling `squadronStores` path in
  `squadron.cpp` was converted to `int32_t` **and its decoder changed to match**.
  But `falconflightplanmsg.cpp` reads `loadoutData` with `sizeof(long)` too, so
  sender and receiver agree. `lbsfuel` is a genuine `long`, so there is no
  overread of the variable either.
* `dataBlock.size` is declared `long`, so the `sizeof(long)` encode/decode pair
  around it writes 8 bytes into an 8-byte field. Not the "8 bytes out of a 4-byte
  object" hazard that `squadron.cpp:1435` documents.

The only real divergence is **wire format vs Windows** (8-byte fields where the
32-bit original put 4), which matters solely for cross-platform multiplayer. That
is not a current goal and the PO has not raised it. Recorded so this does not get
re-opened as a suspected defect.

### ORDER-3 — assertions that perform the access they are checking

ORDER-1's most serious find (`FarTexDB::Deactivate`) was an *assertion* doing the
illegal access, as `AttachChild` had been. That is worth its own sweep here
because **`ShiAssert` is live in this build** — CLAUDE.md records that
`shi/assert.h` promotes `DEBUG`/`_DEBUG` to each other and CMake defines `_DEBUG`,
so these are not dead debug lines. An assert that performs a bad access *is* the
crash.

Scanned all **2818 `ShiAssert` calls**: 197 contain an array index, 425 contain a
`->` dereference. Narrowed each to the dangerous case — the *same function*
validates the array, index, or pointer **later** (ORDER-1's pass 1 only looked ten
lines back, so it could not see a guard further down).

**Index half — 4 candidates, 1 real, fixed in both trees.**
`division.cpp` has `ShiAssert(divels[t][d])` sitting directly above a bounds check
tagged `// JB 010223 CTD` — *crash to desktop*. Someone added that check because
these indices go out of range, and left the assert above it performing precisely
the access the check exists to prevent. Hoisted the test into `divIdxOk`; this also
guards the `USE_SH_POOLS` branch, which indexed `divels[t][d]` with **no** bounds
check at all. Out-of-range behaviour is unchanged (`element` left unassigned,
exactly as the existing `#else` path did). The other three: one commented out, one
false positive (`objectlod.cpp` — the assert is inside `if (TheObjectLODs != NULL)`),
one duplicate tree.

**Dereference half — 3 candidates, 0 real.** `radardoppler.cpp`, `handoff.cpp` and
`cbsplist.cpp` are each guarded by an enclosing condition (`if (… and lockedTarget
and lastLocked)`, `while (simobj)`, `if (bspobj->object)`); the later NULL check the
scanner matched belongs to a different branch. Recorded so this half is not
re-scanned.

**Note on the tool tree:** `camptool/camptask/division.cpp` carries the same defect
in pre-`bitor` syntax and was fixed too, but `ninja` reports no work for it — that
file is not compiled into FFViper. The shipped fix is the `camptask` one.

**ORDER-1 triage footnote — build membership, verified not assumed.** Several
pass-1 candidates were set aside as "separate tools". That was an assumption at
the time, so it was checked: `src/tools/ui_tools/*`, `src/tools/bexpand/*`,
`src/voicecomunication/*` and `src/campaign/camptool/*` produce **no object files
in `build-relg`** and appear in no ninja target. (The ten targets matching "tools"
are `src/codelib/tools/lists` and `graphics/dxengine/dxtools.cpp`, which are
unrelated.) So dismissing them was correct — but the fixes applied to the camptool
copies of `squadron.cpp` and `division.cpp` ship nothing; the built-path fixes are
the `camptask` ones.

### TEAMROE-1 — the assertion every mission fires, found by reading the sweep

**Most** completed TESWEEP rows report `asserts=2`, and both lines are the same
one: `team.cpp:1800`, `ShiAssert(TeamInfo[a])` inside `GetRoE`. It was sitting in
plain sight in a metric we already collect.

*(Corrected: I first wrote "every mission". Rows 2 and 11 report `asserts=0` and
row 12 reports 8 from a different set entirely. I had sampled odd rows 1–9, which
all happened to be 2 — the same sampling error as the `[GEARZ]` artefact earlier
this session, where a probe that only ran near the ground made every sample
post-impact.)*

**It is not a crash.** The code below it is already correct — bounds-checks
`a`/`b`/`type`, then `if (TeamInfo[a]) {…} else return ROE_ALLOWED`. The defect is
that **the assertion contradicts the code immediately beneath it**, asserting a
condition the rest of the codebase treats as routine: `TeamClass::RemovalCallback`
explicitly sets `TeamInfo[who] = NULL`, and `AddInitiative` tests
`TeamInfo[who] == NULL or not TEAM_ACTIVE` as an ordinary case.

**Why it is worth fixing rather than tolerating:** the sweep uses assertion *count*
as its regression signal (the baseline is recorded as "62 assertion lines vs 64").
A false positive firing twice per mission degrades the exact metric we use to
detect regressions, and teaches everyone to skim past assertion output.

**Deliberately not fixed yet.** The obvious move — delete the assert — is wrong
until the caller is known. `misseval.cpp:1966` already guards its loop with
`TeamInfo[d]`, so it is *not* the source. The remaining callers (`nofly.cpp:43`,
`campmap.cpp:428/515`, `misseval.cpp:2560/2990`) all pass a team derived from
entity or message data that could be stale or inactive. **If a caller is asking
about a team that was removed, this assert is reporting a real upstream bug and
deleting it would hide it.** Next step needs a backtrace at the assert site, so it
is queued behind the running sweep rather than guessed at.


### BSPSLOT-1 — TE-12 fires a different assertion set, in the file ORDER-1 touched

Row 12 "Nav and Timing" is the outlier at `asserts=8`, and **none of them are the
`team.cpp` one**: 2× `drawbsp.cpp:87`, 2× `drawbsp.cpp:190`, 2× `texbank.cpp:314`,
2× `objlist.cpp:268`.

`drawbsp.cpp:190` is `ShiAssert(slotNumber < instance.ParentObject->nSlots)` in
`GetChildOffset` — **exactly the out-of-range slot condition that `AttachChild` and
`DetachChild` were silently overrunning** before the ASAN-5/ORDER-1 fixes. Directly
beneath it is a guard the code labels *"THIS IS A HACK TO TOLERATE OBJECTS WHICH
DON'T YET HAVE SLOTS — THIS SHOULD BE REMOVED IN THE LATE BETA AND SHIPPING
VERSIONS"*. `drawbsp.cpp:87` is `ShiAssert(id >= 0)` in `Update`, firing on a
negative id.

**What is not established, and must not be assumed: whether these assertions are
new.** It is tempting to read this as "the fixes converted a silent overflow into a
visible assertion", which would be a satisfying story — but the TESWEEP-4 baseline
was recorded only as an aggregate ("62 assertion lines vs 64"), not per row, so
there is nothing to diff row 12 against. Settle it by running row 12 alone against
the pre-fix commit before drawing either conclusion.

**BSPSLOT-1, part settled statically — and a defect in my own ORDER-1 fix.**

Diffing `drawbsp.cpp` across `bb8935e0^..HEAD` shows the ASAN-5/ORDER-1 work
touched **only** `AttachChild` and `DetachChild`. `Update` (:87) and
`GetChildOffset` (:190) were never modified, so my fixes cannot have introduced
row 12's assertions. The satisfying story — "the fix turned a silent overflow into
a visible assertion" — is wrong for those two sites, and it did not need a test run
to rule out; the diff was enough.

The same diff shows `AttachChild` **lost** this line:

```c
ShiAssert(slotNumber < instance.ParentObject->nSlots);
```

That assertion is a pure **comparison**. It never indexed `SlotChildren`, so it was
always safe where it stood — only the `SlotChildren[slotNumber]` assertion had to
move. Deleting it silenced the diagnostic reporting the exact state ASAN originally
caught (`SMSBaseClass::AddWeaponGraphics` passing a hardpoint index the model has no
slot for), which is the very condition the rest of this ticket is chasing.
`DetachChild` **kept** its copy at `drawbsp.cpp:136`, so two sibling fixes to the
same shape in the same file ended up inconsistent.

This is precisely the error TEAMROE-1 was written to avoid three commits earlier —
*deleting an assertion hides the upstream bug it reports*. Restoring it, immediately
before the existing bounds guard so it matches `DetachChild`. Queued for the
post-sweep batch: rebuilding `FFViper` mid-sweep already made the running sweep a
moving target once, and once is enough.

### UCNULL-1 — `GetUnitClassData()` is nullable; 37 sites dereference it unguarded

Reached by pulling on `atm.cpp:835`, one of the assertions the sweep reports.

**NULL is genuinely reachable — three independent pieces of in-tree evidence:**

1. `UnitClass::GetUnitClassData()` (`unit.cpp:4640`) just returns `class_data`. The
   function **immediately below it**, `GetUnitClassName()`, guards
   `if (class_data)` and returns `"Nothing"` otherwise. The class treats its own
   field as nullable.
2. `class_data` is assigned from `Falcon4ClassTable[type - VU_LAST_ENTITY_TYPE].dataPtr`
   — the *same expression* whose NULL case `SquadronClass`'s constructor handles with
   `if (uc) … else memset(…)`. That is the ORDER-1 defect fixed this session, where
   the code went on to dereference `uc` anyway.
3. `gndunit.cpp` carries three explicit `if (!uc) return …` guards on this call.

**The split** (excluding `camptool`, which is not built): **22** call sites
NULL-check the result, **25** assign-then-dereference without checking, plus **12**
inline `GetUnitClassData()->field` dereferences. Roughly half the codebase believes
this can be NULL and half does not. `unit.cpp:1313` dereferences `class_data`
unguarded *inside `UnitClass` itself*.

**Deliberately not mass-edited.** None of these is observed crashing, and a blanket
`if (!uc) return` is the wrong fix — each site needs a context-appropriate fallback
(`return 0`? `Tracked`? skip the iteration?), and 37 mechanical edits to campaign
code carry more regression risk than the latent defect does. Fix opportunistically:
when ASAN or a crash points at one, and when touching the surrounding code anyway.

The deliverable is knowing the family exists and that NULL is reachable, so the next
crash here is diagnosed in minutes rather than hours.

**BSPSLOT-1 part B — evidence that the dropped assertion was actually firing.**
Sweep row 26 ("HARMs") now reports `asserts=0`. That is the mission whose
`AttachChild` overflow ASAN caught, i.e. `slotNumber >= nSlots` is *proven* to occur
there — so the pre-fix build, which contained
`ShiAssert(slotNumber < instance.ParentObject->nSlots)`, must have fired it on that
mission. Removing that line did not just lose a hypothetical diagnostic; it silenced
one that was firing on the exact mission that motivated the fix. (Caveat: ASAN ran
in `build-asan` and the sweep runs release, so this is strong inference rather than
a direct before/after measurement — the restore's re-run of row 26 will settle it.)

### ORDER-1 — VERIFIED by the full 34-mission sweep

| metric | result | baseline (TESWEEP-4) |
|---|---|---|
| rows reaching sim | **34 / 34** | 34 / 34 |
| crashes | **0** | 0 |
| assertion lines | **60** | 62 (vs 64 before) |

No regression from the nine ORDER-1 fixes plus ORDER-3's `division.cpp` fix.

**The −2 assertion delta is not noise and is fully accounted for:** it is the
`ShiAssert(slotNumber < instance.ParentObject->nSlots)` that the `AttachChild` fix
wrongly dropped (BSPSLOT-1 part B). Restoring it should return the count to ~62.
That the metric moved by exactly the amount the mistake predicts is a small
vindication of tracking assertion counts at all.

**Two caveats recorded rather than buried.** (1) `FFViper` was relinked mid-sweep
for ORDER-3, so rows 1–5 ran against the earlier binary — they are being re-run,
with row 26, against the final one. (2) The baseline is an aggregate, so this
compares totals, not per-row identity. That gap is now closed for the future: a
**per-row assertion map** is recorded below.

#### Per-row assertion baseline (this sweep) — the artefact the aggregate lacked

| assertion | rows |
|---|---|
| `team.cpp:1800` `ShiAssert(TeamInfo[a])` | 1, 3, 4, 5, 6, 7, 8, 9, 10, 19 |
| `atm.cpp:835` | 21, 22, 23, 24, 28 |
| `texbank.cpp:314` (guarded, benign) | 12, 14, 15, 22, 23, 27 |
| `objectiv.cpp:3663` (guarded, benign) | 16, 17 |
| `drawbsp.cpp:87` / `:190` + `objlist.cpp:268` | 12 |
| `drawbsp.cpp:120` | 23 |
| silent | 2, 11, 13, 18, 20, 25, 26 |

`drawbsp.cpp:120` is the assertion ORDER-1 **relocated** below the guards —
`SlotChildren[slotNumber] == NULL`. It firing in row 23 means `AttachChild` was
called for an already-occupied slot, and the fix means that state is now *reported*
instead of being discovered by an out-of-bounds read. The relocation preserved the
diagnostic; only the separate comparison assert was lost.

**Bookkeeping correction — the BSPSLOT-1 restore is in `902c0b03`, not its own commit.**
The `ShiAssert(slotNumber < instance.ParentObject->nSlots)` restore was applied by
the staged post-sweep batch running in the background, and a `git add -A` issued
moments later for the ORDER-1 verification swept it into that commit. So
`902c0b03`'s message describes only the sweep result while its diff also contains a
source fix. The history is correct; the message is incomplete. Recorded here rather
than rewritten — the commit is pushed, and amending shared history to tidy a message
costs more than an accurate note.

**Method note worth keeping: never `git add -A` while a background job can write to
the working tree.** This session runs sweeps and batches in the background as a
matter of course, so a blanket add is a live hazard, not a theoretical one. Stage
explicit paths (`git add docs/STATUS.md`) when anything is running.

**Moving-target caveat resolved.** Rows 1–5, re-run against the final binary (with
the restored assertion and ORDER-3's `division.cpp` fix), reproduce their original
results exactly: row 1 = 2, row 2 = 0, rows 3–5 = 2, every one `team.cpp:1800`. The
mid-sweep relink introduced no discrepancy, so the 34/34 ORDER-1 result stands
without qualification.

### BSPSLOT-1 part B — CLOSED, and it proves the underlying bug is still live

Re-running row 26 (HARMs) with the restored assertion: **`asserts=0` → `asserts=2`,
both `drawbsp.cpp:122`**, the `ShiAssert(slotNumber < instance.ParentObject->nSlots)`
that the `AttachChild` fix had wrongly dropped.

Three things follow, and the third is the one that matters:

1. **The inference was right, and is now measured.** I argued from ASAN's original
   finding that the pre-fix build *must* have fired this assertion on TE-26. It did.
2. **The sweep total returns to 62**, the recorded baseline, from 60. The metric
   flagged my mistake and then confirmed its repair — which is the whole case for
   tracking assertion counts rather than only crashes.
3. **The out-of-range slot condition is still occurring, twice per run, on TE-26.**
   The ORDER-1 fix made it *safe* — the guard returns instead of reading out of
   bounds — but it did not make it *go away*. Something still passes `AttachChild` a
   hardpoint index the model has no slot for. Had the assertion stayed deleted, that
   live bug would have been permanently invisible: no ASAN error (the read is gone),
   no crash, no assertion.

That last point is the argument for TEAMROE-1's stance in miniature, now with
evidence: **deleting an assertion that fires does not fix anything, it removes the
only remaining witness.** BSPSLOT-1 part A keeps a reliable reproducer — TE-26, two
occurrences per run — so tracing which index `SMSBaseClass::AddWeaponGraphics` passes
is a bounded task rather than a hunt.

### BSPSLOT-1 part A — ROOT CAUSE: vehicle data disagrees with the visual model

Traced with `FF_DEBUG_SLOT=1` on TE-26, which reproduces reliably.

`SMSBaseClass::AddWeaponGraphics` (`sms.cpp:717`) — the caller ASAN originally named
— passes the **hardpoint index straight through as the model slot number**:

```c
OTWDriver.AttachObject(drawPtr, …->weaponPointer->drawPointer, i);   // i == hardpoint
```

That silently assumes `VehicleClassDataType::VisibleFlags` agrees with the LOD's
slot count. Measured on TE-26 it does not:

| vehicle type | numHardpoints | visFlags | model | result |
|---|---|---|---|---|
| 16 | 3 | `0x7` | ≥3 slots | fine |
| 3420 | 2 | `0x5` | — | fine (bit 1 clear) |
| **3337** | 3 | `0x7` | **LOD 233, nSlots=2** | **slot 2 requested, refused** |
| **711** | 3 | `0xf` | **LOD 233, nSlots=2** | **slot 2 requested, refused** |

So it is a **data mismatch**, not a logic error: the class data marks hardpoint 2
visible while the visual model defines only slots 0–1. The consequence today is
cosmetic — that weapon is simply not drawn on the vehicle. Before ORDER-1 it was a
heap-buffer-overflow READ, because the assertion indexed `SlotChildren[2]` to
discover the problem.

**Not "fixed", and deliberately so.** The guard in `AttachChild` already handles it
correctly, so a duplicate check at the caller would be redundant; the real
correction is in game data (either the model gains a third slot or `VisibleFlags`
stops claiming one), which is not ours to change. **One genuine open question left:
`CreateVisualObject` is called for that weapon *before* the attach is refused**, so
a visual object may be created and orphaned each time. Whether that leaks or is
reclaimed elsewhere is unverified — flagged rather than assumed.

`FF_DEBUG_SLOT=1` is kept in-tree (both sites, cached `getenv`, repo convention).

### TEAMROE-1 — caller identified: TE objectives owned by teams the mission never creates

`FF_DEBUG_ROE=1` on TE-01. **2849 occurrences in one run**, every one identical:
`TeamInfo[2] is NULL (b=2 type=3)`.

Backtrace (`addr2line` on the release build):

```
GetRoE(unsigned char, unsigned char, int)     team.cpp:1816
RebuildFrontList(int, int)                    camplist.cpp:879
StandardRebuild()                             camplist.cpp:1256
DoTacticalLoop(int)                           campaign.cpp:2995
HandleCampaignThread()                        campaign.cpp:2859
```

The call is `GetRoE(n->GetTeam(), o->GetTeam(), ROE_GROUND_CAPTURE)` with
**a == b == 2** — two neighbouring objectives *both owned by team 2*, while team 2
was never instantiated. TE missions inherit the theater's full objective database,
whose owner fields reference teams the mission itself does not create.

**This is not merely noise, which is why it was worth tracing rather than deleting.**
`ROE_ALLOWED = 1`, and the caller reads:

```c
else if (isolated and not GetRoE(n->GetTeam(), o->GetTeam(), ROE_GROUND_CAPTURE))
    isolated = 0;
```

So the NULL fallback returns **1** and leaves the objective flagged `isolated`. Had
team 2 existed, the question "may team 2 capture team 2's territory" would very
likely answer 0 and clear the flag. **The fallback plausibly inverts the outcome for
same-team neighbours**, affecting front-line and isolation computation for those
objectives.

**Deliberately not changed.** `GetRoE`'s fallback is shared with campaign mode, where
teams *are* instantiated and the AI depends on these answers; altering it to fix a TE
artefact could shift campaign behaviour in ways no test here would catch. What is
established is the caller, the condition, and that the consequence is behavioural
rather than cosmetic. Whether the isolation flag matters to anything a player sees is
**unverified** and stated as such.

**Measurement recalibration worth keeping.** The sweep reported `asserts=2` for TE-01.
The real event count is **2849** — `ShiAssert` output is rate-limited per site, so
assertion counts measure *which* assertions fired, never *how often*. This confirms
the existing note that ShiAssert counts are coverage, not frequency, and it means the
"62 assertion lines" baseline says nothing about volume. Diagnostic capped at 5
backtraces with a progress line every 1000.

### UAF-1 — a direct test of the root-cause hypothesis (result pending)

The recorded root cause is a *candidate*: collections index entities by raw pointer
(`VuGridTree::Insert` takes no reference) while deletion is refcount-driven, so an
entity could be freed while still indexed. That was argued from source, never
measured. Reading further makes it testable:

* `AddToGc` is called **only** from `VuDatabase` (`vu_database.cpp:196,464`).
* `CreateEntitiesAndRunGc` calls `ReallyRemove` **only** for entities whose state is
  already not `VU_MEM_ACTIVE`, and processes at most **5 per cycle**.
* `gclist_` holds `VuEntityBin`, i.e. real references — so anything queued for GC is
  protected from reaching refcount zero.

Therefore an entity arriving at refcount zero **while still `VU_MEM_ACTIVE`** was
never taken through removal, and is still reachable from the collections indexing it.
That is precisely the dangling pointer `VuGridTree::Move` walks. `FF_DEBUG_VUDEL=1`
reports exactly this case.

**First run (release, TE-29, 110 s): 0 active-state deletes — and that proves nothing.**
A control counter was added for *every* refcount-zero delete, because "no findings"
and "the probe never executed" look identical and that trap has cost this project
time before. The control fired **2** deletes in the whole run, both `state=1` (never
inserted into the VU database, so safe to delete). Two deletions in 110 seconds means
the run simply never exercised entity destruction: UAF-1 is a **death-path** defect,
and a scripted flight with no combat input does not kill anything.

So the hypothesis is at this point **neither confirmed nor refuted**, and is recorded
that way rather than as a result. A full-length ASAN run on TE-29 is in progress.

**Probe coverage verified.** A zero result is only meaningful if the probe sits on
*every* path that destroys an entity, so that was checked rather than assumed:
`src/vu2/src/vuentity.cpp` contains exactly **one** `delete ent;` (line 227, inside
`VuDeReferenceEntity`, the only such function), and it is the instrumented site.
There is no second destruction path for `VuEntity`. So a zero from a run that *does*
exercise deaths would be a real refutation, not a blind spot — which is what makes
the pending ASAN run worth running at all.

### UAF-1 — my recorded root cause is REFUTED. Three theories down.

Retrieving the **original** ASAN report (`/tmp/asan-rest-29.log`, preserved from
ASAN-7) was worth more than everything derived from source. Its `freed by` stack is
not an entity at all:

```
operator delete[]  <-  D3D7Surface::~D3D7Surface()      (a 416 KB texture buffer)
                   <-  TextureHandle::~TextureHandle()
                   <-  CPLight::DiscardLit() <- CPPanel::DiscardLitSurfaces()
                   <-  CockpitManager::SetActivePanel() <- Cleanup2DCockpitMode()
```

and the faulting read lands **inside** that region, so `ent` itself points into
recycled memory. Use and free are both on thread **T15**.

**Refuted #1 — "collections index entities by raw pointer while deletion is
refcount-driven".** This was recorded as the root cause and it is wrong as stated.
`FF_DEBUG_VUDEL` instruments the *only* `delete ent;` in the codebase and reports
**zero** entities deleted while still `VU_MEM_ACTIVE`. Entities are being removed
properly before deletion.

**Refuted #2 — "`VuCollectionManager::Remove` skips grid trees".** `Remove` does
iterate only `collcoll_`, never `gridcoll_`, which looked decisive. But
`VuGridTree : public VuCollection`, and both live grids — `RealUnitProxList` and
`ObjProxList` — are explicitly `Register()`ed (`camplist.cpp:641,656`) *in addition*
to `GridRegister`. So removal does reach them.

**Refuted #3 — "asymmetric insert/remove predicates strand entities".** `Insert`
gates on `filter_->Test()` while `Remove` gates on `filter_->RemoveTest()` — genuinely
different predicates, which looked like the bug. But `UnitProxFilter::RemoveTest` is
strictly **weaker** than `Test`: identical except it omits the `Inactive()` check.
Everything insertable is removable. The asymmetry is deliberate and correct — do not
index inactive units, but do allow removing a unit that went inactive while indexed.

**Strongest surviving candidate, untested:** removal is **position-keyed**.
`VuGridTree::Remove` computes `table_[Row(filter_->Key1(entity))]` and calls
`row->Remove(entity)` on *that* row only. If an entity's key changes without the tree
being re-indexed, `Remove` searches the wrong row and fails **silently** (`VU_NO_OP`),
leaving the pointer in the row it actually occupies. `VuGridTree` has
`SuspendUpdates()`/`ResumeUpdates()`, and `HandleMove` **skips grids with
`suspendUpdates_` set** — so an entity that moves during a suspension is exactly the
case where its stored row and its key diverge.

**Nothing has been changed in the VU layer.** Had the recorded root cause been acted
on, the "fix" — collections taking references globally, or enforcing
removal-before-final-unref at every call site — would have been a large change to
entity lifetime built on a theory that measurement disproves.

### UAF-1 — a real defect found on the way: `~VuGridTree` tears down before deregistering

Run 1 of the repeat attempts **reproduced the defect**, this time as a hard SIGSEGV
rather than an ASAN report (unmapped memory rather than poisoned). Identical stack:

```
UnitProxFilter::RemoveTest <- VuGridTree::Move <- VuEntity::SetPosition
  <- SimBaseClass::SetRemoveFlag <- AircraftClass::SetDead <- AircraftClass::Exec
```

**That run performed only 2 refcount-zero deletes, both `state=1`** (entities never
inserted into the VU database). So **no `VuEntity` was destroyed at all** in the run
that crashed — which independently confirms refutation #1 and means the dangling
thing is not a freed entity.

Following that led to a genuine defect in both `~VuGridTree` overloads:

```c
VuGridTree::~VuGridTree() {
    Purge();
    delete [] table_;
    delete filter_;  filter_ = 0;              // second overload
    vuCollectionManager->GridDeRegister(this); // ← only now unreachable
}
```

The grid remains in `gridcoll_` with a freed `table_` and a freed `filter_`.
`VuCollectionManager::HandleMove` iterates `gridcoll_` **under `gridsMutex_`** and
calls `Move()`, which indexes `table_` and calls `filter_->RemoveTest(entity)` — and
this teardown does **not** hold `gridsMutex_`, so the two can overlap. **Fixed by
deregistering first.**

This is the ORDER-1 family again, in the VU layer: *the step that makes an object
unreachable was sequenced after the object had already been torn down.* That is now
five instances this session (`AttachChild`, `DetachChild`, `GetAvailablePilot`,
`FarTexDB::Deactivate`, and this).

**Explicitly not claimed: that this fixes UAF-1.** The defect is real and the fix is
correct on its own terms, but UAF-1 is intermittent, so the error ceasing would not
be evidence — a point already recorded on the ticket before any fix existed. It is
committed as a defect fix, not as a UAF-1 resolution.

### UAF-1 — ROOT CAUSE: the entity **type table** is freed while entities still point into it

The five-run baseline settled it: **2 of 5 runs failed** (run 1 SIGSEGV, run 4 UAF
with 6 reports), matching the ~1-in-3 intermittency, and **every run reported
`ACTIVEdeletes=0`** — including the two that failed.

**The `freed by` stack is a red herring, and I had built a theory on it.** Run 1's
free came from `D3D7Surface::~D3D7Surface` (cockpit texture teardown); run 4's came
from inside `libnvidia-glcore.so`, the GPU driver's own heap. Two reproductions, two
unrelated frees. ASAN names whoever last owned the *recycled* memory, not who freed
the object. The "cockpit teardown" narrative built on run 1's stack was wrong.

**What is invariant tells the real story.** ASAN reports `READ of size 1`.
`EntityType()` returns the entity's `entityTypePtr_` member and `classInfo_` is
`VU_BYTE[]`, so the faulting byte is `classInfo_[VU_DOMAIN]` reached *through*
`entityTypePtr_`. The entity is readable; **its type object is freed.**

Following that pointer home:

```c
VuxType(id)  ->  &Falcon4ClassTable[id - VU_LAST_ENTITY_TYPE]   // f4vu.cpp:370
Falcon4ClassTable = new Falcon4EntityClassType[NumEntities];    // classtbl.cpp:126
UnloadClassTable(): delete [] Falcon4ClassTable;                // entity.cpp:402
```

Every entity whose type is beyond the static `vuTypeTable` stores a pointer **into
that heap array**. `UnloadClassTable()` frees it — along with ~17 sibling data tables
— while entities are still alive and the sim thread is still running `Exec()`. Any
later `ent->EntityType()->classInfo_[…]` is a use-after-free. `AircraftClass::SetDead`
is simply a reliable place where one happens.

This accounts for every observation: no entity is deleted (`ACTIVEdeletes=0`,
correct); the free attribution varies by run (a large block, recycled by whoever);
and it strikes at mission teardown on the sim thread.

**Not fixed.** The correct repair is teardown *ordering* — entities must be gone, or
the sim thread stopped, before the class table is unloaded — and this session has
already watched three confident UAF-1 theories collapse under one more check. The
ordering question deserves its own sprint rather than a same-turn patch.

**UAF-1 — where the free happens, and why the fix is not a one-liner.**
`UnloadClassTable()` has exactly two callers: shutdown (`winmain.cpp:1405`) and
**`SetNewTheater`** (`theaterdef.cpp:398`), which does
`UnloadClassTable(); LoadClassTable("Falcon4");` — freeing the table and reallocating
it **at a new address**, while every existing entity still holds `entityTypePtr_`
into the old block.

All five baseline runs execute this path (`[SetNewTheater]` markers present in each);
only two crashed. That fits exactly: the hazard is created on *every* mission load,
and whether a stale entity survives to dereference its type pointer is timing —
hence the ~1-in-3 intermittency and the difficulty of "verifying" any fix by absence.

**"Skip the reload when the theater is unchanged" is not a safe shortcut**, tempting
as it looks: `LoadClassTable` resolves its data through `FalconObjectDataDir`, which
`SetNewTheater` has just re-pointed, so the table's *contents* really are
theater-specific even though the filename argument is always `"Falcon4"`.

That leaves the honest options, all of them ordering work:
1. **Destroy or quiesce all VU entities before unloading** — correct, and the largest change.
2. **Re-point `entityTypePtr_` across the reload** for surviving entities — smaller, but needs a reliable enumeration of live entities and assumes type indices are stable across the reload.
3. **Make the stale read loud rather than silent** — a debug-only guard; diagnostic value, no repair.

Deliberately stopping here. Three UAF-1 theories died today, each after one further
check; the fourth is well-evidenced but the repair touches shutdown ordering in a
codebase where this session has already fixed five sequencing defects. It gets its
own sprint, with option 1 as the default and option 2 evaluated first for cost.

### UAF-1 — fix implemented: re-point entity type pointers across the class-table reload

**Option 2 chosen over the alternatives, on cost.** Only **14** direct uses of
`entityTypePtr_` exist (nearly all inside `SetEntityType` itself), but **215** call
sites go through the `EntityType()` accessor — so "stop caching, resolve on demand"
would fix all 215 at a stroke. It was rejected: `VuxType()` is a cross-module call
with an assert, and `EntityType()` sits in per-entity-per-grid-per-move hot paths
like `UnitProxFilter::RemoveTest`. Paying that on every call to close a
teardown-window bug is a bad trade in a flight sim.

The chosen fix costs **nothing** on the hot path. In `SetNewTheater`, immediately
after `LoadClassTable("Falcon4")`, walk the database with `VuDatabaseIterator` and
call `SetEntityType(e->Type())` on every live entity, which recomputes
`entityTypePtr_` against the table that now exists. `FF_NO_TYPEPTR_REFRESH=1` reverts.

**Validation design, and its limits.** The pre-fix baseline is **2 failures in 5
runs** of TE-29 under ASAN (one SIGSEGV, one UAF). Six post-fix runs are underway.
Two things are being watched, not one:

* the failure count, and
* **the re-pointed entity count per run** — a fix that executes but touches zero
  entities would produce exactly the same "clean" result as a working one. That
  distinction has already caught one false conclusion this session (the
  `FF_DEBUG_VUDEL` control), so it is instrumented rather than assumed.

**This cannot prove the fix.** At a 2-in-5 base rate, six clean runs is suggestive,
not conclusive, and the ticket has said from the start that an intermittent defect
cannot be validated by absence. The post-fix build also carries the `~VuGridTree`
deregistration fix, so a change in failure rate is **not attributable to either fix
alone**.

**Validation attempt 1 produced no data.** The combined rebuild-and-validate job was
killed during the ASAN rebuild, before a single run started: no output, no
`/tmp/uafpost-*.log` files, and the ASAN binary left stale at 03:06 — predating both
the `~VuGridTree` and type-pointer fixes. The kill also corrupted ninja's build log
(`premature end of file; recovering`).

So **the UAF-1 fix is committed and completely unvalidated.** No failure-rate
comparison exists yet, and none is implied. Recorded explicitly because a killed job
and a clean run are easy to confuse in a log, and this session has already been
caught twice by "the tool never ran" looking like "nothing was found". Rebuild
relaunched as its own job so the build cannot silently consume the runs' budget again
— which is what happened the first time, when 847 targets ate a 10-minute window.

**A startup crash I introduced, caught by reviewing my own fix.** The type-pointer
refresh constructs a `VuDatabaseIterator`, whose constructor uses the global
`vuDatabase` with **no null check** (and dereferences `vuDatabase->dbHash_` under
`VU_ALL_FILTERED`). But `main_linux.cpp:1675` calls `SetNewTheater(td)` and line 1676
logs *"SetNewTheater returned, calling InitVU..."* — so on the startup pass
`vuDatabase` is NULL and the iterator would have faulted on **every launch**.

Guarded with `vuDatabase not_eq NULL`; there are no entities to re-point before the
database exists. Smoke-tested: the release build starts and reaches the UI with zero
crashes, and the refresh correctly prints nothing on a UI-only run.

Two things worth naming. First, this is the *same defect shape* this session has now
fixed six times — an access sequenced ahead of the check that makes it safe — and I
wrote it myself, in the fix for a bug of that exact family. Second, it would not have
been caught by the validation that was about to run: the ASAN attempts would have
crashed identically at startup on every run, which is easy to read as "the harness is
broken again" rather than "the fix is broken", especially after a killed job an hour
earlier.

### UAF-1 — the type-table root cause is REFUTED BY TIMING. Fourth theory down.

The validation run answered a different question than intended, and the answer is
negative. `FF_DEBUG_SLOT`-style instrumentation on the refresh reports
**`re-pointed 2 entity type pointers`** — so the fix executes and is not a no-op. But
the log positions are decisive:

| event | line (post-fix run) | line (baseline crash run) |
|---|---|---|
| `UnloadClassTable` #1 (startup, guarded) | 57 | 57 |
| `UnloadClassTable` #2 → re-points **2** entities | 335 / 386 | 335 |
| sim enters `RunningGraphics` | 1353 | 1499 |
| **crash** | — | **1564** |

**Every mission entity is created *after* the last class-table reload** (150
deaggregation events all follow it), so their `entityTypePtr_` values were never
stale, and the class table is not freed anywhere near the crash. The two entities
that *do* get re-pointed are long-lived session objects, not the dying aircraft.

**So the committed fix does not address the observed crash.** It closes a real but
narrow hazard — those two entities genuinely held pointers into a freed block — and
it is being kept on that basis, with `FF_NO_TYPEPTR_REFRESH=1` to revert. It must not
be described as the UAF-1 fix.

**Where this leaves the diagnosis.** `RemoveTest` reads three 1-byte fields through
`entityTypePtr_`, matching ASAN's `READ of size 1`. With the class table alive, the
only consistent reading is that **`ent` itself is stale** and its memory has since
been reallocated to something live — so reading `ent->entityTypePtr_` is *not*
flagged, and the garbage pointer it yields lands in a freed region. That returns
suspicion to a stale entity pointer reaching `Move()`, which is what the
`~VuGridTree` deregistration fix (`287b71ea`) actually addresses — a still-registered
grid with freed members is exactly a source of garbage `ent`.

**Four theories refuted now** (entity freed while ACTIVE; `Remove` skips grids;
asymmetric predicates; type table freed under live entities). The pattern in every
one: a mechanism that is *genuinely present in the code* but not *active at the time
of the crash*. Presence is not timing, and only the log line numbers settled it.

**Fifth refutation, of my own successor theory — and it points at the `~VuGridTree`
fix.** I proposed that `ent` reaching `Move()` is a stale pointer. It cannot be:
`VuEntity::SetPosition` calls `vuDatabase->HandleMove(this, x, y)`, so **`ent` *is*
`this`** — the live entity executing the call. A search confirms there are no direct
deletes of entity-derived objects anywhere in `sim/` or `campaign/`, and only 2
refcount-zero deletes occur per run, so essentially no entity is destroyed during a
mission at all.

So `ent` is live, and with the class table alive too, `ent->EntityType()` should be
valid. The remaining consistent explanation is that **the filter or the grid is the
dangling object, not the entity**. `HandleMove` iterates `gridcoll_` and calls
`g->Move(ent, …)`, which does `bkf->RemoveTest(ent)` via `GetBiKeyFilter()`. A grid
that has been torn down but is still registered supplies exactly that: a freed
`filter_` and a freed `table_`. `UnitProxFilter` reaches `Real(...)` and the filter
carries a 1-byte `uchar real` member, so a garbage `this` yields ASAN's `READ of
size 1` just as a bad `classInfo_` byte would — the two are indistinguishable in the
report.

**That is precisely the defect fixed in `287b71ea`** (`~VuGridTree` freed `table_`
and `filter_` before `GridDeRegister`, and outside `gridsMutex_`). It was committed
as an incidental find; on this reading it is the **leading candidate fix**, and the
type-pointer refresh is the incidental one. Stated as a reading, not a conclusion —
five theories have died here, and the honest position is that the mechanism is now
narrowed, not settled.

### UAF-1 — post-fix validation: 6 of 6 clean (suggestive, not proof)

| | runs | failures | detail |
|---|---|---|---|
| **pre-fix baseline** | 5 | **2** | run 1 SIGSEGV, run 4 UAF (6 reports) |
| **post-fix** | 6 | **0** | all reached sim; `re-pointed 2` every run |

**What this is worth, stated numerically rather than rhetorically.** Under the
baseline point estimate (failure rate 0.4), six consecutive clean runs has
probability `0.6^6 ≈ 0.047` — about 5%. That is suggestive at roughly the
conventional threshold and no more. Three things keep it short of proof:

1. **The baseline is itself a 5-run sample**, so 0.4 is a rough point estimate with
   wide error bars, not a known rate.
2. **The binary carries both fixes** — `~VuGridTree` deregister-before-teardown and
   the type-pointer refresh — so neither can be credited individually. On the current
   reading the former is the likely one and the latter demonstrably cannot address
   the observed crash (its 2 re-pointed entities are long-lived session objects, not
   the dying aircraft).
3. **The ticket said from the outset** that an intermittent defect cannot be
   validated by absence, and that constraint binds this result too.

`re-pointed 2` in all six runs also confirms the refresh executes and is not a no-op
— the check that was instrumented specifically because a fix touching zero entities
would look identical to a working one.

**Recommended next step for whoever takes this up:** run the pre-fix binary and a
build with `FF_NO_TYPEPTR_REFRESH=1` against ~15 runs each. That separates the two
fixes and tightens the interval enough to matter. The harness is `/tmp/uaf-post.sh`.

### UAF-1 — discriminating experiment (arm A), in progress

**Purpose.** The 6/6 post-fix result cannot credit either fix: the binary carries both
`~VuGridTree` deregister-before-teardown and the type-pointer refresh. Arm A runs the
**same binary** with `FF_NO_TYPEPTR_REFRESH=1`, isolating the grid fix as the single
variable. Failures returning ⇒ the refresh mattered; still clean ⇒ the grid fix is
doing the work, or neither is and 6/6 was luck.

**Control verified.** Run A1 prints `repointed=[]` — empty — confirming the env var
genuinely disables the refresh. The variable name was also checked against the source
before launching, since a typo would silently invalidate the whole arm while looking
like a clean result.

**Running total: 1 clean of 1** with the refresh off. Batches of four from here;
two long background jobs were killed mid-run earlier, and smaller batches lose less
when that happens.

**Harness correction:** the first version of this script `rm -f`'d logs for clean
runs to save space. That is exactly backwards — when a job is killed, a surviving log
then reads as "a failure was kept" rather than "the deletion had not run yet". The
deletion has been removed; all logs are retained.

**Arm A, first data: A3 reproduced the UAF with the refresh disabled.** Same stack
(`RemoveTest` ← `Move` ← `SetPosition` ← `SetRemoveFlag`), same `D3D7Surface` texture
free attribution, and `UnloadClassTable` again at lines 57/335 with the crash at 1485
— so the **timing refutation still holds**: the class table is not freed anywhere
near the crash.

Two conclusions, one of them against my own argument:

1. **The `~VuGridTree` fix alone does not prevent the defect.** Arm A carries it and
   still failed. It remains a real lock-scope defect worth fixing; it is not
   sufficient.
2. **My claim that the type-pointer refresh "demonstrably cannot" matter is now in
   tension with the data.** Arm A (refresh off) is 1 failure in 3 — indistinguishable
   from the 2-in-5 baseline — while both-fixes was 0 in 6. If more runs hold that
   apart, the refresh is doing something my timing argument says it cannot, and the
   mechanism would be unexplained rather than understood.

**Neither conclusion is yet supported by the numbers.** 1/3 versus 0/6 is far too
small to separate; the honest position is that arm A currently *looks like* baseline
and nothing more. Collecting more runs before saying anything stronger — and if the
split does hold up, the correct response is to find the mechanism, not to declare
victory on a p-value.

**Ordering evidence closes the type-pointer question (pending the probe).** From the
run logs: `UnloadClassTable`/reload finishes at line **386**, and `FM_LOAD_CAMPAIGN`
does not begin until line **604**. Campaign objectives, units and every sim entity
are therefore created *after* the reload, holding pointers into the table that now
exists. The refresh's own count agrees: it re-points exactly **2** entities, the
long-lived session objects that predate campaign load.

So the type-pointer mechanism cannot reach the entities involved in the crash, and
the honest projection is that **the 0/6 vs 1/5 split is noise** — neither committed
fix is validated by it. `FF_DEBUG_TYPEPTR` will settle it deterministically: if every
`entityTypePtr_` reaching `RemoveTest` is inside `Falcon4ClassTable`, the theory is
dead and the refresh can be reverted on evidence rather than opinion.

That would leave UAF-1 with **five refuted theories and no surviving mechanism** —
an unsatisfying but accurate state, and a better one than a fix credited by a
coin-flip. What remains solid: the reproducer (2 in 5), the harness, the per-arm
data, and one genuine lock-scope defect fixed along the way.

### UAF-1 — the type-pointer theory is dead, deterministically

`FF_DEBUG_TYPEPTR=1`, TE-29, one run:

```
[TYPEPTR-OK] 20000 checked, 20000 in range, 0 outside
```

**20,000 evaluations, zero out-of-range.** Every `entityTypePtr_` reaching
`UnitProxFilter::RemoveTest` — the exact function where the UAF lands — points inside
the live `Falcon4ClassTable`. The control counter is what makes this a result rather
than an absence: the first run reported "0 outside" with no evidence the probe had
executed at all, which is the same shape as three earlier false readings this session.

**Consequences, stated plainly:**

* The **type-pointer refresh cannot be what produced the 0/6 result.** It is not the
  fix, and the `0/6 vs 2/9` split is noise — arm A finished at **2 failures in 9**
  (≈0.22) against a 0.40 baseline, entirely consistent.
* **Neither committed fix is validated.** `~VuGridTree` is a genuine lock-scope defect
  and stays on its own merits; arm A carried it and still failed twice, so it is not
  sufficient either.
* The refresh **stays**, relabelled: those 2 session entities really do hold pointers
  into a freed block across the reload, so it closes a real if narrow hazard. It is
  simply unrelated to this crash.

**UAF-1 now has six refuted theories and no surviving mechanism.** That is the
accurate state. The value delivered is a reproducer (2 in 5), a run harness, per-arm
data, a deterministic probe that retires a whole class of explanation in one run
instead of a dozen, one real defect fixed, and one startup crash caught before it
shipped. The bug itself is not solved, and nothing here claims otherwise.

**Next investigator should start here:** `ent` is provably `this` and live; its type
pointer is provably in range; no entity is destroyed during a mission
(`ACTIVEdeletes=0`). So the 1-byte read that faults is reached through something else
in `RemoveTest`'s frame — the filter object (`UnitProxFilter` carries a 1-byte `uchar
real`) or the grid supplying it. Instrument `GetBiKeyFilter()`'s return and `Move`'s
grid pointer the same deterministic way.

### UAF-1 — the grid-liveness theory is also dead (clean runs), with the caveat named

Added a liveness sentinel to `VuGridTree`: set to `'VGRT'` in both constructors,
poisoned to `0xDEADDEAD` in both destructors **before anything is freed**, and checked
in both `Move()` overloads under `FF_DEBUG_GRIDMAGIC=1`. If a torn-down grid were
still reachable from `gridcoll_`, this catches it on the first visit rather than
waiting for an intermittent crash.

```
[GRIDMAGIC-OK] 40000 Move() calls, 40000 on live grids     dangling: 0
```

**The caveat, stated rather than glossed:** that run did not crash, and neither did
the type-pointer run. A clean run showing "0 dangling" says nothing about a *failing*
run — the defect appears in roughly 2 of 9. Both deterministic results are therefore
strong for the common case and **not yet conclusive for the failure case**.

Removing that caveat is mechanical, so it is being done rather than argued around:
the probe build now runs in a loop until it fails, with both checks active, so the
invariants are measured **at the moment of the crash**. If a failing run also reports
`0 dangling` and `0 out-of-range`, both theories are dead for good and the faulting
byte is reached through something neither probe covers. If either fires, the bug is
found.

**Probe loop was running the wrong build.** Every failure observed this session —
baseline run 1 (SIGSEGV), baseline run 4 (UAF), arm A3 (UAF), arm A9 (SIGSEGV) — was
an **ASAN** build. The **release** build has never failed here: TE-29 was clean in the
full 34-mission sweep, and probe runs P1–P3 are clean with `dangling_grid=0`,
`typeptr_bad=0` across 20k–60k `Move()` calls each.

So the release probe loop could have run all night without ever reaching a failure,
and its clean results would have looked like confirmation. The probes are being
rebuilt into the ASAN configuration, which is the only one demonstrated to reproduce.

Worth recording as a rule: **an intermittent defect has a reproducing configuration,
and instrumentation must be added to *that* one.** ASAN changes allocation layout and
timing, which is very likely why it reproduces and release does not — the same
property that makes it useful here makes results from the other build inapplicable.

**Heisenbug risk in the probe build — named before the data comes in, not after.**
The probes add work to `VuGridTree::Move`, which runs ~40,000 times per mission: a
cached `getenv`, two counters, a compare, and a periodic `fprintf`. That is cheap, but
it is not nothing, and this defect is timing-sensitive enough that ASAN reproduces it
while release never has.

So a long clean streak from the **instrumented** ASAN build would be ambiguous: it
could mean both theories are dead, or it could mean the instrumentation perturbed the
timing that produces the failure. The two are not distinguishable from clean runs
alone.

**The control already exists.** Plain ASAN (arm A) failed **2 in 9** ≈ 0.22. If the
instrumented ASAN build reaches ~10 clean runs, that is `0.78^10 ≈ 0.08` — starting to
look like perturbation rather than luck, and the right response would be to make the
probes cheaper (drop the periodic `fprintf`, keep only the counters and report at
exit) rather than to declare the theories refuted.

Recording this now so the interpretation is fixed **before** the numbers arrive. It is
easy to accept a clean streak as confirmation when it is the answer you were hoping
for; harder once you have written down in advance what it would take to believe it.

### UAF-1 — `RemoveTest` is now exhaustively accounted for, which points outside it

Every memory read inside `UnitProxFilter::RemoveTest` is now enumerated:

* `ent->EntityType()->classInfo_[VU_DOMAIN | VU_CLASS | VU_TYPE]` — three 1-byte reads
  through `entityTypePtr_`.
* `Real(int type)` (`campbase.cpp:885`) — **pure comparison**, no memory access, and a
  free function, so it never touches the filter's `this`. That kills the "dangling
  filter read its `uchar real`" idea outright.

Combined with the established facts — `ent` is `this` and provably live; no entity is
destroyed during a mission; `entityTypePtr_` is in range across 20,000 checks — there
is **nothing left in that function** that can fault.

**Which makes the line attribution itself suspect.** These are optimised builds, and
`RemoveTest` is a small function called from `VuGridTree::Move`. If it is inlined,
reads belonging to `Move` — the `bkf->Key1(entity)` call, the `table_[Row(...)]`
index, the red-black tree walk — can be attributed to `camplist.cpp:304` in the ASAN
report. Every theory so far has taken that line number literally and hunted inside
`RemoveTest`; that may be why all six failed.

**This reframes the next experiment.** The probe loop should be read not only for
"did the invariants hold" but for whether a failing run reports `typeptr_bad=0`. If it
does, the fault is *not* in `RemoveTest` at all, and the search moves to `Move`'s own
memory: the filter's `Key1()`, `table_`, and the tree nodes. Those are exactly the
structures the `~VuGridTree` teardown fix touched — which would make it relevant again
for a reason quite different from the one it was committed under.

**The inlining hypothesis is wrong — refuted in one turn.** `RemoveTest` is declared
`virtual` (`camplist.h:32,55`) and reached through `bkf->RemoveTest(ent)`, a virtual
call on a base pointer with multiple subclasses, so devirtualisation is unlikely. And
decisively: ASAN reports `#0 RemoveTest` and `#1 Move` as **separate numbered frames**
— inlined frames are marked as such. It was not inlined, so `camplist.cpp:304` is
where the fault really is.

**Prediction, recorded before the data.** Since the fault is genuinely one of the
three `classInfo_` byte reads, and those are reached only through `entityTypePtr_`, a
**failing** probe run must report `typeptr_bad > 0`. If a failing run instead reports
`typeptr_bad = 0`, then something is wrong with my model of that function that seven
theories have not surfaced, and the honest move is to stop theorising and dump the
actual `ent`, `entityTypePtr_` and `classInfo_` bytes at the fault.

Writing the prediction down first is the point. Six of the seven refuted theories were
plausible *after* seeing evidence and worthless *before* it; committing to what the
next observation must show is the only way to keep score.

### UAF-1 — pre-registered threshold reached: probes made cheaper instead of concluding

**10 instrumented ASAN runs, all clean.** `0.78^10 ≈ 0.084` against the uninstrumented
2-in-9 baseline — exactly the threshold written down *before* the runs, where the
agreed response was **not** "the theories are refuted" but "the probes are perturbing
the timing; make them cheaper".

That is what has been done. `VuGridTree::Move` runs ~40,000 times per mission and was
doing a `%`-test and a periodic `fprintf` on that path; `RemoveTest` the same. Both now
only increment counters, and totals are reported once via `atexit`:

```
[GRIDMAGIC-EXIT] <moves> Move() calls, <live> on live grids, <dangling> dangling
[TYPEPTR-EXIT]   <checked> checked, <in range> in range, <outside> outside
```

The controls are preserved — the point of the counters was never the periodic print —
while the hot path loses its I/O entirely.

**Why this matters more than the result.** Ten clean runs is precisely the evidence
that would have let me declare two theories dead and call UAF-1 understood. Having
written the interpretation down in advance, that reading is not available: a clean
streak from an instrumented build is exactly what perturbation looks like, and the
distinction is only visible if you decide which you would believe *before* seeing the
number. Re-running with the cheaper probes now.

**The cheap-probe control was itself broken — and I introduced it.** Moving the probe
totals to an `atexit` reporter looked like the obvious way to keep the counters while
removing hot-path I/O. It does not work in this program: `main()` ends with
`_exit(0)` (`main_linux.cpp:3408`), and the harness terminates runs with SIGINT.
Neither path flushes an `atexit` handler. Runs P11 and P12 therefore emitted **no
probe output at all** and are uninterpretable — I cannot say whether the probes ran.

That is the fourth instance this session of "no findings" being indistinguishable
from "the tool never ran", and the first one I created while explicitly trying to
preserve a control. Replaced with a **one-shot print at 1000 calls**: one line of I/O
per run, negligible against 40,000, and it proves the probe is live.

**Also recorded: a `pkill` that killed the wrong thing.** Stopping the loop killed the
task wrapper (exit 144) while the `bash uaf-probe-loop.sh` child survived as an orphan
and kept running — and my own command died before applying any edits, so the state I
believed I had was not the state on disk. Checking rather than assuming is what caught
it. `pkill` has now caused trouble four times in this project; prefer letting a
bounded loop finish.

**Self-matching `pgrep`, fifth occurrence — this time in a wait condition.** The
chained job began `while pgrep -f 'uaf-probe-loop' …; do sleep 20; done`. That shell's
own command line *contains* the string, so the pattern matched the waiter itself and
the loop could never exit: the rebuild and the runs never started, and the job would
have sat there indefinitely looking busy.

The tell was `orphan: 3` when at most one loop could exist. `pgrep -af` showed both
matches were `/bin/bash -c source …snapshot…` — my own shells.

**The fix is the same one already recorded twice in this project: a bracket class.**
`pgrep -f '[u]af-probe-loop'` matches the process and not the pattern. Used everywhere
else in this session's scripts; omitted here because the pattern was inside a `while`
condition rather than a status check, which felt different and is not.

Stopped via `TaskStop` rather than `pkill` — the previous `pkill` killed a task wrapper
while its child survived as an orphan, and killed the issuing command too.

**A control that read as a finding.** The first run with the working control printed
`[TYPEPTR-RUN] probe live: 1000 checked, 999 in range` — which looks exactly like one
out-of-range type pointer, i.e. the very thing the probe exists to catch and the
result my recorded prediction says a *failing* run must show.

It is an artefact. The control printed **before** the current call was classified, so
`nInRange` was one behind by construction. `grep` confirms **zero** actual `[TYPEPTR]`
out-of-range reports in that run. Print moved after the classification.

Worth recording because of how close this came to being announced as the breakthrough:
the number was surprising, it matched a prediction already on record, and it arrived
after a long run of null results. All three make a claim *more* tempting and none of
them make it true. The check took one look at the source.

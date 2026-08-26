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

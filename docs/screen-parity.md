# EPIC SP — Screen parity vs the PO gold standard

_Sprint 8, 2026-07-25. Method per the cross-port exchange: capture at the present
point on the GL-context-owning thread, `GL_PACK_ALIGNMENT=1`, objective band
statistics + side-by-side eyeballing of the saved frames. Method upgrades adopted
from julia-racer's E59 note (`docs/QA_METHOD_GOLD_PARITY_from-julia-racer.md`):
inventory the gold set as data first; classify deviations into
renderer-bug / authentic-asset / asset-gap / prior-decision before "fixing"._

Gold standard: 5 PO-supplied PNGs in `/run/media/admin/BEA6-BBCE/free falcon/`.
Native captures live in `/tmp/ffval/sp*.png` (regenerate with the recipes below).

**Inventory note for the PO:** shot 3 (`2026-06-05 20-37-29`) is titled
"Free Falcon 6 Linux Port" — it is a capture of the NATIVE build (June 5), not of
the Windows game under Wine. It still documents the intended look of the dogfight
setup screen, but it cannot serve as a Windows oracle. Shots 1/2/4/5 are titled
"FreeFalcon - Wine desktop" and are genuine Wine golds.

## Parity table (verdicts: Sprint 9, 2026-07-27)

Native evidence thumbnails (512×384) are committed alongside this file in
`docs/screen-parity/`; full 1024×768 frames regenerate with the recipes below.
NOTE capture timing: `FF_SIM_SCREENSHOT`/`FF_VIEW_SCRIPT` times are
process-relative and a sim mission load currently takes ~80–100 s, so sim
captures before ~110 s catch the (white-under-X11) load screen — the recipes
below are the ones that actually produced healthy frames.

| # | Gold file (2026-…) | Screen / view | Native repro recipe (verified 07-27) | Verdict |
|---|---|---|---|---|
| 1 | 06-05 16-31-46 | ~~UI main menu~~ **actually the LOADING screen** (blue blueprint/cobra, aircraft column) | n/a — mislabelled | ✅ **CLOSED (Sprint 13)** — not the main menu. The real menu is at PARITY (0.9908 vs the video gold). Withdrawn as an oracle |
| 2 | 06-05 16-32-02 | Sim, 2D cockpit (view 1), Instant Action, banking over coastline, HUD + MFDs | `tools/ff_validate.sh sp2-iapit -m sim -t 110 -r 140 -v 1 -e FF_DEBUG_PITSEL=1` | **PARITY (Sprint 11)** — symbology/layout parity; DEV-2 and DEV-4 both resolved as non-defects (time-of-day and a player option). PO waiver requested |
| 3 | 06-05 20-37-29 | Dogfight setup lobby (Furball options, 4 team tiles, roster pane, Korea map) — NATIVE capture, see note | `tools/ff_validate.sh sp3-dfsetup -m ui -t 4 -r 44 -c "870,745@8;130,121@14;884,741@22"` (select saved game *before* COMMIT; COMMIT→lobby load ~5 s) | **PARITY** — layout/art/options identical; roster contents differ by saved-game state only |
| 4 | 06-06 07-34-04 | Sim, 2D cockpit (view 1), dogfight arena entry, level over ocean | superseded by the video gold (`views` clip t=20s) | ✅ **CLOSED (Sprint 15)** — DEV-3 does not reproduce; native pit art reaches the bottom edge exactly as the gold's. At matched TOD the panel means agree (59.9 vs 58.6), corroborating DEV-2's waiver |
| 5 | 06-09 15-32-20 | ~~UI main menu~~ **actually the LOADING screen**, same as #1 | n/a — mislabelled | ✅ **CLOSED (Sprint 13)** — same as #1 |

## Tolerance statement

UI screens: same layout, art, and control positions; palette within ordinary
gamma/driver variance (the Wine desktop adds a title bar and border the native
window does not have — crop before comparing). Sim screens: same cockpit
geometry, HUD symbology and terrain classes rendered; exact terrain texels and
weather state are not required to match (different mission instant).

## Deviations found

Each deviation gets fixed (SP.2) or PO-waived here. Classification per the
julia-racer method: renderer-bug / authentic-asset / asset-gap / prior-decision.

## ⭐ VIDEO GOLD STANDARD (2026-08-08) — supersedes the PNG golds

PO-supplied Wine screen recordings in
`~/gold standard/free falcon/260808/`, 1920×1080 @ 60 fps, with FreeFalcon
running **windowed at 1024×768** — the same resolution our captures use, so
frames crop out **pixel-for-pixel with no rescaling**. That removes the single
biggest weakness of the PNG golds (1011×771 client → every comparison needed a
resize, and no size-based verdict was safe).

Build shown in the title bar: **FreeFalcon 6.0 · FFViper 2.3.3.44**.

Extract frames with **`tools/gold_video.sh <clip> <timestamp> <out.png>`**
(`--list` for the inventory, `--probe` to re-derive the crop for a new
recording). The window sits at a different desktop offset per clip, so the crop
origin is per-video: `views`/`landing` (100,114), `ia` (233,226).

| clip | len | contents |
|---|---|---|
| `views` | 1:33 | view-mode cycling |
| `landing` | 3:44 | TE "09 Landing Final Approach" — the RWY-2 acceptance flight |
| `ia` | 10:25 | Instant Action from 05:04 dawn, ending at 16× time accel with a visible sunrise |

`ia` timeline: 0–6 s main menu · 9–27 s IA setup · **33 s loading screen** ·
36 s+ 2D pit at dawn · ~520–550 s HUD view · 560–595 s pit with the sun up ·
610 s "prepare to debrief". ACMI tapes `TAPE0006–0010.vhs` and
`260808_landing_final_approach.vhs` accompany them.

### ✅ DEV-1 CLOSED — PARITY. Golds 1 and 5 were a LOADING SCREEN, not the main menu

The video settles it outright, and **overturns both the Sprint-9 finding and the
Sprint-12 conclusion above.**

At `ia` t=0–6 s the Windows main menu is the **legacy F-16-in-shelter photo with
the bottom button bar** — exactly what the native renders. At **t=33 s**, during
the transition into the sim, Windows shows the **blue blueprint/cobra screen
with the right-hand aircraft column**: that is the artwork of golds 1 and 5. It
is a **transient loading screen**, and the still golds captured it and labelled
it "main menu".

Native `sp12-main` vs gold `ia` t=3 s, both 1024×768, no rescaling:

| region | gold luma | native luma | correlation |
|---|---|---|---|
| full frame | — | — | **0.9908** |
| logo area y40–190 x60–260 | 160.6 | 162.4 | +0.9990 |
| photo centre y250–600 | 154.1 | 155.6 | +0.9992 |
| bottom bar y728–768 | 36.6 | 36.8 | +0.9998 |

**The main menu is at parity.** DEV-1 is closed as a non-defect; golds 1 and 5
are withdrawn as main-menu oracles (mislabelled screen, same family as gold 3's
provenance note). My Sprint-12 conclusion — "the gold's menu is not reproducible
from this install, likely a different install" — was **wrong**: the artwork is
produced by this very build, just on a screen I never thought to look for.

The lesson repeats for the third time in four sprints: **a still cannot tell you
what state produced it.** A 33-second-long loading screen and a main menu are
indistinguishable as PNGs, and cost three sprints of DEV-1 speculation.

_(Sidebar on the Sprint-12 method warning: normalised correlation is fine here —
0.9908 on a known-positive. Its failure in Sprint 12 was specific to comparing
**palette-index** data as if it were luminance, not to the metric itself.)_

### DEV-1 — Main menu is a different screen entirely (golds 1 & 5) — ~~MAJOR, open~~ **CLOSED, see above**

Native renders the legacy Falcon4-style main screen: taxiing-F-16 photo
background, "FREE FALCON 5.0" winged logo, bottom button bar
(LOGBOOK…CAMPAIGN). Both Wine golds show the FF-themed menu: dark cobra +
world-map/blueprint artwork with a right-hand column of gold aircraft
(the FF-skin menu buttons), no bottom bar.

Classification so far: **UI resource/window-selection divergence** (bug class,
not asset gap). Ruled out 07-27: the missing `art/resource/mainbg.irc`
(recreated per the Jan-2026 port notes — byte-identical menu, no change); UI
resource load failures (only the known-benign `help_res.lst` fails).

### Sprint 12 — DEV-1 reclassified: the native is CORRECT for this data; the gold is not reproducible from it

**Correction to the Sprint-9 entry above.** It recorded that "the game data DOES
carry FF-themed full-screen menu art the native build never displays
(`mainbg.idx/.rsc` image `MAIN_SCRN` … decodes to a blueprint/logo
composition)". **That is wrong on both halves:**

- `MAIN_SCRN` (in `art/resource/mainbg`) decodes to a **radar-scope / circular
  dial** image, not a blueprint/logo menu.
- The image the main window actually asks for is **`UI_MAIN_BG`**, not
  `MAIN_SCRN`: `art/main/main_win.scf:20` is
  `[SETUP] NID C_STATE_DOWN UI_MAIN_BG`. That resolves via
  `art/uiskin/UISkin.irc` → `art/UISkin/ff4/UIMAINBG`, and decoding it shows the
  **F-16-in-shelter photo the native displays**. So the native renders exactly
  the resource the script names, and it is displayed, not skipped.

`main_win.scf` also places the buttons at **y=728 — the bottom bar** the native
draws (`EXIT_CTRL 0 728`, `IA_MAIN_CTRL 724 728`, `DF_MAIN_CTRL 824 728`), with
`BAR_M` tiled across y=728 and `BAR_O` across y=0. Every `main_win.scf` in the
install (base + all four theater trees) has that same layout. **There is no
window script in this data that produces the gold's right-hand aircraft column
and no bottom bar.**

**Search for the gold's background.** Every `.idx` in the install was walked
(751 resource indexes), every 1024×768 entry decoded, deduplicated to **69
unique full-screen images**, and rendered as a contact sheet. They are aircraft
photos, a few radar scopes, a Balkans title card and a FreeFalcon text page.
Only two are near-black, and both are **entirely empty** (mean 0.0). None is the
gold's blue blueprint/cobra composition.

**Conclusion: golds 1 and 5 cannot be reproduced from this game-data install.**
The native build is behaving correctly for the data it has. This is the same
class as the Sprint-9 inventory note about gold 3 — **a gold-provenance problem,
not a renderer bug.**

**PO question:** were golds 1 and 5 captured against a different FreeFalcon
install, version or UI skin than
`~/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6`? If so DEV-1 should be withdrawn;
if not, we need the data that contains that artwork.

**Stated limitation:** the search covered full-screen 1024×768 resources. If the
gold's menu is *composed* (a smaller or tiled background plus separately placed
elements) rather than one full-screen image, it would not have been found.

**A method that had to be thrown away — worth reading.** The first pass ranked
candidates by normalised correlation against the gold using palette indices as
greyscale. It reported "no match, best 0.447" — which looked like a result. Run
as a **control against the NATIVE capture, whose correct answer is known**, the
same metric put `art/uiskin/ff4` `UI_MAIN_BG` **outside the top 6** (best 0.557).
The metric cannot identify a known-positive, so its negatives are worthless, and
the conclusion drawn from it was withdrawn and redone by decode + eyeball.
**Validate a similarity metric on a known-positive control before believing a
negative result.**

### DEV-2 — 2D pit renders far brighter than gold (golds 2 & 4) — ✅ symptom CONFIRMED, ⚠️ **stated mechanism WRONG** (Sprint 10)

_Original Sprint-9 text, kept for the record:_ Systematic across both sim shots
(IA and dogfight arena — the dogfight pair is same-scenario, so this is
decisive): the native 2D-pit panel art renders light-gray/high-brightness where
the Wine gold is near-black/charcoal, and the sky/sea palette is correspondingly
shifted (native sky darker steel-blue, gold brighter blue). Same panel art, same
instrument layout — a shading/palette difference, not different assets.
Suspected class: palettized 2D-pit bitmap displayed with its raw/day-max
palette, i.e. the TOD-based palette shading the Windows path applies is missing
on native. HUD/MFD/DED symbology is unaffected.

**The premise "same panel art, same instrument layout" is wrong — see below.**

### ✅ DEV-3 CLOSED (Sprint 15) — does not reproduce against the video gold

Settled offline, no display run needed. The `views` clip at t=20 s is the 2D
cockpit at exactly 1024×768; the pit art occupies fixed screen space regardless
of the world outside, so this is a valid like-for-like comparison against our
view-confirmed `sp11-base` capture.

Row-mean luminance down the bottom edge — gold vs native:

| y | 700 | 712 | 724 | 736 | 748 | 760 | 766 |
|---|---|---|---|---|---|---|---|
| gold | 42.3 | 61.3 | 62.9 | 84.5 | 55.7 | 65.7 | 62.6 |
| native | 41.8 | 67.3 | 65.3 | 85.5 | 57.4 | 69.1 | 53.5 |

The two track each other to within a few units all the way to y=766. Visually
the bottom strips are structurally identical — same MFD bezels, same standby
ASI/altimeter, same button rows, same `MASTER ARM`/`SIMULATE` panel — and
**neither shows pilot hands**. There is no missing sliver: the native pit art
reaches the bottom edge exactly as the gold's does.

Region correlations (pit art only; MFD *content* differs because the mission
state differs, which caps these below 1.0):

| region | gold | native | corr |
|---|---|---|---|
| lower panel y500–768 | 59.9 | 58.6 | +0.8712 |
| bottom strip y740–768 | 60.3 | 61.7 | +0.9164 |
| left console y560–760 x10–200 | 73.1 | 72.1 | +0.9120 |

**Bonus corroboration for DEV-2:** this clip is daytime, and at matched
time-of-day the panel means agree — **gold 59.9 vs native 58.6**. That is the
independent confirmation DEV-2's waiver wanted: with TOD matched, the panel
brightness matches. The Sprint-9 "far brighter than gold" reading came entirely
from comparing a noon native against a dimmer-TOD gold.

DEV-3's original text follows for the record.

### DEV-3 — Bottom sliver of 2D-pit art (pilot hands) not visible (gold 4) — ~~open~~ **CLOSED, see above**

_Original Sprint-9 text:_ The gold shows the pit art's bottom edge (glove/hand
shapes) below the standby gauges; the native frame ends at the gauge bottoms.
Possible small vertical offset/scale difference in the 2D-pit blit.

### ⚠️ DEV-2 / DEV-3 re-examined — the frames also differ STRUCTURALLY (Sprint 10, SP.2-A)

DEV-2's stated root cause (missing TOD/palette shading on a palettized 2D-pit
bitmap) rests on the premise "same panel art, same instrument layout, a shading
difference only". **That premise is false**: the native and gold frames differ
**structurally** as well as tonally. The tonal delta could not be attributed to
shading until the structural difference was explained, so both verdicts were
suspended pending a view-confirmed re-capture. SP.2-B (below) supplied it: the
tonal symptom is confirmed real, the mechanism is not the one recorded, and the
structural difference is now tracked separately as DEV-4.

**What is established (offline, from the committed Sprint-9 thumbnails + the
game data — no new capture required):**

1. **The native frames show a canopy bow arching over the HUD glass**, drawn in
   perspective with 3D shading, with mirror-shaped geometry at its top corners.
   Present in *both* `sp2-native-iapit.png` and `sp4-native-dfpit.png`.
2. **Both golds show the bare HUD combiner posts with open sky above** — no bow.
   The native frames have those same posts **plus** the bow.
3. **Structure profile** (both normalised to 1024×768, std-dev of luminance in
   the centre band x380–640, 16-px rows): native carries structure at y0–112
   (the bow) where gold is clean sky; gold carries structure at y176–336 (the
   posts) where native is clear glass. Panel top matches in both (~y340), so
   this is **not** a scale difference.

**What is NOT established — and a correction to an earlier reading in this
sprint.** The mirror-shaped geometry was first taken as proof that the native
frames are the *3D virtual cockpit*, on the grounds that no 2D-pit `.dat` in
this install declares a `MIRROR` object (parser token `TYPE_MIRROR_STR` →
`CPMirror`, cpmanager.cpp:646; zero hits in `art/ckptart/16_ckpit.dat`,
`F-16CG/16_ckpit.dat`, `F-16CJ_MicroProse/10_ckpit.dat`,
`F-16CJ_PaulWilson_Widescreen/ws_ckpit.dat`). **That inference is wrong.** The
2D pit in this art set is not a flat blit: its MANAGER block carries
`cockpit2d 2358 2358` (`PROP_DO2DPIT_STR`), which drives
`CreateCockpitGeometry(&mpGeometry, 2358, 2358)` — the 2D pit is itself rendered
from a 3D model. Mirror and canopy-bow shapes can therefore belong to the 2D pit
legitimately, as model geometry rather than as `CPMirror` objects. The view
actually in force at capture time remains **unknown**.

**Live hypotheses for the structural difference:**
- ~~the two frames are different view modes~~ — **ELIMINATED**, see below;
- ~~different vertical placement so the bow sits off-screen in gold~~ —
  **ELIMINATED**: panel top matches (~y340 normalised) in both;
- the bow is drawn natively but should be culled, switched off or hidden — LIVE;
- the Windows install resolved a different pit art set — LIVE but weak (same
  game data; the native fallback is deterministic, see below).

### SP.2-B result — view and art set CONFIRMED; the deviations are real (Sprint 10)

Re-captured with the new trace, `gl-lock`-serialised, `-test-ia`, view pinned:
`tools/ff_validate.sh sp2-iapit -m sim -t 110 -r 140 -v 1 -e FF_DEBUG_PITSEL=1`
→ `/tmp/ffval/sp2-iapit.png` (1024×768, 94.4 % non-black, 96 242 distinct
colours, VERDICT REAL CONTENT).

What the trace establishes:

```
[PITSEL] requested='16_ckpit.dat'
         resolved='…/art/ckptart/16_ckpit.dat' main=1
         hScale=0.6399 vScale=0.6399 visType=1746 cpName='F-16CJ' nctr='F16CJ'
[PITSEL] open '…/art/ckptart/16_ckpit.dat' -> OK
[PITSEL] displayMode=2 (CHANGED) hybrid=0      … steady at 2 for the whole flight
```

- **The view is `Mode2DCockpit` (2) for the entire run, with hybrid pit off.**
  The native capture *is* the 2D pit. The view-mode hypothesis is dead, and the
  earlier "these are the 3D virtual cockpit" reading is definitively wrong.
- **The art set is the generic `art/ckptart/16_ckpit.dat` fallback**, opened OK,
  at scale 0.6399 (= 1024/1600). The aircraft's cockpit name is `F-16CJ`, and
  there is **no `art/ckptart/F-16CJ/` directory** in this install (only
  `F-16CJ.txt`, `F-16CJ_MicroProse/`, `F-16CJ_PaulWilson/`,
  `F-16CJ_PaulWilson_Widescreen/`), so `FindCockpit` falls through to the
  theater-standard file. This is deterministic and would resolve identically on
  Windows against the same data.
- `gDoCockpitHack` is not involved — the pit `.dat` opened successfully.

**Re-measured, view-confirmed, gold 2 vs the fresh native capture** (gold client
area cropped and both normalised to 1024×768):

| Region | gold luma | native luma | Δ |
|---|---|---|---|
| sky y0–100 | 97.8 | 84.5 | −13.3 |
| bow band y40–110, x380–640 | 149.7 | 101.8 | **−47.9** |
| panel y500–768 | 38.0 | 58.7 | **+20.7** |
| panel left y500–768, x0–200 | 31.0 | 66.3 | +35.3 |

Panel percentiles (p5/p25/p50/p75/p95): gold `5 / 16 / 29 / 49 / 107` vs native
`0 / 5 / 46 / 88 / 189`.

**So DEV-2's symptom is real and reinstated — but its stated mechanism is
wrong.** The 2D pit in this art set is **not** a palettized bitmap, so "missing
TOD/palette shading on the pit bitmap" cannot be the cause. Its MANAGER block
carries `cockpit2d 2358 2358`, i.e. the pit is drawn from **lit 3D model
geometry** (`CreateCockpitGeometry`). The brightness question is therefore a
**pit-geometry lighting question**, and this port has prior form exactly there —
commit `3bc96916` fixed three compounding lighting bugs for the *3D* pit
(per-vertex emissive dropped, DXT1 punch-through alpha, light directions baked
through a stale modelview). Whether those fixes cover the 2D pit's geometry pass
is the first thing to check. `mFloodLight`/`mInstLight`/`lightLevel` on
`CockpitManager` (from `floodlight = 0xff666666; instlight = 0xff666600;` in the
`.dat`) are the other input.

### Sprint 11 / S11-B — DEV-2 localised: the native panel is drawn at RAW ART brightness

Measured the pit's own art file against both frames. `16_1200_0.gif`
(1600×1200, 8-bpp, the main panel surface referenced by `16_ckpit.dat`), taking
only non-chroma-key pixels (palette index ≠ 0, 25.7 % of the image):

| Sample | p5 | p25 | **p50** | p75 | p95 |
|---|---|---|---|---|---|
| **raw pit art** `16_1200_0.gif` | 0.0 | 13.7 | **49.0** | 105.3 | 206.0 |
| native panel (y500–768) | 0.0 | 5.2 | **45.5** | 87.7 | 188.7 |
| gold panel (y500–768) | 5.2 | 16.0 | **28.7** | 49.4 | 107.0 |

**The native tracks the raw art almost exactly; the gold sits at ≈0.55× it**
(28.7/49.0 = 0.59 at the median, 107/206 = 0.52 at p95). So the deviation is not
that native is "too bright" in some vague sense — **native draws the panel
essentially unlit, at raw palette brightness, and Windows applies a cockpit
light factor of roughly 0.55.**

The single choke point that decides this is
`CockpitManager::ComputeLightFactors`, which feeds `CPPanel::SetPalette`:

```
eLight = max(lightLevel, 0.01)     // lightLevel IS the environment light
cLight[i] = eLight                 // then flood/instrument lights are added
```

so if `lightLevel` is ~1.0 the panel palette comes out unmodified. Two facts
worth carrying into the fix:

- **`lightLevel` is set exactly once**, in `RenderFirstFrame`
  (otwloop.cpp:544) from `TheTimeOfDay.GetLightLevel()`.
  `CockpitManager::TimeUpdateCallback` exists to keep it current but **is never
  registered anywhere** — only `DrawableTrail`'s and `DrawableBSP`'s
  same-named callbacks are. So the cockpit never re-lights as the sun moves.
- **A dead end, checked and excluded:** `CPPanel::SetTOD(float lightLevel)`
  begins `SetPalette(); return;`, leaving its whole 16.16 fixed-point TOD
  palette-lighting body unreachable. That looks like the bug but **is not ours**
  — `git log -L` shows it predates the port (present before the reformat commit
  `7b8d31a2`), so Windows short-circuits identically.

Instrumented at the choke point (`FF_DEBUG_PITSEL=1` now also prints
`lightLevel`, `eLight`, the live `TheTimeOfDay.GetLightLevel()`, the flood and
instrument light states, and the resulting `cLight`/`iLight`). Measured:

```
[PITSEL] lightLevel=1.0000 eLight=1.0000 todLight=1.0000 flood=0 inst=0
         -> cLight=1.0000 iLight=1.0000
```

`todLight` is **1.0 from real TOD data**, not a fallback — the constructor
defaults are `Ambient=.3 / Diffuse=.6`, which would sum to 0.9. Windows runs
identical code against identical theater data, so it computes `cLight=1.0` for
the same mission instant too. That points away from a port defect.

### ✅ DEV-2 RESOLVED — the panel path is CORRECT; the gap is time of day

Decisive test: force the environment light factor to the value implied by the
gold (`FF_PIT_LIGHT=0.55`) and re-capture. Prediction stated before the run —
the panel band is not pure art (MFDs and lit instruments use `lightPlt` at full
brightness regardless), so a pure 0.55 scale would give 32.3 and the true
expectation was 32–40 against the gold's 38.0.

| Sample | panel mean | p50 | p95 |
|---|---|---|---|
| native `cLight=1.00` | 58.6 | 45.4 | 188.8 |
| **native `cLight=0.55`** | **35.8** | **25.3** | **106.0** |
| **gold 2** | **38.0** | **28.7** | **107.0** |

**p95 matches within 1 unit; the mean within 2.2.** Feed the 2D-pit panel path
the gold's light level and it reproduces the gold's brightness almost exactly.

**Therefore the panel rendering is correct and DEV-2 is not a port defect** — it
is a **time-of-day difference between the two captures**. The Wine gold was shot
at an environment light of roughly 0.55; our Instant Action capture runs at 1.0.

**Recommended disposition: WAIVE**, on the same footing as DEV-4. To compare
panel brightness meaningfully, a future capture must match the gold's mission
instant (or force `FF_PIT_LIGHT` to the gold's level, which is what makes this
measurable at all).

**Two real observations survive as backlog items, neither causing DEV-2:**
- **LIGHT-1: ❌ WITHDRAWN — THE DEFECT DOES NOT EXIST (Sprint 14, 2026-08-09).**
  I claimed `CockpitManager::TimeUpdateCallback` was never registered, so
  `lightLevel` was written once by `RenderFirstFrame` and frozen for the mission.
  **That is false.** The constructor has always registered it, at
  cpmanager.cpp:677–679, under the comment *"Initialize the lighting conditions
  and register for future time of day updates"* — the same pattern every other
  TOD consumer uses, correctly paired with the destructor's release. **The
  cockpit re-lights already.**

  How the error survived: a `grep` for `TheTimeManager` was piped through
  `head -20`, and the `cpmanager.cpp` registration was line 21. Everything after
  — the "missing half of a pair" argument, the claim that the destructor's
  release proved intent, the whole fix — was built on that truncated output.

  It was caught by its own control run. The A/B measured 15 `SetTOD` calls with
  the "fix" against **9** with it disabled; the prediction was 9 vs **1**. Nine
  callback-driven re-lights in the control is exactly what an already-registered
  callback looks like. The contradiction, not the review, is what exposed it.

  **The "fix" was a regression and is reverted.** It added a *second*
  registration: the callback then fired twice per cycle, and since the destructor
  releases only one entry, the leftover `CBlist` slot retained a pointer to the
  freed `CockpitManager` — a **use-after-free on the next TOD tick after a
  mission ends**. A comment at the old site now warns against re-adding it.

  What remains true: the earlier measurement that the gold cockpit brightens as
  the sun rises (glare-free console 38.2 → 47.0). That is now simply **expected
  behaviour on both sides**, not evidence of a defect. Whether our port matches
  that curve is untested — it needs a dawn mission, and is worth a future sprint.

- **LIGHT-2:** `ComputeLightFactors` assigns `cLight[i] = eLight` with **no
  upper clamp** — the 1.0 clamp exists only inside the flood-light branch — and
  `GetLightLevel()` returns `Ambient + Diffuse`, which can exceed 1.0. An
  over-bright palette is reachable; not hit here because the value was exactly
  1.0.

_(Capture note: the `FF_PIT_LIGHT` run ended in the documented SIGABRT in the
SDL2/pipewire audio teardown on harness kill — after the frame was captured, and
unrelated to the change. See CLAUDE.md.)_

### Sprint 11 / S11-A — DEV-4 SOLVED: the bow is the canopy-reflection visual cue

A/B under `gl-lock`, identical recipe, single variable:

| Run | `SimVisualCueMode` | bow band y40–110 x380–640 (luma) | panel y500–768 |
|---|---|---|---|
| `sp11-base` | 2 (`VCReflection`) | 102.4 | 58.6 |
| `sp11-noref` | 0 (`VCNone`, forced) | 109.7 | 58.8 |
| gold 2 | — | 149.7 | 38.0 |

With the cue forced off **the canopy bow disappears completely** — the crop is
clean sky, matching the gold's structure. The panel is untouched (58.6 → 58.8),
confirming the bow and the brightness are **two independent deviations**, which
is why DEV-2's "same art, shading only" premise failed.

**Mechanism:** switch 3 of the 2D pit's external geometry (`cockpit2d 2358`,
"wings and reflection") is the canopy-reflection component. It is enabled
whenever `PlayerOptions.SimVisualCueMode` is `VCReflection` or `VCBoth`
(cpmanager.cpp:4518). The native run reports `SimVisualCueMode=2` — an
**in-range, legal value**, which is both the constructor default
(`playerop.cpp:97`) and a legitimate user setting.

**Recommended disposition: WAIVE as a player-option difference, not a renderer
bug.** The port is doing exactly what the option asks. We cannot read the
Windows user's `.pop`, so we cannot prove the gold ran with the cue off — but
the native behaviour is correct for its own setting, and there is nothing to
fix in the renderer. **PO call requested.** If the PO wants the gold matched
exactly, the change is a settings change (`Setup → visual cues`), not code.

Checked and excluded rather than assumed: `PlayerOptionsClass` is read with a
raw `fread(this, 1, size, fp)`, which looked like a 32/64-bit layout hazard —
but every member is `int`/`float`/`enum`/`char` (no `long`, no pointers), so the
layout is portable. The 228-byte `default.pop` vs 240-byte `Viper.pop`
difference is **version**, not word size, and `LoadOptions` handles the older
format deliberately ("dont break compatibility with 1.03 - 1.08 options").

**DEV-4 — native draws a canopy bow the gold does not (original entry).** In a confirmed
identical view and a deterministically identical art set, the native frame
renders a canopy bow with mirror geometry arching over the HUD glass; the gold
shows open sky there. Note the 2D pit sets only **two** switch masks on its
geometry — `SetSwitchMask(7, …)` (wing, gated on `ObjectDetailLevel`) and
`SetSwitchMask(3, …)` (canopy reflection, gated on
`PlayerOptions.SimVisualCueMode`) at cpmanager.cpp:4509/4520 — whereas the 3D
pit sets 263 of them in `vcock.cpp`. A switchable component of model 2358 left
at its default-visible state is the leading candidate, and that is the port's
recurring "nothing ever wrote this" class. Next step is to dump model 2358's
switch/DOF components and A/B the two masks the 2D path does set.

**Measured tonal delta (kept — it is a real measurement, just not of what
DEV-2 claimed).** Gold 4 client area vs `sp4-native-dfpit`, normalised 512×384:

| Region | gold luma | native luma | Δ |
|---|---|---|---|
| sky, y0–60 | 137.2 | 76.6 | **−60.7** |
| horizon band, y130–170 | 122.6 | 150.7 | +28.1 |
| lower panel, y250–384 | 31.0 | 69.5 | **+38.5** |
| panel left, y250–384 x0–100 | 24.0 | 92.3 | +68.3 |
| glareshield, y175–215 | 64.3 | 90.4 | +26.1 |

Panel luminance percentiles (p5/p25/p50/p75/p95): gold `0 / 6 / 17 / 46 / 101`
vs native `0 / 9 / 50 / 116 / 184`.

Note the sky moved the **opposite way** from the panel (−61 vs +39). A single
global gamma/TOD-shading error cannot produce that, which is the first thing
that should have cast doubt on the DEV-2 hypothesis.

**Why this matters beyond these two deviations:** the Sprint-9 recipe pinned the
view with `FF_VIEW_SCRIPT="1@<t>"` (1 = `Mode2DCockpit`) and the verdicts were
written as though that had taken effect. Nothing in the capture output confirmed
the view actually in force, so an unset/overridden view mode was indistinguishable
from a render bug. **A parity capture must record the state it claims to be
capturing.** New trace `FF_DEBUG_PITSEL=1` now reports, per run: the requested
vs resolved cockpit `.dat`, whether it opened, the scales, and the live
`displayMode` (1=Hud 2=2DCockpit 3=3DCockpit) once a second — so every future sim
capture can be checked against the view it claims.

Same class as BoB S101, already recorded in `STATUS.md`: *a diagnostic that lies
is worse than no diagnostic.* Here the diagnostic did not lie so much as stay
silent about the one variable the whole verdict rested on.

**Open (SP.2-B/C):** read the display mode and the resolved pit `.dat` at capture
time from the `FF_DEBUG_PITSEL` trace, which discriminates all four hypotheses
above in a single run. Then re-capture golds 2 and 4 in a *confirmed*
`Mode2DCockpit` with the resolved art set recorded, and re-issue verdicts.

Notes for whoever picks this up:
- `SetOTWDisplayMode` (access.cpp:481) does **not** special-case the pit modes —
  it cannot silently reject `Mode2DCockpit`.
- `SetGraphicsOwnship` (otwdrive.cpp:1920) sets `Mode2DCockpit` when there is no
  `curPlatform`, so the 2D pit is the expected default on sim entry.
- Hybrid pit auto-switch (`RunHybridPitMode`, otwdrive.cpp:1090) *does* flip
  2D→3D on head movement, but is gated on `GetHybridPitMode() == 1`
  (otwloop.cpp:327) and `HybridPitModeEnabled` initialises to 0 — so it is the
  weakest candidate unless the trace shows otherwise.
- A failed pit-`.dat` open sets `gDoCockpitHack` and suppresses the 2D path
  (cockpit.cpp:54); the trace reports the open result explicitly.

### Notes (not deviations)

- Gold 3 is a native June capture (see inventory note): the 07-27 parity there
  confirms **no regression** in the dogfight lobby since June, not Windows
  parity. The add/remove roster toolbar icons visible in the gold are absent in
  the 07-27 frame — roster state differs (saved game now contains Crimson1);
  check icon visibility when re-testing with an empty roster in SP.2.
- Gold 2 vs native: different mission instant (terrain area/bank angle) —
  within stated tolerance; the pale far terrain in the native IA frame needs a
  matched-instant re-check after DEV-2 before being called a separate deviation.

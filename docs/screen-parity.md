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
| 1 | 06-05 16-31-46 | UI main menu (cobra/blueprint splash, aircraft column right) | `tools/ff_validate.sh sp1-main -m ui -t 8 -r 16` | **DEVIATION (major)** — see DEV-1 |
| 2 | 06-05 16-32-02 | Sim, 2D cockpit (view 1), Instant Action, banking over coastline, HUD + MFDs | `tools/ff_validate.sh sp2-iapit -m sim -t 110 -r 140 -v 1 -e FF_DEBUG_PITSEL=1` | ⚠️ **WITHDRAWN (Sprint 10)** — the native frame is the 3D virtual pit, not the 2D pit; verdict void, re-capture pending |
| 3 | 06-05 20-37-29 | Dogfight setup lobby (Furball options, 4 team tiles, roster pane, Korea map) — NATIVE capture, see note | `tools/ff_validate.sh sp3-dfsetup -m ui -t 4 -r 44 -c "870,745@8;130,121@14;884,741@22"` (select saved game *before* COMMIT; COMMIT→lobby load ~5 s) | **PARITY** — layout/art/options identical; roster contents differ by saved-game state only |
| 4 | 06-06 07-34-04 | Sim, 2D cockpit (view 1), dogfight arena entry, level over ocean | manual run: `FF_UI_CLICK="870,745@8;130,121@14;884,741@22;900,750@36;900,750@44"` + `FF_VIEW_SCRIPT="1@120"` + `FF_SIM_SCREENSHOT="125:…"` (TAKEOFF must fire *after* FM_JOIN_SUCCEEDED is processed — a TAKEOFF click at 30 s raced the join and silently no-op'd) | ⚠️ **WITHDRAWN (Sprint 10)** — native frame is the 3D virtual pit, not the 2D pit; verdict void, re-capture pending. Gold alt 10 000 vs native 16 000 ft = saved Furball altitude option, not a deviation |
| 5 | 06-09 15-32-20 | UI main menu (same screen as #1, later Wine build) | same as #1 | **DEVIATION (major)** — same as #1 / DEV-1 |

## Tolerance statement

UI screens: same layout, art, and control positions; palette within ordinary
gamma/driver variance (the Wine desktop adds a title bar and border the native
window does not have — crop before comparing). Sim screens: same cockpit
geometry, HUD symbology and terrain classes rendered; exact terrain texels and
weather state are not required to match (different mission instant).

## Deviations found

Each deviation gets fixed (SP.2) or PO-waived here. Classification per the
julia-racer method: renderer-bug / authentic-asset / asset-gap / prior-decision.

### DEV-1 — Main menu is a different screen entirely (golds 1 & 5) — MAJOR, open (SP.2)

Native renders the legacy Falcon4-style main screen: taxiing-F-16 photo
background, "FREE FALCON 5.0" winged logo, bottom button bar
(LOGBOOK…CAMPAIGN). Both Wine golds show the FF-themed menu: dark cobra +
world-map/blueprint artwork with a right-hand column of gold aircraft
(the FF-skin menu buttons), no bottom bar.

Classification so far: **UI resource/window-selection divergence** (bug class,
not asset gap). Ruled out 07-27: the missing `art/resource/mainbg.irc`
(recreated per the Jan-2026 port notes — byte-identical menu, no change); UI
resource load failures (only the known-benign `help_res.lst` fails). Ruled in:
the game data DOES carry FF-themed full-screen menu art the native build never
displays (`mainbg.idx/.rsc` image `MAIN_SCRN`, 1024×768 8-bpp, decodes to a
blueprint/logo composition). Root cause — which window/art set the Windows exe
selects vs what `MAIN_SCF.LST` gives the native parser — is SP.2 work.

### DEV-2 — 2D-cockpit bitmap renders far brighter than gold (golds 2 & 4) — ⚠️ **INVALID COMPARISON, see DEV-2/3 RETRACTION below** (Sprint 10)

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

### DEV-3 — Bottom sliver of 2D-pit art (pilot hands) not visible (gold 4) — ⚠️ **INVALID COMPARISON, see below** (Sprint 10)

_Original Sprint-9 text:_ The gold shows the pit art's bottom edge (glove/hand
shapes) below the standby gauges; the native frame ends at the gauge bottoms.
Possible small vertical offset/scale difference in the 2D-pit blit.

### ⚠️ DEV-2 / DEV-3 RETRACTION — the two frames are DIFFERENT VIEWS (Sprint 10, SP.2-A)

**The native "2D pit" captures are not the 2D pit. They are the 3D virtual
cockpit.** DEV-2 and DEV-3 therefore compare a lit 3D cockpit *model* against a
flat 2D pit *bitmap*, which is not a parity comparison at all. Both deviations
are withdrawn pending a matched-view re-capture; no renderer fix should be
attempted on either until then.

**Evidence (offline, from the committed Sprint-9 thumbnails + the game data —
no new capture required):**

1. **The native frames contain a canopy bow with rear-view mirrors**, drawn in
   perspective with 3D shading, arching over the HUD glass. Visible in *both*
   `sp2-native-iapit.png` and `sp4-native-dfpit.png`.
2. **No 2D-pit art set in this install declares a mirror.** The manager's parser
   token is `MIRROR` (`TYPE_MIRROR_STR`, cpmanager.cpp:646 → `CPMirror`); it has
   **zero** occurrences in all four candidate pit `.dat`s —
   `art/ckptart/16_ckpit.dat`, `F-16CG/16_ckpit.dat`,
   `F-16CJ_MicroProse/10_ckpit.dat`, `F-16CJ_PaulWilson_Widescreen/ws_ckpit.dat`.
   A 2D pit in this data therefore *cannot* draw the mirrors that are in the
   native frames.
3. **Both golds show the bare HUD combiner posts with open sky above** — no bow,
   no mirrors. The native frames have those same posts **plus** the bow.
4. **Structure profile** (both normalised to 1024×768, std-dev of luminance in
   the centre band x380–640, 16-px rows): native carries structure at y0–112
   (the bow) where gold is clean sky; gold carries structure at y176–336 (the
   posts) where native is clear glass. Panel top is the same in both (~y340), so
   this is not a scale difference.

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

**Open (SP.2-B/C):** confirm the display mode at capture time from the
`FF_DEBUG_PITSEL` trace; establish why mode 1 did not stick (candidates: the
request never fired within the run; the 2D `CockpitManager` failed to load and
`gDoCockpitHack` suppressed the 2D path; hybrid pit auto-switch — gated on
`GetHybridPitMode()==1`, which initialises to 0, so this is the weakest);
then re-capture golds 2 and 4 in a *confirmed* `Mode2DCockpit` and re-issue
verdicts.

### Notes (not deviations)

- Gold 3 is a native June capture (see inventory note): the 07-27 parity there
  confirms **no regression** in the dogfight lobby since June, not Windows
  parity. The add/remove roster toolbar icons visible in the gold are absent in
  the 07-27 frame — roster state differs (saved game now contains Crimson1);
  check icon visibility when re-testing with an empty roster in SP.2.
- Gold 2 vs native: different mission instant (terrain area/bank angle) —
  within stated tolerance; the pale far terrain in the native IA frame needs a
  matched-instant re-check after DEV-2 before being called a separate deviation.

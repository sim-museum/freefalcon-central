# FreeFalcon — Run & Check Progress

## Run the game

```bash
cd /home/admin/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6
/home/admin/free-falcon/build/src/ffviper/FFViper -d "$PWD" -w
```

Requires a healthy GL display session. The window title shows the build's git
hash — check it if behaviour doesn't match the latest commit.

**RWY-2 acceptance check (PO):** Tactical Engagement → "09 Landing Final
Approach" → fly the approach. The runway should be visible over the terrain from
approach through touchdown (the Sprint-8 depth-bias fix, default-ON).
`FF_RUNWAY_NOBIAS=1` shows the old broken behaviour for comparison.

Rebuild: `cd /home/admin/free-falcon/build && ninja` (ASAN variant in `build-asan/`).

## Behaviour-changing flags and how to revert them

Most `FF_*` environment variables are diagnostics. A few **change how the sim behaves by
default** — these are the ones to reach for if something looks wrong after an update.

| flag | effect |
|---|---|
| `FF_NO_RUNWAY_LODGATE=1` | Reverts the runway LOD-provenance gate (**default ON** since 2026-08-26). The gate makes the drawn runway take the nearest terrain post when a coarse LOD answers the ground query, instead of an interpolation that does not preserve the flattened airbase plateau. It fixes the aircraft appearing to sink into the runway and emerge again while rolling. Revert if runway surfaces look wrong or the tarmac disappears. |
| `FF_NO_FEATURE_RESNAP=1` | Reverts the static-feature ground re-snap (airbase objects baking a ground height sampled before terrain streamed in). |
| `FF_NO_GEAR_LIFT=1`, `FF_RUNWAY_ZLIFT=<ft>`, `FF_GEAR_LIFT=<ft>` | The runway decal / aircraft visual-lift stack. Flat surfaces are drawn ~3 ft above terrain to win the depth test, and the aircraft drawable is lifted to match. Setting these to 0 makes the tarmac disappear — see docs/STATUS.md. |
| `FF_NO_SEEKER_TTG_FIX=1`, `FF_FCC_HANDOFF_RADAR=1` | Revert the SEEKER-1 and HANDOFF-2 fixes. |
| `FF_NO_TYPEPTR_REFRESH=1` | Reverts **the UAF-1 fix**: re-pointing every live entity's cached `entityTypePtr_` after `SetNewTheater` reloads the class table. Without it, entity type 1144 holds a pointer into the freed pre-reload table in **6 of 6** runs and the sim crashes in ~1 of 3; with it, **0 of 15** runs show a stale pointer. Only set this to reproduce the bug. |

### Useful diagnostics

| flag | prints |
|---|---|
| `FF_DEBUG_GEAR=1` | `[GEAR2]` gear position/DOF/flags twice a second, and `[GEAROVR]` if the gear-overspeed trip fires |
| `FF_DEBUG_GROUND=1` | `[GROUND]` aircraft vs terrain height, and which LOD answered |
| `FF_DEBUG_MESHZ=1` | `[MESHZ]` terrain post height at every LOD beside both ground queries |
| `FF_DEBUG_CHKHT=1` | `[CHKHT]` the nose/wing/gear/body ground-contact terms |
| `FF_DEBUG_SLOT=1` | `[SLOT]`/`[SLOT-SRC]` — a weapon attach asking for a model slot the LOD does not define. Reports parent LOD id, slot, `nSlots`, and on the caller side the vehicle type, hardpoint index and `VisibleFlags`. Reproduces on TE-26 (HARMs), twice per run. |
| `FF_DEBUG_ROE=1` | `[ROE]` — `GetRoE` called for a team with no `TeamInfo` entry, with a backtrace. **Capped at 5 backtraces**: it fires ~2849 times in one TE-01 run. |
| `FF_DEBUG_VUDEL=1` | `[VUDEL]` an entity deleted while still `VU_MEM_ACTIVE` (i.e. never removed from its collections), plus `[VUDEL-ALL]` counting **every** refcount-zero delete. The second is the control — without it, "no bad deletes" and "the probe never ran" are indistinguishable. |
| `FF_DEBUG_TYPEPTR=1` | `[TYPEPTR]` an `entityTypePtr_` reaching `UnitProxFilter::RemoveTest` that lies **outside** the live `Falcon4ClassTable`, plus a first-call liveness line. **This is the probe that solved UAF-1** and is now its regression check: expect 0 reports. With `FF_NO_TYPEPTR_REFRESH=1` it reports exactly 1 per run. |
| `FF_DEBUG_GRIDMAGIC=1` | `[GRIDMAGIC]` `VuGridTree::Move()` called on a grid whose liveness sentinel is poisoned — a torn-down grid still reachable from `gridcoll_` — plus a first-call liveness line. Measured 0 across 40,000 `Move()` calls; kept as the regression check for the `~VuGridTree` deregister-before-teardown fix. |
| `FF_DEBUG_NVGOPS=1` | `[NVGOP]` each D3D texture op that **is** implemented, once per op, and `[NVGOP-UNHANDLED]` each op that is **not** — where `ApplyTextureStageState` falls through to a silent `GL_MODULATE`. The pair matters: without the second line, "the pipeline is fully implemented" and "an op is quietly mapped to the wrong thing" look identical from outside. NVG-5 diagnostic. |
| `FF_TEST_GEARDOWN=<sec>` | drops the gear handle at a fixed time, so gear behaviour can be tested by script |

**Known gotcha, not a bug:** lowering the landing gear above ~275 kt breaks a random gear
(`eom.cpp:1608`), which then parks part-deployed and leaves the aircraft resting on its
fuselage — it looks "half-buried in the airstrip". Slow below 250 KIAS before lowering
the gear. See `docs/STATUS.md` (GEAR-5).

## Check progress

| What | Where |
|---|---|
| Sprint board + backlog + evidence | `docs/STATUS.md` (single source: board rows, RWY-2 root cause + evidence, TE-02/TE-09 defects, EPIC SP) |
| Runway before/after evidence crops | `docs/rwy2/` |
| Screen-parity table | `docs/screen-parity.md` (starts in Sprint 9 — inventory was lost to a session limit and will be redone) |
| Cross-port notes | `docs/CROSS-PORT-*.md` |
| History | `git log --oneline` on `develop` |

Gold standard: `/run/media/admin/BEA6-BBCE/free falcon/` (5 PNGs). Open PO
question: shot 3 may be a native capture rather than a Wine gold — provenance to
confirm.

## Current state (2026-08-10, after Sprint 22)

**Headline: the port's landing is quantitatively equivalent to the Windows
landing.** PO hand-flew TE "09 Landing Final Approach" under gdb with ACMI
recording; the tape decodes to touchdown at **36 ft** (gold: 36 ft), rollout
minimum **28.3 ft** (gold: 28.3 ft), spread **7.6 ft** (gold: 8.8 ft). First
hard evidence that flight and collision behaviour match the oracle.

**ACMI is now a working measurement instrument.**
- `tools/acmi_dump.py <tape.vhs>` — summary, entity list, `--track`,
  `--approach`, `--csv`. Validated on 5 tapes.
- `tools/acmi_dump.py` also exposes `read_flt()` for raw `acmibin/acmi*.flt`
  recordings — needed because the in-game import is broken (ACMI-4).
- A Windows-recorded tape now **loads in the in-game player** (Sprint 20).

**To record a flight:** launch with `FF_ACMI_RECORD=1`; recording starts at
mission start. **Do not press `f`** — it toggles ACMI recording
(`SimAVTRToggle` → `ACMIToggleRecording`) and will stop a recording already
running. There is no on-screen indicator because `g_bACMIRecordMsgOff` defaults
true. **Back up `acmibin/acmi0000.flt` before any import** — ACMI-4 deletes it
and can produce nothing.

### Open defects, priority order

| id | what | note |
|---|---|---|
| **CAMP-1** | campaign 3D entry **deadlocks forever** on a white screen | `WaitForSingleObject(wait_for_sim_cleanup, INFINITE)` at `simloop.cpp:1386` on the failed-launch bail path, never signalled. TE is unaffected. See `docs/STATUS_2026-08-14_session.md` |
| **SETUP-1** | clicking **Setup** on the main menu SIGSEGVs | NULL `KeyDescrips[key2]` into `strcat`, `controltab.cpp:2887`. **Avoid the Setup button until fixed** |
| **ACMI-4** | `ACMI_ImportFile` fails silently **and deletes the `.flt`** | data-destroying; read `.flt` directly meanwhile |
| **RWY-3** | 12/31 runway posts change elevation mid-approach | the "retracting airfield"; **rendering** defect. Criterion: no post may change elevation during an approach |
| **JOINFAIL-1** | `CampaignJoinFail` → `RemoveUserCallback` SIGSEGV | the graceful-failure path itself crashes |
| **TERRAIN-1** | grey untextured surface under the dogfight arena | |
| **LOAD-1** | white screen instead of the loading animation | regression vs `cc4e2517` |
| **PIT-1 / GEAR-1** | 2D pit low/small in view 1; no landing gear in view 0 | |
| **AP-1** | duplicated `(not StrgSel) and (not StrgSel)` guard | upstream; PO call |
| **ACMI-5** | no automatic `.flt`→`.vhs` on mission end; indicator suppressed | usability |

**AUTOSAVE-1 is fixed but still unverified in flight** — it needs one campaign
mission flown to completion, returning to the map without crashing.

## Previous state (2026-08-09, after Sprint 12)

- **Sprint 12 PARTIAL 5/8 — DEV-1 reclassified, not fixed.** The native renders
  the main menu **correctly for this data**: `main_win.scf:20` names
  `UI_MAIN_BG`, which resolves to `art/UISkin/ff4/UIMAINBG` = the F-16-in-shelter
  photo you see, and the bottom button bar is `main_win.scf`'s own y=728 layout.
  A wrong Sprint-9 finding is corrected: `MAIN_SCRN` is a **radar-scope** image
  and the window never asks for it.
- **Golds 1 & 5 are probably a provenance problem.** All 751 `.idx` files were
  searched; of 69 unique 1024×768 images none matches the gold's blue
  blueprint/cobra menu (the only two near-black candidates are empty).
  **PO question: were golds 1 & 5 shot against a different install/version/skin?**
  Same class as the existing note on gold 3.
- **Three PO decisions now pending**, none needing code: waive DEV-2, waive
  DEV-4, and confirm gold 1/5 provenance.
- **DEV-3 still carried** — gold 4 has never been re-shot with the trace. That is
  the obvious next sprint, and it is a sim run (~2.5 min under the lock).

## Previous state (2026-08-08, after Sprint 11)

- **Sprint 11 closed 7/8.** Both of Sprint 9's "major" 2D-pit deviations are
  **resolved as non-defects**, and gold 2 is upgraded to **PARITY**:
  - **DEV-4 (canopy bow) = the canopy-reflection visual cue.** A/B-proven:
    `FF_PIT_VISCUE=0` makes it vanish. It is on because
    `PlayerOptions.SimVisualCueMode = VCReflection` — a legal setting, not a
    renderer bug. **Recommend PO waive.**
  - **DEV-2 (panel too bright) = a time-of-day difference.** Force the gold's
    light level (`FF_PIT_LIGHT=0.55`) and the native panel reproduces the gold
    almost exactly (p95 106.0 vs 107.0). The panel path is correct.
    **Recommend PO waive.**
  - **Two PO decisions requested**, both waivers. Neither needs code.
  - New backlog: ~~LIGHT-1~~ **WITHDRAWN 2026-08-09 — the defect does not exist**
    (the callback has always been registered at cpmanager.cpp:677–679; a
    truncated grep hid it, and the "fix" double-registered → use-after-free.
    Reverted). **LIGHT-2** (`cLight` unclamped) is still open.
  - **DEV-3 still carried** — gold 4 not yet re-shot with the trace.
- **New diagnostics:** `FF_PIT_VISCUE=<n>` (force visual-cue mode),
  `FF_PIT_LIGHT=<f>` (force cockpit environment light). Both diagnostic-only.
- `ff_validate.sh` now uses `timeout -k 5`, and **must be invoked from the repo
  root** (it resolves `tools/…` relatively — running it from `build/` fails 127
  after taking the display lock).

## Previous state (2026-08-08, after Sprint 10)

- **Sprint 10 closed 8/8 (EPIC SP.2).** No renderer fix shipped — deliberately.
  The sprint's output is that the Sprint-9 sim-screen verdicts could not be
  trusted as written, plus the diagnostic that makes them trustworthy from now
  on.
  - **`FF_DEBUG_PITSEL=1`** (new) prints the cockpit art file requested vs
    actually resolved, whether it opened, the h/v scales, and the live display
    mode once a second. **Use it on every sim parity capture** — see the
    Sprint-10 retro in `docs/STATUS.md` for why.
  - **DEV-2 (2D pit too bright): symptom confirmed, mechanism disproved.** It is
    not "missing TOD/palette shading on a palettized bitmap" — the 2D pit is
    drawn from lit 3D model geometry (`cockpit2d 2358 2358`). It is a
    pit-geometry *lighting* question. Panel median luma 28.7 gold vs 45.5 native.
  - **DEV-4 (new): native draws a canopy bow the gold lacks**, in a confirmed
    identical view and art set. Leading candidate is a switchable component of
    model 2358 left at its default-visible state (the 2D path sets only switch
    masks 7 and 3; the 3D pit sets 263).
  - **DEV-3 suspended** until gold 4 is re-shot with the trace.
  - Outbound cross-port note 15 (methodology, class-level) delivered to MA + BoB.

## Previous state (2026-07-27)

- Sprint 8 closed (`9ed8f3b2`/`c479a0d0`): landing-strip z-fighting fixed
  default-ON (slope-scaled depth bias on tagged runway batches) — **awaiting the
  PO acceptance flight (RWY-2)**.
- Sprint 9 closed (`e79889c9`/`dd94d06b`): parity verdicts for all 5 PO gold
  shots — 1 parity (dogfight lobby), 2 partial, 3 classified deviations:
  **DEV-1** native shows the legacy Falcon4 photo menu instead of the FF
  cobra/blueprint menu (art present but never selected — PO fix-or-waive call
  queued); **DEV-2** 2D-pit bitmap far brighter than gold (suspected missing
  TOD/palette shading); **DEV-3** pit-art bottom sliver/pilot hands missing.
  Evidence thumbnails in `docs/screen-parity/`.
- Scope re-expanded 2026-07-26/27: FF is back in the sprint rotation
  (ma → bob → free-falcon, sequential). Recommended next FF sprint: SP.2
  starting with DEV-2. Operational: sim-mission load is ~80–100 s — capture
  sim screens at ≥110 s process-relative (recipes in `docs/screen-parity.md`).

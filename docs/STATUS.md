# FreeFalcon Linux Port — Current Status

_Last updated: 2026-06-23. Branch `develop`, all commits pushed to origin._

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

1. **Runway landing (highest value).** Two distinct sub-problems, both now fixed and
   awaiting a PO verification flight of the "Landing Final Approach" TE:
   - *Elevation:* flat runway surfaces only refreshed z on an LOD change, so on a
     steady approach they stayed frozen at the far-time coarse value while collision
     used the fine elevation. Fixed: flat surfaces re-fetch the accurate
     `GetGroundLevel` every frame (`FF_RUNWAY_OLD=1` reverts).
   - *Visibility:* placing the runway exactly AT the terrain z made it z-fight the
     terrain mesh and vanish. Fixed: lift it ~3 ft above the terrain (a decal),
     tunable via `FF_RUNWAY_ZLIFT`. (`GetGroundLevel` returns the real height only
     when the player is near the airfield; far away it falls back to ~0 — so the value
     resolves correctly on approach.)
   - **Verify:** fly the landing TE; runway should now be visible and the jet land on
     it. `FF_DEBUG_RUNWAY=1` logs `[RUNWAY] flat GetGroundLevel=.. decal=.. -> z=..`.
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

### RWY-2 — Landing-strip z-fighting REOPENED (top priority — failed acceptance)

The PO reports the defect **persists**: z-fighting at landing strips makes the landing
strip itself invisible — the terrain covers the runway. This is the acceptance test of
the Sprint-3 fix (runway decal lift, `FF_RUNWAY_ZLIFT`, commit `8e5db807`) **failing**,
so the story returns to the sprint backlog as a defect, not a new discovery.
Investigation starters: (a) is the decal lift actually default-ON in the shipped path,
or still env-gated? (b) is ~3 ft enough given the far-terrain LOD z error at approach
distances (the elevation fix refreshes z per frame only for *flat* surfaces — does the
runway polygon take that path in all LODs)? (c) consider a true depth-bias route now
that the flat-surface flush path is understood (`glPolygonOffset` on the runway batch),
since a world-z lift fights the collision height. Cross-port note: BoB fixed its
external-view z-fighting with default-on scene depth-sorting (S118–S119) — different
renderer, same defect class; read their notes before re-attempting.
Acceptance: PO (or `tools/ff_validate.sh` capture on final approach) sees the full
runway surface rendered over the terrain from approach through touchdown.

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

Fly the landing TE to verify the runway fix — it's the single highest-value action,
and only the PO can do it. Everything the agent can validate autonomously is done.
</content>

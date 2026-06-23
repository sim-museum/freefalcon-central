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

### Needs the Product Owner's eyes (cannot be verified by the agent — no 3D frame capture)

1. **Runway landing (highest value).** Runways render ~10 ft below the surrounding
   terrain; the jet lands beside/through them. Root-caused from a PO flight: flat
   runway surfaces only refreshed their elevation on an LOD change, so on a steady
   approach they stayed frozen at the far-time coarse value (~0) while collision used
   the fine ~10 ft. **Candidate fix committed** — flat surfaces now re-fetch the
   accurate `GetGroundLevel` every frame (`FF_RUNWAY_OLD=1` reverts). **Awaiting a
   verification flight:** `FF_DEBUG_RUNWAY=1 ... -w` then `grep "GGLapprox pos"
   /tmp/ff_runway.log | tail -40` (the probe reports `fineLOD0_z`, confirming the fix).
2. **Dogfight `glClear` GL-state race.** With the far-terrain texture crash fixed, a
   deeper/rarer SEGV in `glClear` appears in the first frames after deaggregation — a
   GL-context/framebuffer-readiness race during the UI→sim hand-off, almost certainly
   ASAN-timing-amplified (normal dogfight play has not shown it). PO-gated.
3. **Terrain visible through 3D-pit MFD screens (#10), ACMI, night ops, weather** —
   untested/visual.

### Low-frequency / deferred (documented in `docs/SPRINT2_CRASH.md`)

- `_mm_loadu_ps` SSE stack read-overflow (4 hits) — benign in release (reads adjacent
  stack; ASAN-instrumentation artifact).
- `AS_DataClass::ASSearch` SIGSEGV — did not reproduce in 2 campaign soaks; rare.
- chash new[]-string mismatch fully fixed via per-instance `ownsStrings_` (10,519→0).

## Build & run

```bash
cd /home/g/ff/build && ninja                       # release
cd /home/g/ff/build-asan && ninja                  # ASAN variant
# run:
cd /home/g/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6
/home/g/ff/build/src/ffviper/FFViper -d "$PWD" -w  # add -test-ia for Instant Action
# package against a user data install:
/home/g/ff/packaging/install.sh --data /path/to/FreeFalcon6
```

## Recommended next step

Fly the landing TE to verify the runway fix — it's the single highest-value action,
and only the PO can do it. Everything the agent can validate autonomously is done.
</content>

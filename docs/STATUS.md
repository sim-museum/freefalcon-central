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

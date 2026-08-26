# QA coverage scripts

Six memory-safety defects were found in this project in code that **no soak had
deliberately exercised** — they surfaced only because one script happened to route
through the THEATER screen, and another happened to load HARMs. A soak's coverage is
whatever path its click script walks and nothing more, so these scripts exist to make
coverage a *stated* claim rather than an accident.

Run them after changes in the area each one covers.

| script | covers | build | runtime |
|---|---|---|---|
| `te-sweep.sh [first] [last]` | all 34 TE missions — reaches sim, crashes, assertion counts | release (`build-relg`) | ~90 min |
| `asan-sim-pass.sh` | 8 TE missions chosen for **distinct subsystems**: basic handling, takeoff, landing, flameout landing, AIM-9, CCRP bombing, guns A-G, HARMs | ASAN (`build-asan`) | ~30 min |
| `asan-sim-rest.sh` | the other 26 TE missions | ASAN | ~85 min |
| `camp-fly-asan.sh [secs]` | campaign flight — entity churn, deaggregation, ATM | ASAN | ~7 min |
| `te02-repro.sh [secs]` | single TE ground start with the terrain/gear probes wired up | release | ~2 min |

**Not covered by any of these**: the main-menu screens. That pass is a click script over
LOGBOOK / TACTICAL REFERENCE / ACMI / SETUP / COMMS / THEATER / TACTICAL ENGAGEMENT /
INSTANT ACTION / DOGFIGHT under ASAN — it found the seventh defect and is worth
re-running after UI changes. See `docs/STATUS.md` (ASAN-4) for the click list.

## Reading the results

`sim=1` means the run reached 3D. **A zero error count from a run that never reached the
code proves nothing** — check the run got there before trusting a clean result. The
campaign soak, for instance, is only meaningful if the deaggregation trace count is high.

Assertion counts are **coverage, not frequency**: `ShiAssert` fires once per site per
process and prints two lines, so `asserts=8` means four distinct sites, not eight events.
Counts also vary run to run when an unpiloted aircraft crashes somewhere different, so
**compare two runs per configuration** before calling a difference a regression.

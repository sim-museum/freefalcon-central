# FreeFalcon Linux port — session status, 2026-08-09

Sprints 10–22, branch `develop`, all commits pushed to
`origin` (`sim-museum/freefalcon-central`).

Detail lives in `docs/STATUS.md` (per-sprint reviews) and
`docs/screen-parity.md` (EPIC SP). This file is the readable snapshot.

---

## Headline

**The port is in better shape than the backlog claimed.** Of the deviations
carried into this session, **five were not defects at all** — they were
artifacts of how they had been measured. The genuine defects found were found by
reading code and decoding real data, not by comparing screenshots.

The single strongest result: **our landing is quantitatively equivalent to the
Windows landing.**

| | gold (Wine) | ours (native) |
|---|---|---|
| touchdown | t+156.6 s, 36 ft | t+160.9 s, 36 ft |
| touchdown position | (772728, 1309737) | (773345, 1309997) |
| rollout min altitude | 28.3 ft | 28.3 ft |
| rollout spread | 8.8 ft | 7.6 ft |

Two hand-flown approaches, ~670 ft apart, identical touchdown and minimum ground
altitude, ours marginally tighter. Profiles: `docs/acmi/landing_{gold,ours}.csv`.

---

## Fixed this session

| ID | defect | verification |
|---|---|---|
| **AUTOSAVE-1** | `Encode` wrote 32-byte native event nodes while `Decode` read the 20-byte 32-bit-Windows layout — 12 bytes of drift per event, garbage count, `bad_alloc`, **crash on every campaign mission end** | code fix symmetric and unambiguous; **not yet seen working in flight** |
| **ACMI-1** | `.vhs` on-disk structs declared with `long` (4 bytes on Win32, 8 here) — Windows tapes unreadable, ours unreadable on Windows | 5 tapes parse; 4 block-offset identities `static_assert`-pinned |
| **ACMI-2** | `SetupSimTapeEntities` SIGSEGV: `ACMI_Callsigns[uniqueID]` indexed with no bound (ids run to 477); `ACMI_CallRec.teamColor` `long`→`int32_t` (24→20 B, wrong memcpy stride); two `delete`/`new[]` mismatches | **a Windows tape now loads in the player**; in-game clock `05:04:03` matches the independent Python decode |
| **ACMI-4 (partial)** | `ACMI_ImportFile` imported a *live* recording, truncating the tape (15.8 s of a multi-minute flight) | now calls `StopRecording()` first, inside `ACMI_ImportFile` so every caller is safe |
| — | `ff_validate.sh` used `timeout` without `-k`, so a run ignoring SIGINT could outlive its cap holding the display lock | hardened |

---

## Open defects, by priority

1. **RWY-3 — the retracting airfield.** 12 of 31 runway posts change elevation
   mid-approach (`-19.6 → -25.1 → -26.0`; one starts at 0.0, i.e. sea level) as
   terrain LOD refines. Feature data carries `ORIGINAL z = 0.00` for every post,
   so elevation comes entirely from `GetGroundLevel`, coarse at range.
   **This is a rendering/geometry defect, not flight dynamics** — the aircraft
   lands correctly because collision uses the converged value; the *rendered*
   surface is what moves.
   **Criterion: no runway post may change elevation during an approach.**
2. **AUTOSAVE-1 verification** — needs one campaign mission flown to completion
   and returning to the map without crashing. Cannot be reliably automated.
3. **JOINFAIL-1** — `CampaignJoinFail()` → `RemoveUserCallback()` SIGSEGV. The
   try/catch that turns a failed load into a clean return-to-menu *does* catch,
   then the handler crashes. AUTOSAVE-1 removes the main trigger; the hole
   remains for any other load failure.
4. **ACMI-4 (remaining)** — `ACMITape::Import` fails silently **and deletes the
   `.flt` anyway**. A recording is consumed and nothing written in its place.
   Keep a backup before importing. Worked around by reading `.flt` directly.
5. **TERRAIN-1** — dogfight arena shows uniformly grey terrain, airfields on flat
   grey polygons.
6. **LOAD-1 (regression)** — white screen instead of the animated loading
   progress bar; that animation was fixed in June (`cc4e2517`) and
   `FF_LoadingClear` should paint black, never white.
7. **PIT-1 / GEAR-1** — 2D pit low and small in view 1; no landing gear in view 0.
8. **AP-1 (upstream, registered not fixed)** — `SimRightAPSwitch` guards with
   `(not StrgSel) and (not StrgSel)`; two sibling sites use
   `(not StrgSel) and (not HDGSel)`. With the left switch in HDG SEL the guard
   passes and force-sets `RollHold`. Predates the port, so Windows has it too —
   fixing it diverges from the oracle. **PO call.**
9. **LIGHT-2** — `ComputeLightFactors` assigns `cLight[i] = eLight` with no upper
   clamp (the 1.0 clamp is only in the flood-light branch) while
   `GetLightLevel()` returns `Ambient + Diffuse`, which can exceed 1.0.
10. **No ACMI recording is reachable without env vars in single-player** —
    `f` works (`SimAVTRToggle` → `ACMIToggleRecording` → `doFile` →
    `gACMIRec.ToggleRecording()`), but nothing converts the `.flt`, and
    `g_bACMIRecordMsgOff` defaults true so there is no on-screen indicator.

---

## EPIC SP — closed, all four deviations were method artifacts

| ID | was | outcome |
|---|---|---|
| DEV-1 | "main menu is a different screen" | the golds were a **loading screen**; the real menu matches at **0.9908** correlation |
| DEV-2 | "2D pit far brighter than gold" | **time of day**; at matched TOD the panels agree (59.9 vs 58.6) |
| DEV-3 | "pit bottom sliver / hands missing" | **does not reproduce** |
| DEV-4 | "native draws a canopy bow" | the **`VCReflection` player option**; the gold shows it too |

Common root cause: **comparing frames without recording the state that produced
them** — view mode, time of day, player options, or even which screen it was.

---

## Tools added

| tool / hook | purpose |
|---|---|
| `tools/acmi_dump.py` | decode a `.vhs` (or raw `.flt` via `read_flt()`) into a position/altitude/attitude time series |
| `tools/gold_video.sh` | pixel-exact 1024×768 client frames from the Wine gold videos at any timestamp |
| `FF_DEBUG_PITSEL=1` | resolved cockpit art set, whether it opened, scales, and the **live display mode** |
| `FF_PIT_VISCUE`, `FF_PIT_LIGHT` | force visual-cue mode / cockpit environment light for A/B |
| `FF_AP_MODE`, `FF_DEBUG_AP` | force the autopilot mode **at the dispatch site**; trace `ToggleAutopilot` at entry |
| `FF_ACMI_RECORD`, `FF_ACMI_STOP`, `FF_ACMI_IMPORT` | start / flush / convert an ACMI recording |

---

## Gold standards

- **Video (preferred).** `~/gold standard/free falcon/260808/` — 1920×1080@60
  with the game windowed at **1024×768**, i.e. our own capture resolution, so
  frames crop out pixel-for-pixel with no rescaling. Build:
  FreeFalcon 6.0 / FFViper 2.3.3.44.
- **Stills (superseded).** The five PNGs; shots 1 and 5 were a loading screen and
  shot 3 is a native capture. Not reliable oracles.
- ACMI tapes `TAPE0006–0010.vhs` + the landing tape — now readable, and the
  richest oracle available (the landing tape is 716 samples; `TAPE0006` is 45 755).

---

## Process lessons banked

1. **A parity capture must record the state it claims to be capturing.** Sent as
   cross-port note 15.
2. **Never pipe a `grep` through `head` when the question is "does X exist".**
   Three wrong conclusions in one session, the worst being a "LIGHT-1" defect
   that did not exist plus a "fix" that double-registered a callback and created
   a **use-after-free**. Reverted.
3. **Absence of output is evidence only when the instrumentation is proven able
   to produce output in the case being ruled out.** Failed twice — a truncated
   grep, then a trace scoped inside the wrong branch.
4. **Always run the control arm, and state its predicted value first.** The bogus
   LIGHT-1 fix was caught *only* by its control: predicted 9-vs-1, measured
   9-vs-15.
5. **Validate a similarity metric on a known-positive before believing a
   negative.** A resource search reported "no match" using a metric that could
   not find the known-correct answer either.
6. **Check the full series before deriving a criterion from it.** "The gold is
   pinned at 28 ft" was a subsampling artifact.
7. **Establish that an instrument measures the quantity in question.** ACMI
   records aircraft state, so it can never measure whether *scenery* moved.
8. **A positive measurement against real data beats an absence-of-evidence
   argument.** ACMI-1 (parse a real tape both ways) held up; LIGHT-1 ("I can't
   find the registration") did not.

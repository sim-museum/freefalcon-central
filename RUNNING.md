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

## Current state (2026-07-27)

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

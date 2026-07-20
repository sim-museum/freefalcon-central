---
title: "FreeFalcon 6 — Linux Port: Completion Plan (Scrum)"
date: "June 21, 2026"
---

# Product Goal

Make the **Falcon 4.0 lineage** (FreeFalcon 6) fully playable and *permanently
preserved* on a native, open-source Linux stack (SDL2 / OpenGL / OpenAL / CMake),
no longer tied to the finite lifetime of any Microsoft Windows release.

Product Owner: **the user** (pre-approves each sprint result and the start of the
next sprint). Developer/Scrum execution: Claude (autonomous between impediments).

# Definition of Done (project)

1. All single-player modes playable end-to-end on Linux: Instant Action, Dogfight,
   Tactical Engagement, Campaign (strategic map + tactical mission + debrief).
2. No known crashes in the main flows; clean exit (code 0).
3. Rendering visually correct: no terrain bleed through the cockpit; runways
   visible and landable; night + weather paths verified.
4. Code hardened against the 8 documented bug classes (no remaining known sites).
5. Windows build kept green via `#ifdef FF_LINUX` discipline (audited).
6. Packaged for distribution (AppImage or install script that ingests a
   user-supplied data install) + reproducible build documentation.
7. Pushed to the public preservation repo.

# The 8 bug classes (regression guard — every new symptom is checked against these)

1. 32-bit `long`/`ulong` in Windows binary file formats (use `int32_t`/`uint32_t`).
2. 64-bit pointer truncation via `(int)`/`(DWORD)`/`(GLint)` casts (use `intptr_t`).
3. `sizeof(DDSURFACEDESC2)` read from a 124-byte on-disk DDS header.
4. MSVC `RAND_MAX`==32767 assumptions vs glibc `rand()`.
5. CRLF left on the last token after text-mode `fgets`.
6. Silently default-returning compat stubs.
7. Signal-less infinite waits / lock-order (Camp↔Vu) inversions.
8. OpenGL state-at-call-time vs D3D state-at-draw-time (clear masks, light xform,
   per-vertex emissive, DXT1 RGBA variant).

# Known hard impediment — RESOLVED (July 2026)

~~The agent cannot capture sim-mode (3D) frames.~~ **Sim-mode frame capture works.**
The capture must happen on the thread that owns the GL context — in sim mode that is
the SIM thread — inside its own swap path, i.e. in `ImageBuffer::SwapBuffers`
(`src/graphics/ddstuff/imagebuf.cpp`) immediately before `FF_SwapBuffers()` /
`SDL_GL_SwapWindow`, when the frame is complete. A `glReadPixels` issued from the main
thread reads a context it does not have current (→ white), and an external window grab
is black under Wayland/XWayland while the sim thread is presenting.

The read is additionally guarded (`SaveGLFramebufferAsBMP`, `src/compat/d3d_gl.cpp`)
against the state the sim thread can be in: default framebuffer bound (a cockpit RTT
canvas FBO may still be current), no pixel-pack buffer, `GL_BACK` read buffer,
`GL_PACK_ALIGNMENT` 1, `glFinish` first. `FF_NO_CAPTURE_FIX=1` reverts the guards.

Hooks: `FF_SIM_SCREENSHOT="<sec>[:<path>];..."` (sim), the `s` action of
`FF_VIEW_SCRIPT` (sim), `FF_UI_SCREENSHOT=<sec>` (UI). Objective validation harness:
`tools/ff_validate.sh <tag> [-m sim|ui] [-t sec] [-v viewmode] [-c clicks]` — captures
one frame and prints size / distinct colours / non-black % / per-band average RGB,
exiting non-zero on a blank frame.

Note: the documented "FF_VIEW_SCRIPT screenshots SIGSEGV in libnvidia-glcore right
after glReadPixels" is **not** a capture bug — the same SIGSEGV (far-terrain
`DrawVertices`, Sprint 3 issue C) reproduces identically in a run with no screenshot
at all.

---

# Sprint plan

Each sprint has its own Definition of Done (DoD) and a review/demo. Sprints that
need the PO's eyes are explicitly gated.

## Sprint 0 — Plan & baseline (this session)
- Establish the Scrum board, this plan, baseline build + ASAN status.
- **DoD:** plan committed; `ninja` clean; ASAN build present; board populated.

## Sprint 1 — Correctness hardening sweep (autonomous)
Systematically find and fix every remaining instance of the 8 bug classes,
focusing on the ones flagged "latent / audit remaining" in the logs:
remaining `sizeof(long)` in *save* (write) paths and loaders; `new[]`/`delete`
mismatches in `src/falclib/msgsrc/*`; remaining `sizeof(DDSURFACEDESC2)` reads;
`rand()` vs hardcoded constants; default-returning compat stubs; pointer-cast
truncation; CRLF text parsers.
- **DoD:** each class audited to exhaustion; fixes applied or each remaining site
  explicitly triaged as safe; `ninja` + `build-asan` clean; committed & pushed.

## Sprint 2 — Crash elimination (autonomous)
- Reproduce & fix the intermittent `AS_DataClass::ASSearch` campaign-thread SIGSEGV.
- Sweep `src/falclib/msgsrc/*` dtor `new[]/delete` (latent per logs).
- Fix the `objectiv.cpp` `GetFeatureStatus`/`BestTargetFeature` over-reads.
- 5-minute ASAN soak of Instant Action **and** a running campaign.
- **DoD:** no ASAN errors / no SIGSEGV through the soak; committed & pushed.

## Sprint 3 — Rendering correctness  ⟵ **PO visual gate**
- #10 terrain visible through 3D-pit MFD screens / lower panels.
- Runway/airfield elevation decoupling (invisible runways, cannot land).
- **DoD:** candidate fixes behind env toggles + a concise PO test script; PO
  confirms visually. This is the expected impediment hand-off.

## Sprint 4 — Feature coverage (mixed; visual items batched to PO)
- Tactical Engagement missions launch & run.
- Campaign strategic war: time compression, ATO, mission planning, debrief.
- ACMI record/playback; night ops; weather states.
- **DoD:** each area launches and runs without crash; visual confirmations batched.

## Sprint 5 — Cross-platform discipline (autonomous)
- Audit `#ifdef FF_LINUX` guards so the Windows build stays green.
- Recommendations on choice-points 1–3 (renderer path, `_exit`, data layer).
- **DoD:** audit report committed; no obviously Windows-breaking unguarded changes.

## Sprint 6 — Packaging & preservation (autonomous)
- AppImage or install script ingesting a user-supplied install; data-import step.
- Reproducible-build doc; bundle `extern/` recreation; desktop entry.
- **DoD:** a fresh checkout builds and runs per documented steps; artifact produced.

## Sprint 7 — Performance & polish (autonomous)
- Profile texture-upload + immediate-mode submission; cleanup remaining debug spam.
- **DoD:** no per-frame debug output in release; profiling notes captured.

---

# Working agreement

- Commit at each sprint's DoD; push to `origin/develop` (preservation is the goal).
- Keep the Windows path compiling: every change is `#ifdef FF_LINUX`-guarded unless
  it is a portable correctness fix (fixed-width types, `delete[]`, bounds checks).
- Surface an impediment to the PO only when blocked (chiefly: needing eyes on a
  3D frame). Otherwise keep moving to the next backlog item.
</content>
</invoke>

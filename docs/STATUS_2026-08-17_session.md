# FreeFalcon Linux port — session status, 2026-08-16/17

Autonomous scrum session on branch `develop`, all commits pushed to origin.
Started from a PO crash-test session and ran through to an unattended stretch.

Sixteen commits, `d5838b3a` … `3e7e9bbd`. Every fix below has its evidence
recorded with it; where something is **not** verified, that is stated rather
than implied.

---

## Headline

**One heap corruptor was producing six unrelated crash signatures, and the trail
to it ran through three data-format bugs, each hiding the next.**

The port's radio voice chain had never actually worked. Fixing it made the game
reach a stubbed speech codec whose buffer-size variables are never initialised,
which then wrote ~640 bytes past an 80 KB heap buffer on the voice thread. That
stray write landed in whatever allocation followed, so the crashes surfaced in
the campaign UI, the map refresher, a hash table and the sound streamer — none of
which had anything wrong with them.

Measured: campaign crash rate ~1 run in 3 → **0 in 8**; ASAN on the identical
command, heap-buffer-overflow → **0 errors**.

---

## Fixed and verified

| item | defect | evidence |
|---|---|---|
| **CRASH-3** `d5838b3a` | Opening Setup called `strcat(dest, NULL)`. `KeyDescrips` is a 256-entry table memset to NULL and filled only for scancodes that have a description; three other call sites in the same file already guard, `UpdateKeyMapButton` did not | PO repro "selected setup → window disappears"; caught under gdb; Setup and the Controllers key list now render |
| **RES-1** `35187a09` | Setup offered only 640×480, and nothing it saved was ever loaded. Six defects stacked: zeroed `dwDeviceRenderBitDepth` emptied the list; a hardcoded 10-mode table; two copies of a 4:3-only whitelist; `main_linux.cpp` never called `LoadOptions()`; it never loaded the logbook either, so every launch reset logbook + player + display options; `SetWindowPos`/`GetSystemMetrics` were no-ops | Dropdown now 640×480 → 1920×1080; 1920×1080 survives a restart; sim device `1920x1080x32`; X window 1074×855 → 1970×1167; cockpit renders correctly |
| **CRASH-4** `800c010f` | AI ground-attack targeting read the feature table out of bounds. `FindSimGroundTarget` passes its loop counter over a target *group*'s components as a *feature* index; the accessors only guarded `f > 255`, which bounds nothing when the table is indexed at `FirstFeature + f` | From the PO's gdb backtrace (`GetFeatureID` ← `FindSimGroundTarget`); also fixed a `flightMember[4]` stack overflow whose only bound was a comment |
| **CRASH-5** `8edb3334` `95252f29` | Selecting A/G with bombs cast a bomb to `MissileClass`. `MaverickSetup` casts twice (`GetRMax`, `RunSeeker`) with only a `ShiAssert` between check and cast | **Reproduced locally**, and causality shown both ways: guard removed → segfault on first attempt, twice; guard in → 4 clean runs. Same shape swept from `smsdraw`, `advancedhts` ×2, `harmpod` |
| **VOICE-1** `8ed6e9b3` | Radio voice database indexed at doubled stride: `FRAG_/EVAL_/COMM_FILE_INFO` are documented on-disk layouts written by 32-bit Windows but declared their offset `long` — 16/16/18 bytes here vs 8/8/14. `maxfrags` also came out halved | 4 voicefilter assertions per campaign flight → **0**. `static_assert`s now pin all three sizes |
| **VOICE-2** `38e71b51` | `.tlk` speech file, same defect three ways: index stride `sizeof(long)`, an 8-byte read of a 4-byte field, `TlkBlock.data` at offset 16 instead of 8 | 12 assertions that only became reachable after VOICE-1 → **0** |
| **VOICE-3** `c45d15de` | **The corruptor.** `LHSP::LHSP()` is empty and the only assignments to `PMSIZE`/`CODESIZE` are commented out (ST80 codec stubbed), so garbage was accumulated into the returned decode length and `AddNoise` **wrote** across an 80 960-byte buffer. Also a latent hang: garbage `CODESIZE ≤ 0` subtracts 0 from the loop counter forever | ASAN same command: heap-buffer-overflow/`rc=139` → 0 errors/`rc=124`. Release: ~1 in 3 → **0 in 8** |
| **SAVE-2** `fcb3aff6` | `SaveSize()` over-counted every campaign event by 12 bytes — my own loose end from AUTOSAVE-1, which changed `Encode` to the 20-byte Windows node but left the size counting `sizeof(uieventnode)` = 32. Saves stored 120 bytes of uninitialised tail as content | Encoder now reports 0 mismatches (was `diff=-120`, twice per session); `cmpclass.cpp:2003` assertion gone |
| **WARN-1** `242c20b9` `c70bb57e` `69151cf5` `575f8cbd` | Diagnostics backlog worked by category: a real radio-subtitle leak (`delete` on a `void*` skipped `~SubTitleNode`, which frees the line); three ill-formed `delete[] <void*>`; implicit declarations **12 → 0** (the `_getdcwd` pointer-truncation class); a 16-byte stack overread writing garbage weapon IDs on the *current* save format; an 8-bytes-from-a-4-byte-int overread in the squadron-stores message; a laser pod loaded from uninitialised indices | Warnings cleared in the `FF_WARN` build; campaign flight to 3D clean after each |

---

## Open

**PIT-1 — 3-view tarmac extent. Unmeasured.** The blocking question is unchanged:
where does the runway end, in *ground distance*, in each view. I built
`FF_PROBE_DEPTH` for it and **it does not work** — every sample reads
`depth=0.999999` and the recovered eye is `(-1.0, -5.3, -0.9)`, i.e. the captured
matrices are a cockpit-local pass, not the world camera. `SaveGLFramebufferAsBMP`
runs at end of frame, after the cockpit pass; depth and matrices have to be
grabbed *during* the terrain draw. Left in, env-gated and off by default, with
the failure mode written up so the next attempt starts from it (`36e06c89`).

**TEX-1 — the PO's text-as-blocks / white-square screenshots. Unreproduced.**
Not seen locally in a 520 s flight at 4× compression. Suspected to be the same
VOICE-3 heap corruption, since corrupted font metrics would produce exactly that
symptom — but that is a hypothesis. If it recurs on this build the hypothesis is
wrong and that is worth knowing.

**CRASH-4 unconfirmed against the PO's repro.** The fix follows directly from the
backtrace, but I could not steer the automation into an OCA strike with the AI
actually selecting ground targets, so the guard never fired in my runs.

---

## Things I got wrong

Recorded because the mis-diagnoses cost more than the fixes.

1. **"The campaign UI holds dangling pointers."** Six symbolised crash sites,
   three of them under one callback, all consistent with a lifetime bug in
   `UI_Refresher`. It was wrong — the pointers were fine, their memory was being
   overwritten by the voice thread. A cluster of crash sites in structurally
   unrelated code is evidence of a *corruptor*, not of a bug at those sites.
   Reach for the sanitiser before building a theory about any one of them.

2. **I introduced a regression and it reproduced the bug it was meant to fix.**
   Trying to fix an invalid free in `O_Output`, I added ownership tracking and
   freed the old buffer when a borrow replaced it — producing `free(): invalid
   pointer` *inside* `SetText`. `Label_` is `protected` and assigned from several
   places, so a flag recorded at one site is not authoritative. The rule that
   survived is one-sided: `Cleanup` frees only on `ffOwnsLabel_` **and**
   `C_BIT_FIXEDSIZE`, which is narrower than the original test and so can only
   ever skip a bad free, never add one.

3. **A guard that was worse than the bug.** My first font-metrics fix rejected
   any line without 7 fields — but the shipped metrics genuinely contain
   `"3 43 234 10 18 1"`, whose missing value is `trail`, which nothing uses. That
   guard would have dropped glyph 3 entirely. Corrected to require the six fields
   actually consumed, plus the `idx < 256` bound that was missing all along.

4. **A false positive I did not "fix".** `guidance.cpp`'s five
   "may be used uninitialized" warnings are unreachable — `guidencephase` only
   ever holds 0/1/2. Left flight-critical code alone.

5. **Two stale-resolution instruments** were silently lying: `FF_PROBE_PIXEL`
   flipped its y with a literal `768`, and sim captures were sized from the
   window's *creation* dimensions, so every one saved the bottom-left 1024×768
   corner of a 1080p frame. That is what produced my earlier, wrong
   "SIM SURFACE: 1024x768" reading.

---

## Recurring shape worth expecting

**Fixing a data-format bug exposes code that has never run.** It happened three
times this session — VOICE-1 revealed VOICE-2, which revealed VOICE-3; and the
voice fixes exposed an unguarded `Stream->DSoundBuffer` in the sound streamer.
Expect the next layer down to be untested, not merely unfixed, and budget a
verification pass for it.

**Recurring per-session assertions are unfixed bugs, not noise.** Treating the
assertion inventory as a work queue is what found VOICE-1/2 and SAVE-2. The
inventory for an identical campaign flight went:

```
before   voicefilter ×4, voicemanager ×12, cmpclass ×2, texbank ×2,
         mission ×2, atm ×2, render2d ×2
after    mission ×2, atm ×2
```

The two that remain are genuine data conditions, both already guarded:
`atm.cpp:2134` is over-strict for legitimate data (`DataRate ≥ 3` yields 3), and
`mission.cpp:2040` is coincident waypoints.

---

## Coverage gaps found

* `run-asan-campaign-flight-soak.sh` never reaches the Maverick branch — at 500 s
  and 900 s the ground-attack weapon path records zero `gndattck` assertions,
  because the sanitiser build is slow enough that the mission never gets there.
* The campaign soaks go straight to 3D and never run
  `GlobalPositioningSystem::Update`, which is where the corruption was surfacing.
* The campaign click script does not reach a rendered cockpit until **~340 s**
  after sim entry. Earlier PIT-1 runs captured at 55–70 s, which is still the
  loading screen — worth knowing before trusting any capture from it.

---

## Build state

Release and ASAN builds green. `FF_WARN` build green. New diagnostics this
session: `FF_DEBUG_DISPCFG`, `FF_PROBE_DEPTH` (non-functional, documented),
plus `[FARTEX]`, `[TEXBANK]`, `[FONT]`, `[VOICE]` and `[CAMPUI-1]` counters left
in place so a recurrence produces evidence instead of another guess.

The PO's display config is currently set to **1920×1080** as requested. Windowed
at the desktop's native size the frame is slightly larger than the screen;
fullscreen would be exact.

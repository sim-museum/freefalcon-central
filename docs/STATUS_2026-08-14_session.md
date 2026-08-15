# FreeFalcon Linux port — session status, 2026-08-14

PO test-drive session, branch `develop`. **Diagnosis only — no code changed.**
Three defects characterised, one of them a newly root-caused permanent deadlock.

Binary under test: `build/src/ffviper/FFViper`, built 2026-08-09 18:33 from the
sources at `0e6b6ee9` (Sprint 22). Verified current: no `.cpp`/`.h` under `src/`
is newer than the binary.

**Capture state** (all runs): Korea theater, 1024×768 windowed (`-w`), texture
mode DDS (`m_texMode = TEX_MODE_DDS`; every UI path that could change it is
commented out), GNOME session unlocked, `gl-lock` held, no other sim running.

---

## Headline

**The "white screen on entering 3D" is two unrelated bugs, and only one of them
is a hang.**

| path | behaviour | verdict |
|---|---|---|
| Tactical Engagement | white ~31 s, **full cockpit by 50 s** | slow load + LOAD-1, recovers |
| Campaign | white **indefinitely** (PO: 5+ min) | **deadlock — CAMP-1, new** |

The PO originally reported both as "hangs at a white screen". Separating them
was the whole value of the session: TE is cosmetic-plus-slow, campaign is a hard
deadlock with a one-line cause.

---

## CAMP-1 (NEW) — campaign 3D entry deadlocks on the failed-launch bail path

**Status: CONFIRMED.** Root cause identified; fix not yet written.

Both flights of one live session, same process, in log order:

```
738:  FM_START_TACTICAL                                     <- TE 9
2552: Starting deagg wait: IsAggregate=0                    <- deaggregated fine
2859: Setting currentMode = StartRunningGraphics            <- 3D ran, PO flew it

3526: FM_START_CAMPAIGN                                     <- campaign
5491: Starting deagg wait: IsAggregate=128
5492: Deagg wait done:     IsAggregate=128 delayCounter=120  <- exited instantly
      F4SoundFXEnd... / AnnounceExit... / ClearCameras...    <- frozen here
```

No `StartRunningGraphics` after 3526. Every thread 0 % CPU in
`futex_do_wait`/`hrtimer_nanosleep`; the log stopped growing. **Deadlock, not a
spin, not a slow load.**

### Mechanism

`delayCounter` is untouched at 120, so the deagg wait ran **zero** iterations.
With `flight` non-NULL and `IsAggregate()` = 128, the only remaining condition in

```c
while (flight and flight->IsAggregate() and not flight->IsDead() and delayCounter)
```

is `flight->IsDead()` — the guard added in `ddd20274` precisely so a dead flight
could not spin the wait forever. It breaks out as designed, sets `player = NULL`,
and takes the "failed to fly" bail path, which runs into:

```c
// simloop.cpp:1385-1386
SimDriver.NotifyExit();
WaitForSingleObject(wait_for_sim_cleanup, 0xFFFFFFFF);   // INFINITE
```

`wait_for_sim_cleanup` is set in **exactly one place** — `simdrive.cpp:857`,
inside `SimulationDriver::Cycle()`'s `doExit` branch. On this path the Loop
thread never reached `RunningGraphics`, so it never calls `Cycle()`, so `doExit`
is never observed and the event is never set. StartLoop blocks forever; the sim
thread still owns the GL context, so the last presented frame — white — stays on
screen indefinitely.

**Third instance of the "signal-less INFINITE wait" class** already recorded in
CLAUDE.md (issues #6 and #13, both fixed by bounding the wait and signalling from
the other side).

### Proposed fix

Bound the wait at `simloop.cpp:1386` and fall through to the existing recovery
(post `FM_START_UI`), the same shape as the `stop_campaign_thread` fix — the
compat `WaitForSingleObject` already supports real timed waits via
`pthread_timedjoin_np`. Turns a permanent hang into a graceful bounce back to the
campaign map.

### Still open — why is the flight dead?

`flight->IsDead()` is **deduced from the loop condition, not directly observed**
(no trace prints it). It is consistent with the immediately preceding log:

```
[Deaggregate] Before IsFlight check, IsFlight=1
[SimCampMsg] Deaggregate returned, IsAggregate=128
```

`UnitClass::Deaggregate` early-returns on a dead flight, which is exactly why
`IsAggregate` comes back unchanged. Two candidates: the documented live-ATO
problem (the flight list reorders as the clock runs, so the committed row can age
out or be killed), or a genuine campaign defect. Note the campaign attempt came
**after a completed TE flight in the same process**, so second-launch state is
also in play.

**Cheapest next experiment:** does campaign 3D entry hang on a *fresh* launch,
before flying anything else? If it only hangs after a prior flight, that narrows
it sharply. A trace of `IsDead()`/`IsAggregate` at the top of the deagg wait would
settle the trigger outright.

---

## SETUP-1 (NEW) — Setup screen crashes the process

**Status: CONFIRMED.** SIGSEGV on clicking Setup (main menu, id 70003):

```
OpenSetupCB -> LoadSetupWindows() -> CreateKeyMapList()
  -> UpdateKeyMap() -> UpdateKeyMapButton() -> __strcat_chk -> SIGSEGV
```

`controltab.cpp:2887`:

```c
DoShiftStates(totalDescrip, Map.mod2);
strcat(totalDescrip, KeyDescrips[Map.key2]);
```

`KeyDescrips` is `new char*[256]` **memset to 0** (`keydescrips.cpp:144-146`) and
populated only for scancodes that have names — it is deliberately sparse. The
guard above only rejects `Map.key2 == -1`, so any binding whose scancode has no
description hands `strcat` a NULL source. The `Map.key1 > 0` branch at line 2882
has the same exposure via `_stprintf("%s")`, and neither path bounds-checks
`key2 < 256`.

Keymaps come from `keystrokes.key`, so which binding trips it is data-dependent —
that is why Setup has opened fine in earlier sessions. Same "unbounded
file-supplied index" class as the `tac_class.cpp` and ACMI-2 fixes.

**Fix:** NULL/range guard on both branches, plus a decision on what to display for
an unnamed scancode.

---

## LOAD-1 — quantified, still open

Sim-thread captures (`FF_SIM_SCREENSHOT`), TE "09 Landing Final Approach":

| t (sim mode) | frame |
|---|---|
| ~31 s | **pure white** — 2 distinct colours, avgRGB 254,254,254 |
| 50 s | full cockpit, HUD, terrain, sea; 2347 distinct colours |
| 80 s | still rendering correctly |

So the load screen presents white for the whole setup phase and the mission then
comes up normally. `FF_LoadingClear` (black) and the `SplashScreenUpdate` calls in
`OTWDriver::Enter` are not reaching the screen across the long stretch between
`SetupSplashScreen` (`otwdrive.cpp:2170`) and the first post-setup present.

Note the deagg-wait splash animation (`simloop.cpp:1040`) cannot help here: on a
healthy launch the flight is already deaggregated, so that loop runs **zero**
iterations. Whatever presents during setup has to come from `OTWDriver::Enter`
itself.

Frames: `sim08/sim25/sim50/sim80.bmp`.

---

## TERRAIN-1 — probable root cause found (data, not code)

**Status: strong, not visually confirmed against the grey-terrain report.**

Every load throws ~430 terrain-texture failures over **416 unique files — 300
`L*` and 131 `M*`**:

```
Failed to open .../terrdata/korea/texture/LBASE336.pcx   (terrtex.cpp:918)
Failed to read terrain texture. CD Error?                (terrtex.cpp:926)
```

`TextureDB::Load` / `ReadImageDDS` overwrite the filename's first character with
`M` or `L` for the medium/low LOD levels. In this install:

- `terrdata/korea/texture/texture/` holds 2174 DDS files, **all `H*`** — no `M*`
  or `L*` at all;
- the PCX set that would supply them is still inside an **unextracted 178 MB
  `texture.zip`**.

So every medium/low LOD tile misses DDS, falls into the port's Linux-only
DDS→PCX fallback (`terrtex.cpp:864-890`), misses again, and lands on `ShiError` —
which on Linux logs and continues, leaving `bits[res] = NULL`. Distant/low-LOD
terrain therefore has no texture, which is what TERRAIN-1 describes.

**Not a regression and not the load-time sink:** `terrtex.cpp` is unchanged since
2026-03-22, the game data is dated 2010-2012, and the failed lookups are cheap
(the miss directory has 5 entries; the DDS miss scans 2174 names). Fixing it means
extracting `texture.zip` into the game data — **a 178 MB change to the PO's
install, deliberately left as a PO call.**

---

## Method / artifacts

Repro recipe, TE (from Sprint 8, still valid):

```
FF_UI_CLICK="624,745@8;210,247@14;825,750@18;976,750@30"
```

All four clicks were verified to land on real controls and fire `Process()`
before any conclusion was drawn from the run.

Diagnostics used: `FF_DEBUG_DEAG=1`, `FF_SIM_SCREENSHOT`, `gl-lock`, thread-state
inspection via `ps -L`/`top -H`.

**gdb could not attach**: `/proc/sys/kernel/yama/ptrace_scope = 1` restricts
tracing to descendants, and the game was launched from a different shell. The
deadlock was characterised from the log, the code, and thread wchan states
instead — sufficient here, but for future live-hang work launch the game *under*
gdb, or the backtrace is unavailable.

### Trap re-encountered

`[TERRAIN_DIAG]` is a one-shot capped at 3 (`otw.cpp:1262`), and "only 3
TERRAIN_DIAG blocks after entering 3D" was briefly misread as "only 3 frames
rendered". It is the **filter-don't-cap** trap already booked twice in CLAUDE.md.
The frame captures, not the trace, settled whether rendering was happening.

---

## Not done

- No code changed, nothing verified by rebuild.
- CAMP-1 fix (bounded wait) — designed, not written.
- SETUP-1 fix — designed, not written.
- CAMP-1's trigger (why the flight is dead) — unresolved.
- `texture.zip` extraction — PO call.

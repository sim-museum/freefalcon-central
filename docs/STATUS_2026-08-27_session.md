# FreeFalcon Linux port — session status, 2026-08-27

Autonomous scrum on `develop`. **~60 commits**, `762a98c4` … `e386614b`, all pushed.
Working tree clean. Every claim below is measured or explicitly marked unverified.

---

## Headline

**The intermittent use-after-free (UAF-1) is solved**, and one defect *shape* — a
guard sequenced after the access it protects — accounted for most of the session's
other real bugs. **12 defects fixed**, verified at **34/34 missions, 0 crashes**.

---

## Fixed and verified

| item | defect |
|---|---|
| **UAF-1** | `SetNewTheater` frees and reallocates `Falcon4ClassTable`; `VuxType()` hands out interior pointers, so entities created before the reload keep dangling ones. Entity type 1144 is such an entity and participates in grid moves. **Fix re-points every live entity's `entityTypePtr_` after the reload.** Deterministic: stale pointer in **6/6** runs without it, **0/15** with it. |
| **ORDER-1** | Nine defects where a guard ran *after* the access it protects — incl. `FarTexDB::Deactivate` asserting `texArray[offset]` above its own NULL check (the exact shape of the `AttachChild` heap overflow), `damage.cpp` (nine derefs before its guard), `HitsOnTrack`, `NewUnit`, `SetText`, and four UI sites calling methods on unchecked `FindControl`/`GetParent` results. |
| **ORDER-3** | `division.cpp` asserted `divels[t][d]` **above** a bounds check tagged `// JB 010223 CTD` — the check was added *because it crashed*. |
| **VU teardown** | `~VuGridTree` freed `table_` and `filter_` before `GridDeRegister`, outside `gridsMutex_`. Real lock-scope defect; **not** the UAF-1 fix. |
| **TEAMROE-1** | False-alarm assertion removed **on evidence** (caller traced to legitimate TE data). |

**TESWEEP-5** (post-UAF-1-fix): 34/34 reach sim, 0 crashes, and the assertion count
*fell* 62 → 58 — a negative object id and a NaN distance stopped occurring, both
downstream of the stale type pointer. The fix removed bad states, not just a crash.

---

## Open

| item | state |
|---|---|
| **NVG-5** | Still unsolved. Narrowed this session: all four 3D COLOROPs **are** implemented (eliminates a family of theories), and the block sets **no `ALPHAOP` at all** while setting `m_AlphaTextureStage = 3` — so stage 3's alpha op is *inherited*. New hypothesis is state-inheritance, not a missing feature. `FF_DEBUG_NVGOPS` now reports unimplemented ops too. |
| **`GetRoE` fallback** | **Decision for you.** With `TeamInfo[a]` NULL it returns `ROE_ALLOWED`; `ROE_NOT_ALLOWED` is arguably more correct and would flip a front-line "isolated" result. **~120 call sites** read it across ground/air/naval AI, base usability, RWR and mission evaluation. No evidence of intent in the code — I did not change it. |
| **BSPSLOT-1** | Closed as **game data**: vehicle types 3337 and 711 mark hardpoint 2 visible while LOD 233 defines 2 slots. Cosmetic (weapon not drawn); no leak. Fixing means changing model or `VisibleFlags`. |
| **UCNULL-1** | Closed by measurement: `GetUnitClassData()` returned NULL **0 times** across 6 missions. 25 unguarded sites left alone with evidence. Bounds TE paths only. |
| **Verification sweep** | Interrupted at 11/34 when you needed the machine — all clean. **Re-run `scripts/qa/te-sweep.sh 1 34` when convenient; expect ~38 assertion lines, not 62** (the removed assertion accounts for −20). |

---

## Corrections to the record — six stale claims retired

CLAUDE.md loads every session, so a wrong claim there misdirects work.

1. **TE-09/10 "still won't LOAD"** — false; both fly. TE-09 is our landing test case.
2. **msgsrc allocator family "latent"** — already fixed, and the advice was unactionable (multiplayer paths, single-player soaks).
3. **"runways invisible / collision flat z=0"** — both halves false, symptoms fixed.
4. **"terrain through MFD screens"** — fixed, and its diagnosis was refuted 200 lines later in the same file.
5. **"Texture assertions UNRESOLVED"** — resolved by the pointer-truncation work.
6. **My own docs** — the UAF-1 fix was documented as *ineffective* in two places; corrected once measurement proved otherwise.

---

## Method notes that earned their place

* **For an intermittent defect, measure the invariant it violates — never the crash.**
  UAF-1 cost seven refuted theories and ~40 crash-counting runs; a probe checking one
  invariant settled it in six. **The defect was present 100% of the time — only the
  symptom was intermittent.**
* **Presence is not timing.** Six of seven refuted theories described a mechanism
  genuinely in the code but not active at the crash. Source proves a hazard exists;
  only timing shows whether it could fire.
* **Every probe needs a control proving it executed.** Five control failures this
  session, two of which I created *while building controls* — an `atexit` reporter
  (`main()` calls `_exit(0)`) and a threshold control that fails silently below its
  threshold.
* **Never delete an assertion that fires.** Proven twice, once against my own dropped
  `AttachChild` assert — restoring it is what made BSPSLOT-1 traceable.
* **"Few" is not "none."** I refuted the *correct* UAF-1 theory by reasoning that
  because only 2 entities predate the reload, none could matter. One was the culprit.

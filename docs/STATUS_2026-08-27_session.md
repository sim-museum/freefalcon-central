# FreeFalcon Linux port — session status, 2026-08-27

Autonomous scrum on `develop`. Thirty-two commits, `762a98c4` … `b213ec5a`, all
pushed. Every claim below is measured or explicitly marked unverified.

---

## Headline

**One defect *shape* accounted for most of this session's real bugs: a guard
sequenced after the access it protects.** Ten fixed instances across graphics,
campaign UI, texture management and the VU layer. It is now recorded as a memory
(`ff-guard-after-access`) with a validated scanner (`scripts/qa/order-audit.py`).

**I wrote one of these myself**, inside the fix for a bug of that exact family, and
it would have crashed the game on every launch. Knowing the pattern gave no
protection.

---

## Fixed and verified

**ORDER-1 — nine defects, verified 34/34.** Guards sequenced after their access.
Full TE sweep: **34/34 missions reach sim, 0 crashes**, 62 assertion lines against a
62/64 baseline. Notable: `FarTexDB::Deactivate` asserted `texArray[offset]` above its
own NULL check — the exact shape of the `AttachChild` heap-buffer-overflow.

**ORDER-3 — one more, in `division.cpp`.** `ShiAssert(divels[t][d])` sat above a
bounds check tagged `// JB 010223 CTD` — *crash to desktop*. The check was added
because it crashed; the assert above it performed the very access it prevents.

**VU grid teardown.** `~VuGridTree` freed `table_` and `filter_` **before**
`GridDeRegister`, and outside `gridsMutex_` — the lock `HandleMove` holds while
calling `Move()`. Leading candidate fix for UAF-1 (see below).

---

## Open, with evidence — nothing here is guesswork

| item | state |
|---|---|
| **UAF-1** | Five theories refuted, four of them mine. Post-fix **6/6 clean vs a 2/5 baseline** — `p ≈ 0.047`, suggestive only. Discriminating experiment running. |
| **BSPSLOT-1** | Root-caused to **game data**: vehicle types 3337 and 711 mark hardpoint 2 visible while their model (LOD 233) has 2 slots. Cosmetic today; was a heap overflow before ORDER-1. |
| **TEAMROE-1** | Caller identified: TE missions inherit objectives owned by teams they never instantiate. The NULL fallback **plausibly inverts** a front-line isolation result — not merely noise. Not changed: the fallback is shared with campaign AI. |
| **UCNULL-1** | `GetUnitClassData()` is nullable (proven three ways); **25 of 47 call sites dereference it unguarded**. Deliberately not mass-edited. |
| **NVG-5** | Still parked; resisted several sprints. |

---

## Corrections to the record — five stale claims retired

CLAUDE.md is loaded every session, so a wrong claim there actively misdirects.

1. **TE-09/10 "still won't LOAD"** — false; both fly. TE-09 is our primary landing test.
2. **msgsrc allocator family "latent, fix as ASAN surfaces them"** — already fixed, and the advice was unactionable: those paths are multiplayer, every ASAN pass is single-player.
3. **"OPEN: runways invisible / collision data flat z=0"** — both halves false, symptoms fixed.
4. **"OPEN: terrain through MFD screens"** — fixed as MFD-THRU-1, and its diagnosis was refuted 200 lines later in the same file.
5. **"Texture assertions UNRESOLVED"** — resolved by the pointer-truncation work.

---

## Method notes that earned their place

* **Validate a scanner against known positives before trusting one finding.** Mine
  missed `pilot.cpp` three times; that failure is what forced its second pass into
  existence.
* **"No findings" and "the tool never ran" are indistinguishable.** Hit three times
  today — a `nohup` exit code read as completion, a probe on a path the run never
  reached, and a killed job that wrote nothing. Every probe now ships with a control
  counter.
* **Presence is not timing.** Four UAF-1 theories described mechanisms genuinely
  present in the code but not active at the crash. Only log line numbers settled it.
* **Assertion counts are coverage, not frequency.** One TE mission reported
  `asserts=2` while the underlying condition fired **2849** times.
* **Never delete an assertion that fires.** Deleting one converts an observable
  defect into a silent one — proven twice today, once against my own change.

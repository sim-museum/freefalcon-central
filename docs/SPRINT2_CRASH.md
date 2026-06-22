# Sprint 2 — Crash elimination

Goal: eliminate the heap corruption behind the long-reported "intermittent" crashes.
Method: an **ASAN soak** of the Instant Action flow (`run-asan-soak.sh`) — ASAN is
the only ground truth for these (zero false positives), and each soak peels back the
next layer of mismatches; rebuild `build-asan` + re-soak to confirm a batch cleared.
`rc=124` = clean timeout (no crash through the run).

## Root cause: systemic `new T[]` freed with scalar `delete`

Widespread across the codebase. Each corrupts the heap on cleanup; glibc tolerates it
until it doesn't — exactly the profile of an "intermittent" crash. Fixed (all to
`delete[]`, only the non-`USE_SH_POOLS` branch, Windows-neutral):

- **Network messages** (Sprint 1, 8): falconflightplanmsg, sendimage, sendevalmsg,
  simdirtydatamsg, sendvcmsg, requestcampaigndata, sendcampaignmsg, sendpersistantlist.
- **Sound** (ASAN-found): psound `srcbuffer`, csoundrc `filename`.
- **ui95 / graphics / cockpit** (ASAN-found): chash (see below), tblock `posts`,
  cfontres `fontTable_`/`kernList_`/`fontData_`, cphsi `mpCompassCircle`,
  cstringrc `IDTable_`, ui_main `gScreenShotBuffer`.
- **26 class-owned member buffers** from a comment-aware static scan (each var is
  `new T[...]` and never scalar-new in its file): sijoy axis/dir/force/condition
  arrays, cresmgr/tacref `Data_`, ooutput `Label_`, csclbmp `Overlay_`, ccustom
  `Items_`/`ItemValues_`, cfill `DitherPattern_`, cfonts `Widths_`, uihash `Table_`,
  cptext `mpString`, name `NameIndex`, division `element`, cmusic `ImaInfo->src`,
  sendobjdata/sendunitdata receive buffers. **ASAN-validated: zero reverse-mismatches.**

### The `C_Hash` trap (caught a self-inflicted regression)
`C_Hash::Record` is a heterogeneous `void*`: `C_Resmgr` stores scalar `new FLAT_RSC`
records; `AddText`/`AddTextID` (used by `gStringMgr` UI labels and parser `TokenOrder_`)
store `new _TCHAR[]` strings. **No single delete form is correct.** A blind `delete[]`
flip fixed the string path but introduced 17,753 reverse-mismatches in resmgr (ASAN
caught it; reverted). **Lesson: never flip a generic `void*` container's free based on
a static scan.** Correct fix shipped: a per-instance `ownsStrings_` flag (set by
`AddText`/`AddTextID`) so each hash frees with the matching form — scalar-record hashes
keep scalar `delete`, string hashes use `delete[]`. No caller changes.

## Bounds fix
`ObjectiveClass::SetFeatureStatus` wrote `fstatus[f/4]` and indexed
`FeatureEntryDataTable` with an unbounded `f` (critical-link recursion sets `f±1` past
the ends → OOB heap write + negative-shift UB); `GetFeatureStatus`'s guard ran after
the `f%4` modulo. Bounded both against `((Features*2)+7)/8`.

## Deferred / open (tracked)
- **sendchatmessage.cpp:29** — a 2-hit reverse-direction mismatch (a scalar-`new`
  assignment to `dataBlock.message` somewhere vs the `delete[]` dtor). Low frequency.
- **`_mm_loadu_ps` stack-buffer-overflow** (4 hits) — an SSE 16-byte load reading past
  a smaller float array; needs the call-site backtrace.
- **`AS_DataClass::ASSearch` SIGSEGV** (campaign-thread, intermittent) — analyzed: the
  per-instance node pool + critical section rule out the simple races, and NULL/OOB
  node handling is correct. Hypothesis: use-after-free of a campaign object
  (`neighbors[n].where`) in the `extend` callback during concurrent campaign updates.
  Needs a **campaign-mode** ASAN repro (the IA soak doesn't exercise heavy ground-unit
  A*); did not reproduce in any IA soak this sprint.

## Status
Two full ASAN-validated batches cleared; IA soaks run crash-free (rc=124). The
dominant corruption (`chash`, 10,519/soak) fixed via `ownsStrings_`. Remaining items
above are low-frequency or need a campaign-mode repro.
</content>

# Sprint 1 — Correctness Hardening Sweep: audit findings & triage

Method: 7 parallel read-only audit agents, one per bug class, across ~3,500 files.
Each finding was re-verified by hand before any edit. Below: what was fixed, and
what was triaged as *not a bug* or *deliberately deferred* (with the reason).

## FIXED (committed)

### Bug class 2 — `new[]`/`delete` heap corruption (8 sites)
`falclib/msgsrc/{falconflightplanmsg,sendimage,sendevalmsg,simdirtydatamsg,
sendvcmsg,requestcampaigndata,sendcampaignmsg,sendpersistantlist}.cpp` —
array buffers (`new uchar[]`/`new VU_BYTE[]`) freed with scalar `delete` → heap
corruption. Changed to `delete[]`. (4 sibling files were already fixed earlier.)

### Bug class 4 — MSVC RAND_MAX (2 active sites)
`sim/guns/shells.cpp:256,281` — flak `rand()/32767.0f`. glibc `rand()` is 0..2³¹,
so air-burst hit probability was ~65000× too low (enemy AAA ineffective). Now
`(float)RAND_MAX` (==32767 on MSVC, so Windows unchanged).

### Bug class 2 — pointer truncation (3 sites)
`graphics/renderer/gmcomposit.cpp:416,482,851` — `SelectTexture1((UInt)texHandle)`
truncated a `TextureHandle*` to 32 bits. Now `(intptr_t)`, matching the already-
correct line 699.

### Bug class 5 — CRLF after fgets (3 sites)
`falclib/entity.cpp` rack parser (`strtok(0,"\n")` kept trailing `\r`) and the two
`graphics/weather/realweather.cpp` METAR readers (per-field `strcpy` captured the
line terminator). Strip trailing `\r`/`\n` after fgets.

### Bug class 1 — `sizeof(long)` write paths whose reads are already 4-byte (4 sites)
Confirmed Save↔Decode mismatches (Decode already reads 4 bytes under `#ifdef
FF_LINUX`, Save still wrote 8 → Linux save/network round-trip desync):
`camptask/flight.cpp:539` (fuel_burnt), `camptask/package.cpp:628`
(package_flags), `camplib/campwp.cpp:257` (Flags), `camplib/objectiv.cpp:478`
(obj_flags). Now emit a fixed-width temp (byte-identical to Windows where long==4).

## TRIAGED — NOT a bug (no change)

- **Weapon-table index diffs** `((int)dataPtr - (int)WeaponDataTable)/sizeof(...)`
  in `lau.cpp`, `simweapn.cpp`, `wpnstatn.cpp`. `(int)a-(int)b` keeps the low 32
  bits of each; since both pointers are in the *same array* the true difference is
  small, so the result is correct mod 2³². Left as-is.
- **voicemapper.cpp:113** — `sscanf(buf,"%d %99s %99s")`. `%s` stops at any
  whitespace and `\r` *is* whitespace, so no `\r` is captured. Safe.
- **fsound.cpp:1022** — the only string field (filename) is the *first* token (no
  trailing `\r`); subsequent fields are numeric (`atoi` ignores `\r`). Safe.
- **DDS header reads** — all four loaders (texbank, fartex, image, terrtex) already
  use the 124-byte `DDS_FILE_HEADER` on Linux. Zero remaining sites. Class closed.

## DEFERRED (tracked, lower risk/priority)

- **Compat stubs returning defaults** (`GetSystemMetrics`, `GetKeyState`,
  `GetAsyncKeyState`, `GetClientRect/WindowRect`, `GetDlgItemInt`, `SetTimer`).
  These are real gaps but the main flows work without them (input is via SDL→IO,
  not GetKeyState). Impact is limited to UI niceties (shift/ctrl multi-select,
  window-centering). Wiring them to SDL is an *enhancement*, scheduled for polish
  (Sprint 7), not a defect blocking play. NOT dangerous as claimed.
- **`objectiv.cpp` `SaveBaseObjectives`/`SaveObjectiveDeltas` chunk-size `fwrite`**
  (3080/3104/3122) and **`cmpclass.cpp` Encode** (1974) — `sizeof(long)` chunk
  sizes. Need the matching read side confirmed before flipping to int32; the read
  side currently also uses native sizes in the same file, so they are internally
  consistent on Linux (8/8). Only matters for cross-OS file exchange. Deferred.
- **`falcgame.cpp`** domainMask_ (103/166) and **`requestcampaigndata.cpp`**
  dataNeeded (74/99) `sizeof(ulong)` — read+write are *both* unguarded, so they're
  internally consistent on Linux (8/8); only a cross-OS / mixed-binary concern.
  Deferred with the other cross-OS items.
- **`bspnodes.cpp` RestorePointers `int` offset** (11 sites) — model node pointer
  relocation. 3D models currently load and render correctly, so these offsets fit
  in 32 bits in practice. Changing the signature is a larger, riskier edit with no
  observed failure; deferred unless a model-load bug surfaces.
- **camptool (mission editor)** `sizeof(long)` sites — the editor reads/writes its
  own files with native `long` on both sides (self-consistent on Linux). Not on
  the gameplay path. Deferred.
</content>

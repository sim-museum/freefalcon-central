# Tacview — install, and getting a FreeFalcon flight into it

Tacview is the project's quantitative instrument: it is how a Linux flight and a
Wine gold-standard flight get compared as *data* rather than as two people's
impressions of two videos.

**Tacview is third-party proprietary freeware and is not in this repo.** Nothing
here vendors it or its installer. This file is instructions only.

Everything below was verified against the tree and the machine on 2026-08-30
rather than copied from memory — several path notes elsewhere in this repo have
gone stale, so re-check before trusting anything here that looks surprising.

---

## 1. Install

There is already a working installer script at `~/sgl/SAT/tacview/tacview.sh`.
Use it rather than installing by hand: it creates Tacview its **own win64 Wine
prefix**, separate from the game's, and installs Wine Mono, which Tacview's addons
need and which is easy to miss.

```
~/sgl/SAT/tacview/
    INSTALL/Tacview*Setup*.exe     <- put the installer here
    tacview.sh                     <- run this
    WP/                            <- prefix it creates
```

```sh
~/sgl/SAT/tacview/tacview.sh
```

First run installs; later runs just launch, inside
`wine explorer /desktop=Tacview,1280x800` so it gets its own window rather than
fighting the desktop. `Tacview64.exe` ends up at
`WP/drive_c/Program Files (x86)/Tacview/Tacview64.exe`.

**Do not install Tacview into the game's Wine prefix**
(`~/sgl/SAT/freeFalcon/WP`). Separate prefixes mean a Tacview upgrade cannot
disturb the game install, and the game prefix is the gold-standard reference.

---

## 2. Record a flight

Enable ACMI recording in the game UI before flying.

The recorder writes **only** a raw flight file:

```
<gamedata>/acmibin/acmi0000.flt        acmirec.cpp:168
```

Tape size is **unlimited by default** (ACMI-1) — `ACMIFileSize = 0` means no
rotation, so a long flight is not silently truncated part-way.

---

## 3. Convert it to a tape — the step that is easy to miss

**A raw `.flt` is not a tape, and this build never converts one on its own.**

`ACMI_ImportFile()` is what turns `acmibin/acmi*.flt` into
`acmibin/TAPE%04d.vhs`, and in this build *nothing calls it*: both call sites in
`acmiui.cpp` are commented out and the only live caller is a multiplayer chat
command. So a recorded flight stays a raw `.flt` forever unless you ask for the
conversion:

```sh
FF_ACMI_IMPORT=1 ./FFViper -d "$GAMEDATA" -w
```

The conversion runs on return to the UI (`FM_START_UI`), so **fly, then exit to
the menu** — quitting straight from the sim skips it. On success the source
`.flt` is retired, so one flight yields exactly one tape.

```
[ACMI] FF_ACMI_IMPORT: converting acmibin/acmi*.flt -> TAPEnnnn.vhs
[ACMI] FF_ACMI_IMPORT: done
```

If you see no such line, the import did not run and there is no new tape.

---

## 4. Open it

Copy the tape somewhere convenient and open it with **File > Open** inside
Tacview. Tapes that have been verified to load:

```
~/Documents/Tacview/wine_ff_260829.vhs               Wine gold standard
~/Documents/Tacview/linux_ff_260829_PREFIX_lp64.vhs  Linux, post-ACMI-2
~/Documents/Tacview/linux_ff_260829_FIXED.flt        Linux
```

## The gotcha that cost real time

A **RAW** `.flt` — one that was never imported — makes Tacview report:

> The file format of `…_RAW.flt` is not supported

That is not a corrupt file and not a Tacview problem. It is section 3: the
recording never became a tape. Import it, or record again with
`FF_ACMI_IMPORT=1`.

---

## Why tapes used to be unreadable at all

Before **ACMI-2** (`770cfade`, verified `24c10dd5`), the on-disk ACMI records used
LP64 `long` — 8 bytes on 64-bit Linux — where the Win32 tape format specifies 4.
Every field after the first in each record was therefore misaligned, so the whole
tape was garbage to any reader expecting the real format. Eight fields in
`acmirec.h` are now `int32_t`.

Tapes written by the current build are Win32-layout and load in Tacview. Tapes
written by a build older than `770cfade` will not, and cannot be repaired — re-fly
the sortie.

---

## Comparing Linux against Wine

The point of all this. Fly the same TE in both builds, import both, and load them
together. Because both tapes are now in the same format, differences in the data —
positions, altitudes, weapon events — are differences in the *simulation*, which
is the comparison that a screenshot or a video cannot give you.

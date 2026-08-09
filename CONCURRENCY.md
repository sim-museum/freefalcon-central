# Parallel-session rules — free-falcon

Other Claude sessions may be working on the sibling projects at the same time. This box has
**one display and 4 cores**; see `~/CONCURRENCY.md` for the full picture.

## The rule that matters

Wrap anything that renders, opens a window, or captures a screenshot:

```bash
export PATH="$HOME/bin:$PATH"
gl-lock <your command>
gl-lock --status          # who has the display right now
```

`gl-lock` also refuses to start when the desktop is locked, which otherwise looks exactly like
the port hanging at the title screen.

### What the lock is actually for (corrected 2026-08-02)

An earlier version of this note claimed that two sims rendering at once corrupts screenshots.
**That was wrong.** Every capture path here — Julia's `JM_SHOTS` and the MA/BoB parity dumps —
uses `glReadPixels` against its OWN GL framebuffer (Julia's window is even created with
`GLFW.VISIBLE, false`). Two processes drawing at once each read their own buffer; neither can
see the other's pixels. Pixel content is safe.

The real reason to serialise is **contention for one GTX 1660 SUPER and 4 cores**. That
matters because results here can be frame-rate dependent — MiG Alley's stress gate scores a
run `HANG` when it misses its frame target, so a second sim hammering the GPU can manufacture
a failure that looks like a port defect. Julia captures also slow down under load.

So: still wrap GL runs in `gl-lock`, but if you see a contention alert, the question to ask is
"were any timing-sensitive results taken in that window?" — not "must I discard my captures?".


## This repo's slot

`free-falcon` runs concurrently with the other two flight sims — separate repos, no shared files.
The **Julia Racer** session works through its 5 tracks sequentially and takes the display in
~3-minute blocks; expect to wait occasionally at gate time.

Keep build and capture output in your own scratch directory. `~/gold standard/` is the
shared parity oracle and is read-only for port work.

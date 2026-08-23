# scripts

## record-display.sh

Records one monitor to `~/Videos` with `gpu-screen-recorder`. This is how the
gold-standard Wine comparison videos are captured — the frame at t=130 s of
`260822_wine_ff_TE_CCIP.mp4` is the reference for BOMB-1's missing explosion.

```sh
scripts/record-display.sh --list     # show connected monitors
scripts/record-display.sh            # record monitor 2 (the HDMI one)
scripts/record-display.sh DP-1       # or record by name
```

Ctrl+C stops and saves.

### Dependency

Needs `gpu-screen-recorder` on `PATH`. It is **not** vendored here — it is a
separate GPL-3.0 project by dec05eba, so it is downloaded on demand instead:

```sh
scripts/build-record-display.sh              # clone + build into extern/
scripts/build-record-display.sh --install    # ...and install system-wide (sudo)
```

The checkout lands in `extern/gpu-screen-recorder`, which is gitignored, so no
downloaded code ever enters this repository. Without `--install` the binary stays
at `extern/gpu-screen-recorder/build/gpu-screen-recorder`; put that directory on
`PATH` and `record-display.sh` will find it.

Pinned to upstream commit `9c2c0e1`, the revision this was verified against. Set
`GSR_REF=master` to track upstream instead — unpinned, so it may not match the
build the existing reference videos were made with.

Verified end to end: the script clones, builds, and `record-display.sh --list`
then works against the freshly built binary. Note that capture needs a
GPU-specific path at *run* time, so a successful build does not by itself
guarantee it can record on another machine.

## wine-capture.sh / wine-drive.sh

Launch and drive the Windows build under Wine for side-by-side comparison
(see WINE-1/WINE-2 in `docs/STATUS.md`). Use `~/sgl/SAT/freeFalcon/freeFalcon.sh`
for the supported launch path — these wrap capture and input on top of it.

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

## Driving the Wine gold standard (`wine-drive.sh`, `wine-capture.sh`)

The Windows build under Wine is the reference the Linux port is checked against.
Driving and capturing it on this machine needs three non-obvious workarounds, each
of which fails **silently** if you get it wrong:

| what | naive approach | why it fails | what works |
|---|---|---|---|
| launch | `wine` / `wine32` | cannot boot this 32-bit prefix at all | pinned `lutris-GE-Proton8-26-x86_64` |
| input | `xdotool click` (XTEST) | GNOME **Wayland** silently discards fake input | `xdotool click --window` (XSendEvent) |
| capture | `import`, `ffmpeg -f x11grab` | X11 grabs return an **all-black** image under Wayland | `gpu-screen-recorder` on a monitor, cropped |

Two further traps:

* The game window **spawns outside its own virtual desktop** (fixed `+3815,-62`),
  so the desktop looks empty while the game runs and plays audio. It is also
  **recreated when the sim loads**, so it must be repositioned again after entering
  3D — use a `p:` step in the timeline.
* gsr's *window* capture (`-w <id>`) is unsupported under XWayland; only monitor
  capture works. Default H.264 blurs HUD digits past legibility, hence
  `-q ultra -tune quality -cr full`.

Example — Korea TE-09, drop the gear, capture the orbit view:

```sh
scripts/wine-drive.sh "c:674,750@5;c:140,247@10;c:822,748@16;c:984,748@22;p:@75;k:g@80;k:0@86;s:gear@92"
```

Theater is **per build**: the Linux port reads `config/registry.ini`
(`curTheater=1,<hex>`), Wine reads its prefix `system.reg` (`"curTheater"="Korea"`).
Setting the file directly is far more reliable than clicking the THEATER UI.

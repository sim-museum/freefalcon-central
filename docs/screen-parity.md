# EPIC SP — Screen parity vs the PO gold standard

_Sprint 8, 2026-07-25. Method per the cross-port exchange: capture at the present
point on the GL-context-owning thread, `GL_PACK_ALIGNMENT=1`, objective band
statistics + side-by-side eyeballing of the saved frames. Method upgrades adopted
from julia-racer's E59 note (`docs/QA_METHOD_GOLD_PARITY_from-julia-racer.md`):
inventory the gold set as data first; classify deviations into
renderer-bug / authentic-asset / asset-gap / prior-decision before "fixing"._

Gold standard: 5 PO-supplied PNGs in `/run/media/admin/BEA6-BBCE/free falcon/`.
Native captures live in `/tmp/ffval/sp*.png` (regenerate with the recipes below).

**Inventory note for the PO:** shot 3 (`2026-06-05 20-37-29`) is titled
"Free Falcon 6 Linux Port" — it is a capture of the NATIVE build (June 5), not of
the Windows game under Wine. It still documents the intended look of the dogfight
setup screen, but it cannot serve as a Windows oracle. Shots 1/2/4/5 are titled
"FreeFalcon - Wine desktop" and are genuine Wine golds.

## Parity table

| # | Gold file (2026-…) | Screen / view | Native repro recipe | Verdict |
|---|---|---|---|---|
| 1 | 06-05 16-31-46 | UI main menu (cobra/blueprint splash, aircraft column right) | `tools/ff_validate.sh sp1-main -m ui -t 8 -r 16` | (pending) |
| 2 | 06-05 16-32-02 | Sim, 2D cockpit (view 1), Instant Action, banking over coastline, HUD + MFDs | `tools/ff_validate.sh sp2-iapit -m sim -t 30 -v 1` | (pending) |
| 3 | 06-05 20-37-29 | Dogfight setup screen (Furball options, 4 team tiles, roster pane, Korea map) — NATIVE capture, see note | `FF_UI_CLICK="870,745@8;201,134@14" FF_UI_SCREENSHOT=4` then keep last `/tmp/ff_ui.bmp` | (pending) |
| 4 | 06-06 07-34-04 | Sim, 2D cockpit (view 1), dogfight arena entry, level over ocean 10 000 ft | dogfight flow `FF_UI_CLICK="870,745@12;201,134@18;884,741@24;900,750@34"` + `-v 1` capture | (pending) |
| 5 | 06-09 15-32-20 | UI main menu (same screen as #1, later Wine build) | same as #1 | (pending) |

## Tolerance statement

UI screens: same layout, art, and control positions; palette within ordinary
gamma/driver variance (the Wine desktop adds a title bar and border the native
window does not have — crop before comparing). Sim screens: same cockpit
geometry, HUD symbology and terrain classes rendered; exact terrain texels and
weather state are not required to match (different mission instant).

## Deviations found

(to be filled per shot; each deviation gets fixed (SP.2) or PO-waived here)

# FreeFalcon multiplayer (Linux port)

**Status (2026-09-28):** two-PC discovery, join, dogfight team select and a shared 3-D flight with
weapons were confirmed by the PO on 2026-09-19. Two of the three defects that test found are fixed and
measured on one box with two instances (`scripts/qa/mp-dogfight.sh`); the third is the HUD FLIR (Shift+H).
A campaign joiner can now take off with the host on the map or in the same flight, and real gun hits
damage and kill the other player (host's trigger, joiner's jet: 5 hits, jet destroyed, death message
received by the host):

| item | symptom (PO, 09-19) | cause | fix | revert |
|---|---|---|---|---|
| **MP-DMG-1** | host's missile hits the joiner: explosion, no damage on the joiner, host sees a kill, debriefs say `miss` | the joiner's OWN jet carried `FEC_INVULNERABLE`: the host's `LockPlayer` set it on its copy and broadcast it; the joiner's `ReleasePlayer` cleared it but dirtied the flags *before* `ChangeOwner`, so the clear was dropped, and `SimDirtyData` then applied the host's stale flags to the joiner's local jet | owner re-publishes its flags after `ChangeOwner`; a local entity keeps its own invulnerability bit against remote dirty data | `FF_NO_MPINVULN_FIX=1` |
| **MP-CLOCK-1** | joiner's campaign clock frozen, different event text on each side | on Windows the sim loop ran behind the UI and its CLIENT branch followed the host's timing messages; the port idles that loop in the UI and advanced the clock with the single-player rule, where a client's ratio is always 0 | the UI tick runs the client branch (`FF_RemoteClientTimeStep`) | `FF_NO_MPCLOCK_FIX=1` |
| **MPHOST-SIM-1** | a campaign joiner never reached takeoff (one-PC runs) | the host, sitting in the UI, never deaggregated remote players' flights nor simulated the host-owned aircraft in their bubbles (same idled loop); then two HUD/altimeter buffer overflows on the joiner's first frames | while hosting online with a remote player loading/flying, the idle loop runs `RebuildBubble` + `SimDriver.Cycle`; buffers widened | `FF_NO_MPHOSTSIM=1` |
| **MP-NAN-1** | the joiner's jet had a garbage/NaN position after entering a campaign flight | (a) the joiner's first sim frame integrated over minutes (`SimLibMajorFrameTime` measured at 293 s: `lastRealTime` stale because the Loop idles through the load); (b) `GetGroundLevel` returned an uninitialised (and leaked) ground normal when no viewpoint answered, normalised into NaN | `lastRealTime` resynced at `StartRunningGraphics`, flying steps >10 s resynced; stack normals with the flat default | `FF_NO_FRAMESTEP_CLAMP=1` |
| **MP-GLTHREAD-1** | joiner SIGSEGV in libGL while entering a campaign flight | the joiner's network thread built the host's aircraft (`DeaggregateFromData` → `AircraftClass::Init` → `GetGroundLevel` → `InitViewpoint`) and created the terrain viewpoint (GL calls) the moment the sim thread had taken the GL context | only the thread that owns the context builds the viewpoint | `FF_NO_GLTHREAD_FIX=1` |
| HUDBOX-1 | joiner's HUD on a dark box | reproduced exactly, and it is not a defect: the picture is the HUD FLIR (`SimFLIRToggle`, **Shift+H**), a grayscale sensor image inside the HUD field of view | none — press Shift+H again | — |

## Connecting

Both machines run the same AppImage. Default port **UDP 2934** (the game also uses the port above).

    # HOST (listens)
    FF_MP_CONNECT="2934" ./FreeFalcon-x86_64-<date>.AppImage
    # JOINER
    FF_MP_CONNECT="2934:2934:<host-ip>" ./FreeFalcon-x86_64-<date>.AppImage

Or use the COMMS screen: URL/IP field, Connect as Server/Client, CONNECT, then close the status
window with Cancel. `FF_MP_CONNECT="localPort[:remotePort[:host]]"` prints `[MPCONNECT] ... Online=1`.

## Two instances on one box

`scripts/qa/mp-dogfight.sh` (MODE=dogfight|campaign) drives both peers through the real UI. The second
instance needs its OWN data tree (`FF2=~/ff2`, a `cp -a --reflink=auto` of the install) and ports ~10
apart; windows are placed with `FF_WINPOS=x,y` on different monitors (a covered XWayland window runs at
1 frame/s). Harness hooks: `FF_TEST_MPDMG=<sec>[:<str>]` (one proximity hit on the remote player's
jet), `FF_TEST_MPFORMUP=<sec>[:<ft>[:<dur>]]` (hold position ahead of the remote player's nose).

## Diagnostics

`FF_DEBUG_MPCOMMS=1` (connect/game list), `FF_DEBUG_MPMSG=1` (`[mpmsg]` message census and `[mpdmg]`
lock/release/flags/damage/RegisterHit lines), `FF_DEBUG_MPCLOCK=1` (`[mpclock]` joiner clock step),
`FF_DEBUG_MPHOSTSIM=1` (`[mphostsim]`), `FF_DEBUG_CAMPCLOCK=1`, `FF_DEBUG_STARTCAMP=1`.

## Not known

* More than two players; internet play (no NAT traversal); the same flight taken by two players.

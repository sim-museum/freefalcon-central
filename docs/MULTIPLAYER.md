# FreeFalcon multiplayer between two PCs (MP-1 part a)

**Status: the transport path exists and is wired; a real two-machine connection has NEVER been
tested from this box.** Everything below follows from the code. Treat the first run as the
experiment, not as a checklist that is known to work.

## The short version

Both machines run the AppImage. Default port is **UDP 2934** (`CAPI_UDP_PORT`).

    # HOST (listens; no address needed)
    FF_MP_CONNECT="2934" ~/Documents/260904/FreeFalcon-x86_64.AppImage

    # CLIENT (connects to the host's LAN address)
    FF_MP_CONNECT="2934:2934:192.168.254.14" ~/Documents/260904/FreeFalcon-x86_64.AppImage

Find the host's address on the host with `hostname -I | awk '{print $1}'`.

## What FF_MP_CONNECT does

`FF_MP_CONNECT="localPort[:remotePort[:host]]"` (`src/ui/src/comms/phonebk.cpp:339`) drives the
phonebook connect path directly, so the connection is made **without going through the phonebook
dialog**. It is called during startup from `main_linux.cpp:1903`, right after `gCommsMgr->Setup()`.

* Host absent or empty  -> `ip_address = 0` -> listen as server.
* Host present          -> `ComAPIGetIP(host)` -> connect to that address.

It prints what it did, which is the first thing to read if nothing happens:

    [MPCONNECT] StartComms local=2934 remote=2934 ip=0x...
    [MPCONNECT] returned, Online=1

`Online=0` means the comms manager did not come up, and the problem is local -- not a network
question yet.

## Why this exists rather than "type the address into the UI"

The PO reported (2026-09-04) that no text could be typed anywhere in the FF UI, which made the
phonebook's address field unusable and multiplayer unreachable. That was **three stacked defects**,
now fixed (MP-1 part b, 2026-09-05):

1. the scancode was posted in `wParam` while ui95 decodes it from `lParam` -- every keystroke
   decoded to `Key = 0`;
2. `BuildAscii()` corrupted the DIK->ASCII table on Linux via a `MapVirtualKey` stub that returns
   its input unchanged;
3. `GetKeyState` was a stub returning 0, so no shift, caps, ctrl or alt.

**(3) is implemented but NOT yet verified** -- see STATUS.md. So the phonebook dialog should now
accept typing, but `FF_MP_CONNECT` remains the route that does not depend on any of that.

## If it does not connect

1. `ping <host-ip>` from the client.
2. On the host, confirm something is listening: `ss -lun | grep 2934`.
3. On the host, `sudo ufw status` -- if a firewall is active it must allow **UDP 2934**.
4. Re-run with the trace visible and read the `[MPCONNECT]` lines on BOTH machines.

## What is NOT known

* Whether the two machines actually complete a session -- untested, and untestable from one box.
* Whether more than two players work.
* Whether the in-game UI path (phonebook -> Connect) now works end to end, since the typing fix's
  shift handling is unverified.

Do not describe any of these as working until someone has run them on two machines.

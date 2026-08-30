#!/usr/bin/env python3
"""Seed phonebkn.da2 so a scripted client join needs no UI text entry.

MP-1's remaining work is driving peer B through the REAL comms UI. Client mode
takes the host address from the IP_ADDRESS_1 edit box (phonebk.cpp
CopyDataFromWindow -> ComAPIGetIP), and FF_UI_CLICK can only click -- FF_SIM_KEY
is sim-mode only, so there is no way to type into a UI edit box from a script.

But CopyDataToWindow pre-fills that box from a selected phone-book entry, and the
book is persisted. Seeding an entry means peer B can click the row and connect.

Format, from PhoneBook::Load/Save (cphoneb.cpp):
    int32  count                    (already LP64-corrected at both ends)
    per entry:
        char[100] url               MAX_URL_SIZE, NOT NUL-terminated on disk
        uint16    localPort
        uint16    remotePort
"""
import struct, sys, os

MAX_URL_SIZE = 100
CAPI_UDP_PORT = 2934          # falclib default; override with argv[3]

path = sys.argv[1] if len(sys.argv) > 1 else \
    os.path.expanduser("~/sgl/SAT/freeFalcon/WP/drive_c/FreeFalcon6/phonebkn.da2")
addr = sys.argv[2] if len(sys.argv) > 2 else "127.0.0.1"
port = int(sys.argv[3]) if len(sys.argv) > 3 else CAPI_UDP_PORT

if os.path.exists(path) and not os.path.exists(path + ".orig"):
    with open(path, "rb") as f:
        open(path + ".orig", "wb").write(f.read())
    print("backed up existing book -> %s.orig" % path)

url = addr.encode("latin-1")[:MAX_URL_SIZE]
url += b"\0" * (MAX_URL_SIZE - len(url))

with open(path, "wb") as f:
    f.write(struct.pack("<i", 1))
    f.write(url)
    f.write(struct.pack("<HH", port, port))

print("seeded %s with one entry: %s ports %d/%d" % (path, addr, port, port))

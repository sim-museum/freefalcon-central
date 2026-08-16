/*
 * FreeFalcon Linux Port - stubs for Windows-only modules excluded from
 * the Linux build (tools/mono debugger, tools/IsBad, tools/ui_tools font
 * editor, camptool dialogs, movie player, DirectPlay voice chat).
 */

#ifdef FF_LINUX

#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <unistd.h>

#include "compat_types.h"
#include "d3dxcore.h"

/* ============================================================
 * tools/mono/debuggr.cpp - mono debug display (C linkage)
 * ============================================================ */
extern "C" {

int gDumping = 0;

void InitDebug(void) {}
void MonoPrint(char *fmt, ...) {
#ifdef FF_MONO_TO_STDERR
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
#else
    (void)fmt;
#endif
}
void MonoLocate(unsigned char x, unsigned char y) { (void)x; (void)y; }
void MonoGetLoc(int *x, int *y) { if (x) *x = 0; if (y) *y = 0; }
void MonoCls(void) {}
void MonoScroll(void) {}
void MonoColor(char attribute) { (void)attribute; }
void WriteDebugPixel(int x, int y) { (void)x; (void)y; }
void WriteToFile(char *s) { (void)s; }
void DebugSwapbuffer(void) {}
void DebugClear(void) {}
void stoppingvoice(void) {}

char *_getdcwd(int drive, char *buffer, int maxlen) {
    (void)drive;
    return getcwd(buffer, maxlen);
}

BOOL GetVolumeInformation(LPCSTR lpRootPathName, LPSTR lpVolumeNameBuffer, DWORD nVolumeNameSize,
                          LPDWORD lpVolumeSerialNumber, LPDWORD lpMaximumComponentLength,
                          LPDWORD lpFileSystemFlags, LPSTR lpFileSystemNameBuffer, DWORD nFileSystemNameSize) {
    (void)lpRootPathName;
    if (lpVolumeNameBuffer && nVolumeNameSize) lpVolumeNameBuffer[0] = '\0';
    if (lpVolumeSerialNumber) *lpVolumeSerialNumber = 0x46465650; /* 'FFVP' */
    if (lpMaximumComponentLength) *lpMaximumComponentLength = 255;
    if (lpFileSystemFlags) *lpFileSystemFlags = 0;
    if (lpFileSystemNameBuffer && nFileSystemNameSize) {
        strncpy(lpFileSystemNameBuffer, "ext4", nFileSystemNameSize - 1);
        lpFileSystemNameBuffer[nFileSystemNameSize - 1] = '\0';
    }
    return TRUE;
}

/* movie player (src/movie - Windows Video for Windows only) */
int movieInit(void *hwnd, void *dd, void *surf) { (void)hwnd; (void)dd; (void)surf; return 0; }
int movieUnInit(void) { return 0; }
int movieOpen(char *filename) { (void)filename; return 0; }
int movieClose(void) { return 0; }
int movieStart(void) { return 0; }
int movieIsPlaying(void) { return 0; }

} /* extern "C" */

/* ============================================================
 * tools/IsBad/IsBad.cpp - pointer validators (C++ linkage)
 * ============================================================ */
bool F4IsBadReadPtr(const void *lp, unsigned int ucb) { (void)ucb; return lp == NULL; }
bool F4IsBadCodePtr(void *lpfn) { return lpfn == NULL; }
bool F4IsBadWritePtr(void *lp, unsigned int ucb) { (void)ucb; return lp == NULL; }
extern "C" {
int F4IsBadReadPtrC(const void *lp, unsigned int ucb) { (void)ucb; return lp == NULL; }
int F4IsBadCodePtrC(void *lpfn) { return lpfn == NULL; }
int F4IsBadWritePtrC(void *lp, unsigned int ucb) { (void)ucb; return lp == NULL; }
}

/* ============================================================
 * voicecomunication/voice.cpp - DirectPlay voice chat (excluded)
 * ============================================================ */
void StopVoice() {}
void Transmit(int com) { (void)com; }
void RefreshVoiceFreqs(void) {}
void startupvoice(char *p) { (void)p; }
void DirectVoiceSetVolume(int Channel) { (void)Channel; }

void set_spinner1(int s) { (void)s; }
void set_spinner3(int s) { (void)s; }

/* DirectPlay globals normally defined in voice.cpp */
struct IDirectPlay8Client;
struct IDirectPlay8Server;
IDirectPlay8Client *g_pDPClient = NULL;
IDirectPlay8Server *g_pDPServer = NULL;

/* ============================================================
 * camptool dialogs (Windows-only campaign editor)
 * ============================================================ */
BOOL OpenCampFile(HWND h) { (void)h; return FALSE; }
BOOL SaveCampFile(HWND h, int mode) { (void)h; (void)mode; return FALSE; }
BOOL SaveAsCampFile(HWND h, int mode) { (void)h; (void)mode; return FALSE; }
typedef unsigned long ff_ulong;
void CheckForCheatFlight(ff_ulong time) { (void)time; }

/* ============================================================
 * tools/ui_tools/savefont.cpp - font editor tool callbacks
 * ============================================================ */
class C_Base;
void SaveFontCB(long, short, C_Base *) {}
void CreateFontCB(long, short, C_Base *) {}
void CreateTheFontCB(long, short, C_Base *) {}
void ChooseFontCB(long, short, C_Base *) {}
void IncreaseWidth(long, short, C_Base *) {}
void DecreaseWidth(long, short, C_Base *) {}
void IncreaseKern(long, short, C_Base *) {}
void DecreaseKern(long, short, C_Base *) {}
void IncreaseLead(long, short, C_Base *) {}
void DecreaseLead(long, short, C_Base *) {}
void IncreaseTrail(long, short, C_Base *) {}
void DecreaseTrail(long, short, C_Base *) {}
void InitFontTool() {}

/* ============================================================
 * D3DX texture helpers - minimal failure stubs.
 * Render2DBitmap falls back gracefully when these fail.
 * ============================================================ */
// FF_LINUX: D3DXCreateTexture / D3DXLoadTextureFromMemory are now IMPLEMENTED in
// compat/d3d_gl.cpp against the real D3D7Surface machinery. They used to be stubs
// here returning E_FAIL with a NULL surface, which silently disabled
// ContextMPR::Render2DBitmap and with it the loading splash bitmap and the in-sim
// mouse cursor.


#endif /* FF_LINUX */

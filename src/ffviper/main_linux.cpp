// FFViper Linux main entry point
// Creates SDL2 window, OpenGL context, and initializes the game

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <time.h>
#include <fenv.h>
#include <signal.h>
#include <execinfo.h>
#include <mutex>
#include <queue>
#include <vector>

// Linux port debug configuration
#include "ff_linux_debug.h"

// SDL2
#include <SDL2/SDL.h>

// OpenGL
#include <GL/glew.h>
#include <GL/gl.h>

// OpenAL
#include <AL/al.h>
#include <AL/alc.h>

// FreeFalcon core headers - minimal set for initial startup
#include "falclib.h"
#include "f4find.h"
#include "f4vu.h"
#include "classtbl.h"
#include "camplib.h"
#include "threadmgr.h"
#include "sim/include/simloop.h"
#include "codelib/resources/reslib/src/resmgr.h"
#include "theaterdef.h"
#include "campaign.h"
#include "campaign/include/cmpclass.h"  // For TheCampaign
#include "falcsess.h"                     // For FalconLocalSession
#include "dispcfg.h"
#include "playerop.h"
#include "falclib/include/dispopts.h"
#include "rules.h"
#include "fsound.h"
#include "simdrive.h"
#include "graphics/include/drawparticlesys.h"
#include "graphics/include/timemgr.h"
#include "graphics/include/setup.h"
#include "graphics/include/renderow.h"
#include "graphics/include/drawbsp.h"
#include "sim/include/otwdrive.h"

// D3D/OpenGL rendering
#include "d3d.h"
#include "ddraw.h"
#include "graphics/dxengine/dxengine.h"
#include "graphics/include/tex.h"
#include "graphics/include/context.h"
#include "graphics/include/device.h"

// UI system (for gMainHandler)
#include "ui95/chandler.h"
#include "ui/include/falcuser.h"
#include "ui/include/uicomms.h"  // FF_LINUX: For gCommsMgr
#include "ui/include/logbook.h"  // FF_LINUX: For LogBook / UI_logbk

// Simulation input (for IO structure and joystick data)
#include "sim/include/simio.h"
#include "sim/include/sinput.h"
#include "sim/include/inpfunc.h"  // FF_LINUX: For LoadFunctionTables()

// Campaign / instant action headers
#include "campaign/include/iaction.h"
#include "campaign/include/dogfight.h"
#include "campaign/include/weather.h"  // FF_LINUX: For WeatherClass / realWeather
#include "campaign/include/asearch.h"   // FF_LINUX: For AS_DataClass / ASD pathfinder

// External initialization functions
extern void LoadTheaterList();
extern void FF_PresentPrimarySurface();  // Present DirectDraw primary surface via OpenGL
extern void LoadTrails();
extern int UI_Startup();
extern void UI_Cleanup();

// External globals from the UI/game systems
extern C_Handler *gMainHandler;
extern int doUI;
extern void ReadCampAIInputs(char* name);
extern int LoadTactics(char *name);
extern void InitVU();
extern void BuildAscii();

// Campaign/mission lifecycle externs
extern void EndUI(void);
extern void CampaignJoinSuccess(void);
extern void CampaignJoinFail(void);
extern void ShutdownCampaign(void);
extern void CampaignPreloadSuccess(int remote);
extern void CampaignAutoSave(FalconGameType type);
extern char gUI_CampaignFile[];
extern void UI_CommsErrorMessage(WORD error);
extern int gameCompressionRatio;
extern DogfightClass SimDogfight;
extern void StartCampaignGame(int local, int game_type);
extern void tactical_restart_mission(void);

// Communications initialization (needed for CAPI function pointers)
typedef struct WSAData {
    unsigned short wVersion;
    unsigned short wHighVersion;
    char szDescription[257];
    char szSystemStatus[129];
} WSADATA;
extern "C" int initialize_windows_sockets(WSADATA *wsaData);

// Default data directory - can be overridden with -d flag or env var
#define DEFAULT_DATA_DIR "/home/g/ese/SAT/WP/drive_c/FreeFalcon6"

// Window settings - must match UI resolution (1024x768 for HiRes UI)
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
int g_nWindowWidth = WINDOW_WIDTH;
int g_nWindowHeight = WINDOW_HEIGHT;
#ifndef FF_GIT_HASH
#define FF_GIT_HASH "dev"
#endif
#define WINDOW_TITLE "Free Falcon 6 Linux Port [" FF_GIT_HASH "]"

// External globals from falclib
extern char FalconDataDirectory[];
extern char FalconCampaignSaveDirectory[];
extern char FalconCampUserSaveDirectory[];
extern char FalconTerrainDataDir[];
extern char FalconMiscTexDataDir[];
extern char FalconPictureDirectory[];
extern char FalconObjectDataDir[];
extern char Falcon3DDataDir[];

// Sound-related directories (defined in winmain.cpp, we just reference them)
extern char FalconSoundThrDirectory[];
extern char FalconUISoundDirectory[];
extern char FalconCockpitThrDirectory[];
extern char FalconZipsThrDirectory[];
extern char FalconTacrefThrDirectory[];
extern char FalconSplashThrDirectory[];
extern char FalconMovieDirectory[];
extern char FalconMovieMode[];
extern char FalconUIArtDirectory[];
extern char FalconUIArtThrDirectory[];

// Global SDL objects - these replace Windows HWND etc.
SDL_Window* g_SDLWindow = nullptr;
SDL_GLContext g_GLContext = nullptr;

// FF_LINUX: ShiAssert control globals
// winmain.cpp defines these under #ifdef DEBUG, but CMake only defines _DEBUG (with underscore)
// So we always need to define them here for Linux builds
// shiHardCrashOn MUST be 0 to prevent crash on assertions
int shiAssertsOn = 1;
int shiWarningsOn = 1;
int shiHardCrashOn = 0;

// OpenGL context transfer for multi-threaded rendering
// The sim thread needs the GL context for OTWDriver.Cycle(), but the main thread
// owns it during UI mode. These functions transfer ownership.
static std::mutex g_glContextMutex;
bool g_simOwnsGLContext = false;  // Non-static: accessed from simloop.cpp via extern

// FF_LINUX: Set on the MAIN thread in the FM_START_* handlers before EndUI(), so
// UI_Cleanup() reliably skips tearing down the display device while the sim
// thread is concurrently (re)building it in EnterMode(Sim)/DisplayDevice::Setup.
// The old guard checked FalconDisplay.currentMode, which the sim thread only sets
// to Sim at the END of EnterMode - a TOCTOU race (crash in CDXEngine::Release).
volatile int g_simTakingOverDisplay = 0;

// Release GL context from current thread (call before another thread acquires it)
void FF_ReleaseGLContext() {
    std::lock_guard<std::mutex> lock(g_glContextMutex);
    if (g_SDLWindow) {
        SDL_GL_MakeCurrent(g_SDLWindow, NULL);
    }
}

// Acquire GL context on current thread
void FF_AcquireGLContext() {
    std::lock_guard<std::mutex> lock(g_glContextMutex);
    if (g_SDLWindow && g_GLContext) {
        SDL_GL_MakeCurrent(g_SDLWindow, g_GLContext);
    }
}

// Called by sim thread before it starts rendering
void FF_SimThreadAcquireGL() {
    fprintf(stderr, "[GL] FF_SimThreadAcquireGL() ENTER\n");
    fflush(stderr);
    FF_AcquireGLContext();
    g_simOwnsGLContext = true;
    fprintf(stderr, "[GL] Sim thread acquired GL context, g_simOwnsGLContext=true\n");
    fflush(stderr);
}

// Called by sim thread when it's done rendering
void FF_SimThreadReleaseGL() {
    fprintf(stderr, "[GL] FF_SimThreadReleaseGL() ENTER\n");
    fflush(stderr);
    g_simOwnsGLContext = false;
    FF_ReleaseGLContext();
    fprintf(stderr, "[GL] Sim thread released GL context, g_simOwnsGLContext=false\n");
    fflush(stderr);
}

// Swap buffers - callable from any thread that owns the GL context
// FF_LINUX: Paint both buffers black during mission load, before the renderer
// is ready to draw the splash. Without this the uninitialized GL back buffer
// shows as a white screen during the (several-second) device/terrain setup at
// the start of OTWDriver::Enter(). Called on the sim thread, which owns the GL
// context at that point.
void FF_LoadingClear() {
    if (!g_SDLWindow || !g_GLContext) return;
    if (SDL_GL_GetCurrentContext() != g_GLContext) return;  // only if we own it
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GLboolean savedScissor = glIsEnabled(GL_SCISSOR_TEST);
    if (savedScissor) glDisable(GL_SCISSOR_TEST);
    // FF_LINUX: D3D/GL setup on the sim thread can leave colour/depth writes
    // masked; force full write state so the clear actually takes effect (without
    // this the sim-thread loading clears ran but were masked out -> white).
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // FF_LINUX: On Wayland, a single SDL_GL_SwapWindow on the sim thread during
    // the (non-continuously-swapping) mission-load setup does NOT become visible
    // - the surface only presents once swapping is continuous (e.g. the cockpit).
    // The back buffer IS cleared black (verified by glReadPixels), it just isn't
    // shown. A short burst of swaps reliably commits the surface, so the load
    // shows black instead of a white (uninitialized) window. Call this at every
    // heavy setup checkpoint so black persists through the blocking steps.
    for (int i = 0; i < 4; i++) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        SDL_GL_SwapWindow(g_SDLWindow);
    }
    if (savedScissor) glEnable(GL_SCISSOR_TEST);
}

void FF_SwapBuffers() {
    // FF_LINUX: frame marker for FF_PROBE_PIXEL draw attribution
    {
        static int s_probeMark = -1;
        if (s_probeMark == -1) s_probeMark = getenv("FF_PROBE_PIXEL") ? 1 : 0;
        if (s_probeMark) fprintf(stderr, "[PIXPROBE] ---- SWAP ----\n");
    }
    static int swapCount = 0;
    swapCount++;

    if (g_SDLWindow && g_GLContext) {
        // Make sure the GL context is current on this thread before swapping
        SDL_GLContext current = SDL_GL_GetCurrentContext();
        if (current != g_GLContext) {
            SDL_GL_MakeCurrent(g_SDLWindow, g_GLContext);
        }

        // FF_LINUX: Ensure we're presenting from the default framebuffer, not an FBO
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Capture screenshot of the GL framebuffer
        // Wait until frame 200 to ensure lighting/sky is fully initialized
        if (swapCount == 200 || swapCount == 600) {
            int w = 0, h = 0;
            SDL_GL_GetDrawableSize(g_SDLWindow, &w, &h);
            if (w > 0 && h > 0) {
                unsigned char* pixels = new unsigned char[w * h * 3];
                // Read from back buffer (where rendering goes)
                glReadBuffer(GL_BACK);
                glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels);
                // Save as BMP
                FILE* f = fopen("/tmp/screenshot_sim.bmp", "wb");
                if (f) {
                    int rowSize = (w * 3 + 3) & ~3;
                    int imageSize = rowSize * h;
                    // BMP header (14 bytes)
                    uint16_t bfType = 0x4D42;
                    uint32_t bfSize = 54 + imageSize;
                    uint16_t bfReserved = 0;
                    uint32_t bfOffBits = 54;
                    fwrite(&bfType, 2, 1, f);
                    fwrite(&bfSize, 4, 1, f);
                    fwrite(&bfReserved, 2, 1, f);
                    fwrite(&bfReserved, 2, 1, f);
                    fwrite(&bfOffBits, 4, 1, f);
                    // DIB header (40 bytes)
                    uint32_t biSize = 40;
                    int32_t biWidth = w, biHeight = h;
                    uint16_t biPlanes = 1, biBitCount = 24;
                    uint32_t biCompression = 0, biSizeImage = imageSize;
                    int32_t biXPPM = 2835, biYPPM = 2835;
                    uint32_t biClrUsed = 0, biClrImportant = 0;
                    fwrite(&biSize, 4, 1, f);
                    fwrite(&biWidth, 4, 1, f);
                    fwrite(&biHeight, 4, 1, f);
                    fwrite(&biPlanes, 2, 1, f);
                    fwrite(&biBitCount, 2, 1, f);
                    fwrite(&biCompression, 4, 1, f);
                    fwrite(&biSizeImage, 4, 1, f);
                    fwrite(&biXPPM, 4, 1, f);
                    fwrite(&biYPPM, 4, 1, f);
                    fwrite(&biClrUsed, 4, 1, f);
                    fwrite(&biClrImportant, 4, 1, f);
                    // Pixel data - glReadPixels gives us bottom-up RGB, BMP wants bottom-up BGR
                    unsigned char* row = new unsigned char[rowSize];
                    for (int y = 0; y < h; y++) {
                        memset(row, 0, rowSize);
                        for (int x = 0; x < w; x++) {
                            unsigned char* src = pixels + (y * w + x) * 3;
                            row[x * 3 + 0] = src[2]; // B
                            row[x * 3 + 1] = src[1]; // G
                            row[x * 3 + 2] = src[0]; // R
                        }
                        fwrite(row, rowSize, 1, f);
                    }
                    delete[] row;
                    fclose(f);
                }
                delete[] pixels;
            }
        }

        // Present the frame
        SDL_GL_SwapWindow(g_SDLWindow);
    }
}

// OpenAL - global for DirectSound compatibility layer
ALCdevice* g_alDevice = nullptr;
ALCcontext* g_alContext = nullptr;

// D3D7 interfaces (OpenGL-backed)
static IDirect3D7* g_pD3D = nullptr;
static IDirect3DDevice7* g_pD3DDevice = nullptr;
static IDirectDraw7* g_pDD = nullptr;
static IDirectDrawSurface7* g_pRenderTarget = nullptr;
static DXContext* g_pDXContext = nullptr;
static bool g_graphicsInitialized = false;

// DXEngine global (declared in dxengine.h as extern)
bool g_Use_DX_Engine = false;

// Game state
static bool g_running = true;
static bool g_gameInitialized = false;
static bool g_autoTestInstantAction = false;  // TEST: Set by auto-launch code
bool g_testInstantActionFlag = false;  // Command-line flag for auto-testing
volatile int g_requestedPanel = -1;  // Set by main thread, read by sim thread for view testing
volatile unsigned long g_ffImpactShotAt = 0;   // deferred impact capture (FF_SHOT_ON_IMPACT=<ms>)
const char *g_ffImpactShotName = 0;
volatile int g_requestedViewMode = -1;  // Set by main thread, -1=none, 0=HUD, 1=cockpit, 2=chase, 3=orbit
volatile int g_requestedNVGToggle = 0;  // FF_LINUX (NVG-2): set by main thread, consumed on the sim thread
volatile int g_screenshotRequest = 0;   // Set by main thread, read by sim thread to take screenshot
const char* g_screenshotFilename = "/tmp/ff_screenshot.bmp"; // Filename for next screenshot

// These globals are defined in ui/src/winmain.cpp - use extern
extern HWND mainAppWnd;
extern HWND mainMenuWnd;
extern HINSTANCE hInst;
extern const char* FREE_FALCON_BRAND;
extern const char* FREE_FALCON_PROJECT;
extern const char* FREE_FALCON_VERSION;

// Message queue for Windows-style message passing

struct GameMessage {
    UINT message;
    WPARAM wParam;
    LPARAM lParam;
};

static std::queue<GameMessage> g_messageQueue;
static std::mutex g_messageMutex;

// Post a message to the queue (like Windows PostMessage)
void PostGameMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    std::lock_guard<std::mutex> lock(g_messageMutex);
    g_messageQueue.push({msg, wParam, lParam});
}

// Process messages in the queue
bool ProcessGameMessages();

// Post a message without locking (for use inside ProcessGameMessages)
static std::vector<GameMessage> g_pendingMessages;

// =============================================================================
// SDL TO DIRECTINPUT SCANCODE TRANSLATION
// SDL scancodes differ from DirectInput DIK_* codes - build a translation table
// =============================================================================

// =============================================================================
// SDL KEYBOARD EVENT BUFFER FOR SIM INPUT
// The sim reads keyboard input via DirectInput's GetDeviceData() (sikeybd.cpp).
// On Linux, DirectInput is disabled. Instead, SDL keyboard events are buffered
// here and read by OnSimKeyboardInput() via FF_PopKeyEvents().
// =============================================================================
#include "dinput.h"  // For DIDEVICEOBJECTDATA

static std::mutex g_keyEventMutex;
static DIDEVICEOBJECTDATA g_keyEventBuf[64];
static int g_keyEventHead = 0;
static int g_keyEventTail = 0;
static unsigned char g_keyState[256] = {0};  // Current key state (0x80 = pressed)

// Push a keyboard event into the buffer (called from SDL event loop)
void FF_PushKeyEvent(int dikCode, bool isDown) {
    if (dikCode <= 0 || dikCode >= 256) return;
    std::lock_guard<std::mutex> lock(g_keyEventMutex);
    g_keyState[dikCode] = isDown ? 0x80 : 0x00;
    int next = (g_keyEventHead + 1) % 64;
    if (next == g_keyEventTail) return;  // Buffer full, drop event
    g_keyEventBuf[g_keyEventHead].dwOfs = (DWORD)dikCode;
    g_keyEventBuf[g_keyEventHead].dwData = isDown ? 0x80 : 0x00;
    g_keyEventBuf[g_keyEventHead].dwTimeStamp = GetTickCount();
    g_keyEventBuf[g_keyEventHead].dwSequence = 0;
    g_keyEventHead = next;
}

// Pop keyboard events from the buffer (called from sim thread's OnSimKeyboardInput)
int FF_PopKeyEvents(DIDEVICEOBJECTDATA* outBuf, int maxEvents) {
    std::lock_guard<std::mutex> lock(g_keyEventMutex);
    int count = 0;
    while (g_keyEventTail != g_keyEventHead && count < maxEvents) {
        outBuf[count] = g_keyEventBuf[g_keyEventTail];
        g_keyEventTail = (g_keyEventTail + 1) % 64;
        count++;
    }
    return count;
}

// Get current keyboard state (called from sim thread)
void FF_GetKeyState(unsigned char* outState, int size) {
    std::lock_guard<std::mutex> lock(g_keyEventMutex);
    int copySize = (size < 256) ? size : 256;
    memcpy(outState, g_keyState, copySize);
}

// =============================================================================
// SDL Mouse Event Buffer for Sim Input
// Similar to the keyboard buffer - SDL mouse events are pushed from the main
// thread and popped by the sim thread's OnSimMouseInput().
// =============================================================================
static std::mutex g_mouseEventMutex;
static DIDEVICEOBJECTDATA g_mouseEventBuf[128];
static int g_mouseEventHead = 0;
static int g_mouseEventTail = 0;

void FF_PushMouseEvent(DWORD dwOfs, DWORD dwData) {
    std::lock_guard<std::mutex> lock(g_mouseEventMutex);
    int next = (g_mouseEventHead + 1) % 128;
    if (next == g_mouseEventTail) return;  // Buffer full, drop event
    g_mouseEventBuf[g_mouseEventHead].dwOfs = dwOfs;
    g_mouseEventBuf[g_mouseEventHead].dwData = dwData;
    g_mouseEventBuf[g_mouseEventHead].dwTimeStamp = GetTickCount();
    g_mouseEventBuf[g_mouseEventHead].dwSequence = 0;
    g_mouseEventHead = next;
}

int FF_PopMouseEvents(DIDEVICEOBJECTDATA* outBuf, int maxEvents) {
    std::lock_guard<std::mutex> lock(g_mouseEventMutex);
    int count = 0;
    while (g_mouseEventTail != g_mouseEventHead && count < maxEvents) {
        outBuf[count] = g_mouseEventBuf[g_mouseEventTail];
        g_mouseEventTail = (g_mouseEventTail + 1) % 128;
        count++;
    }
    return count;
}

// SDL joystick globals
static SDL_Joystick* g_SDLJoystick = nullptr;
// FF_LINUX (SESS-4 harness): index of the SDL virtual joystick when
// FF_VIRTUAL_JOYSTICK is set, else -1. See InitSDLJoystick.
static int g_VirtualJoyIndex = -1;
// SESS-4 instrumentation: distinguish "no SDL hat event arrived" from
// "arrived but was filtered/overwritten".
static int g_hatEventCount = 0, g_hatAcceptedCount = 0;
static int g_hatLastWhich = -999, g_hatLastValue = -1;
static int g_JoystickIndex = -1;
static int g_JoystickNumAxes = 0;
static int g_JoystickNumButtons = 0;
static int g_JoystickNumHats = 0;

// Axis value cache (raw SDL values)
static int16_t g_JoystickAxes[16] = {0};

// SDL scancode to DIK code translation table
static int SDL_to_DIK[SDL_NUM_SCANCODES] = {0};

// Initialize the SDL to DIK translation table
static void InitScancodeTranslation() {
    memset(SDL_to_DIK, 0, sizeof(SDL_to_DIK));

    // Escape and function keys
    SDL_to_DIK[SDL_SCANCODE_ESCAPE] = DIK_ESCAPE;
    SDL_to_DIK[SDL_SCANCODE_F1] = DIK_F1;
    SDL_to_DIK[SDL_SCANCODE_F2] = DIK_F2;
    SDL_to_DIK[SDL_SCANCODE_F3] = DIK_F3;
    SDL_to_DIK[SDL_SCANCODE_F4] = DIK_F4;
    SDL_to_DIK[SDL_SCANCODE_F5] = DIK_F5;
    SDL_to_DIK[SDL_SCANCODE_F6] = DIK_F6;
    SDL_to_DIK[SDL_SCANCODE_F7] = DIK_F7;
    SDL_to_DIK[SDL_SCANCODE_F8] = DIK_F8;
    SDL_to_DIK[SDL_SCANCODE_F9] = DIK_F9;
    SDL_to_DIK[SDL_SCANCODE_F10] = DIK_F10;
    SDL_to_DIK[SDL_SCANCODE_F11] = DIK_F11;
    SDL_to_DIK[SDL_SCANCODE_F12] = DIK_F12;

    // Number row
    SDL_to_DIK[SDL_SCANCODE_1] = DIK_1;
    SDL_to_DIK[SDL_SCANCODE_2] = DIK_2;
    SDL_to_DIK[SDL_SCANCODE_3] = DIK_3;
    SDL_to_DIK[SDL_SCANCODE_4] = DIK_4;
    SDL_to_DIK[SDL_SCANCODE_5] = DIK_5;
    SDL_to_DIK[SDL_SCANCODE_6] = DIK_6;
    SDL_to_DIK[SDL_SCANCODE_7] = DIK_7;
    SDL_to_DIK[SDL_SCANCODE_8] = DIK_8;
    SDL_to_DIK[SDL_SCANCODE_9] = DIK_9;
    SDL_to_DIK[SDL_SCANCODE_0] = DIK_0;
    SDL_to_DIK[SDL_SCANCODE_MINUS] = DIK_MINUS;
    SDL_to_DIK[SDL_SCANCODE_EQUALS] = DIK_EQUALS;
    SDL_to_DIK[SDL_SCANCODE_BACKSPACE] = DIK_BACK;

    // Top row letters
    SDL_to_DIK[SDL_SCANCODE_TAB] = DIK_TAB;
    SDL_to_DIK[SDL_SCANCODE_Q] = DIK_Q;
    SDL_to_DIK[SDL_SCANCODE_W] = DIK_W;
    SDL_to_DIK[SDL_SCANCODE_E] = DIK_E;
    SDL_to_DIK[SDL_SCANCODE_R] = DIK_R;
    SDL_to_DIK[SDL_SCANCODE_T] = DIK_T;
    SDL_to_DIK[SDL_SCANCODE_Y] = DIK_Y;
    SDL_to_DIK[SDL_SCANCODE_U] = DIK_U;
    SDL_to_DIK[SDL_SCANCODE_I] = DIK_I;
    SDL_to_DIK[SDL_SCANCODE_O] = DIK_O;
    SDL_to_DIK[SDL_SCANCODE_P] = DIK_P;
    SDL_to_DIK[SDL_SCANCODE_LEFTBRACKET] = DIK_LBRACKET;
    SDL_to_DIK[SDL_SCANCODE_RIGHTBRACKET] = DIK_RBRACKET;
    SDL_to_DIK[SDL_SCANCODE_RETURN] = DIK_RETURN;

    // Home row letters
    SDL_to_DIK[SDL_SCANCODE_CAPSLOCK] = DIK_CAPITAL;
    SDL_to_DIK[SDL_SCANCODE_A] = DIK_A;
    SDL_to_DIK[SDL_SCANCODE_S] = DIK_S;
    SDL_to_DIK[SDL_SCANCODE_D] = DIK_D;
    SDL_to_DIK[SDL_SCANCODE_F] = DIK_F;
    SDL_to_DIK[SDL_SCANCODE_G] = DIK_G;
    SDL_to_DIK[SDL_SCANCODE_H] = DIK_H;
    SDL_to_DIK[SDL_SCANCODE_J] = DIK_J;
    SDL_to_DIK[SDL_SCANCODE_K] = DIK_K;
    SDL_to_DIK[SDL_SCANCODE_L] = DIK_L;
    SDL_to_DIK[SDL_SCANCODE_SEMICOLON] = DIK_SEMICOLON;
    SDL_to_DIK[SDL_SCANCODE_APOSTROPHE] = DIK_APOSTROPHE;
    SDL_to_DIK[SDL_SCANCODE_GRAVE] = DIK_GRAVE;

    // Bottom row letters
    SDL_to_DIK[SDL_SCANCODE_LSHIFT] = DIK_LSHIFT;
    SDL_to_DIK[SDL_SCANCODE_BACKSLASH] = DIK_BACKSLASH;
    SDL_to_DIK[SDL_SCANCODE_Z] = DIK_Z;
    SDL_to_DIK[SDL_SCANCODE_X] = DIK_X;
    SDL_to_DIK[SDL_SCANCODE_C] = DIK_C;
    SDL_to_DIK[SDL_SCANCODE_V] = DIK_V;
    SDL_to_DIK[SDL_SCANCODE_B] = DIK_B;
    SDL_to_DIK[SDL_SCANCODE_N] = DIK_N;
    SDL_to_DIK[SDL_SCANCODE_M] = DIK_M;
    SDL_to_DIK[SDL_SCANCODE_COMMA] = DIK_COMMA;
    SDL_to_DIK[SDL_SCANCODE_PERIOD] = DIK_PERIOD;
    SDL_to_DIK[SDL_SCANCODE_SLASH] = DIK_SLASH;
    SDL_to_DIK[SDL_SCANCODE_RSHIFT] = DIK_RSHIFT;

    // Modifiers and space
    SDL_to_DIK[SDL_SCANCODE_LCTRL] = DIK_LCONTROL;
    SDL_to_DIK[SDL_SCANCODE_LALT] = DIK_LMENU;
    SDL_to_DIK[SDL_SCANCODE_SPACE] = DIK_SPACE;
    SDL_to_DIK[SDL_SCANCODE_RALT] = DIK_RMENU;
    SDL_to_DIK[SDL_SCANCODE_RCTRL] = DIK_RCONTROL;

    // Navigation keys
    SDL_to_DIK[SDL_SCANCODE_INSERT] = DIK_INSERT;
    SDL_to_DIK[SDL_SCANCODE_HOME] = DIK_HOME;
    SDL_to_DIK[SDL_SCANCODE_PAGEUP] = DIK_PRIOR;
    SDL_to_DIK[SDL_SCANCODE_DELETE] = DIK_DELETE;
    SDL_to_DIK[SDL_SCANCODE_END] = DIK_END;
    SDL_to_DIK[SDL_SCANCODE_PAGEDOWN] = DIK_NEXT;

    // Arrow keys
    SDL_to_DIK[SDL_SCANCODE_UP] = DIK_UP;
    SDL_to_DIK[SDL_SCANCODE_DOWN] = DIK_DOWN;
    SDL_to_DIK[SDL_SCANCODE_LEFT] = DIK_LEFT;
    SDL_to_DIK[SDL_SCANCODE_RIGHT] = DIK_RIGHT;

    // Numpad
    SDL_to_DIK[SDL_SCANCODE_NUMLOCKCLEAR] = DIK_NUMLOCK;
    SDL_to_DIK[SDL_SCANCODE_KP_DIVIDE] = DIK_DIVIDE;
    SDL_to_DIK[SDL_SCANCODE_KP_MULTIPLY] = DIK_MULTIPLY;
    SDL_to_DIK[SDL_SCANCODE_KP_MINUS] = DIK_SUBTRACT;
    SDL_to_DIK[SDL_SCANCODE_KP_7] = DIK_NUMPAD7;
    SDL_to_DIK[SDL_SCANCODE_KP_8] = DIK_NUMPAD8;
    SDL_to_DIK[SDL_SCANCODE_KP_9] = DIK_NUMPAD9;
    SDL_to_DIK[SDL_SCANCODE_KP_PLUS] = DIK_ADD;
    SDL_to_DIK[SDL_SCANCODE_KP_4] = DIK_NUMPAD4;
    SDL_to_DIK[SDL_SCANCODE_KP_5] = DIK_NUMPAD5;
    SDL_to_DIK[SDL_SCANCODE_KP_6] = DIK_NUMPAD6;
    SDL_to_DIK[SDL_SCANCODE_KP_1] = DIK_NUMPAD1;
    SDL_to_DIK[SDL_SCANCODE_KP_2] = DIK_NUMPAD2;
    SDL_to_DIK[SDL_SCANCODE_KP_3] = DIK_NUMPAD3;
    SDL_to_DIK[SDL_SCANCODE_KP_ENTER] = DIK_NUMPADENTER;
    SDL_to_DIK[SDL_SCANCODE_KP_0] = DIK_NUMPAD0;
    SDL_to_DIK[SDL_SCANCODE_KP_PERIOD] = DIK_DECIMAL;

    // Lock keys
    SDL_to_DIK[SDL_SCANCODE_SCROLLLOCK] = DIK_SCROLL;
    SDL_to_DIK[SDL_SCANCODE_PRINTSCREEN] = DIK_SYSRQ;

    // Windows keys
    SDL_to_DIK[SDL_SCANCODE_LGUI] = DIK_LWIN;
    SDL_to_DIK[SDL_SCANCODE_RGUI] = DIK_RWIN;
    SDL_to_DIK[SDL_SCANCODE_APPLICATION] = DIK_APPS;
}

// Convert SDL scancode to DIK code
static int ConvertSDLToDIK(SDL_Scancode scancode) {
    if (scancode >= 0 && scancode < SDL_NUM_SCANCODES) {
        return SDL_to_DIK[scancode];
    }
    return 0;
}

// =============================================================================
// SDL JOYSTICK SUPPORT
// Initialize, poll, and convert joystick data to FreeFalcon format
// =============================================================================

// Initialize SDL joystick subsystem and open first available joystick
static void InitSDLJoystick() {
    // Initialize joystick subsystem if not already done
    if (SDL_WasInit(SDL_INIT_JOYSTICK) == 0) {
        if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0) {
            FF_ERROR("Failed to initialize joystick subsystem: %s\n", SDL_GetError());
            return;
        }
    }

#ifdef FF_LINUX
    // FF_LINUX (SESS-4 test harness): FF_VIRTUAL_JOYSTICK=1 attaches an SDL
    // virtual joystick (4 axes, 8 buttons, 1 hat) so the POV-hat -> view-pan path
    // can be exercised with no hardware. SDL feeds it through the ordinary
    // SDL_JOYHATMOTION/SDL_JOYAXISMOTION event path, so this tests the real code,
    // not a bypass. Drive it with FF_SIM_HAT (see ServiceVirtualHatScript).
    // Only attaches when no real stick is present, so it can never shadow one.
    static int s_virtJoy = -1;

    if (s_virtJoy < 0) s_virtJoy = getenv("FF_VIRTUAL_JOYSTICK") ? 1 : 0;

    if (s_virtJoy == 1)
    {
        // Attach even when a real stick is present -- the test needs a device
        // whose hat we can drive, and SDL_JoystickSetVirtualHat only works on a
        // virtual one. g_VirtualJoyIndex makes the open below prefer it.
        int vidx = SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER, 4, 8, 1);

        if (vidx < 0)
            FF_ERROR("FF_VIRTUAL_JOYSTICK: attach failed: %s\n", SDL_GetError());
        else
        {
            g_VirtualJoyIndex = vidx;
            fprintf(stderr, "[VJOY] attached virtual joystick at index %d "
                    "(4 axes, 8 buttons, 1 hat); real sticks present: %d\n",
                    vidx, SDL_NumJoysticks() - 1);
        }

        fflush(stderr);
        s_virtJoy = 2;   // once
    }

#endif

    int numJoysticks = SDL_NumJoysticks();
    FF_DEBUG_JOYSTICK("Found %d joystick(s)\n", numJoysticks);

    if (numJoysticks > 0) {
        // Open the first joystick
        int openIdx = (g_VirtualJoyIndex >= 0) ? g_VirtualJoyIndex : 0;
        g_SDLJoystick = SDL_JoystickOpen(openIdx);
        if (g_SDLJoystick) {
            g_JoystickIndex = SDL_JoystickInstanceID(g_SDLJoystick);
            g_JoystickNumAxes = SDL_JoystickNumAxes(g_SDLJoystick);
            g_JoystickNumButtons = SDL_JoystickNumButtons(g_SDLJoystick);
            g_JoystickNumHats = SDL_JoystickNumHats(g_SDLJoystick);

            FF_DEBUG_JOYSTICK("Opened joystick 0: %s\n", SDL_JoystickName(g_SDLJoystick));
            FF_DEBUG_JOYSTICK("  Axes: %d, Buttons: %d, Hats: %d\n",
                    g_JoystickNumAxes, g_JoystickNumButtons, g_JoystickNumHats);

            // FF_LINUX: ProcessJoyButtonAndPOVHat() dispatches hat movement to the
            // mapped view functions with `for (i = 0; i < NumberOfPOVs; i++)`, and
            // NumberOfPOVs is otherwise assigned in ONE place only: the DirectInput
            // setup path in siloop.cpp, from gpDIDevice->GetCapabilities(). That path
            // never runs here (DirectInput8Create fails, gDIEnabled = FALSE), so it
            // stayed 0, the loop never iterated, and the hat did nothing -- we filled
            // IO.povHatAngle[] every event and nobody ever read it. Same trap as
            // ReadThrottle/gTotalJoy. Publish the real SDL hat count.
            {
                extern unsigned int NumberOfPOVs;
                NumberOfPOVs = (unsigned int)((g_JoystickNumHats < SIMLIB_MAX_POV)
                                              ? g_JoystickNumHats : SIMLIB_MAX_POV);
                FF_DEBUG_JOYSTICK("  NumberOfPOVs set to %u\n", NumberOfPOVs);
            }

            // Enable joystick events
            SDL_JoystickEventState(SDL_ENABLE);
        } else {
            FF_ERROR("Failed to open joystick 0: %s\n", SDL_GetError());
        }
    }
}

// Cleanup SDL joystick
static void CleanupSDLJoystick() {
    if (g_SDLJoystick) {
        SDL_JoystickClose(g_SDLJoystick);
        g_SDLJoystick = nullptr;
        g_JoystickIndex = -1;
        FF_DEBUG_JOYSTICK("Joystick closed\n");
    }
}

// Convert SDL hat value to DirectInput POV angle (hundredths of degrees)
// SDL_HAT_CENTERED=0, SDL_HAT_UP=1, SDL_HAT_RIGHT=2, SDL_HAT_DOWN=4, SDL_HAT_LEFT=8
static DWORD ConvertSDLHatToPOV(Uint8 hat) {
    switch (hat) {
        case SDL_HAT_UP:        return 0;       // 0 degrees
        case SDL_HAT_RIGHTUP:   return 4500;    // 45 degrees
        case SDL_HAT_RIGHT:     return 9000;    // 90 degrees
        case SDL_HAT_RIGHTDOWN: return 13500;   // 135 degrees
        case SDL_HAT_DOWN:      return 18000;   // 180 degrees
        case SDL_HAT_LEFTDOWN:  return 22500;   // 225 degrees
        case SDL_HAT_LEFT:      return 27000;   // 270 degrees
        case SDL_HAT_LEFTUP:    return 31500;   // 315 degrees
        default:                return (DWORD)-1; // Centered
    }
}

// Poll joystick and update IO structure for simulation
// NOTE: Currently unused - we use event-driven input (SDL_JOYAXISMOTION etc.)
// This function is kept for potential future use if polling is preferred.
#if 0
static void PollSDLJoystick() {
    if (!g_SDLJoystick) return;

    // Update joystick state (needed if not using event loop for all input)
    SDL_JoystickUpdate();

    // Read axis values (SDL: -32768 to 32767)
    // FreeFalcon expects -10000 to 10000 for bipolar axes, 0 to 15000 for unipolar
    for (int i = 0; i < g_JoystickNumAxes && i < 16; i++) {
        g_JoystickAxes[i] = SDL_JoystickGetAxis(g_SDLJoystick, i);
    }

    // Map SDL joystick axes to FreeFalcon IO structure
    // Common joystick mapping:
    //   Axis 0: X axis (roll/aileron)
    //   Axis 1: Y axis (pitch/elevator)
    //   Axis 2: Z axis or throttle
    //   Axis 3: Rudder (Rz on some sticks)

    // Convert and store in IO structure
    // SDL range: -32768 to 32767
    // FreeFalcon bipolar range: -10000 to 10000

    if (g_JoystickNumAxes >= 1) {
        // Roll (X axis)
        IO.analog[AXIS_ROLL].ioVal = (int)(g_JoystickAxes[0] * 10000 / 32767);
        IO.analog[AXIS_ROLL].engrValue = (float)IO.analog[AXIS_ROLL].ioVal / 10000.0f;
        IO.analog[AXIS_ROLL].isUsed = true;
    }

    if (g_JoystickNumAxes >= 2) {
        // Pitch (Y axis) - note: may need inversion depending on joystick
        IO.analog[AXIS_PITCH].ioVal = (int)(g_JoystickAxes[1] * 10000 / 32767);
        IO.analog[AXIS_PITCH].engrValue = (float)IO.analog[AXIS_PITCH].ioVal / 10000.0f;
        IO.analog[AXIS_PITCH].isUsed = true;
    }

    if (g_JoystickNumAxes >= 3) {
        // Throttle (Z axis) - unipolar, 0 to 15000
        // SDL: -32768 (full forward) to 32767 (full back)
        // Invert and map to 0-15000
        int rawZ = g_JoystickAxes[2];
        int throttleVal = (32767 - rawZ) * 15000 / 65535;
        IO.analog[AXIS_THROTTLE].ioVal = throttleVal;
        IO.analog[AXIS_THROTTLE].engrValue = (float)throttleVal / 15000.0f;
        IO.analog[AXIS_THROTTLE].isUsed = true;
    }

    if (g_JoystickNumAxes >= 4) {
        // Yaw/Rudder (Rz or axis 3)
        IO.analog[AXIS_YAW].ioVal = (int)(g_JoystickAxes[3] * 10000 / 32767);
        IO.analog[AXIS_YAW].engrValue = (float)IO.analog[AXIS_YAW].ioVal / 10000.0f;
        IO.analog[AXIS_YAW].isUsed = true;
    }

    // Read button states
    for (int i = 0; i < g_JoystickNumButtons && i < SIMLIB_MAX_DIGITAL; i++) {
        IO.digital[i] = SDL_JoystickGetButton(g_SDLJoystick, i) ? TRUE : FALSE;
    }

    // Read POV hat
    for (int i = 0; i < g_JoystickNumHats && i < SIMLIB_MAX_POV; i++) {
        Uint8 hat = SDL_JoystickGetHat(g_SDLJoystick, i);
        IO.povHatAngle[i] = ConvertSDLHatToPOV(hat);
    }
}
#endif

// =============================================================================
// FALLBACK MENU SYSTEM
// Simple OpenGL-based menu when UI95 rendering isn't working
// =============================================================================
static bool g_useFallbackMenu = false;  // Disabled - testing texture rendering
static const char* g_menuStatusMessage = nullptr;  // Status message to display
static Uint32 g_menuStatusTime = 0;  // When the status message was set

struct MenuButton {
    const char* label;
    float x, y, w, h;
    void (*callback)();
};

// Callback declarations
static void FallbackExit();
static void FallbackDogfight();
static void FallbackCampaign();
static void FallbackSetup();
static void FallbackComms();
static void FallbackACMI();
static void FallbackLogbook();
static void FallbackInstantAction();

// Menu buttons - positioned at bottom of screen like the real menu
static MenuButton g_menuButtons[] = {
    {"EXIT",      20,  720, 80, 30, FallbackExit},
    {"LOGBOOK",   110, 720, 90, 30, FallbackLogbook},
    {"ACMI",      210, 720, 70, 30, FallbackACMI},
    {"SETUP",     290, 720, 80, 30, FallbackSetup},
    {"COMMS",     380, 720, 80, 30, FallbackComms},
    {"INSTANT",   560, 720, 90, 30, FallbackInstantAction},
    {"DOGFIGHT",  750, 720, 100, 30, FallbackDogfight},
    {"CAMPAIGN",  920, 720, 100, 30, FallbackCampaign},
};
static const int g_numMenuButtons = sizeof(g_menuButtons) / sizeof(g_menuButtons[0]);
static int g_hoveredButton = -1;

// Simple bitmap font rendering using OpenGL lines
static void DrawChar(float x, float y, char c, float scale) {
    // Simple 5x7 bitmap font represented as line segments
    glBegin(GL_LINES);

    switch(c) {
        case 'A':
            glVertex2f(x, y+7*scale); glVertex2f(x+2.5f*scale, y);
            glVertex2f(x+2.5f*scale, y); glVertex2f(x+5*scale, y+7*scale);
            glVertex2f(x+1*scale, y+4*scale); glVertex2f(x+4*scale, y+4*scale);
            break;
        case 'B':
            glVertex2f(x, y); glVertex2f(x, y+7*scale);
            glVertex2f(x, y); glVertex2f(x+4*scale, y);
            glVertex2f(x+4*scale, y); glVertex2f(x+5*scale, y+1*scale);
            glVertex2f(x+5*scale, y+1*scale); glVertex2f(x+5*scale, y+3*scale);
            glVertex2f(x+5*scale, y+3*scale); glVertex2f(x+4*scale, y+3.5f*scale);
            glVertex2f(x, y+3.5f*scale); glVertex2f(x+4*scale, y+3.5f*scale);
            glVertex2f(x+4*scale, y+3.5f*scale); glVertex2f(x+5*scale, y+4.5f*scale);
            glVertex2f(x+5*scale, y+4.5f*scale); glVertex2f(x+5*scale, y+6*scale);
            glVertex2f(x+5*scale, y+6*scale); glVertex2f(x+4*scale, y+7*scale);
            glVertex2f(x, y+7*scale); glVertex2f(x+4*scale, y+7*scale);
            break;
        case 'C':
            glVertex2f(x+5*scale, y+1*scale); glVertex2f(x+4*scale, y);
            glVertex2f(x+4*scale, y); glVertex2f(x+1*scale, y);
            glVertex2f(x+1*scale, y); glVertex2f(x, y+1*scale);
            glVertex2f(x, y+1*scale); glVertex2f(x, y+6*scale);
            glVertex2f(x, y+6*scale); glVertex2f(x+1*scale, y+7*scale);
            glVertex2f(x+1*scale, y+7*scale); glVertex2f(x+4*scale, y+7*scale);
            glVertex2f(x+4*scale, y+7*scale); glVertex2f(x+5*scale, y+6*scale);
            break;
        case 'D':
            glVertex2f(x, y); glVertex2f(x, y+7*scale);
            glVertex2f(x, y); glVertex2f(x+3*scale, y);
            glVertex2f(x+3*scale, y); glVertex2f(x+5*scale, y+2*scale);
            glVertex2f(x+5*scale, y+2*scale); glVertex2f(x+5*scale, y+5*scale);
            glVertex2f(x+5*scale, y+5*scale); glVertex2f(x+3*scale, y+7*scale);
            glVertex2f(x, y+7*scale); glVertex2f(x+3*scale, y+7*scale);
            break;
        case 'E':
            glVertex2f(x, y); glVertex2f(x, y+7*scale);
            glVertex2f(x, y); glVertex2f(x+5*scale, y);
            glVertex2f(x, y+3.5f*scale); glVertex2f(x+4*scale, y+3.5f*scale);
            glVertex2f(x, y+7*scale); glVertex2f(x+5*scale, y+7*scale);
            break;
        case 'F':
            glVertex2f(x, y); glVertex2f(x, y+7*scale);
            glVertex2f(x, y); glVertex2f(x+5*scale, y);
            glVertex2f(x, y+3.5f*scale); glVertex2f(x+4*scale, y+3.5f*scale);
            break;
        case 'G':
            glVertex2f(x+5*scale, y+1*scale); glVertex2f(x+4*scale, y);
            glVertex2f(x+4*scale, y); glVertex2f(x+1*scale, y);
            glVertex2f(x+1*scale, y); glVertex2f(x, y+1*scale);
            glVertex2f(x, y+1*scale); glVertex2f(x, y+6*scale);
            glVertex2f(x, y+6*scale); glVertex2f(x+1*scale, y+7*scale);
            glVertex2f(x+1*scale, y+7*scale); glVertex2f(x+4*scale, y+7*scale);
            glVertex2f(x+4*scale, y+7*scale); glVertex2f(x+5*scale, y+6*scale);
            glVertex2f(x+5*scale, y+6*scale); glVertex2f(x+5*scale, y+4*scale);
            glVertex2f(x+5*scale, y+4*scale); glVertex2f(x+3*scale, y+4*scale);
            break;
        case 'H':
            glVertex2f(x, y); glVertex2f(x, y+7*scale);
            glVertex2f(x+5*scale, y); glVertex2f(x+5*scale, y+7*scale);
            glVertex2f(x, y+3.5f*scale); glVertex2f(x+5*scale, y+3.5f*scale);
            break;
        case 'I':
            glVertex2f(x+1*scale, y); glVertex2f(x+4*scale, y);
            glVertex2f(x+2.5f*scale, y); glVertex2f(x+2.5f*scale, y+7*scale);
            glVertex2f(x+1*scale, y+7*scale); glVertex2f(x+4*scale, y+7*scale);
            break;
        case 'K':
            glVertex2f(x, y); glVertex2f(x, y+7*scale);
            glVertex2f(x+5*scale, y); glVertex2f(x, y+3.5f*scale);
            glVertex2f(x, y+3.5f*scale); glVertex2f(x+5*scale, y+7*scale);
            break;
        case 'L':
            glVertex2f(x, y); glVertex2f(x, y+7*scale);
            glVertex2f(x, y+7*scale); glVertex2f(x+5*scale, y+7*scale);
            break;
        case 'M':
            glVertex2f(x, y+7*scale); glVertex2f(x, y);
            glVertex2f(x, y); glVertex2f(x+2.5f*scale, y+3*scale);
            glVertex2f(x+2.5f*scale, y+3*scale); glVertex2f(x+5*scale, y);
            glVertex2f(x+5*scale, y); glVertex2f(x+5*scale, y+7*scale);
            break;
        case 'N':
            glVertex2f(x, y+7*scale); glVertex2f(x, y);
            glVertex2f(x, y); glVertex2f(x+5*scale, y+7*scale);
            glVertex2f(x+5*scale, y+7*scale); glVertex2f(x+5*scale, y);
            break;
        case 'O':
            glVertex2f(x+1*scale, y); glVertex2f(x+4*scale, y);
            glVertex2f(x+4*scale, y); glVertex2f(x+5*scale, y+1*scale);
            glVertex2f(x+5*scale, y+1*scale); glVertex2f(x+5*scale, y+6*scale);
            glVertex2f(x+5*scale, y+6*scale); glVertex2f(x+4*scale, y+7*scale);
            glVertex2f(x+4*scale, y+7*scale); glVertex2f(x+1*scale, y+7*scale);
            glVertex2f(x+1*scale, y+7*scale); glVertex2f(x, y+6*scale);
            glVertex2f(x, y+6*scale); glVertex2f(x, y+1*scale);
            glVertex2f(x, y+1*scale); glVertex2f(x+1*scale, y);
            break;
        case 'P':
            glVertex2f(x, y); glVertex2f(x, y+7*scale);
            glVertex2f(x, y); glVertex2f(x+4*scale, y);
            glVertex2f(x+4*scale, y); glVertex2f(x+5*scale, y+1*scale);
            glVertex2f(x+5*scale, y+1*scale); glVertex2f(x+5*scale, y+3*scale);
            glVertex2f(x+5*scale, y+3*scale); glVertex2f(x+4*scale, y+4*scale);
            glVertex2f(x+4*scale, y+4*scale); glVertex2f(x, y+4*scale);
            break;
        case 'R':
            glVertex2f(x, y); glVertex2f(x, y+7*scale);
            glVertex2f(x, y); glVertex2f(x+4*scale, y);
            glVertex2f(x+4*scale, y); glVertex2f(x+5*scale, y+1*scale);
            glVertex2f(x+5*scale, y+1*scale); glVertex2f(x+5*scale, y+3*scale);
            glVertex2f(x+5*scale, y+3*scale); glVertex2f(x+4*scale, y+4*scale);
            glVertex2f(x+4*scale, y+4*scale); glVertex2f(x, y+4*scale);
            glVertex2f(x+2*scale, y+4*scale); glVertex2f(x+5*scale, y+7*scale);
            break;
        case 'S':
            glVertex2f(x+5*scale, y+1*scale); glVertex2f(x+4*scale, y);
            glVertex2f(x+4*scale, y); glVertex2f(x+1*scale, y);
            glVertex2f(x+1*scale, y); glVertex2f(x, y+1*scale);
            glVertex2f(x, y+1*scale); glVertex2f(x, y+3*scale);
            glVertex2f(x, y+3*scale); glVertex2f(x+1*scale, y+3.5f*scale);
            glVertex2f(x+1*scale, y+3.5f*scale); glVertex2f(x+4*scale, y+3.5f*scale);
            glVertex2f(x+4*scale, y+3.5f*scale); glVertex2f(x+5*scale, y+4.5f*scale);
            glVertex2f(x+5*scale, y+4.5f*scale); glVertex2f(x+5*scale, y+6*scale);
            glVertex2f(x+5*scale, y+6*scale); glVertex2f(x+4*scale, y+7*scale);
            glVertex2f(x+4*scale, y+7*scale); glVertex2f(x+1*scale, y+7*scale);
            glVertex2f(x+1*scale, y+7*scale); glVertex2f(x, y+6*scale);
            break;
        case 'T':
            glVertex2f(x, y); glVertex2f(x+5*scale, y);
            glVertex2f(x+2.5f*scale, y); glVertex2f(x+2.5f*scale, y+7*scale);
            break;
        case 'U':
            glVertex2f(x, y); glVertex2f(x, y+6*scale);
            glVertex2f(x, y+6*scale); glVertex2f(x+1*scale, y+7*scale);
            glVertex2f(x+1*scale, y+7*scale); glVertex2f(x+4*scale, y+7*scale);
            glVertex2f(x+4*scale, y+7*scale); glVertex2f(x+5*scale, y+6*scale);
            glVertex2f(x+5*scale, y+6*scale); glVertex2f(x+5*scale, y);
            break;
        case 'X':
            glVertex2f(x, y); glVertex2f(x+5*scale, y+7*scale);
            glVertex2f(x+5*scale, y); glVertex2f(x, y+7*scale);
            break;
        case 'Y':
            glVertex2f(x, y); glVertex2f(x+2.5f*scale, y+3.5f*scale);
            glVertex2f(x+5*scale, y); glVertex2f(x+2.5f*scale, y+3.5f*scale);
            glVertex2f(x+2.5f*scale, y+3.5f*scale); glVertex2f(x+2.5f*scale, y+7*scale);
            break;
        case ' ':
            break;
        default:
            // Draw a box for unknown chars
            glVertex2f(x, y); glVertex2f(x+5*scale, y);
            glVertex2f(x+5*scale, y); glVertex2f(x+5*scale, y+7*scale);
            glVertex2f(x+5*scale, y+7*scale); glVertex2f(x, y+7*scale);
            glVertex2f(x, y+7*scale); glVertex2f(x, y);
            break;
    }
    glEnd();
}

static void DrawString(float x, float y, const char* str, float scale) {
    float charWidth = 6 * scale;
    while (*str) {
        DrawChar(x, y, *str, scale);
        x += charWidth;
        str++;
    }
}

static void DrawFallbackMenu() {
    // Set up 2D orthographic projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(2.0f);

    // Dark background
    glColor3f(0.15f, 0.2f, 0.25f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(WINDOW_WIDTH, 0);
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
    glVertex2f(0, WINDOW_HEIGHT);
    glEnd();

    // Title
    glColor3f(1.0f, 0.9f, 0.3f);  // Gold
    DrawString(350, 100, "FREE FALCON", 4.0f);

    glColor3f(0.7f, 0.7f, 0.8f);  // Silver
    DrawString(380, 160, "LINUX PORT", 3.0f);

    // Instructions
    glColor3f(0.6f, 0.8f, 0.6f);  // Light green
    DrawString(300, 300, "CLICK A BUTTON BELOW", 2.0f);
    DrawString(350, 340, "OR PRESS ESC TO EXIT", 2.0f);

    // Draw buttons
    for (int i = 0; i < g_numMenuButtons; i++) {
        MenuButton& btn = g_menuButtons[i];

        // Button background
        if (i == g_hoveredButton) {
            glColor3f(0.3f, 0.5f, 0.7f);  // Highlighted
        } else {
            glColor3f(0.2f, 0.25f, 0.3f);  // Normal
        }
        glBegin(GL_QUADS);
        glVertex2f(btn.x, btn.y);
        glVertex2f(btn.x + btn.w, btn.y);
        glVertex2f(btn.x + btn.w, btn.y + btn.h);
        glVertex2f(btn.x, btn.y + btn.h);
        glEnd();

        // Button border
        if (i == g_hoveredButton) {
            glColor3f(1.0f, 1.0f, 0.5f);  // Yellow highlight
        } else {
            glColor3f(0.5f, 0.5f, 0.6f);
        }
        glBegin(GL_LINE_LOOP);
        glVertex2f(btn.x, btn.y);
        glVertex2f(btn.x + btn.w, btn.y);
        glVertex2f(btn.x + btn.w, btn.y + btn.h);
        glVertex2f(btn.x, btn.y + btn.h);
        glEnd();

        // Button text
        glColor3f(1.0f, 1.0f, 1.0f);
        float textX = btn.x + 5;
        float textY = btn.y + 8;
        DrawString(textX, textY, btn.label, 1.5f);
    }

    // Draw status message if present (fades out after 3 seconds)
    if (g_menuStatusMessage) {
        Uint32 elapsed = SDL_GetTicks() - g_menuStatusTime;
        if (elapsed < 3000) {
            // Fade out effect
            float alpha = 1.0f - (elapsed / 3000.0f);
            glColor3f(1.0f * alpha, 0.5f * alpha, 0.5f * alpha);  // Red-ish
            DrawString(300, 400, g_menuStatusMessage, 2.0f);
        } else {
            g_menuStatusMessage = nullptr;
        }
    }

    // Restore matrices
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

static int GetButtonAtPosition(int x, int y) {
    for (int i = 0; i < g_numMenuButtons; i++) {
        MenuButton& btn = g_menuButtons[i];
        if (x >= btn.x && x <= btn.x + btn.w &&
            y >= btn.y && y <= btn.y + btn.h) {
            return i;
        }
    }
    return -1;
}

static void HandleFallbackMenuClick(int x, int y) {
    int btn = GetButtonAtPosition(x, y);
    if (btn >= 0 && g_menuButtons[btn].callback) {
        fprintf(stderr, "[FallbackMenu] Button clicked: %s\n", g_menuButtons[btn].label);
        fflush(stderr);
        g_menuButtons[btn].callback();
        fprintf(stderr, "[FallbackMenu] Callback returned, g_running=%d\n", g_running ? 1 : 0);
        fflush(stderr);
    }
}

static void HandleFallbackMenuHover(int x, int y) {
    g_hoveredButton = GetButtonAtPosition(x, y);
}

// Helper to show a status message on the fallback menu
static void ShowMenuStatus(const char* message) {
    g_menuStatusMessage = message;
    g_menuStatusTime = SDL_GetTicks();
    fprintf(stderr, "[FallbackMenu] Status: %s\n", message);
    fflush(stderr);
}

// Fallback menu callbacks
static void FallbackExit() {
    fprintf(stderr, "[FallbackMenu] EXIT selected - shutting down\n");
    fflush(stderr);
    g_running = false;
}

static void FallbackDogfight() {
    ShowMenuStatus("DOGFIGHT - Not yet implemented");
}

static void FallbackCampaign() {
    ShowMenuStatus("CAMPAIGN - Not yet implemented");
}

static void FallbackSetup() {
    ShowMenuStatus("SETUP - Not yet implemented");
}

static void FallbackComms() {
    ShowMenuStatus("COMMS - Not yet implemented");
}

static void FallbackACMI() {
    ShowMenuStatus("ACMI - Not yet implemented");
}

static void FallbackLogbook() {
    ShowMenuStatus("LOGBOOK - Not yet implemented");
}

static void FallbackInstantAction() {
    ShowMenuStatus("INSTANT ACTION - Not yet implemented");
}

// =============================================================================
// END FALLBACK MENU SYSTEM
// =============================================================================

// Signal handler for clean shutdown when process is killed
static volatile sig_atomic_t g_signalReceived = 0;

static void signal_handler(int sig) {
    g_signalReceived = sig;
    g_running = false;

    // FF_LINUX: do NOT tear down SDL/GL from here.
    //
    // This handler runs on the main thread while the SIM thread may be mid-draw with
    // g_GLContext. Deleting the context under it segfaults inside the driver --
    // captured under gdb on a SIGINT during a TE 2 flight:
    //
    //   SimulationLoopControl::Loop -> OTWDriverClass::RenderFrame
    //     -> ContextMPR::FlushPolyLists -> RenderPolyList
    //       -> D3D7Dev_DrawPrimitiveVB -> D3D7Device::DrawVertices
    //         -> libnvidia-glcore  <-- SIGSEGV
    //
    // and SDL_Quit() can deadlock against the still-running campaign/sim threads,
    // which is how a process ended up surviving both SIGINT and SIGTERM for 9+
    // minutes with 19 threads parked in futex_do_wait. None of
    // SDL_GL_DeleteContext / SDL_DestroyWindow / SDL_Quit is async-signal-safe in
    // the first place.
    //
    // A signal means "die now". _exit IS async-signal-safe, and the kernel reclaims
    // the window, the GL context and every thread without racing any of them. This
    // is what main() already does on the normal path (_exit(0) after "Goodbye!"),
    // so the orderly Exit-button shutdown is unaffected -- it never comes here.
    _exit(128 + sig);
}

// FF_LINUX: SIGSEGV/SIGFPE handler with backtrace for crash diagnosis
static void crash_signal_handler(int sig) {
    const char *signame = (sig == SIGSEGV) ? "SIGSEGV" : (sig == SIGFPE) ? "SIGFPE" : (sig == SIGABRT) ? "SIGABRT" : "UNKNOWN";
    fprintf(stderr, "\n=== CRASH: %s (signal %d) ===\n", signame, sig);

    void *frames[64];
    int nframes = backtrace(frames, 64);
    backtrace_symbols_fd(frames, nframes, STDERR_FILENO);

    fprintf(stderr, "=== END BACKTRACE ===\n");
    fflush(stderr);

    // Re-raise with default handler
    signal(sig, SIG_DFL);
    raise(sig);
}

static void setup_signal_handlers() {
    // FF_LINUX: prime backtrace() now - its FIRST call lazily dlopens
    // libgcc (which allocates). If the first call happens inside the crash
    // handler while the crashing thread holds the malloc lock, the handler
    // deadlocks after printing the "=== CRASH ===" header with no frames
    // (seen on the TE mission-select crash).
    {
        void *primeFrames[4];
        backtrace(primeFrames, 4);
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGTERM, &sa, nullptr);  // Terminate (default from kill)
    sigaction(SIGINT, &sa, nullptr);   // Interrupt (Ctrl+C)
    sigaction(SIGHUP, &sa, nullptr);   // Hangup

    // Crash signal handlers with backtrace
    struct sigaction crash_sa;
    memset(&crash_sa, 0, sizeof(crash_sa));
    crash_sa.sa_handler = crash_signal_handler;
    sigemptyset(&crash_sa.sa_mask);
    crash_sa.sa_flags = SA_RESETHAND;  // One-shot to avoid infinite loops

    sigaction(SIGSEGV, &crash_sa, nullptr);
    sigaction(SIGFPE, &crash_sa, nullptr);
    sigaction(SIGABRT, &crash_sa, nullptr);
}

// Forward declarations
static void print_usage(const char* progname);
static bool init_data_directory(const char* dataDir);
static bool init_resource_manager(void);
static bool init_sdl(bool fullscreen);
static bool init_opengl(void);
static bool init_openal(void);
static bool init_d3d_graphics(void);
static void cleanup(void);
static void handle_sdl_events(void);
static void render_frame(void);
static void main_loop(void);

// Game initialization stages
static bool init_game_paths(void);
static bool init_game_core(void);

static void print_usage(const char* progname) {
    printf("Usage: %s [options]\n", progname);
    printf("Options:\n");
    printf("  -d <path>    Set game data directory\n");
    printf("  -w           Run in windowed mode (default)\n");
    printf("  -f           Run in fullscreen mode\n");
    printf("  -nosound     Disable sound\n");
    printf("  -test-ia     Auto-launch Instant Action after 3 seconds (for testing)\n");
    printf("  -h           Show this help\n");
}

static bool init_data_directory(const char* dataDir) {
    // Check if directory exists
    if (access(dataDir, R_OK) != 0) {
        fprintf(stderr, "Error: Cannot access data directory: %s\n", dataDir);
        return false;
    }

    // Set the global data directory
    strncpy(FalconDataDirectory, dataDir, _MAX_PATH - 1);
    FalconDataDirectory[_MAX_PATH - 1] = '\0';

    printf("Data directory: %s\n", FalconDataDirectory);

    return true;
}

static bool init_game_paths(void) {
    printf("Setting up game paths...\n");

    // Set up derived paths (using forward slashes for Linux)
    // Note: FalconTerrainDataDir needs to include the theater name (default: "korea")
    // because DeviceIndependentGraphicsSetup is called before theater loading
    snprintf(FalconCampaignSaveDirectory, _MAX_PATH, "%s/campaign/save", FalconDataDirectory);
    snprintf(FalconCampUserSaveDirectory, _MAX_PATH, "%s/campaign/save", FalconDataDirectory);

    // FF_LINUX (THEATERS-1): this is a FIXED, non-theater path. Whether it runs
    // before or after SetNewTheater decides whether it is harmless or a clobber,
    // and the log cannot answer that (printf below is buffered, stderr is not).
    if (getenv("FF_DEBUG_PATHS"))
    {
        fprintf(stderr, "[PATHS] init_game_paths -> CampUserSave='%s'\n",
                FalconCampUserSaveDirectory);
        fflush(stderr);
    }

    snprintf(FalconTerrainDataDir, _MAX_PATH, "%s/terrdata/korea", FalconDataDirectory);  // Include default theater
    snprintf(FalconMiscTexDataDir, _MAX_PATH, "%s/terrdata/misctex", FalconDataDirectory);
    snprintf(FalconPictureDirectory, _MAX_PATH, "%s/pictures", FalconDataDirectory);
    snprintf(FalconObjectDataDir, _MAX_PATH, "%s/terrdata/objects", FalconDataDirectory);
    snprintf(Falcon3DDataDir, _MAX_PATH, "%s/terrdata/objects", FalconDataDirectory);

    // Sound-related directories (using forward slashes for Linux)
    snprintf(FalconSoundThrDirectory, _MAX_PATH, "%s/sounds", FalconDataDirectory);
    snprintf(FalconUISoundDirectory, _MAX_PATH, "%s/sounds", FalconDataDirectory);
    snprintf(FalconCockpitThrDirectory, _MAX_PATH, "%s/art/ckptart", FalconDataDirectory);
    snprintf(FalconZipsThrDirectory, _MAX_PATH, "%s/zips", FalconDataDirectory);
    snprintf(FalconTacrefThrDirectory, _MAX_PATH, "%s/tacref", FalconDataDirectory);
    snprintf(FalconSplashThrDirectory, _MAX_PATH, "%s/art/splash", FalconDataDirectory);
    snprintf(FalconMovieDirectory, _MAX_PATH, "%s/movies", FalconDataDirectory);
    snprintf(FalconMovieMode, _MAX_PATH, "normals");  // Default movie mode
    snprintf(FalconUIArtDirectory, _MAX_PATH, "%s/art", FalconDataDirectory);
    snprintf(FalconUIArtThrDirectory, _MAX_PATH, "%s/art", FalconDataDirectory);

    // Create picture directory if it doesn't exist
    mkdir(FalconPictureDirectory, 0755);

    printf("  Campaign saves: %s\n", FalconCampaignSaveDirectory);
    printf("  Terrain data: %s\n", FalconTerrainDataDir);
    printf("  Object data: %s\n", FalconObjectDataDir);
    printf("  Sound directory: %s\n", FalconSoundThrDirectory);

    return true;
}

static bool init_resource_manager(void) {
    printf("Initializing resource manager...\n");

    // Initialize the resource manager with the data directory
    ResInit(FalconDataDirectory);
    ResCreatePath(FalconDataDirectory, FALSE);

    // Add common paths
    char tmpPath[_MAX_PATH];

    snprintf(tmpPath, _MAX_PATH, "%s/campaign/save", FalconDataDirectory);
    ResAddPath(tmpPath, FALSE);

    snprintf(tmpPath, _MAX_PATH, "%s/config", FalconDataDirectory);
    ResAddPath(tmpPath, FALSE);

    snprintf(tmpPath, _MAX_PATH, "%s/art", FalconDataDirectory);
    ResAddPath(tmpPath, TRUE);

    snprintf(tmpPath, _MAX_PATH, "%s/sim", FalconDataDirectory);
    ResAddPath(tmpPath, TRUE);

    snprintf(tmpPath, _MAX_PATH, "%s/pictures", FalconDataDirectory);
    ResAddPath(tmpPath, TRUE);

    ResSetDirectory(FalconDataDirectory);

    return true;
}

static bool init_sdl(bool fullscreen) {
    printf("Initializing SDL2...\n");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    fprintf(stderr, "[TRACE] SDL_Init completed, setting GL attributes\n"); fflush(stderr);

    // Set OpenGL attributes - minimal requirements, let driver pick best match
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);  // FF_LINUX: Required for 3D cockpit stencil masking
    // Don't specify version - let driver use default compatibility context
    // Don't specify color/depth sizes - let driver pick available format

    fprintf(stderr, "[TRACE] About to create window\n"); fflush(stderr);

    // Create window - simple fixed-size window for stability
    Uint32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
    if (fullscreen) {
        windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    // Note: Not using RESIZABLE for now to avoid potential issues

    g_SDLWindow = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        windowFlags
    );

    if (!g_SDLWindow) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

    // Set mainAppWnd to a pseudo-handle (the SDL window pointer cast)
    mainAppWnd = (HWND)g_SDLWindow;
    mainMenuWnd = mainAppWnd;

    printf("  Window created: %dx%d\n", WINDOW_WIDTH, WINDOW_HEIGHT);

    // Initialize keyboard scancode translation table
    InitScancodeTranslation();
    printf("  Keyboard scancode translation initialized.\n");

    // Initialize joystick support
    InitSDLJoystick();

    return true;
}

static bool init_opengl(void) {
    printf("Initializing OpenGL...\n");
    fprintf(stderr, "[TRACE] About to call SDL_GL_CreateContext\n"); fflush(stderr);

    // Create OpenGL context
    g_GLContext = SDL_GL_CreateContext(g_SDLWindow);
    if (!g_GLContext) {
        fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        return false;
    }

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK) {
        fprintf(stderr, "glewInit failed: %s\n", glewGetErrorString(glewErr));
        return false;
    }

    // Enable vsync
    SDL_GL_SetSwapInterval(1);

    // Print OpenGL info
    printf("  OpenGL Vendor: %s\n", glGetString(GL_VENDOR));
    printf("  OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
    printf("  OpenGL Version: %s\n", glGetString(GL_VERSION));
    {
        GLint stencilBits = 0;
        glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
        printf("  OpenGL Stencil Bits: %d\n", stencilBits);
    }

    // Set up basic OpenGL state - minimal for test pattern
    glClearColor(0.2f, 0.2f, 0.3f, 1.0f);  // Neutral gray-blue
    glDisable(GL_DEPTH_TEST);   // Not needed for 2D test pattern
    glDisable(GL_LIGHTING);     // No lighting
    glDisable(GL_TEXTURE_2D);   // No textures
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Clear any OpenGL errors from init
    while (glGetError() != GL_NO_ERROR) {}

    return true;
}

static bool init_d3d_graphics(void) {
    printf("Initializing D3D7-to-OpenGL graphics layer...\n");

    // Get window dimensions
    int width, height;
    SDL_GetWindowSize(g_SDLWindow, &width, &height);

    // Create DirectDraw7 interface first (needed for surface creation)
    g_pDD = FF_CreateDirectDraw7();
    if (!g_pDD) {
        fprintf(stderr, "Failed to create DirectDraw7 interface\n");
        return false;
    }
    printf("  Created DirectDraw7 interface\n");

    // Create D3D7 interface
    g_pD3D = FF_CreateDirect3D7();
    if (!g_pD3D) {
        fprintf(stderr, "Failed to create Direct3D7 interface\n");
        return false;
    }
    printf("  Created Direct3D7 interface\n");

    // Create render target surface
    g_pRenderTarget = FF_CreateRenderTargetSurface(width, height);
    if (!g_pRenderTarget) {
        fprintf(stderr, "Failed to create render target surface\n");
        return false;
    }
    printf("  Created render target surface: %dx%d\n", width, height);

    // Create D3D device
    g_pD3DDevice = FF_CreateDirect3DDevice7(g_pD3D, g_pRenderTarget);
    if (!g_pD3DDevice) {
        fprintf(stderr, "Failed to create Direct3DDevice7\n");
        return false;
    }
    printf("  Created Direct3DDevice7\n");

    // Set up initial viewport
    D3DVIEWPORT7 vp;
    memset(&vp, 0, sizeof(vp));
    vp.dwX = 0;
    vp.dwY = 0;
    vp.dwWidth = width;
    vp.dwHeight = height;
    vp.dvMinZ = 0.0f;
    vp.dvMaxZ = 1.0f;
    g_pD3DDevice->SetViewport(&vp);

    // Create DXContext with our OpenGL-backed interfaces
    g_pDXContext = FF_CreateDXContext(width, height, g_pD3D, g_pD3DDevice, g_pDD);
    if (!g_pDXContext) {
        fprintf(stderr, "Failed to create DXContext\n");
        return false;
    }
    printf("  Created DXContext\n");

    // Initialize the texture system with our DXContext
    // Get texture path from data directory
    char texturePath[256];
    snprintf(texturePath, sizeof(texturePath), "%s/terrdata/objects", FalconDataDirectory);
    printf("  Initializing texture system (path: %s)...\n", texturePath);
    Texture::SetupForDevice(g_pDXContext, texturePath);
    printf("  Texture system initialized\n");

    // Initialize the DXEngine with our OpenGL-backed D3D device
    printf("  Initializing DXEngine...\n");
    TheDXEngine.Setup(g_pD3DDevice, g_pD3D, g_pDD);
    // Don't enable DXEngine rendering yet - game needs to fully load first
    // g_Use_DX_Engine will be set to true when the game enters simulation mode
    g_Use_DX_Engine = false;
    printf("  NOTE: DXEngine initialized but rendering disabled until game enters simulation mode\n");

    g_graphicsInitialized = true;
    printf("  D3D7-to-OpenGL graphics layer initialized successfully\n");

    return true;
}

static bool init_openal(void) {
    printf("Initializing OpenAL...\n");

    g_alDevice = alcOpenDevice(nullptr);
    if (!g_alDevice) {
        fprintf(stderr, "Warning: Could not open OpenAL device\n");
        return false;
    }

    g_alContext = alcCreateContext(g_alDevice, nullptr);
    if (!g_alContext) {
        fprintf(stderr, "Warning: Could not create OpenAL context\n");
        alcCloseDevice(g_alDevice);
        g_alDevice = nullptr;
        return false;
    }

    alcMakeContextCurrent(g_alContext);

    printf("  OpenAL Vendor: %s\n", alGetString(AL_VENDOR));
    printf("  OpenAL Renderer: %s\n", alGetString(AL_RENDERER));
    printf("  OpenAL Version: %s\n", alGetString(AL_VERSION));

    return true;
}

// Initialize core game systems
static bool init_game_core(void) {
    printf("\n--- Initializing Game Core Systems ---\n");

    // Initialize communications / Winsock emulation
    // This sets up CAPI function pointers needed by VU network code
    printf("  Initializing communications layer...\n");
    WSADATA wsaData;
    int wsaResult = initialize_windows_sockets(&wsaData);
    if (wsaResult == 0) {  // EXIT_SUCCESS = 0 means failure in this API
        fprintf(stderr, "Warning: initialize_windows_sockets returned failure code\n");
    } else {
        printf("  Communications layer initialized successfully\n");
    }

    // Set FPU rounding mode to truncate (equivalent to Windows _controlfp)
    fesetround(FE_TOWARDZERO);

    // Seed random number generator
    srand((unsigned int)time(NULL));

    // Set up FalconDisplay for Linux
    // Set the SDL window handle as the app window
    printf("  Setting up FalconDisplay for Linux...\n");
    FalconDisplay.appWin = (HWND)g_SDLWindow;
    FalconDisplay.displayFullScreen = false;  // Windowed mode for now
    int width, height;
    SDL_GetWindowSize(g_SDLWindow, &width, &height);
    FalconDisplay.width[FalconDisplayConfiguration::UI] = width;
    FalconDisplay.height[FalconDisplayConfiguration::UI] = height;
    FalconDisplay.depth[FalconDisplayConfiguration::UI] = 16;  // Use 16-bit for UI mode (matches FreeFalcon convention)
    FalconDisplay.currentMode = FalconDisplayConfiguration::UI;

    // FF_LINUX: load the saved display options. winmain.cpp does this on Windows
    // (immediately before FalconDisplay.Setup, which is the order kept here), but
    // that file is not the entry point on this port, so nothing read display.dsp
    // at all -- the resolution chosen in Setup was written to disk and then never
    // loaded again.
    printf("  Loading display options...\n");
    DisplayOptions.LoadOptions("display");

    // Initialize the DeviceManager (enumerates display modes and D3D devices)
    printf("  Initializing DeviceManager...\n");
    FalconDisplay.Setup(0);  // Language number 0 for English

    // Initialize memory pools
    printf("  Initializing simulation memory pools...\n");
    SimDriver.InitializeSimMemoryPools();

    // FF_LINUX: Create the global A* pathfinder (winmain's SystemLevelInit does
    // this on Windows). Without it the campaign thread dereferences a NULL/garbage
    // ASD as soon as a ground unit needs a path and segfaults.
    {
        extern AS_DataClass *ASD;
        if (!ASD)
            ASD = new AS_DataClass();
    }
    srand((unsigned int) time(NULL));

    // Load particle system parameters
    printf("  Loading particle system parameters...\n");
    DrawableParticleSys::LoadParameters();

    // Load theater list
    printf("  Loading theater list...\n");
    LoadTheaterList();

    // Try to get a theater - either from saved preference or first available
    TheaterDef *td = g_theaters.GetCurrentTheater();
    if (!td) {
        // No saved theater preference, try to find any available theater
        printf("  No saved theater, looking for first available...\n");
        td = g_theaters.GetTheater(0);  // Get first theater in list
    }

    if (td) {
        fprintf(stderr, "  Setting theater: %s\n", td->m_name);
        g_theaters.SetNewTheater(td);
        fprintf(stderr, "  [main_linux] SetNewTheater returned, calling InitVU...\n");

        // Initialize VU (Virtual Universe - entity management)
        fprintf(stderr, "  Initializing VU system...\n");
        InitVU();
        fprintf(stderr, "  [main_linux] InitVU returned\n");
    } else {
        printf("  No theater found, loading default data...\n");

        // Add sim path for raw sim files
        char tmpPath[_MAX_PATH];
        snprintf(tmpPath, _MAX_PATH, "%s/sim", FalconDataDirectory);
        ResAddPath(tmpPath, TRUE);

        // Load AI inputs
        printf("  Loading campaign AI inputs...\n");
        ReadCampAIInputs((char*)"Falcon4");

        // Load class table (entity definitions)
        printf("  Loading class table...\n");
        if (!LoadClassTable((char*)"Falcon4")) {
            fprintf(stderr, "Error: Failed to load class table\n");
            return false;
        }

        // Initialize VU
        printf("  Initializing VU system...\n");
        InitVU();

        // Load tactics
        printf("  Loading tactics...\n");
        if (!LoadTactics((char*)"Falcon4")) {
            fprintf(stderr, "Warning: Failed to load tactics\n");
        }

        // Load flight trails
        printf("  Loading trails...\n");
        LoadTrails();
    }

    // Initialize threading
    fprintf(stderr, "  Setting up thread manager...\n");
    ThreadManager::setup();
    fprintf(stderr, "  [main_linux] ThreadManager::setup() returned\n");

    // NOTE: TheTimeManager.Setup() is already called by DeviceIndependentGraphicsSetup()
    // in FalconDisplay.Setup(), so we don't need to call it again here.

    // Initialize sound system (creates gSoundDriver and calls InstallDSound)
    fprintf(stderr, "  Initializing sound manager...\n");
    if (InitSoundManager(FalconDisplay.appWin, 0, FalconDataDirectory)) {
        fprintf(stderr, "  [main_linux] InitSoundManager() succeeded\n");
    } else {
        fprintf(stderr, "  [main_linux] InitSoundManager() failed - sounds disabled\n");
    }

    // Start sound system (resumes playback if initialized)
    fprintf(stderr, "  Starting sound system...\n");
    F4SoundStart();
    fprintf(stderr, "  [main_linux] F4SoundStart() returned\n");

    // Start simulation loop
    fprintf(stderr, "  Starting simulation loop...\n");
    SimulationLoopControl::StartSim();
    fprintf(stderr, "  [main_linux] SimulationLoopControl::StartSim() returned\n");

    // FF_LINUX: Initialize weather system (required before campaign loading)
    fprintf(stderr, "  Initializing weather system...\n");
    realWeather = new WeatherClass();
    fprintf(stderr, "  [main_linux] realWeather created\n");

    // Initialize campaign
    fprintf(stderr, "  Initializing campaign system...\n");
    Camp_Init(1);
    fprintf(stderr, "  [main_linux] Camp_Init() returned\n");

    // Build ASCII key mappings
    fprintf(stderr, "  Building key mappings...\n");
    BuildAscii();
    fprintf(stderr, "  [main_linux] BuildAscii() returned\n");

    // FF_LINUX: Initialize comms manager (required for campaign loading)
    fprintf(stderr, "  Initializing comms manager...\n");
    gCommsMgr = new UIComms;
    gCommsMgr->Setup(FalconDisplay.appWin);
    fprintf(stderr, "  [main_linux] gCommsMgr->Setup() returned\n");

    // FF_MP_CONNECT drives the phonebook connect path headlessly (see phonebk.cpp).
    {
        extern void FF_MpAutoConnect();
        FF_MpAutoConnect();
    }

    // FF_LINUX: load the player's logbook (winmain.cpp does this at the same
    // point on Windows). Skipping it left LB_LOADED_ONCE clear, and UI_Startup()
    // reads that as "first ever run" and resets the logbook, the player options
    // AND the display options to defaults -- on every single launch.
    fprintf(stderr, "  Loading logbook...\n");

    if (UI_logbk.Load())
    {
        LogBook.LoadData(&UI_logbk.Pilot);
        fprintf(stderr, "  [main_linux] logbook loaded for '%s'\n", LogBook.Callsign());
    }
    else
    {
        fprintf(stderr, "  [main_linux] no logbook found; defaults will be used\n");
    }

    // FF_LINUX: Load keyboard command bindings from keystrokes.key
    // This populates UserFunctionTable so keyboard commands work in the sim.
    // Must be called after SetupInputFunctions() (called by SimInputInit via OTWDriver.Enter),
    // but we call it here during init so bindings are ready before first flight.
    fprintf(stderr, "  Loading keyboard function tables...\n");
    LoadFunctionTables();
    fprintf(stderr, "  [main_linux] LoadFunctionTables() returned\n");

    fprintf(stderr, "  Game core initialization complete.\n");
    g_gameInitialized = true;

    return true;
}

static void cleanup(void) {
    FF_DEBUG_CLEANUP("Starting cleanup...\n");
    FF_DEBUG_FLUSH();

    // First, destroy the window immediately so it disappears
    // This provides visual feedback that the app is shutting down
    FF_DEBUG_CLEANUP("Destroying SDL window first for immediate visual feedback...\n");
    FF_DEBUG_FLUSH();

    if (g_GLContext) {
        SDL_GL_DeleteContext(g_GLContext);
        g_GLContext = nullptr;
    }
    if (g_SDLWindow) {
        SDL_DestroyWindow(g_SDLWindow);
        g_SDLWindow = nullptr;
    }
    FF_DEBUG_CLEANUP("Window destroyed.\n");
    FF_DEBUG_FLUSH();

    // Cleanup game systems (in reverse order of initialization)
    // These may block, but at least the window is gone
    if (g_gameInitialized) {
        FF_DEBUG_CLEANUP("Shutting down game systems...\n");
        FF_DEBUG_FLUSH();

        // Stop campaign - this can block on threading issues
        FF_DEBUG_CLEANUP("Stopping campaign...\n");
        FF_DEBUG_FLUSH();
        Camp_Exit();
        FF_DEBUG_CLEANUP("Campaign stopped.\n");
        FF_DEBUG_FLUSH();

        // Only stop simulation loop if we were actually in simulation mode
        // StopSim() has a blocking wait for RunningSim state that will hang
        // if we're just in UI mode (which uses the fallback menu)
        if (SimulationLoopControl::InSim()) {
            FF_DEBUG_CLEANUP("Stopping simulation loop...\n");
            FF_DEBUG_FLUSH();
            SimulationLoopControl::StopSim();
            FF_DEBUG_CLEANUP("Simulation loop stopped.\n");
            FF_DEBUG_FLUSH();
        } else {
            FF_DEBUG_CLEANUP("Skipping StopSim (not in simulation mode)\n");
            FF_DEBUG_FLUSH();
        }

        // Cleanup particle system
        FF_DEBUG_CLEANUP("Unloading particle system...\n");
        FF_DEBUG_FLUSH();
        DrawableParticleSys::UnloadParameters();
        FF_DEBUG_CLEANUP("Particle system unloaded.\n");
        FF_DEBUG_FLUSH();
    }

    // Cleanup D3D/DXEngine
    if (g_graphicsInitialized) {
        FF_DEBUG_CLEANUP("Releasing DXEngine...\n");
        FF_DEBUG_FLUSH();
        TheDXEngine.Release();
        g_graphicsInitialized = false;
        FF_DEBUG_CLEANUP("DXEngine released.\n");
        FF_DEBUG_FLUSH();
    }

    // Release D3D interfaces
    FF_DEBUG_CLEANUP("Releasing D3D interfaces...\n");
    FF_DEBUG_FLUSH();
    if (g_pD3DDevice) {
        g_pD3DDevice->Release();
        g_pD3DDevice = nullptr;
    }
    if (g_pRenderTarget) {
        g_pRenderTarget->Release();
        g_pRenderTarget = nullptr;
    }
    if (g_pD3D) {
        g_pD3D->Release();
        g_pD3D = nullptr;
    }
    FF_DEBUG_CLEANUP("D3D interfaces released.\n");
    FF_DEBUG_FLUSH();

    // Cleanup audio
    FF_DEBUG_CLEANUP("Cleaning up audio...\n");
    FF_DEBUG_FLUSH();
    if (g_alContext) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(g_alContext);
        g_alContext = nullptr;
    }
    if (g_alDevice) {
        alcCloseDevice(g_alDevice);
        g_alDevice = nullptr;
    }
    FF_DEBUG_CLEANUP("Audio cleaned up.\n");
    FF_DEBUG_FLUSH();

    // Cleanup SDL joystick
    FF_DEBUG_CLEANUP("Closing SDL joystick...\n");
    FF_DEBUG_FLUSH();
    CleanupSDLJoystick();
    FF_DEBUG_CLEANUP("SDL joystick closed.\n");
    FF_DEBUG_FLUSH();

    // Cleanup SDL
    FF_DEBUG_CLEANUP("Quitting SDL...\n");
    FF_DEBUG_FLUSH();
    SDL_Quit();
    FF_DEBUG_CLEANUP("SDL quit.\n");
    FF_DEBUG_FLUSH();

    // Cleanup resource manager
    FF_DEBUG_CLEANUP("Cleaning up resource manager...\n");
    FF_DEBUG_FLUSH();
    ResExit();
    FF_DEBUG_CLEANUP("Resource manager cleaned up.\n");
    FF_DEBUG_FLUSH();

    FF_DEBUG_CLEANUP("Cleanup complete!\n");
    FF_DEBUG_FLUSH();
}

// Convert SDL events to Windows-style messages
static void handle_sdl_events(void) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                g_running = false;
                PostGameMessage(WM_QUIT, 0, 0);
                break;

            case SDL_KEYDOWN:
                // FF_LINUX: ESC is passed through to the UI/sim below as
                // DIK_ESCAPE (UI uses it to back out of a screen / open the exit
                // menu). Do NOT quit the whole app on ESC - that surprised the
                // user by closing the window from a menu. Quit is via the Exit
                // button or the window close box (SDL_QUIT).
                if (event.key.keysym.sym == SDLK_F5 && doUI && !g_autoTestInstantAction) {
                    // F5: Quick-launch Instant Action (bypasses UI clicking)
                    fprintf(stderr, "[F5] Launching Instant Action...\n");
                    g_autoTestInstantAction = true;
                    strcpy(gUI_CampaignFile, "Instant");
                    PostGameMessage(FM_LOAD_CAMPAIGN, 0, game_InstantAction);
                }
                if (event.key.keysym.sym == SDLK_F11) {
                    // Toggle fullscreen
                    Uint32 flags = SDL_GetWindowFlags(g_SDLWindow);
                    if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
                        SDL_SetWindowFullscreen(g_SDLWindow, 0);
                    } else {
                        SDL_SetWindowFullscreen(g_SDLWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
                    }
                }
                // Shift+Numpad: cockpit panel switching (sim mode only)
                if (!doUI && (event.key.keysym.mod & KMOD_SHIFT)) {
                    switch (event.key.keysym.sym) {
                        case SDLK_KP_8: g_requestedPanel = 1100; break; // Front
                        case SDLK_KP_4: g_requestedPanel = 600;  break; // Left
                        case SDLK_KP_6: g_requestedPanel = 700;  break; // Right
                        case SDLK_KP_2: g_requestedPanel = 100;  break; // Down
                        default: break;
                    }
                }
                // Post keyboard message with DIK scancode translation
                {
                    int dikCode = ConvertSDLToDIK(event.key.keysym.scancode);
                    if (dikCode != 0) {
                        PostGameMessage(WM_KEYDOWN, dikCode, 0);
                        FF_PushKeyEvent(dikCode, true);
                    }
                }
                break;

            case SDL_KEYUP:
                {
                    int dikCode = ConvertSDLToDIK(event.key.keysym.scancode);
                    if (dikCode != 0) {
                        PostGameMessage(WM_KEYUP, dikCode, 0);
                        FF_PushKeyEvent(dikCode, false);
                    }
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                {
                    int x = event.button.x;
                    int y = event.button.y;
                    // Handle fallback menu clicks
                    if (g_useFallbackMenu && doUI && event.button.button == SDL_BUTTON_LEFT) {
                        HandleFallbackMenuClick(x, y);
                        if (!g_running) {
                            return;
                        }
                    } else if (!doUI) {
                        // FF_LINUX: In sim mode, push to mouse event buffer for OnSimMouseInput
                        DWORD ofs = 0;
                        if (event.button.button == SDL_BUTTON_LEFT)   ofs = DIMOFS_BUTTON0;
                        else if (event.button.button == SDL_BUTTON_RIGHT)  ofs = DIMOFS_BUTTON1;
                        else if (event.button.button == SDL_BUTTON_MIDDLE) ofs = DIMOFS_BUTTON2;
                        else if (event.button.button == SDL_BUTTON_X1)     ofs = DIMOFS_BUTTON3;
                        if (ofs) FF_PushMouseEvent(ofs, 0x80);  // 0x80 = pressed
                    } else {
                        int scaledX = x * 1024 / WINDOW_WIDTH;
                        int scaledY = y * 768 / WINDOW_HEIGHT;
                        // FF_LINUX: Windows synthesizes WM_LBUTTONDBLCLK for the
                        // second click of a double-click; the UI95 toolkit binds
                        // actions to it (joining a dogfight team, list items...).
                        // SDL reports the click count, so do the same here.
                        if (event.button.button == SDL_BUTTON_LEFT) {
                            if (event.button.clicks >= 2)
                                PostGameMessage(WM_LBUTTONDBLCLK, 0, MAKELPARAM(scaledX, scaledY));
                            else
                                PostGameMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(scaledX, scaledY));
                        } else if (event.button.button == SDL_BUTTON_RIGHT) {
                            if (event.button.clicks >= 2)
                                PostGameMessage(WM_RBUTTONDBLCLK, 0, MAKELPARAM(scaledX, scaledY));
                            else
                                PostGameMessage(WM_RBUTTONDOWN, 0, MAKELPARAM(scaledX, scaledY));
                        }
                    }
                }
                break;

            case SDL_MOUSEBUTTONUP:
                {
                    int x = event.button.x;
                    int y = event.button.y;
                    if (!doUI) {
                        // FF_LINUX: In sim mode, push to mouse event buffer for OnSimMouseInput
                        DWORD ofs = 0;
                        if (event.button.button == SDL_BUTTON_LEFT)   ofs = DIMOFS_BUTTON0;
                        else if (event.button.button == SDL_BUTTON_RIGHT)  ofs = DIMOFS_BUTTON1;
                        else if (event.button.button == SDL_BUTTON_MIDDLE) ofs = DIMOFS_BUTTON2;
                        else if (event.button.button == SDL_BUTTON_X1)     ofs = DIMOFS_BUTTON3;
                        if (ofs) FF_PushMouseEvent(ofs, 0x00);  // 0x00 = released
                    } else if (!g_useFallbackMenu) {
                        int scaledX = x * 1024 / WINDOW_WIDTH;
                        int scaledY = y * 768 / WINDOW_HEIGHT;
                        if (event.button.button == SDL_BUTTON_LEFT) {
                            PostGameMessage(WM_LBUTTONUP, 0, MAKELPARAM(scaledX, scaledY));
                        } else if (event.button.button == SDL_BUTTON_RIGHT) {
                            PostGameMessage(WM_RBUTTONUP, 0, MAKELPARAM(scaledX, scaledY));
                        }
                    }
                }
                break;

            case SDL_MOUSEMOTION:
                {
                    int x = event.motion.x;
                    int y = event.motion.y;
                    if (!doUI) {
                        // FF_LINUX: In sim mode, push relative motion to mouse event buffer
                        if (event.motion.xrel != 0)
                            FF_PushMouseEvent(DIMOFS_X, (DWORD)(int)event.motion.xrel);
                        if (event.motion.yrel != 0)
                            FF_PushMouseEvent(DIMOFS_Y, (DWORD)(int)event.motion.yrel);
                    } else if (g_useFallbackMenu) {
                        HandleFallbackMenuHover(x, y);
                    } else {
                        int scaledX = x * 1024 / WINDOW_WIDTH;
                        int scaledY = y * 768 / WINDOW_HEIGHT;
                        PostGameMessage(WM_MOUSEMOVE, 0, MAKELPARAM(scaledX, scaledY));
                    }
                }
                break;

            case SDL_MOUSEWHEEL:
                {
                    if (!doUI) {
                        // FF_LINUX: In sim mode, push scroll wheel to mouse event buffer
                        // SDL wheel y>0 = scroll up, y<0 = scroll down
                        // DirectInput DIMOFS_Z uses positive = forward, negative = backward
                        if (event.wheel.y != 0)
                            FF_PushMouseEvent(DIMOFS_Z, (DWORD)(int)(event.wheel.y * 120));
                    }
                }
                break;

            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_CLOSE:
                        // Window X button clicked
                        g_running = false;
                        PostGameMessage(WM_QUIT, 0, 0);
                        break;
                    case SDL_WINDOWEVENT_RESIZED:
                        glViewport(0, 0, event.window.data1, event.window.data2);
                        PostGameMessage(WM_SIZE, 0, MAKELPARAM(event.window.data1, event.window.data2));
                        break;

                    // FF_LINUX (INPUT-1): there was no focus handling at all, so
                    // once the window lost focus in sim mode nothing ever restored
                    // relative mouse mode -- keyboard and mouse stayed dead while
                    // the joystick kept working, because the joystick is polled
                    // from a device path that does not depend on window focus.
                    // Alt-Tabbing back returned the window but not its input.
                    // Re-grab on focus gain, and drop the grab on focus loss so
                    // the pointer is not held captive by a background window.
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        if (!doUI) {
                            SDL_SetRelativeMouseMode(SDL_TRUE);
                            SDL_RaiseWindow(g_SDLWindow);
                        }

                        fprintf(stderr, "[INPUT] focus gained (doUI=%d) -> relative mouse %s\n",
                                doUI ? 1 : 0, doUI ? "left off" : "re-enabled");
                        fflush(stderr);
                        break;

                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        if (!doUI)
                            SDL_SetRelativeMouseMode(SDL_FALSE);

                        fprintf(stderr, "[INPUT] focus lost (doUI=%d) -> relative mouse released\n",
                                doUI ? 1 : 0);
                        fflush(stderr);
                        break;

                    default:
                        break;
                }
                break;

            // Joystick events - update IO structure directly
            case SDL_JOYAXISMOTION:
                if (event.jaxis.which == g_JoystickIndex && event.jaxis.axis < 16) {
                    g_JoystickAxes[event.jaxis.axis] = event.jaxis.value;

                    // FF_LINUX: axis discovery - log the first few events per axis
                    // so users can identify their throttle axis, and allow
                    // remapping via FF_THROTTLE_AXIS / FF_YAW_AXIS env vars.
                    {
                        static int s_axisSeen[16] = {0};
                        if (event.jaxis.axis < 16 && s_axisSeen[event.jaxis.axis] < 3 &&
                            (event.jaxis.value > 8000 || event.jaxis.value < -8000)) {
                            s_axisSeen[event.jaxis.axis]++;
                            fprintf(stderr, "[Joystick] axis %d active (value %d)\n",
                                    event.jaxis.axis, event.jaxis.value);
                        }
                    }
                    static int s_throttleAxis = -2, s_yawAxis = -2;
                    if (s_throttleAxis == -2) {
                        // Default layout matches common twist-grip sticks
                        // (Logitech 3D Pro etc.): X=0 Y=1 twist(yaw)=2 throttle=3
                        const char* e = getenv("FF_THROTTLE_AXIS");
                        s_throttleAxis = e ? atoi(e) : 3;
                        e = getenv("FF_YAW_AXIS");
                        s_yawAxis = e ? atoi(e) : 2;
                        fprintf(stderr, "[Joystick] throttle axis=%d, yaw axis=%d (override with FF_THROTTLE_AXIS / FF_YAW_AXIS)\n",
                                s_throttleAxis, s_yawAxis);
                    }

                    // Update IO structure based on axis mapping
                    // Default: Axis 0: Roll, Axis 1: Pitch, Axis 2: Throttle, Axis 3: Yaw
                    int axisVal = event.jaxis.value * 10000 / 32767;
                    int axisId = event.jaxis.axis;
                    if (axisId == s_throttleAxis) axisId = 2;
                    else if (axisId == s_yawAxis) axisId = 3;
                    else if (axisId == 2 || axisId == 3) axisId = 16; // displaced default - ignore
                    switch (axisId) {
                        case 0:  // Roll (X)
                            IO.analog[AXIS_ROLL].ioVal = axisVal;
                            IO.analog[AXIS_ROLL].engrValue = (float)axisVal / 10000.0f;
                            IO.analog[AXIS_ROLL].isUsed = true;
                            break;
                        case 1:  // Pitch (Y)
                            IO.analog[AXIS_PITCH].ioVal = axisVal;
                            IO.analog[AXIS_PITCH].engrValue = (float)axisVal / 10000.0f;
                            IO.analog[AXIS_PITCH].isUsed = true;
                            break;
                        case 2:  // Throttle (Z) - unipolar
                            {
                                int throttleVal = (32767 - event.jaxis.value) * 15000 / 65535;
                                // FF_LINUX: PilotInputs::Update() ignores the throttle
                                // axis while UseKeyboardThrottle is set. On Windows,
                                // GetJoystickInput() clears it when the throttle moves,
                                // but that function early-returns on Linux (gTotalJoy==0)
                                // so mirror that behavior here.
                                extern int UseKeyboardThrottle;
                                static int s_lastThrottleVal = -99999;
                                if (abs(throttleVal - s_lastThrottleVal) > 500) {
                                    UseKeyboardThrottle = FALSE;
                                }
                                s_lastThrottleVal = throttleVal;
                                IO.analog[AXIS_THROTTLE].ioVal = throttleVal;
                                // engrValue range is 0.0 (idle) .. 1.0 (mil) .. 1.5 (full AB);
                                // map the full physical travel to 0..1.5 so the
                                // afterburner is reachable (top third of travel).
                                IO.analog[AXIS_THROTTLE].engrValue = (float)throttleVal * 1.5f / 15000.0f;
                                IO.analog[AXIS_THROTTLE].isUsed = true;
                            }
                            break;
                        case 3:  // Yaw (Rz)
                            IO.analog[AXIS_YAW].ioVal = axisVal;
                            IO.analog[AXIS_YAW].engrValue = (float)axisVal / 10000.0f;
                            IO.analog[AXIS_YAW].isUsed = true;
                            break;
                    }
                }
                break;

            case SDL_JOYBUTTONDOWN:
                if (event.jbutton.which == g_JoystickIndex && event.jbutton.button < SIMLIB_MAX_DIGITAL) {
                    IO.digital[event.jbutton.button] = TRUE;
                }
                break;

            case SDL_JOYBUTTONUP:
                if (event.jbutton.which == g_JoystickIndex && event.jbutton.button < SIMLIB_MAX_DIGITAL) {
                    IO.digital[event.jbutton.button] = FALSE;
                }
                break;

            case SDL_JOYHATMOTION:
                g_hatEventCount++;
                g_hatLastWhich = (int)event.jhat.which;
                g_hatLastValue = (int)event.jhat.value;
                if (event.jhat.which == g_JoystickIndex && event.jhat.hat < SIMLIB_MAX_POV) {
                    g_hatAcceptedCount++;
                    IO.povHatAngle[event.jhat.hat] = ConvertSDLHatToPOV(event.jhat.value);
                }
                break;

            case SDL_JOYDEVICEADDED:
                FF_DEBUG_JOYSTICK("Device added: %d\n", event.jdevice.which);
                if (g_SDLJoystick == nullptr) {
                    // Try to open the newly connected joystick
                    InitSDLJoystick();
                }
                break;

            case SDL_JOYDEVICEREMOVED:
                FF_DEBUG_JOYSTICK("Device removed: %d\n", event.jdevice.which);
                if (event.jdevice.which == g_JoystickIndex) {
                    // Our joystick was disconnected
                    CleanupSDLJoystick();
                }
                break;
        }
    }
}

// Helper to queue a message for posting after message processing (avoids deadlock)
static void QueuePendingMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    g_pendingMessages.push_back({msg, wParam, lParam});
}

// Process game messages (FM_* messages from falcuser.h)
bool ProcessGameMessages() {
    // First, process pending messages that were queued during previous processing
    if (!g_pendingMessages.empty()) {
        std::lock_guard<std::mutex> lock(g_messageMutex);
        for (const auto& msg : g_pendingMessages) {
            g_messageQueue.push(msg);
        }
        g_pendingMessages.clear();
    }

    // Now process the queue
    std::vector<GameMessage> messagesToProcess;
    {
        std::lock_guard<std::mutex> lock(g_messageMutex);
        while (!g_messageQueue.empty()) {
            messagesToProcess.push_back(g_messageQueue.front());
            g_messageQueue.pop();
        }
    }

    // Process messages without holding the lock
    for (const auto& msg : messagesToProcess) {
        // Handle game-specific messages
        switch (msg.message) {
            case WM_QUIT:
                return false;

            case FM_START_GAME:
                fprintf(stderr, "[FM] FM_START_GAME received\n");
                // SystemLevelInit is handled by init_game_core()
                // Send FM_START_UI to start the UI (queue it to avoid deadlock)
                QueuePendingMessage(FM_START_UI, 0, 0);
                break;

            case FM_START_UI:
                fprintf(stderr, "[FM] FM_START_UI received\n");

                // FF_LINUX: FF_ACMI_IMPORT=1 converts any recorded acmibin/acmi*.flt
                // into the next free acmibin/TAPEnnnn.vhs. The recorder only ever
                // writes the raw .flt; ACMI_ImportFile() does the conversion, and in
                // this build NOTHING calls it -- both call sites in acmiui.cpp are
                // commented out and the only live one is a multiplayer chat command.
                // So a recorded flight never becomes a loadable tape. ACMI is the
                // project's quantitative instrument (PO), which makes that a real gap,
                // not just an automation inconvenience.
                if (getenv("FF_ACMI_IMPORT"))
                {
                    extern void ACMI_ImportFile(void);
                    fprintf(stderr, "[ACMI] FF_ACMI_IMPORT: converting acmibin/acmi*.flt -> TAPEnnnn.vhs\n");
                    fflush(stderr);
                    ACMI_ImportFile();
                    fprintf(stderr, "[ACMI] FF_ACMI_IMPORT: done\n");
                    fflush(stderr);
                }

                g_simTakingOverDisplay = 0;  // FF_LINUX: back in UI; UI_Cleanup may tear down the device again
                // FF_LINUX: Restore OS cursor for UI mode
                SDL_SetRelativeMouseMode(SDL_FALSE);
                // Re-acquire GL context on main thread if sim was running
                if (!doUI) {
                    FF_AcquireGLContext();
                    fprintf(stderr, "[FM] Main thread re-acquired GL context\n");
                }
                TheCampaign.Suspend();
                if (msg.wParam) {
                    g_theaters.DoSoundSetup();
                }
                FalconLocalSession->SetFlyState(FLYSTATE_IN_UI);
                doUI = TRUE;
                fprintf(stderr, "[FM] Calling UI_Startup()...\n");
                UI_Startup();
                TheCampaign.Resume();
                fprintf(stderr, "[FM] UI_Startup() complete\n");
                break;

            case FM_END_UI:
                fprintf(stderr, "[FM] FM_END_UI received\n");
                doUI = FALSE;
                TheCampaign.Suspend();
                UI_Cleanup();
                TheCampaign.Resume();
                break;

            case FM_TIMER_UPDATE:
                // Called periodically to update the UI
                // On Linux, we handle Update/CopyToPrimary directly here instead of
                // using the background OutputLoop thread (which is disabled on Linux).
                if (gMainHandler != nullptr) {
                    gMainHandler->ProcessUserCallbacks();
                    // Trigger a full screen refresh
                    UI95_RECT fullRect = { 0, 0, gMainHandler->GetW(), gMainHandler->GetH() };
                    gMainHandler->RefreshAll(&fullRect);
                    // Draw windows to the Front_ buffer
                    gMainHandler->Update();
                    // Copy from Front_ to Primary_ surface
                    gMainHandler->CopyToPrimary();
                }
                break;

            case FM_EXIT_GAME:
                fprintf(stderr, "[FM] FM_EXIT_GAME received\n");
                g_running = false;
                return false;

            // FM_DISP_ENTER_MODE is not needed on Linux - EnterMode directly calls _EnterMode
            // FM_DISP_LEAVE_MODE is also not needed on Linux

            // =========================================================
            // Campaign loading / joining / shutdown
            // =========================================================
            case FM_LOAD_CAMPAIGN:
            {
                fprintf(stderr, "[FM] FM_LOAD_CAMPAIGN received (type=%ld)\n", (long)msg.lParam);
                // FF_LINUX: Paint the screen black before the (multi-second)
                // campaign decode that runs on this (main) thread, which would
                // otherwise leave a white screen until OTWDriver::Enter() shows
                // the loading splash. The main thread still owns the GL context
                // here. (FF_LoadingClear is a no-op if we don't own it.)
                { extern void FF_LoadingClear(); FF_LoadingClear(); }
                // For non-campaign/TE types, use "Instant" as campaign file
                if ((FalconGameType)msg.lParam != game_Campaign &&
                    (FalconGameType)msg.lParam != game_TacticalEngagement) {
                    strcpy(gUI_CampaignFile, "Instant");
                }
                fprintf(stderr, "[FM] FM_LOAD_CAMPAIGN: Calling TheCampaign.LoadCampaign()...\n");
                int retval;
                // FF_LINUX: A desynced/incompatible save (e.g. some Tactical
                // Engagement missions whose unit data overruns the decode buffer)
                // makes memcpychk throw InvalidBufferException (std::out_of_range).
                // On Windows this was swallowed by SEH; on Linux an uncaught throw
                // calls std::terminate -> SIGABRT. Catch it and fail the load
                // gracefully (return to the menu) instead of crashing.
                try {
                    retval = TheCampaign.LoadCampaign((FalconGameType)msg.lParam, gUI_CampaignFile);
                } catch (const std::exception& e) {
                    fprintf(stderr, "[FM] FM_LOAD_CAMPAIGN: LoadCampaign threw (%s) - failing load gracefully\n", e.what());
                    // The throw unwinds past the (recursive) campCritical leaves in
                    // Decode/LoadCampaign, leaving this thread holding it N deep.
                    // Drain it so the campaign thread doesn't deadlock later.
                    extern int F4CheckHasCriticalSection(F4CSECTIONHANDLE*);
                    while (campCritical && F4CheckHasCriticalSection(campCritical))
                        CampLeaveCriticalSection();
                    retval = 0;
                }
                fprintf(stderr, "[FM] FM_LOAD_CAMPAIGN: LoadCampaign() returned %d\n", retval);
                if (retval) {
                    fprintf(stderr, "[FM] FM_LOAD_CAMPAIGN: Queueing FM_JOIN_SUCCEEDED\n");
                    QueuePendingMessage(FM_JOIN_SUCCEEDED, 0, 0);
                } else {
                    fprintf(stderr, "[FM] FM_LOAD_CAMPAIGN: Queueing FM_JOIN_FAILED\n");
                    QueuePendingMessage(FM_JOIN_FAILED, 0, 0);
                }
                break;
            }

            case FM_JOIN_SUCCEEDED:
                fprintf(stderr, "[FM] FM_JOIN_SUCCEEDED received\n");
                CampaignJoinSuccess();
                if (!gMainHandler) {
                    QueuePendingMessage(FM_START_UI, 0, 0);
                }
                // TEST: Auto-start instant action if triggered by test code
                if (g_autoTestInstantAction) {
                    g_autoTestInstantAction = false;
                    fprintf(stderr, "[TEST] Campaign joined, now posting FM_START_INSTANTACTION...\n");
                    QueuePendingMessage(FM_START_INSTANTACTION, 0, 0);
                }
                break;

            case FM_JOIN_FAILED:
                fprintf(stderr, "[FM] FM_JOIN_FAILED received\n");
                CampaignJoinFail();
                break;

            case FM_SHUTDOWN_CAMPAIGN:
                fprintf(stderr, "[FM] FM_SHUTDOWN_CAMPAIGN received\n");
                ShutdownCampaign();
                break;

            case FM_AUTOSAVE_CAMPAIGN:
                fprintf(stderr, "[FM] FM_AUTOSAVE_CAMPAIGN received\n");
                CampaignAutoSave((FalconGameType)msg.lParam);
                break;

            case FM_ONLINE_STATUS:
                if (doUI && gMainHandler) {
                    UI_CommsErrorMessage(static_cast<WORD>(msg.wParam));
                }
                break;

            case FM_GOT_CAMPAIGN_DATA:
                fprintf(stderr, "[FM] FM_GOT_CAMPAIGN_DATA received (wParam=%lu)\n",
                        (unsigned long)msg.wParam);
                // Handle campaign data based on what we received
                if (msg.wParam == CAMP_NEED_PRELOAD && FalconLocalGame) {
                    CampaignPreloadSuccess(!FalconLocalGame->IsLocal());
                }
                // FF_LINUX (MP-1): the eight DATA cases were missing entirely.
                //
                // FalconWndProc (winmain.cpp:1810-1898) handles CAMP_NEED_ENTITIES,
                // WEATHER, PERSIST, OBJ_DELTAS, TEAM_DATA, UNIT_DATA, VC and
                // PRIORITIES, and every one of them calls TheCampaign.GotJoinData().
                // That call is the join's completion gate: it clears CAMP_LOADED,
                // runs JoinGame() and posts FM_JOIN_SUCCEEDED once no CAMP_NEED_*
                // bits remain. Here only CAMP_NEED_PRELOAD was handled, so nothing
                // ever re-evaluated completion.
                //
                // Some receive handlers call GotJoinData() themselves (weather.cpp,
                // team.cpp, campdatamsg.cpp, sendpersistantlist.cpp) which is why the
                // joiner got as far as it did -- 8 of 9 bits cleared. But
                // sendvcmsg.cpp does NOT; it only posts this message. So when VC was
                // the last bit outstanding, the flag cleared and nobody noticed the
                // join was complete. Measured: joiner stuck at
                // loaded=0 preloaded=1 suspended=1 with stillNeeded=[VC], forever,
                // while the master had already reported "SENDING VC".
                else if (msg.wParam == CAMP_NEED_ENTITIES   || msg.wParam == CAMP_NEED_WEATHER   ||
                         msg.wParam == CAMP_NEED_PERSIST    || msg.wParam == CAMP_NEED_OBJ_DELTAS ||
                         msg.wParam == CAMP_NEED_TEAM_DATA  || msg.wParam == CAMP_NEED_UNIT_DATA ||
                         msg.wParam == CAMP_NEED_VC         || msg.wParam == CAMP_NEED_PRIORITIES)
                {
                    // Same guard the Windows handler applies to every one of these.
                    if (FalconLocalGame && vuPlayerPoolGroup != vuLocalGame) {
                        extern VU_TIME gCampJoinLastData;
                        extern int gCampJoinTries;
                        gCampJoinLastData = vuxRealTime;
                        gCampJoinTries = 0;
                        TheCampaign.GotJoinData();
                    }
                }
                break;

            // =========================================================
            // UI refresh notifications (issue #13). UI_Refresh() posts
            // these after slot/team/type changes; on Windows they're
            // handled by FalconWndProc (winmain.cpp:1546-1570). Without
            // them the dogfight roster/TE/campaign mission windows never
            // rebuild after joining or adding aircraft.
            // =========================================================
            case FM_UI_UPDATE_GAMELIST:
                if (doUI && gMainHandler) {
                    extern void UI_UpdateGameList();
                    UI_UpdateGameList();
                }
                break;

            case FM_REFRESH_DOGFIGHT:
                if (doUI && gMainHandler) {
                    extern void CopyDFSettingsToWindow(void);
                    CopyDFSettingsToWindow();
                }
                break;

            case FM_REFRESH_TACTICAL:
                if (doUI && gMainHandler) {
                    extern void UpdateMissionWindow(long ID);
                    extern void CheckCampaignFlyButton(void);
                    UpdateMissionWindow(30305 /* TAC_AIRCRAFT (userids.h) */);
                    CheckCampaignFlyButton();
                }
                break;

            case FM_REFRESH_CAMPAIGN:
                if (doUI && gMainHandler) {
                    extern void UpdateMissionWindow(long ID);
                    extern void CheckCampaignFlyButton(void);
                    UpdateMissionWindow(6100 /* CB_MISSION_SCREEN (userids.h) */);
                    CheckCampaignFlyButton();
                }
                break;

            // =========================================================
            // Sim entry: start flying
            // =========================================================
            case FM_START_INSTANTACTION:
                fprintf(stderr, "[FM] FM_START_INSTANTACTION received\n");
                fflush(stderr);
                fprintf(stderr, "[FM] Calling SetFlyState...\n");
                fflush(stderr);
                FalconLocalSession->SetFlyState(FLYSTATE_LOADING);
                fprintf(stderr, "[FM] Calling set_campaign_time (start_time will be %ld)...\n",
                        instant_action::get_start_time());
                fflush(stderr);
                instant_action::set_campaign_time();
                fprintf(stderr, "[FM] After set_campaign_time: vuxGameTime=%u\n",
                        (unsigned)vuxGameTime);
                fflush(stderr);
                fprintf(stderr, "[FM] Calling move_player_flight...\n");
                fflush(stderr);
                instant_action::move_player_flight();
                fprintf(stderr, "[FM] Calling create_wave...\n");
                fflush(stderr);
                instant_action::create_wave();
                fprintf(stderr, "[FM] Starting instant action... vuxRealTime=%lu\n",
                        (unsigned long)vuxRealTime);
                fflush(stderr);
                // CRITICAL ORDER:
                // 1. StartGraphics() - signals sim thread to start graphics (just sets a flag)
                // 2. Release GL context - sim thread will need it when StartLoop() wakes up
                // 3. EndUI() - may block in TheCampaign.Suspend(), so must be last
                g_simTakingOverDisplay = 1;  // FF_LINUX: tell UI_Cleanup not to tear down the display device
                fprintf(stderr, "[FM] Calling StartGraphics()...\n");
                fflush(stderr);
                SimulationLoopControl::StartGraphics();
                fprintf(stderr, "[FM] StartGraphics() returned, releasing GL context...\n");
                fflush(stderr);
                FF_ReleaseGLContext();
                fprintf(stderr, "[FM] GL context released, calling EndUI()...\n");
                fflush(stderr);
                EndUI();
                // FF_LINUX: Hide OS cursor and grab mouse for sim mode
                SDL_SetRelativeMouseMode(SDL_TRUE);
                fprintf(stderr, "[FM] EndUI() returned, relative mouse mode ON\n");
                fflush(stderr);
                break;

            case FM_START_DOGFIGHT:
                fprintf(stderr, "[FM] FM_START_DOGFIGHT received\n");
                FalconLocalSession->SetFlyState(FLYSTATE_LOADING);
                // Same critical order as INSTANTACTION
                g_simTakingOverDisplay = 1;  // FF_LINUX: tell UI_Cleanup not to tear down the display device
                SimulationLoopControl::StartGraphics();
                FF_ReleaseGLContext();
                EndUI();
                SDL_SetRelativeMouseMode(SDL_TRUE);
                fprintf(stderr, "[FM] Main thread released GL context for sim\n");
                break;

            case FM_START_CAMPAIGN:
                fprintf(stderr, "[FM] FM_START_CAMPAIGN received\n");
                FalconLocalSession->SetFlyState(FLYSTATE_LOADING);
                // Same critical order as INSTANTACTION
                g_simTakingOverDisplay = 1;  // FF_LINUX: tell UI_Cleanup not to tear down the display device
                SimulationLoopControl::StartGraphics();
                FF_ReleaseGLContext();
                EndUI();
                SDL_SetRelativeMouseMode(SDL_TRUE);
                break;

            case FM_START_TACTICAL:
                fprintf(stderr, "[FM] FM_START_TACTICAL received\n");
                FalconLocalSession->SetFlyState(FLYSTATE_LOADING);
                // Same critical order as INSTANTACTION
                g_simTakingOverDisplay = 1;  // FF_LINUX: tell UI_Cleanup not to tear down the display device
                SimulationLoopControl::StartGraphics();
                FF_ReleaseGLContext();
                EndUI();
                SDL_SetRelativeMouseMode(SDL_TRUE);
                break;

            // =========================================================
            // Sim exit: return to menu
            // =========================================================
            case FM_END_INSTANTACTION:
            case FM_END_DOGFIGHT:
                fprintf(stderr, "[FM] FM_END_INSTANTACTION/DOGFIGHT received\n");
                // These are currently no-ops in Windows too (winmain.cpp:1736-1737)
                break;

            case FM_REVERT_CAMPAIGN:
            {
                fprintf(stderr, "[FM] FM_REVERT_CAMPAIGN received\n");
                int gametype = FalconLocalGame->GetGameType();

                // Game aborted - reload current campaign
                strcpy(gUI_CampaignFile, TheCampaign.SaveFile);
                PostGameMessage(FM_SHUTDOWN_CAMPAIGN, 0, 0);

                if (gametype == game_Campaign) {
                    StartCampaignGame(1, gametype);
                } else if (gametype == game_TacticalEngagement) {
                    tactical_restart_mission();
                }
                break;
            }

            // Route mouse and keyboard events to the UI handler
            case WM_LBUTTONDOWN:
                if (gMainHandler != nullptr) {
                    static int lbdCount = 0;
                    lbdCount++;
                    if (lbdCount <= 5) {
                        fprintf(stderr, "[Mouse] LBUTTONDOWN at (%d, %d)\n",
                                LOWORD(msg.lParam), HIWORD(msg.lParam));
                    }
                    gMainHandler->EventHandler(NULL, msg.message, msg.wParam, msg.lParam);
                }
                break;
            case WM_LBUTTONUP:
                if (gMainHandler != nullptr) {
                    static int lbuCount = 0;
                    lbuCount++;
                    gMainHandler->EventHandler(NULL, msg.message, msg.wParam, msg.lParam);
                }
                break;
            case WM_LBUTTONDBLCLK:
            case WM_RBUTTONDBLCLK:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_MOUSEMOVE:
            case WM_KEYDOWN:
            case WM_KEYUP:
                if (gMainHandler != nullptr) {
                    gMainHandler->EventHandler(NULL, msg.message, msg.wParam, msg.lParam);
                }
                break;

            default:
                // Message not handled - that's okay for many messages
                break;
        }
    }
    return true;
}

static void render_frame(void) {
    // Use fallback menu if enabled (temporary workaround for UI95 issues)
    if (g_useFallbackMenu && doUI) {
        glClear(GL_COLOR_BUFFER_BIT);
        DrawFallbackMenu();
        SDL_GL_SwapWindow(g_SDLWindow);
        return;
    }

    // When in UI mode (doUI=1), the UI has been drawn to the primary DirectDraw surface
    // We need to present that surface via OpenGL
    if (doUI) {
        FF_PresentPrimarySurface();

        // FF_LINUX (MP-1): can peer B RESOLVE the remote game it was told about?
        // af98c308 showed a hosted game IS distributed -- peer B decodes a real game
        // id (e.g. 856619/28007) and receives CREATE(10)/SESSION(11). Joining is
        // InitCampaign(gametype, joingame), which needs the actual FalconGameEntity.
        // If vuDatabase cannot Find it, no join hook can work, so establish that
        // first. FF_DEBUG_MPCOMMS=1.
        {
            extern unsigned g_ffRemoteGameCreator, g_ffRemoteGameNum;
            static int ffDbg = -1;
            static int ffDone = 0;

            if (ffDbg < 0)
                ffDbg = getenv("FF_DEBUG_MPCOMMS") ? 1 : 0;

            if (ffDbg and not ffDone and g_ffRemoteGameNum)
            {
                ffDone = 1;
                VU_ID ffId;
                ffId.creator_.value_ = g_ffRemoteGameCreator;
                ffId.num_ = g_ffRemoteGameNum;
                VuEntity *ffE = vuDatabase ? vuDatabase->Find(ffId) : NULL;
                fprintf(stderr, "[MPJOIN] remote game %u/%u -> Find()=%p%s\n",
                        g_ffRemoteGameCreator, g_ffRemoteGameNum, (void*)ffE,
                        ffE ? "" : "   <-- NOT RESOLVABLE, a join hook cannot work");

                if (ffE)
                    fprintf(stderr, "[MPJOIN]   entityType=%d isGame=%d\n",
                            (int)ffE->EntityType(),
                            (ffE->IsGame() ? 1 : 0));

                fflush(stderr);
            }
        }

        // FF_LINUX (MP-1): FF_MP_JOIN=1 drives the JOIN the UI performs, using the
        // game's own entry points in the game's own order (see 81d52df3):
        //     1 resolve   vuDatabase->Find(remoteGameId)
        //     2 select    gCommsMgr->LookAtGame(game)          -> UIComms::TargetGame_
        //     3 preload   TheCampaign.RequestScenarioStats(game)
        //     4 join      TheCampaign.JoinCampaign(type, game)
        // Step 4 MUST wait for CAMP_PRELOADED: JoinCampaign (cmpclass.cpp:790) returns
        // 0 immediately unless IsPreLoaded(), and that flag is only set once campaign
        // data has arrived from the master. Calling it early looks like "join broken"
        // when it is only out of order -- which is why the UI's join is two-phase.
        {
            extern unsigned g_ffRemoteGameCreator, g_ffRemoteGameNum;
            static int ffJoin = -1;
            static int ffPhase = 0;
            static DWORD ffPhaseAt = 0;

            if (ffJoin < 0)
                ffJoin = getenv("FF_MP_JOIN") ? 1 : 0;

            if (ffJoin and gCommsMgr)
            {
                if (ffPhase == 0 and g_ffRemoteGameNum)
                {
                    VU_ID ffId;
                    ffId.creator_.value_ = g_ffRemoteGameCreator;
                    ffId.num_ = g_ffRemoteGameNum;
                    VuEntity *ffE = vuDatabase ? vuDatabase->Find(ffId) : NULL;

                    if (ffE and ffE->IsGame())
                    {
                        FalconGameEntity *ffGame = (FalconGameEntity *)ffE;
                        gCommsMgr->LookAtGame((VuGameEntity *)ffGame);
                        const int ffRc = TheCampaign.RequestScenarioStats(ffGame);
                        fprintf(stderr, "[MPJOIN] phase1 LookAtGame + RequestScenarioStats -> %d\n", ffRc);
                        fflush(stderr);
                        ffPhase = 1;
                        ffPhaseAt = GetTickCount();
                    }
                }
                else if (ffPhase == 1)
                {
                    // wait for the master's campaign data to arrive
                    if (TheCampaign.IsPreLoaded())
                    {
                        VU_ID ffId;
                        ffId.creator_.value_ = g_ffRemoteGameCreator;
                        ffId.num_ = g_ffRemoteGameNum;
                        VuEntity *ffE = vuDatabase ? vuDatabase->Find(ffId) : NULL;

                        if (ffE and ffE->IsGame())
                        {
                            FalconGameEntity *ffGame = (FalconGameEntity *)ffE;
                            const int ffRc = TheCampaign.JoinCampaign(
                                                 (FalconGameType)ffGame->gameType, ffGame);
                            fprintf(stderr, "[MPJOIN] phase2 JoinCampaign -> %d  (loaded=%d)\n",
                                    ffRc, TheCampaign.IsLoaded() ? 1 : 0);
                            fflush(stderr);
                        }

                        ffPhase = 2;
                        ffPhaseAt = GetTickCount();
                    }
                    else if (GetTickCount() - ffPhaseAt > 30000)
                    {
                        fprintf(stderr, "[MPJOIN] phase1 TIMEOUT: CAMP_PRELOADED never set "
                                "after 30s -- the master never sent campaign data\n");
                        fflush(stderr);
                        ffPhase = 3; // nothing to watch
                    }
                }
                else if (ffPhase == 2)
                {
                    // FF_LINUX (MP-1): JoinCampaign returning 1 only means the join was
                    // ACCEPTED -- loading is asynchronous (InitCampaign hands back a
                    // thread handle), so IsLoaded() is 0 at that moment by design.
                    // Watch it settle rather than sampling once, otherwise "loaded=0"
                    // reads as failure when it only means "not yet".
                    static DWORD ffTick = 0;
                    DWORD ffNow = GetTickCount();

                    if (TheCampaign.IsLoaded())
                    {
                        fprintf(stderr, "[MPJOIN] phase3 campaign LOADED after %lums "
                                "-- join completed\n",
                                (unsigned long)(ffNow - ffPhaseAt));
                        fflush(stderr);
                        ffPhase = 3;
                    }
                    else if (ffNow - ffTick >= 5000)
                    {
                        ffTick = ffNow;
                        fprintf(stderr, "[MPJOIN] phase3 waiting: loaded=0 preloaded=%d "
                                "suspended=%d online=%d  t+%lums\n",
                                TheCampaign.IsPreLoaded() ? 1 : 0,
                                TheCampaign.IsSuspended() ? 1 : 0,
                                TheCampaign.IsOnline() ? 1 : 0,
                                (unsigned long)(ffNow - ffPhaseAt));
                        fflush(stderr);
                    }

                    if (ffPhase == 2 and ffNow - ffPhaseAt > 60000)
                    {
                        fprintf(stderr, "[MPJOIN] phase3 TIMEOUT: campaign never reached "
                                "LOADED within 60s of a successful join\n");
                        fflush(stderr);
                        ffPhase = 3;
                    }
                }
            }
        }

        // FF_LINUX debug: scripted UI clicks via FF_UI_CLICK="x,y@sec;x,y@sec..."
        // (UI-surface coordinates, 1024x768). Posts the same WM_LBUTTONDOWN/UP
        // messages a real mouse click produces - for automated UI testing.
        {
            static int s_clickInit = 0;
            static struct { int x, y; Uint32 atMs; int fired; int dbl; } s_clicks[16];
            static int s_nClicks = 0;
            static Uint32 s_uiStart = 0;
            if (!s_clickInit) {
                s_clickInit = 1;
                const char* e = getenv("FF_UI_CLICK");
                if (e) {
                    char buf[256];
                    strncpy(buf, e, sizeof(buf) - 1); buf[sizeof(buf) - 1] = 0;
                    for (char* tok = strtok(buf, ";"); tok && s_nClicks < 16; tok = strtok(NULL, ";")) {
                        int cx, cy; float at;
                        char dbl = 0;  // 'd' = double-click, 'r' = right-click
                        if (sscanf(tok, "%d,%d@%f%c", &cx, &cy, &at, &dbl) >= 3) {
                            s_clicks[s_nClicks].x = cx;
                            s_clicks[s_nClicks].y = cy;
                            s_clicks[s_nClicks].atMs = (Uint32)(at * 1000.0f);
                            s_clicks[s_nClicks].fired = 0;
                            s_clicks[s_nClicks].dbl = (dbl == 'd') ? 1 : (dbl == 'r') ? 2 : 0;
                            s_nClicks++;
                        }
                    }
                }
            }
            if (s_nClicks) {
                if (!s_uiStart) s_uiStart = SDL_GetTicks();
                Uint32 el = SDL_GetTicks() - s_uiStart;
                for (int ci = 0; ci < s_nClicks; ci++) {
                    if (!s_clicks[ci].fired && el >= s_clicks[ci].atMs) {
                        s_clicks[ci].fired = 1;
                        LPARAM lp = MAKELPARAM(s_clicks[ci].x, s_clicks[ci].y);
                        fprintf(stderr, "[FF_UI_CLICK] firing (%d,%d) at %ums\n", s_clicks[ci].x, s_clicks[ci].y, el);
                        PostGameMessage(WM_MOUSEMOVE, 0, lp);
                        if (s_clicks[ci].dbl == 2) {
                            // context menus open on RBUTTONUP over a window
                            PostGameMessage(WM_RBUTTONDOWN, 0, lp);
                            PostGameMessage(WM_RBUTTONUP, 0, lp);
                        } else {
                            PostGameMessage(WM_LBUTTONDOWN, 0, lp);
                            PostGameMessage(WM_LBUTTONUP, 0, lp);
                        }
                        if (s_clicks[ci].dbl == 1) {
                            PostGameMessage(WM_LBUTTONDBLCLK, 0, lp);
                            PostGameMessage(WM_LBUTTONUP, 0, lp);
                        }
                    }
                }
            }
        }

        // FF_LINUX debug: FF_DUMP_UI="<sec>[;<sec>...]" dumps every visible window
        // and its controls -- ID and rectangle -- at each listed time.
        //
        // Written for MP-1. Driving a real UI join means clicking the comms and
        // game-list screens, and those coordinates are recorded nowhere; the
        // existing click scripts are pixel positions someone found once by hand.
        // Reading the control rectangles straight out of the UI turns "guess a
        // pixel, see if anything happens" into arithmetic, and a click that lands
        // on nothing is then distinguishable from a button that does nothing.
        {
            static int s_n = -1;
            static Uint32 s_at[8];
            static int s_fired[8];

            if (s_n < 0)
            {
                s_n = 0;
                const char* e = getenv("FF_DUMP_UI");

                if (e)
                {
                    char buf[128];
                    snprintf(buf, sizeof(buf), "%s", e);

                    for (char* tok = strtok(buf, ";"); tok and s_n < 8;
                         tok = strtok(NULL, ";"))
                    {
                        s_at[s_n] = (Uint32)(atof(tok) * 1000.0);
                        s_fired[s_n] = 0;
                        s_n++;
                    }
                }
            }

            if (s_n > 0)
            {
                const Uint32 el = SDL_GetTicks();

                for (int di = 0; di < s_n; di++)
                {
                    if (s_fired[di] or el < s_at[di])
                        continue;

                    s_fired[di] = 1;

                    extern C_Handler *gMainHandler;

                    if ( not gMainHandler)
                    {
                        fprintf(stderr, "[UIDUMP] t=%ums no handler\n", el);
                        fflush(stderr);
                        continue;
                    }

                    fprintf(stderr, "[UIDUMP] ==== t=%ums ====\n", el);

                    for (C_Window* w = gMainHandler->_GetFirstWindow(); w;
                         w = gMainHandler->_GetNextWindow(w))
                    {
                        // Hidden windows stay registered, so an unfiltered dump
                        // lists every screen the UI has ever built and none of the
                        // coordinates mean anything. FF_DUMP_UI_ALL=1 shows them.
                        static int s_all = -1;

                        if (s_all < 0)
                            s_all = getenv("FF_DUMP_UI_ALL") ? 1 : 0;

                        const bool vis = gMainHandler->FFIsWindowVisible(w) ? true : false;

                        if ( not vis and not s_all)
                            continue;

                        fprintf(stderr, "[UIDUMP] window id=%ld at %ld,%ld %ldx%ld vis=%d\n",
                                w->GetID(), w->GetX(), w->GetY(), w->GetW(), w->GetH(),
                                vis ? 1 : 0);

                        for (CONTROLLIST* cl = w->GetControlList(); cl; cl = cl->Next)
                        {
                            C_Base* c = cl->Control_;

                            if ( not c)
                                continue;

                            // Report the CENTRE, which is what a click script wants,
                            // alongside the raw rect. Window coordinates are relative,
                            // so add the window origin.
                            fprintf(stderr,
                                    "[UIDUMP]   ctrl id=%ld rect=%ld,%ld %ldx%ld click=%ld,%ld\n",
                                    c->GetID(), c->GetX(), c->GetY(), c->GetW(), c->GetH(),
                                    w->GetX() + c->GetX() + c->GetW() / 2,
                                    w->GetY() + c->GetY() + c->GetH() / 2);
                        }
                    }

                    fflush(stderr);
                }
            }
        }

        // FF_LINUX debug: periodic UI screenshot when FF_UI_SCREENSHOT env var is set
        // (value = period in seconds, written to /tmp/ff_ui.bmp before buffer swap)
        {
            static int s_shotPeriod = -2;
            if (s_shotPeriod == -2) {
                const char* e = getenv("FF_UI_SCREENSHOT");
                s_shotPeriod = e ? atoi(e) : -1;
            }
            if (s_shotPeriod > 0) {
                static Uint32 s_lastShot = 0;
                Uint32 now = SDL_GetTicks();
                if (now - s_lastShot >= (Uint32)s_shotPeriod * 1000u) {
                    s_lastShot = now;
                    extern void SaveGLFramebufferAsBMP(const char* filename);
                    SaveGLFramebufferAsBMP("/tmp/ff_ui.bmp");
                }
            }
        }

        SDL_GL_SwapWindow(g_SDLWindow);
    } else if (g_simOwnsGLContext) {
        // Sim mode: the sim thread owns the GL context and handles rendering.
        // The main thread has no GL access. Just pump messages and events.
        // Don't call any GL functions here - the context belongs to the sim thread.
        return;
    }
    // If neither doUI nor g_simOwnsGLContext, this is a transitional state - just return
}

static void main_loop(void) {
    fprintf(stderr, "\n========================================\n");
    fprintf(stderr, "Entering main loop\n");
    fprintf(stderr, "  ESC = quit, Click X to close\n");
    fprintf(stderr, "========================================\n\n");

    Uint32 frameCount = 0;
    Uint32 lastFPSTime = SDL_GetTicks();
    Uint32 lastTimerTime = SDL_GetTicks();
    const Uint32 targetFrameTime = 16;  // ~60 FPS cap
    const Uint32 timerUpdateInterval = 100; // UI timer update interval in ms
    int fpsReportCounter = 0;

    // Post FM_START_GAME to trigger the game initialization sequence
    // This will internally call FM_START_UI which starts the UI
    fprintf(stderr, "[Main] Posting FM_START_GAME to initialize game...\n");
    PostGameMessage(FM_START_GAME, 0, 0);

    // TEST: Auto-launch instant action after delay for testing
    // Set to 0 to disable auto-launch (user can click buttons normally)
    // Use -test-ia command line option to enable (3 second delay)
    Uint32 autoLaunchTime = g_testInstantActionFlag ? 3000 : 0;
    bool autoLaunchTriggered = false;
    Uint32 startTime = SDL_GetTicks();

    while (g_running) {
        Uint32 frameStart = SDL_GetTicks();

        // Handle SDL events and convert to game messages
        handle_sdl_events();

        // Check if we should exit after event handling
        if (!g_running) {
            fprintf(stderr, "[main_loop] g_running set to false, breaking out of loop\n");
            fflush(stderr);
            break;
        }

        // Process game message queue
        if (!ProcessGameMessages()) {
            break;
        }

        // Send periodic timer updates for the UI system
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastTimerTime >= timerUpdateInterval) {
            PostGameMessage(FM_TIMER_UPDATE, 0, 0);
            lastTimerTime = currentTime;

            // FF_LINUX: With NEW_SYNC the campaign thread parks in
            // campaign_wait_for_sim(INFINITE) after every iteration and is
            // only woken by sim_signal_campaign() - which only the sim Loop
            // thread calls. In UI mode nothing signals it, so campaign time
            // never advances and DoCompressionLoop() never runs: the
            // dogfight/TE/campaign TAKEOFF buttons hang forever (issue #13).
            // Kick it on the UI timer tick, mirroring the sim loop's
            // per-frame signal.
            if (doUI) {
                // FF_LINUX: NO_TIMER_THREAD is defined, so the Windows timer
                // thread (which advances vuxGameTime in BOTH UI and sim modes)
                // doesn't exist - and only the sim loop advances vuxGameTime.
                // In campaign/UI mode nothing did, so the campaign clock was
                // frozen and time compression had no effect ("acts like 0x").
                // Advance it here, mirroring timerThread()'s core logic.
                {
                    extern VU_TIME vuxGameTime, vuxRealTime;
                    extern uint32_t lastStartTime;
                    extern uint32_t gCompressTillTime;
                    const DWORD MAX_TD = 500;  // MAX_TIME_DELTA
                    vuxRealTime = GetTickCount();
                    DWORD tdelta = (DWORD)(vuxRealTime - lastStartTime);
                    if (tdelta > MAX_TD) tdelta = MAX_TD;
                    if (!gCompressTillTime ||
                        vuxGameTime + tdelta * gameCompressionRatio < gCompressTillTime)
                        vuxGameTime += tdelta * gameCompressionRatio;
                    else if (vuxGameTime < gCompressTillTime)
                        vuxGameTime = gCompressTillTime;
                    lastStartTime = vuxRealTime;
                }
                ThreadManager::sim_signal_campaign();
            }
        }

        // TEST: Auto-launch instant action after delay
        if (autoLaunchTime > 0 && !autoLaunchTriggered && doUI &&
            (currentTime - startTime) >= autoLaunchTime) {
            autoLaunchTriggered = true;
            g_autoTestInstantAction = true;  // Flag to trigger FM_START_INSTANTACTION after join
            fprintf(stderr, "\n============================================\n");
            fprintf(stderr, "[TEST] Auto-launching Instant Action...\n");
            fprintf(stderr, "============================================\n\n");

            // Set up for instant action - mirror what InstantActionFlyCB does
            strcpy(gUI_CampaignFile, "Instant");

            // Set start time to noon (43200 seconds = 12:00:00)
            // This is what the IA setup screen does in instant.cpp:678
            instant_action::set_start_time(static_cast<long>(12.0f * 60.0f * 60.0f));
            fprintf(stderr, "[TEST] Set IA start_time to %ld (noon)\n",
                    instant_action::get_start_time());

            // Load the instant action campaign
            fprintf(stderr, "[TEST] Posting FM_LOAD_CAMPAIGN (game_InstantAction)...\n");
            PostGameMessage(FM_LOAD_CAMPAIGN, 0, game_InstantAction);
        }

        // Render frame
        render_frame();
        frameCount++;

        // FPS counter - only print every 5 seconds to reduce spam
        if (currentTime - lastFPSTime >= 5000) {
            fpsReportCounter++;
            // Only print periodically, don't use \r which causes terminal issues
            if (fpsReportCounter <= 3) {
                fprintf(stderr, "FPS: %u (avg over 5 sec), doUI=%d, gMainHandler=%p\n",
                       frameCount / 5, doUI, (void*)gMainHandler);
            }
            frameCount = 0;
            lastFPSTime = currentTime;
        }

        // Automatic UI screen test
        // Tests that UI screens can load correctly
        // Note: Only tests ONE screen per run since clicking a button changes the active screen
        static bool screenTestDone = false;
        // Automatic UI tests disabled - use manual testing
        // (The test code was automatically clicking Setup button after 3 seconds)

        // Auto view cycling during -test-ia for cockpit panel testing
        // Tests all cockpit panels: front(1100), left(600), right(700), down(100)
        if (g_testInstantActionFlag && !doUI) {
            static Uint32 simStartTime = 0;
            static int viewPhase = 0;
            if (simStartTime == 0) simStartTime = currentTime;
            Uint32 simElapsed = currentTime - simStartTime;

            struct ViewTestStep {
                Uint32 timeMs;
                int viewMode;     // -1=no change, 0=HUD, 1=cockpit
                int panel;        // -1=no change, panel ID otherwise
                const char* screenshotFile; // NULL=no screenshot
                const char* desc;
            };
            static const ViewTestStep steps[] = {
                {  2000, 1, 1100, NULL,                       "Cockpit front panel" },
            };
            static const int numSteps = sizeof(steps) / sizeof(steps[0]);

            if (viewPhase < numSteps && simElapsed >= steps[viewPhase].timeMs) {
                const ViewTestStep& s = steps[viewPhase];
                fprintf(stderr, "[VIEW_TEST] Phase %d: %s\n", viewPhase, s.desc);
                if (s.viewMode >= 0) g_requestedViewMode = s.viewMode;
                if (s.panel >= 0) g_requestedPanel = s.panel;
                if (s.screenshotFile) {
                    g_screenshotFilename = s.screenshotFile;
                    g_screenshotRequest = 1;
                }
                viewPhase++;
            }

            // Auto-exit after 15 seconds to test the exit flow
            static bool autoExitTriggered = false;
            if (simElapsed >= 120000 && !autoExitTriggered) {
                autoExitTriggered = true;
                fprintf(stderr, "[AUTO_TEST] Triggering auto-exit after 15 seconds...\n");
                fflush(stderr);
                extern int endAbort;
                endAbort = 0;  // Normal exit (not abort) - go back to UI
                SimulationLoopControl::StopGraphics();
                fprintf(stderr, "[AUTO_TEST] StopGraphics() called\n");
                fflush(stderr);
            }
        }

        // FF_LINUX debug: scripted view changes + screenshots via
        // FF_LINUX (AVIONICS-1): both scripted clocks below (FF_VIEW_SCRIPT and
        // FF_SIM_KEY) are documented as "seconds after entering sim mode", but each
        // used to start on the FIRST !doUI frame -- and there is one of those at
        // startup, BEFORE the UI comes up. So they really ran from process start,
        // and with the standard click track sim entry is ~55s in, meaning every
        // scripted time below 55 fired at once on the first sim frame. This latches
        // the UI -> sim transition so both clocks can key off it.
        static bool s_sawUI = false;

        if (doUI)
            s_sawUI = true;

        // FF_VIEW_SCRIPT="<mode>@sec;s@sec;..." where <mode> is a view-mode
        // number (0=HUD 1=2Dpit 2=chase 3=orbit 4=virtual pit) or 's' for a
        // screenshot (written to /tmp/ff_view_<N>.bmp). For visual testing.
        if (!doUI) {
            static int s_vsInit = 0;
            static struct { int mode; Uint32 atMs; int fired; } s_vs[16];
            static int s_nVs = 0;
            static Uint32 s_vsStart = 0;
            static char s_shotName[32];
            if (!s_vsInit) {
                s_vsInit = 1;
                const char* e = getenv("FF_VIEW_SCRIPT");
                if (e) {
                    char buf[256];
                    strncpy(buf, e, sizeof(buf) - 1); buf[sizeof(buf) - 1] = 0;
                    for (char* tok = strtok(buf, ";"); tok && s_nVs < 16; tok = strtok(NULL, ";")) {
                        float at; int mode; char c;
                        if (sscanf(tok, "s@%f", &at) == 1) {
                            s_vs[s_nVs].mode = -2;  // screenshot
                            s_vs[s_nVs].atMs = (Uint32)(at * 1000.0f);
                            s_vs[s_nVs].fired = 0;
                            s_nVs++;
                        } else if (sscanf(tok, "%d@%f%c", &mode, &at, &c) >= 2) {
                            s_vs[s_nVs].mode = mode;
                            s_vs[s_nVs].atMs = (Uint32)(at * 1000.0f);
                            s_vs[s_nVs].fired = 0;
                            s_nVs++;
                        }
                    }
                    fprintf(stderr, "[FF_VIEW_SCRIPT] parsed %d steps\n", s_nVs);
                }
            }
            if (s_nVs) {
                // FF_LINUX (AVIONICS-1): same sim-entry latch as FF_SIM_KEY -- this
                // clock also used to start on the pre-UI !doUI frame, so view steps
                // scheduled before ~55s all fired at once on the first sim frame.
                static bool s_vsLatched = false;

                if (s_sawUI && !s_vsLatched)
                {
                    s_vsLatched = true;
                    s_vsStart = SDL_GetTicks();
                }

                if (!s_vsStart) s_vsStart = SDL_GetTicks();
                Uint32 el = SDL_GetTicks() - s_vsStart;
                static int s_shotIdx = 0;
                for (int vi = 0; vi < s_nVs; vi++) {
                    if (!s_vs[vi].fired && el >= s_vs[vi].atMs) {
                        s_vs[vi].fired = 1;
                        if (s_vs[vi].mode == -2) {
                            snprintf(s_shotName, sizeof(s_shotName), "/tmp/ff_view_%d.bmp", s_shotIdx++);
                            g_screenshotFilename = s_shotName;
                            g_screenshotRequest = 1;
                            fprintf(stderr, "[FF_VIEW_SCRIPT] screenshot -> %s at %ums\n", s_shotName, el);
                        } else {
                            g_requestedViewMode = s_vs[vi].mode;
                            fprintf(stderr, "[FF_VIEW_SCRIPT] view mode %d at %ums\n", s_vs[vi].mode, el);
                        }
                    }
                }
            }
        }

        // FF_LINUX (NVG-2): FF_TEST_NVG="<sec>[,<sec>...]" toggles night-vision at
        // those times (clock runs from program start, like FF_VIEW_SCRIPT). NVG
        // is the only path that exercises D3DTOP_ADDSMOOTH, so without a way to
        // turn it on from the harness the op cannot be observed at all.
        if (!doUI) {
            static int s_nvgInit = 0;
            static Uint32 s_nvgAt[8];
            static int s_nvgFired[8];
            static int s_nNvg = 0;
            static Uint32 s_nvgStart = 0;

            if (!s_nvgInit) {
                s_nvgInit = 1;
                const char* e = getenv("FF_TEST_NVG");

                if (e) {
                    char buf[128];
                    strncpy(buf, e, sizeof(buf) - 1); buf[sizeof(buf) - 1] = 0;

                    for (char* tok = strtok(buf, ","); tok && s_nNvg < 8; tok = strtok(NULL, ",")) {
                        float at = 0.0f;

                        if (sscanf(tok, "%f", &at) == 1) {
                            s_nvgAt[s_nNvg] = (Uint32)(at * 1000.0f);
                            s_nvgFired[s_nNvg] = 0;
                            s_nNvg++;
                        }
                    }

                    fprintf(stderr, "[FF_TEST_NVG] parsed %d toggles\n", s_nNvg);
                }
            }

            if (s_nNvg) {
                if (!s_nvgStart) s_nvgStart = SDL_GetTicks();

                Uint32 el = SDL_GetTicks() - s_nvgStart;

                for (int ni = 0; ni < s_nNvg; ni++) {
                    if (!s_nvgFired[ni] && el >= s_nvgAt[ni]) {
                        s_nvgFired[ni] = 1;
                        g_requestedNVGToggle = 1;
                        fprintf(stderr, "[FF_TEST_NVG] requesting toggle at %ums\n", el);
                    }
                }
            }
        }

        // FF_LINUX: sim-mode (3D) frame capture, the counterpart of FF_UI_SCREENSHOT.
        //   FF_SIM_SCREENSHOT="<sec>[:<path>];<sec>[:<path>];..."
        // Each entry requests one capture <sec> seconds after the sim starts
        // rendering; the default path is /tmp/ff_sim_<N>.bmp.
        //
        // The capture itself deliberately does NOT happen here: this is the MAIN
        // thread, and in sim mode the SIM thread owns the GL context (see
        // FF_SimThreadAcquireGL). A glReadPixels from here reads a context this
        // thread does not have current -> the historical "white frame" / driver
        // SIGSEGV. Instead we only raise a request flag; the sim thread services it
        // inside its own swap path (ImageBuffer::SwapBuffers, immediately before
        // FF_SwapBuffers/SDL_GL_SwapWindow), i.e. on the context-owning thread with
        // the frame complete. Same mechanism the FF_VIEW_SCRIPT 's' action uses.
        if (!doUI) {
            static int s_ssInit = 0;
            static struct { Uint32 atMs; int fired; char path[128]; } s_ss[16];
            static int s_nSs = 0;
            static Uint32 s_ssStart = 0;
            if (!s_ssInit) {
                s_ssInit = 1;
                const char* e = getenv("FF_SIM_SCREENSHOT");
                if (e) {
                    char buf[512];
                    strncpy(buf, e, sizeof(buf) - 1); buf[sizeof(buf) - 1] = 0;
                    int idx = 0;
                    for (char* tok = strtok(buf, ";"); tok && s_nSs < 16; tok = strtok(NULL, ";")) {
                        float at = 0.0f; char p[128];
                        p[0] = 0;
                        if (sscanf(tok, "%f:%127s", &at, p) >= 1) {
                            s_ss[s_nSs].atMs = (Uint32)(at * 1000.0f);
                            s_ss[s_nSs].fired = 0;
                            if (p[0]) snprintf(s_ss[s_nSs].path, sizeof(s_ss[s_nSs].path), "%s", p);
                            else snprintf(s_ss[s_nSs].path, sizeof(s_ss[s_nSs].path), "/tmp/ff_sim_%d.bmp", idx);
                            idx++; s_nSs++;
                        }
                    }
                    fprintf(stderr, "[FF_SIM_SCREENSHOT] parsed %d capture(s)\n", s_nSs);
                }
            }
            if (s_nSs) {
                if (!s_ssStart) s_ssStart = SDL_GetTicks();
                Uint32 el = SDL_GetTicks() - s_ssStart;
                for (int si = 0; si < s_nSs; si++) {
                    // Serialise: only arm the next request once the sim thread has
                    // consumed the previous one (g_screenshotRequest back to 0).
                    if (!s_ss[si].fired && el >= s_ss[si].atMs && !g_screenshotRequest) {
                        s_ss[si].fired = 1;
                        g_screenshotFilename = s_ss[si].path;
                        g_screenshotRequest = 1;
                        fprintf(stderr, "[FF_SIM_SCREENSHOT] requested %s at %ums\n", s_ss[si].path, el);
                        fflush(stderr);
                    }
                }
            }
        }

        // FF_LINUX: scripted COCKPIT clicks via FF_SIM_CLICK="x,y@sec[+holdms];..."
        // (sim-mode coords, i.e. the pit's own 0..DispWidth/DispHeight space).
        //
        // The pit has no equivalent of FF_UI_CLICK, which meant every cockpit-button
        // defect had to be reproduced by hand by the PO. Driving it from outside
        // does not work: sim mode holds an SDL relative-mouse grab, so synthetic
        // pointer warps (xdotool) never become cockpit clicks. So place the sim's
        // own cursor and push button events into the same DirectInput buffer the
        // real mouse feeds, which exercises the whole dispatch path.
        if (!doUI) {
            static int s_clickInit = 0;
            static struct { int x, y; Uint32 atMs, holdMs; int phase; Uint32 downAt; } s_clicks[16];
            static int s_nClicks = 0;
            static Uint32 s_clickStart = 0;

            if (!s_clickInit) {
                s_clickInit = 1;
                const char* e = getenv("FF_SIM_CLICK");
                if (e) {
                    const char* p2 = e;
                    while (*p2 && s_nClicks < 16) {
                        int cx = 0, cy = 0; float at = 0.0f; unsigned hold = 120;
                        if (sscanf(p2, "%d,%d@%f+%u", &cx, &cy, &at, &hold) >= 3) {
                            s_clicks[s_nClicks].x = cx;
                            s_clicks[s_nClicks].y = cy;
                            s_clicks[s_nClicks].atMs = (Uint32)(at * 1000.0f);
                            s_clicks[s_nClicks].holdMs = hold;
                            s_clicks[s_nClicks].phase = 0;
                            s_nClicks++;
                        }
                        const char* semi = strchr(p2, ';');
                        if (!semi) break;
                        p2 = semi + 1;
                    }
                    fprintf(stderr, "[FF_SIM_CLICK] parsed %d click(s)\n", s_nClicks);
                    fflush(stderr);
                }
            }

            if (s_nClicks) {
                extern int gxPos, gyPos;

                // FF_LINUX (AVIONICS-1): latch the clock on the UI -> sim transition,
                // exactly as FF_SIM_KEY (276522ff) and FF_VIEW_SCRIPT needed. This is
                // the THIRD copy of the same defect: the clock started on the first
                // !doUI frame, and there is one of those at startup BEFORE the UI, so
                // with the standard click track (sim entry ~55s) every click
                // scheduled below 55 fired at once on the first sim frame. A cockpit
                // click track was therefore never actually sequenced -- which is a
                // good reason this harness was never used successfully to reach an
                // MFD page.
                static bool s_clickLatched = false;

                if (s_sawUI && !s_clickLatched) {
                    s_clickLatched = true;
                    s_clickStart = SDL_GetTicks();
                    fprintf(stderr, "[FF_SIM_CLICK] sim entry latched at %ums; times are relative to this\n",
                            s_clickStart);
                    fflush(stderr);
                }

                if (!s_clickStart) s_clickStart = SDL_GetTicks();
                Uint32 el = SDL_GetTicks() - s_clickStart;
                for (int ci = 0; ci < s_nClicks; ci++) {
                    if (s_clicks[ci].phase == 0 && el >= s_clicks[ci].atMs) {
                        s_clicks[ci].phase = 1;
                        s_clicks[ci].downAt = el;
                        gxPos = s_clicks[ci].x;
                        gyPos = s_clicks[ci].y;
                        fprintf(stderr, "[FF_SIM_CLICK] DOWN (%d,%d) at %ums (wall %.1fs)\n",
                                gxPos, gyPos, el, SDL_GetTicks() / 1000.0);
                        fflush(stderr);
                        FF_PushMouseEvent(DIMOFS_BUTTON0, 0x80);
                    } else if (s_clicks[ci].phase == 1 && el >= s_clicks[ci].downAt + s_clicks[ci].holdMs) {
                        s_clicks[ci].phase = 2;
                        gxPos = s_clicks[ci].x;
                        gyPos = s_clicks[ci].y;
                        fprintf(stderr, "[FF_SIM_CLICK] UP   (%d,%d) at %ums\n", gxPos, gyPos, el);
                        fflush(stderr);
                        FF_PushMouseEvent(DIMOFS_BUTTON0, 0x00);
                    }
                }
            }
        }

        // FF_LINUX debug: scripted sim key injection via FF_SIM_KEY="dik@sec[+holdms];..."
        // dik = DirectInput key code (decimal or 0x-hex, e.g. 57 or 0x39 = SPACE),
        // sec = seconds after entering sim mode, holdms = hold duration (default 250).
        // Pushes events into the same buffer real SDL key presses use (FF_PushKeyEvent),
        // so the full sim input path is exercised. For automated testing of issue #14.
        if (!doUI) {
            static int s_keyInit = 0;
            static bool s_simEntryLatched = false;
            static struct { int dik; int mods[3]; int nMods; Uint32 atMs; Uint32 holdMs; int phase; Uint32 downAt; } s_keys[16];
            static int s_nKeys = 0;
            static Uint32 s_simKeyStart = 0;
            if (!s_keyInit) {
                s_keyInit = 1;
                const char* e = getenv("FF_SIM_KEY");
                if (e) {
                    char buf[256];
                    strncpy(buf, e, sizeof(buf) - 1); buf[sizeof(buf) - 1] = 0;
                    for (char* tok = strtok(buf, ";"); tok && s_nKeys < 16; tok = strtok(NULL, ";")) {
                        unsigned dik; float at; unsigned hold = 250;
                        // FF_LINUX (AVIONICS-1): optional modifier prefix S/C/A.
                        // config/keystrokes.key gives every binding a modifier
                        // column, and bindings SHARE a DIK across modifiers --
                        // 0x3C is SimRadarAGModeStep at modifier 0 and
                        // SimHUDPower at modifier 5. Sending a bare DIK could
                        // only ever reach the modifier-0 half of the key map.
                        // The sim derives its modifier state from seeing
                        // DIK_LSHIFT/LCONTROL/LMENU in the same event stream
                        // (sikeybd.cpp ShiftCount/CtrlCount/AltCount), so a
                        // modifier just needs to be held around the key.
                        const char* kt = tok;
                        int mods[3]; int nMods = 0;

                        while (*kt == ' ') kt++;

                        // Prefixes STACK, e.g. "SA0x3C@60" = shift+alt. They have to:
                        // in config/keystrokes.key the modifier column is a bitmask
                        // (SHIFT 1, CTRL 2, ALT 4) and the combinations 3/5/6/7 hold
                        // 109 of the 275 bindings. Modifier 0 is only 53 of them, so
                        // a bare-DIK harness could reach under a fifth of the key map.
                        while (nMods < 3) {
                            if (*kt == 'S' || *kt == 's') mods[nMods++] = 0x2A;       // DIK_LSHIFT
                            else if (*kt == 'C' || *kt == 'c') mods[nMods++] = 0x1D;  // DIK_LCONTROL
                            else if (*kt == 'A' || *kt == 'a') mods[nMods++] = 0x38;  // DIK_LMENU
                            else break;
                            kt++;
                        }

                        if (sscanf(kt, "%i@%f+%u", &dik, &at, &hold) >= 2) {
                            s_keys[s_nKeys].dik = (int)dik;
                            s_keys[s_nKeys].nMods = nMods;
                            for (int mi = 0; mi < nMods; mi++) s_keys[s_nKeys].mods[mi] = mods[mi];
                            s_keys[s_nKeys].atMs = (Uint32)(at * 1000.0f);
                            s_keys[s_nKeys].holdMs = hold;
                            s_keys[s_nKeys].phase = 0;
                            s_nKeys++;
                        }
                    }
                    fprintf(stderr, "[FF_SIM_KEY] parsed %d key events\n", s_nKeys);
                }
            }
            if (s_nKeys) {
                // Re-latch when the UI hands over to the sim. If a run never shows a
                // UI at all, fall back to the first sim frame so the old behaviour
                // still holds for UI-less harnesses.
                if (s_sawUI && !s_simEntryLatched) {
                    s_simEntryLatched = true;
                    s_simKeyStart = SDL_GetTicks();
                    fprintf(stderr, "[FF_SIM_KEY] sim entry latched at %ums; times are relative to this\n",
                            s_simKeyStart);
                }

                if (!s_simKeyStart) s_simKeyStart = SDL_GetTicks();
                Uint32 el = SDL_GetTicks() - s_simKeyStart;
                for (int ki = 0; ki < s_nKeys; ki++) {
                    if (s_keys[ki].phase == 0 && el >= s_keys[ki].atMs) {
                        s_keys[ki].phase = 1;
                        s_keys[ki].downAt = el;
                        fprintf(stderr, "[FF_SIM_KEY] DOWN dik=0x%02x nmods=%d at %ums\n",
                                s_keys[ki].dik, s_keys[ki].nMods, el);

                        // modifiers go down BEFORE the key so ShiftCount/CtrlCount/
                        // AltCount are already raised when the key is dispatched
                        for (int mi = 0; mi < s_keys[ki].nMods; mi++)
                            FF_PushKeyEvent(s_keys[ki].mods[mi], true);

                        FF_PushKeyEvent(s_keys[ki].dik, true);
                    } else if (s_keys[ki].phase == 1 && el >= s_keys[ki].downAt + s_keys[ki].holdMs) {
                        s_keys[ki].phase = 2;
                        fprintf(stderr, "[FF_SIM_KEY] UP   dik=0x%02x nmods=%d at %ums\n",
                                s_keys[ki].dik, s_keys[ki].nMods, el);
                        FF_PushKeyEvent(s_keys[ki].dik, false);

                        // released AFTER, so the counts never drop early
                        for (int mi = 0; mi < s_keys[ki].nMods; mi++)
                            FF_PushKeyEvent(s_keys[ki].mods[mi], false);
                    }
                }
            }
        }

        // FF_LINUX (SESS-4): re-assert the POV hat from live SDL state every
        // frame. SDL only reports the hat on CHANGE, but DirectInput -- which the
        // sim was written against -- was polled, so IO.povHatAngle held its value
        // for as long as the hat was pressed. Here it was written once per edge
        // and then cleared again (IO.ResetAllInputs zeroes every POV), so a HELD
        // hat produced at most a single frame of pan instead of continuous
        // movement. Measured with FF_VIRTUAL_JOYSTICK + FF_SIM_HAT: the event
        // arrives and is accepted (hatEvents=1 accepted=1) yet povHatAngle reads
        // -1 again 800ms later. Re-asserting each frame makes it level-driven.
        if (g_SDLJoystick && g_JoystickNumHats > 0) {
            for (int h = 0; h < g_JoystickNumHats && h < SIMLIB_MAX_POV; h++) {
                IO.povHatAngle[h] = ConvertSDLHatToPOV(SDL_JoystickGetHat(g_SDLJoystick, h));
            }
        }

        // FF_LINUX debug (SESS-4): scripted POV-hat input via
        // FF_SIM_HAT="dir@sec[+holdms];..." where dir is one of
        // c(entre) u d l r ul ur dl dr, sec is seconds after entering sim mode and
        // holdms defaults to 1000. Requires FF_VIRTUAL_JOYSTICK=1 (or a real
        // stick's index 0 being virtual). Setting the virtual hat makes SDL emit a
        // genuine SDL_JOYHATMOTION, so this drives the real
        // event -> IO.povHatAngle -> ProcessJoyButtonAndPOVHat path.
        if (!doUI) {
            static int s_hatInit = 0;
            static struct { Uint8 val; Uint32 atMs; Uint32 holdMs; int phase; Uint32 downAt; int verified; } s_hats[16];
            static int s_nHats = 0;
            static Uint32 s_hatStart = 0;

            if (!s_hatInit) {
                s_hatInit = 1;
                const char* e = getenv("FF_SIM_HAT");
                if (e) {
                    char buf[256];
                    strncpy(buf, e, sizeof(buf) - 1); buf[sizeof(buf) - 1] = 0;
                    for (char* tok = strtok(buf, ";"); tok && s_nHats < 16; tok = strtok(NULL, ";")) {
                        char dir[8] = {0}; float at = 0; unsigned hold = 1000;
                        if (sscanf(tok, "%7[a-z]@%f+%u", dir, &at, &hold) >= 2) {
                            Uint8 v = SDL_HAT_CENTERED;
                            if (!strcmp(dir, "u"))       v = SDL_HAT_UP;
                            else if (!strcmp(dir, "d"))  v = SDL_HAT_DOWN;
                            else if (!strcmp(dir, "l"))  v = SDL_HAT_LEFT;
                            else if (!strcmp(dir, "r"))  v = SDL_HAT_RIGHT;
                            else if (!strcmp(dir, "ul")) v = SDL_HAT_LEFTUP;
                            else if (!strcmp(dir, "ur")) v = SDL_HAT_RIGHTUP;
                            else if (!strcmp(dir, "dl")) v = SDL_HAT_LEFTDOWN;
                            else if (!strcmp(dir, "dr")) v = SDL_HAT_RIGHTDOWN;
                            s_hats[s_nHats].val = v;
                            s_hats[s_nHats].atMs = (Uint32)(at * 1000.0f);
                            s_hats[s_nHats].holdMs = hold;
                            s_hats[s_nHats].phase = 0;
                            s_hats[s_nHats].verified = 0;
                            s_nHats++;
                        }
                    }
                    fprintf(stderr, "[FF_SIM_HAT] parsed %d hat events\n", s_nHats);
                    fflush(stderr);
                }
            }

            if (s_nHats && g_SDLJoystick) {
                if (!s_hatStart) s_hatStart = SDL_GetTicks();
                Uint32 el = SDL_GetTicks() - s_hatStart;
                for (int hi = 0; hi < s_nHats; hi++) {
                    if (s_hats[hi].phase == 0 && el >= s_hats[hi].atMs) {
                        s_hats[hi].phase = 1;
                        s_hats[hi].downAt = el;
                        SDL_JoystickSetVirtualHat(g_SDLJoystick, 0, s_hats[hi].val);
                        fprintf(stderr, "[FF_SIM_HAT] SET hat=0x%02x at %ums\n", s_hats[hi].val, el);
                        fflush(stderr);
                    } else if (s_hats[hi].phase == 1 && el >= s_hats[hi].downAt + 800 &&
                               !s_hats[hi].verified) {
                        // Read back the value the SDL event path actually
                        // deposited, ~800ms after the hat was set. This is the
                        // thing SESS-4 is about: NumberOfPOVs must be non-zero
                        // (ProcessJoyButtonAndPOVHat loops over it) and
                        // IO.povHatAngle[0] must carry the POV angle.
                        extern unsigned int NumberOfPOVs;
                        s_hats[hi].verified = 1;
                        fprintf(stderr, "[FF_SIM_HAT] VERIFY NumberOfPOVs=%u IO.povHatAngle[0]=%d "
                                "hatEvents=%d accepted=%d lastWhich=%d (g_JoystickIndex=%d) lastVal=0x%02x\n",
                                NumberOfPOVs, (int)IO.povHatAngle[0],
                                g_hatEventCount, g_hatAcceptedCount,
                                g_hatLastWhich, g_JoystickIndex, g_hatLastValue);
                        fflush(stderr);
                    } else if (s_hats[hi].phase == 1 && el >= s_hats[hi].downAt + s_hats[hi].holdMs) {
                        s_hats[hi].phase = 2;
                        SDL_JoystickSetVirtualHat(g_SDLJoystick, 0, SDL_HAT_CENTERED);
                        fprintf(stderr, "[FF_SIM_HAT] CENTRE at %ums\n", el);
                        fflush(stderr);
                    }
                }
            }
        }

        // Simple frame rate limiting
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < targetFrameTime) {
            SDL_Delay(targetFrameTime - frameTime);
        }
    }

    fprintf(stderr, "\n[main_loop] Exiting main loop...\n");
    fflush(stderr);
}

int main(int argc, char** argv) {
    const char* dataDir = DEFAULT_DATA_DIR;
    bool fullscreen = false;
    bool enableSound = true;

    printf("========================================\n");
    printf("%s %s\n", FREE_FALCON_BRAND, FREE_FALCON_VERSION);
    printf("%s\n", FREE_FALCON_PROJECT);
    printf("Build: %s %s\n", __DATE__, __TIME__);
    printf("========================================\n\n");

    // Set up signal handlers for clean window cleanup on kill
    setup_signal_handlers();

    // Note: X11 error handling is handled by SDL2 internally.
    // GLX context errors are caught by SDL and don't cause crashes.

    // Check for data directory environment variable
    const char* envDataDir = getenv("FF_DATA_DIR");
    if (envDataDir) {
        dataDir = envDataDir;
    }

    // Parse command line arguments
    bool testInstantAction = false;  // Auto-launch instant action for testing
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            dataDir = argv[++i];
        } else if (strcmp(argv[i], "-f") == 0) {
            fullscreen = true;
        } else if (strcmp(argv[i], "-w") == 0) {
            fullscreen = false;
        } else if (strcmp(argv[i], "-nosound") == 0) {
            enableSound = false;
        } else if (strcmp(argv[i], "-test-ia") == 0 || strcmp(argv[i], "--test-instant-action") == 0) {
            testInstantAction = true;
            fprintf(stderr, "[TEST] Auto-launch Instant Action enabled (3 second delay)\n");
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }

    // Export testInstantAction flag for use in main loop
    extern bool g_testInstantActionFlag;
    g_testInstantActionFlag = testInstantAction;

    // Initialize data directory
    if (!init_data_directory(dataDir)) {
        fprintf(stderr, "\nFailed to initialize data directory.\n");
        fprintf(stderr, "Make sure the game data is at: %s\n", dataDir);
        fprintf(stderr, "Or specify a different path with: %s -d /path/to/FreeFalcon6\n", argv[0]);
        fprintf(stderr, "You can also set the FF_DATA_DIR environment variable.\n");
        return 1;
    }

    // Change to data directory
    if (chdir(FalconDataDirectory) != 0) {
        fprintf(stderr, "Warning: Could not change to data directory\n");
    }

    // Initialize game paths
    fprintf(stderr, "[TRACE] About to call init_game_paths\n"); fflush(stderr);
    if (!init_game_paths()) {
        fprintf(stderr, "Failed to set up game paths\n");
        return 1;
    }
    fprintf(stderr, "[TRACE] init_game_paths completed\n"); fflush(stderr);

    // Initialize resource manager
    fprintf(stderr, "[TRACE] About to call init_resource_manager\n"); fflush(stderr);
    if (!init_resource_manager()) {
        fprintf(stderr, "Warning: Resource manager initialization issues\n");
    }
    fprintf(stderr, "[TRACE] init_resource_manager completed\n"); fflush(stderr);

    // Initialize SDL2
    fprintf(stderr, "[TRACE] About to call init_sdl\n"); fflush(stderr);
    if (!init_sdl(fullscreen)) {
        fprintf(stderr, "Failed to initialize SDL2\n");
        return 1;
    }
    fprintf(stderr, "[TRACE] init_sdl completed\n"); fflush(stderr);

    // Initialize OpenGL
    fprintf(stderr, "[TRACE] About to call init_opengl\n"); fflush(stderr);
    if (!init_opengl()) {
        fprintf(stderr, "Failed to initialize OpenGL\n");
        cleanup();
        return 1;
    }
    fprintf(stderr, "[TRACE] init_opengl completed\n"); fflush(stderr);

    // Initialize OpenAL (optional - continue if fails)
    if (enableSound) {
        init_openal();
    }

    // Initialize game core systems first (needed for DXEngine)
    fprintf(stderr, "[main] Calling init_game_core...\n");
    if (!init_game_core()) {
        fprintf(stderr, "Warning: Some game systems failed to initialize\n");
        // Continue anyway to show the window
    }
    fprintf(stderr, "[main] init_game_core() returned\n");

    // Initialize D3D7-to-OpenGL graphics layer (after game core is ready)
    fprintf(stderr, "[main] Calling init_d3d_graphics...\n");
    if (!init_d3d_graphics()) {
        fprintf(stderr, "Failed to initialize D3D graphics layer\n");
        // Continue with test rendering
    }
    fprintf(stderr, "[main] init_d3d_graphics() returned\n");

    fprintf(stderr, "\n========================================\n");
    fprintf(stderr, "Initialization complete!\n");
    fprintf(stderr, "========================================\n");

    // Run main loop
    fprintf(stderr, "[main] Entering main_loop...\n");
    main_loop();

    // Cleanup
    cleanup();

    printf("Goodbye!\n");
    fflush(NULL);

    // FF_LINUX: Skip static destructors. After cleanup() everything
    // meaningful is flushed; the remaining global C++ destructors run in
    // arbitrary order over half-torn-down state (~TimeManager warns,
    // ~FarTexDB asserts, static texture DBs Release() D3D7Surfaces whose
    // deferred-GL-delete vectors are already destroyed -> "double free or
    // corruption" SIGABRT). Same exit-time bug class as ~TurbulanceList
    // (issue #6). _exit() ends the process cleanly without running them.
    _exit(0);
}

// FFViper Linux main entry point
// Creates SDL2 window, OpenGL context, and initializes the game

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <time.h>
#include <fenv.h>
#include <signal.h>

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

// Default data directory - can be overridden with -d flag or env var
#define DEFAULT_DATA_DIR "/home/g/ese/SAT/WP/drive_c/FreeFalcon6"

// Window settings - must match UI resolution (1024x768 for HiRes UI)
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define WINDOW_TITLE "Free Falcon 6 Linux Port"

// External globals from falclib
extern char FalconDataDirectory[];
extern char FalconCampaignSaveDirectory[];
extern char FalconCampUserSaveDirectory[];
extern char FalconTerrainDataDir[];
extern char FalconMiscTexDataDir[];
extern char FalconPictureDirectory[];
extern char FalconObjectDataDir[];
extern char Falcon3DDataDir[];

// Global SDL objects - these replace Windows HWND etc.
SDL_Window* g_SDLWindow = nullptr;
SDL_GLContext g_GLContext = nullptr;

// OpenAL
static ALCdevice* g_alDevice = nullptr;
static ALCcontext* g_alContext = nullptr;

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

// These globals are defined in ui/src/winmain.cpp - use extern
extern HWND mainAppWnd;
extern HWND mainMenuWnd;
extern HINSTANCE hInst;
extern const char* FREE_FALCON_BRAND;
extern const char* FREE_FALCON_PROJECT;
extern const char* FREE_FALCON_VERSION;

// Message queue for Windows-style message passing
#include <queue>
#include <mutex>
#include <vector>

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
// FALLBACK MENU SYSTEM
// Simple OpenGL-based menu when UI95 rendering isn't working
// =============================================================================
static bool g_useFallbackMenu = true;  // Enable fallback menu by default
static Uint32 g_menuActiveTime = 0;    // Time when menu became active (to ignore phantom clicks)

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
    // Ignore clicks in the first 2 seconds to avoid phantom clicks on window focus
    Uint32 now = SDL_GetTicks();
    if (g_menuActiveTime == 0) {
        g_menuActiveTime = now;
    }
    if (now - g_menuActiveTime < 2000) {
        fprintf(stderr, "[FallbackMenu] Ignoring early click (menu active for %u ms)\n", now - g_menuActiveTime);
        fflush(stderr);
        return;
    }

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

// Fallback menu callbacks
static void FallbackExit() {
    fprintf(stderr, "[FallbackMenu] EXIT selected - shutting down\n");
    fflush(stderr);
    g_running = false;
}

static void FallbackDogfight() {
    fprintf(stderr, "[FallbackMenu] DOGFIGHT selected\n");
    // TODO: Implement dogfight mode
}

static void FallbackCampaign() {
    fprintf(stderr, "[FallbackMenu] CAMPAIGN selected\n");
    // TODO: Implement campaign mode
}

static void FallbackSetup() {
    fprintf(stderr, "[FallbackMenu] SETUP selected\n");
    // TODO: Implement setup screen
}

static void FallbackComms() {
    fprintf(stderr, "[FallbackMenu] COMMS selected\n");
    // TODO: Implement comms/multiplayer
}

static void FallbackACMI() {
    fprintf(stderr, "[FallbackMenu] ACMI selected\n");
    // TODO: Implement ACMI viewer
}

static void FallbackLogbook() {
    fprintf(stderr, "[FallbackMenu] LOGBOOK selected\n");
    // TODO: Implement logbook
}

static void FallbackInstantAction() {
    fprintf(stderr, "[FallbackMenu] INSTANT ACTION selected\n");
    // TODO: Implement instant action
}

// =============================================================================
// END FALLBACK MENU SYSTEM
// =============================================================================

// Signal handler for clean shutdown when process is killed
static volatile sig_atomic_t g_signalReceived = 0;

static void signal_handler(int sig) {
    g_signalReceived = sig;
    g_running = false;

    // For immediate cleanup on signal, directly destroy the SDL window
    // This ensures the window disappears even if the main loop doesn't get to cleanup()
    if (g_GLContext) {
        SDL_GL_DeleteContext(g_GLContext);
        g_GLContext = nullptr;
    }
    if (g_SDLWindow) {
        SDL_DestroyWindow(g_SDLWindow);
        g_SDLWindow = nullptr;
    }
    SDL_Quit();

    // Re-raise the signal with default handler so the process terminates properly
    signal(sig, SIG_DFL);
    raise(sig);
}

static void setup_signal_handlers() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGTERM, &sa, nullptr);  // Terminate (default from kill)
    sigaction(SIGINT, &sa, nullptr);   // Interrupt (Ctrl+C)
    sigaction(SIGHUP, &sa, nullptr);   // Hangup
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
    snprintf(FalconCampaignSaveDirectory, _MAX_PATH, "%s/campaign/save", FalconDataDirectory);
    snprintf(FalconCampUserSaveDirectory, _MAX_PATH, "%s/campaign/save", FalconDataDirectory);
    snprintf(FalconTerrainDataDir, _MAX_PATH, "%s/terrdata", FalconDataDirectory);
    snprintf(FalconMiscTexDataDir, _MAX_PATH, "%s/terrdata/misctex", FalconDataDirectory);
    snprintf(FalconPictureDirectory, _MAX_PATH, "%s/pictures", FalconDataDirectory);
    snprintf(FalconObjectDataDir, _MAX_PATH, "%s/terrdata/objects", FalconDataDirectory);
    snprintf(Falcon3DDataDir, _MAX_PATH, "%s/terrdata/objects", FalconDataDirectory);

    // Create picture directory if it doesn't exist
    mkdir(FalconPictureDirectory, 0755);

    printf("  Campaign saves: %s\n", FalconCampaignSaveDirectory);
    printf("  Terrain data: %s\n", FalconTerrainDataDir);
    printf("  Object data: %s\n", FalconObjectDataDir);

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

    // Set OpenGL attributes - conservative settings for compatibility
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);   // 16-bit depth is more compatible
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);  // No stencil for now
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);     // 16-bit color (565)

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
    return true;
}

static bool init_opengl(void) {
    printf("Initializing OpenGL...\n");

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

    // Initialize the DeviceManager (enumerates display modes and D3D devices)
    printf("  Initializing DeviceManager...\n");
    FalconDisplay.Setup(0);  // Language number 0 for English

    // Initialize memory pools
    printf("  Initializing simulation memory pools...\n");
    SimDriver.InitializeSimMemoryPools();

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

    // Initialize time manager (needed for time of day, weather, etc.)
    fprintf(stderr, "  Initializing time manager...\n");
    TheTimeManager.Setup(2004, 300);  // Year 2004, day 300 (late October)
    fprintf(stderr, "  [main_linux] TheTimeManager.Setup() returned\n");

    // Start sound system
    fprintf(stderr, "  Starting sound system...\n");
    F4SoundStart();
    fprintf(stderr, "  [main_linux] F4SoundStart() returned\n");

    // Start simulation loop
    fprintf(stderr, "  Starting simulation loop...\n");
    SimulationLoopControl::StartSim();
    fprintf(stderr, "  [main_linux] SimulationLoopControl::StartSim() returned\n");

    // Initialize campaign
    fprintf(stderr, "  Initializing campaign system...\n");
    Camp_Init(1);
    fprintf(stderr, "  [main_linux] Camp_Init() returned\n");

    // Build ASCII key mappings
    fprintf(stderr, "  Building key mappings...\n");
    BuildAscii();
    fprintf(stderr, "  [main_linux] BuildAscii() returned\n");

    fprintf(stderr, "  Game core initialization complete.\n");
    g_gameInitialized = true;

    return true;
}

static void cleanup(void) {
    fprintf(stderr, "\n[cleanup] Starting cleanup...\n");
    fflush(stderr);

    // First, destroy the window immediately so it disappears
    // This provides visual feedback that the app is shutting down
    fprintf(stderr, "[cleanup] Destroying SDL window first for immediate visual feedback...\n");
    fflush(stderr);

    if (g_GLContext) {
        SDL_GL_DeleteContext(g_GLContext);
        g_GLContext = nullptr;
    }
    if (g_SDLWindow) {
        SDL_DestroyWindow(g_SDLWindow);
        g_SDLWindow = nullptr;
    }
    fprintf(stderr, "[cleanup] Window destroyed.\n");
    fflush(stderr);

    // Cleanup game systems (in reverse order of initialization)
    // These may block, but at least the window is gone
    if (g_gameInitialized) {
        fprintf(stderr, "[cleanup] Shutting down game systems...\n");
        fflush(stderr);

        // Stop campaign - this can block on threading issues
        fprintf(stderr, "[cleanup] Stopping campaign (may take a moment)...\n");
        fflush(stderr);
        Camp_Exit();
        fprintf(stderr, "[cleanup] Campaign stopped.\n");
        fflush(stderr);

        // Only stop simulation loop if we were actually in simulation mode
        // StopSim() has a blocking wait for RunningSim state that will hang
        // if we're just in UI mode (which uses the fallback menu)
        if (SimulationLoopControl::InSim()) {
            fprintf(stderr, "[cleanup] Stopping simulation loop...\n");
            fflush(stderr);
            SimulationLoopControl::StopSim();
            fprintf(stderr, "[cleanup] Simulation loop stopped.\n");
            fflush(stderr);
        } else {
            fprintf(stderr, "[cleanup] Skipping StopSim (not in simulation mode)\n");
            fflush(stderr);
        }

        // Cleanup particle system
        fprintf(stderr, "[cleanup] Unloading particle system...\n");
        fflush(stderr);
        DrawableParticleSys::UnloadParameters();
        fprintf(stderr, "[cleanup] Particle system unloaded.\n");
        fflush(stderr);
    }

    // Cleanup D3D/DXEngine
    if (g_graphicsInitialized) {
        fprintf(stderr, "[cleanup] Releasing DXEngine...\n");
        fflush(stderr);
        TheDXEngine.Release();
        g_graphicsInitialized = false;
        fprintf(stderr, "[cleanup] DXEngine released.\n");
        fflush(stderr);
    }

    // Release D3D interfaces
    fprintf(stderr, "[cleanup] Releasing D3D interfaces...\n");
    fflush(stderr);
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
    fprintf(stderr, "[cleanup] D3D interfaces released.\n");
    fflush(stderr);

    // Cleanup audio
    fprintf(stderr, "[cleanup] Cleaning up audio...\n");
    fflush(stderr);
    if (g_alContext) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(g_alContext);
        g_alContext = nullptr;
    }
    if (g_alDevice) {
        alcCloseDevice(g_alDevice);
        g_alDevice = nullptr;
    }
    fprintf(stderr, "[cleanup] Audio cleaned up.\n");
    fflush(stderr);

    // Cleanup SDL
    fprintf(stderr, "[cleanup] Quitting SDL...\n");
    fflush(stderr);
    SDL_Quit();
    fprintf(stderr, "[cleanup] SDL quit.\n");
    fflush(stderr);

    // Cleanup resource manager
    fprintf(stderr, "[cleanup] Cleaning up resource manager...\n");
    fflush(stderr);
    ResExit();
    fprintf(stderr, "[cleanup] Resource manager cleaned up.\n");
    fflush(stderr);

    fprintf(stderr, "[cleanup] Cleanup complete!\n");
    fflush(stderr);
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
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    g_running = false;
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
                // Post keyboard message
                PostGameMessage(WM_KEYDOWN, event.key.keysym.scancode, 0);
                break;

            case SDL_KEYUP:
                PostGameMessage(WM_KEYUP, event.key.keysym.scancode, 0);
                break;

            case SDL_MOUSEBUTTONDOWN:
                {
                    int x = event.button.x;
                    int y = event.button.y;
                    // Handle fallback menu clicks
                    if (g_useFallbackMenu && doUI && event.button.button == SDL_BUTTON_LEFT) {
                        HandleFallbackMenuClick(x, y);
                        // Check if we need to exit immediately after click
                        if (!g_running) {
                            fprintf(stderr, "[SDL_EVENT] g_running=false after button click, returning from event loop\n");
                            fflush(stderr);
                            return;  // Exit event handling immediately
                        }
                    } else {
                        // Scale mouse coordinates for UI95 system
                        int scaledX = x * 1024 / WINDOW_WIDTH;
                        int scaledY = y * 768 / WINDOW_HEIGHT;
                        if (event.button.button == SDL_BUTTON_LEFT) {
                            PostGameMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(scaledX, scaledY));
                        } else if (event.button.button == SDL_BUTTON_RIGHT) {
                            PostGameMessage(WM_RBUTTONDOWN, 0, MAKELPARAM(scaledX, scaledY));
                        }
                    }
                }
                break;

            case SDL_MOUSEBUTTONUP:
                {
                    int x = event.button.x;
                    int y = event.button.y;
                    if (!g_useFallbackMenu || !doUI) {
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
                    // Handle fallback menu hover
                    if (g_useFallbackMenu && doUI) {
                        HandleFallbackMenuHover(x, y);
                    } else {
                        int scaledX = x * 1024 / WINDOW_WIDTH;
                        int scaledY = y * 768 / WINDOW_HEIGHT;
                        PostGameMessage(WM_MOUSEMOVE, 0, MAKELPARAM(scaledX, scaledY));
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
                    default:
                        break;
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
                TheCampaign.Suspend();
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
                if (gMainHandler != nullptr) {
                    gMainHandler->ProcessUserCallbacks();
                    // On Linux, manually trigger a full screen refresh
                    // This compensates for the lack of Windows paint messages
                    UI95_RECT fullRect = { 0, 0, gMainHandler->GetW(), gMainHandler->GetH() };
                    gMainHandler->RefreshAll(&fullRect);
                    // Call Update() to actually draw the windows to the surface
                    // Note: RefreshAll->SetUpdateRect should have set UpdateFlag |= C_DRAW_REFRESH
                    gMainHandler->Update();
                }
                break;

            case FM_EXIT_GAME:
                fprintf(stderr, "[FM] FM_EXIT_GAME received\n");
                g_running = false;
                return false;

            // FM_DISP_ENTER_MODE is not needed on Linux - EnterMode directly calls _EnterMode
            // FM_DISP_LEAVE_MODE is also not needed on Linux

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
                    if (lbuCount <= 5) {
                        fprintf(stderr, "[Mouse] LBUTTONUP at (%d, %d)\n",
                                LOWORD(msg.lParam), HIWORD(msg.lParam));
                    }
                    gMainHandler->EventHandler(NULL, msg.message, msg.wParam, msg.lParam);
                }
                break;
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
        // Present the DirectDraw primary surface (2D UI)
        FF_PresentPrimarySurface();
    } else if (g_graphicsInitialized && g_pD3DDevice) {
        // 3D mode - use Direct3D rendering
        g_pD3DDevice->BeginScene();

        // Clear color and depth buffer
        g_pD3DDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xFF203040, 1.0f, 0);

        // Flush the DXEngine (draws all queued objects)
        if (g_Use_DX_Engine) {
            TheDXEngine.FlushBuffers();
        }

        // End scene
        g_pD3DDevice->EndScene();
    } else {
        // Fallback: simple test pattern if graphics not initialized
        glClear(GL_COLOR_BUFFER_BIT);

        glBegin(GL_TRIANGLES);
            glColor3f(1.0f, 0.0f, 0.0f);   // Red
            glVertex2f(-0.5f, -0.5f);
            glColor3f(0.0f, 1.0f, 0.0f);   // Green
            glVertex2f(0.5f, -0.5f);
            glColor3f(0.0f, 0.0f, 1.0f);   // Blue
            glVertex2f(0.0f, 0.5f);
        glEnd();

        glFlush();
    }

    SDL_GL_SwapWindow(g_SDLWindow);
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

    // Check for data directory environment variable
    const char* envDataDir = getenv("FF_DATA_DIR");
    if (envDataDir) {
        dataDir = envDataDir;
    }

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            dataDir = argv[++i];
        } else if (strcmp(argv[i], "-f") == 0) {
            fullscreen = true;
        } else if (strcmp(argv[i], "-w") == 0) {
            fullscreen = false;
        } else if (strcmp(argv[i], "-nosound") == 0) {
            enableSound = false;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }

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
    if (!init_game_paths()) {
        fprintf(stderr, "Failed to set up game paths\n");
        return 1;
    }

    // Initialize resource manager
    if (!init_resource_manager()) {
        fprintf(stderr, "Warning: Resource manager initialization issues\n");
    }

    // Initialize SDL2
    if (!init_sdl(fullscreen)) {
        fprintf(stderr, "Failed to initialize SDL2\n");
        return 1;
    }

    // Initialize OpenGL
    if (!init_opengl()) {
        fprintf(stderr, "Failed to initialize OpenGL\n");
        cleanup();
        return 1;
    }

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
    return 0;
}

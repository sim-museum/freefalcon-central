// FFViper Linux main entry point
// Creates SDL2 window, OpenGL context, and initializes the game

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <time.h>
#include <fenv.h>

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

// Default data directory - can be overridden with -d flag or env var
#define DEFAULT_DATA_DIR "/home/g/ese/SAT/WP/drive_c/FreeFalcon6"

// Window settings - conservative defaults for stability
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
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

// Forward declarations
static void print_usage(const char* progname);
static bool init_data_directory(const char* dataDir);
static bool init_resource_manager(void);
static bool init_sdl(bool fullscreen);
static bool init_opengl(void);
static bool init_openal(void);
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
    snprintf(FalconObjectDataDir, _MAX_PATH, "%s/objects", FalconDataDirectory);
    snprintf(Falcon3DDataDir, _MAX_PATH, "%s/objects", FalconDataDirectory);

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

// Initialize core game systems (simplified for initial bringup)
static bool init_game_core(void) {
    printf("\n--- Initializing Game Core Systems ---\n");

    // Set FPU rounding mode to truncate (equivalent to Windows _controlfp)
    fesetround(FE_TOWARDZERO);

    // Seed random number generator
    srand((unsigned int)time(NULL));

    // Load class table (entity definitions)
    printf("  Loading class table...\n");
    InitClassTableAndData((char*)"Falcon4", (char*)"objects");

    // Note: InitVU() causes assertion spam without proper entity setup.
    // The VU system needs entities to be fully loaded first.
    // TODO: Investigate proper entity loading sequence

    // Initialize threading
    printf("  Setting up thread manager...\n");
    ThreadManager::setup();

    // Note: SimulationLoopControl::StartSim() and Camp_Init() are commented out
    // until we have proper UI initialization - they require the message system
    // to be functional and cause assertion spam without proper setup.
    //
    // TODO: Need to properly initialize:
    // - Theater system (LoadTheaterList, SetNewTheater)
    // - Sound system (F4SoundStart)
    // - Simulation loop (SimulationLoopControl::StartSim)
    // - Campaign system (Camp_Init)

    printf("  Game core initialization complete.\n");
    g_gameInitialized = true;

    return true;
}

static void cleanup(void) {
    printf("\nCleaning up...\n");

    // Cleanup game systems (in reverse order of initialization)
    if (g_gameInitialized) {
        printf("  Shutting down game systems...\n");
        // Add cleanup calls here as systems are integrated
    }

    // Cleanup audio
    if (g_alContext) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(g_alContext);
        g_alContext = nullptr;
    }
    if (g_alDevice) {
        alcCloseDevice(g_alDevice);
        g_alDevice = nullptr;
    }

    // Cleanup graphics
    if (g_GLContext) {
        SDL_GL_DeleteContext(g_GLContext);
        g_GLContext = nullptr;
    }
    if (g_SDLWindow) {
        SDL_DestroyWindow(g_SDLWindow);
        g_SDLWindow = nullptr;
    }

    // Cleanup SDL
    SDL_Quit();

    // Cleanup resource manager
    ResExit();
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
                if (event.button.button == SDL_BUTTON_LEFT) {
                    PostGameMessage(WM_LBUTTONDOWN, 0, MAKELPARAM(event.button.x, event.button.y));
                } else if (event.button.button == SDL_BUTTON_RIGHT) {
                    PostGameMessage(WM_RBUTTONDOWN, 0, MAKELPARAM(event.button.x, event.button.y));
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    PostGameMessage(WM_LBUTTONUP, 0, MAKELPARAM(event.button.x, event.button.y));
                } else if (event.button.button == SDL_BUTTON_RIGHT) {
                    PostGameMessage(WM_RBUTTONUP, 0, MAKELPARAM(event.button.x, event.button.y));
                }
                break;

            case SDL_MOUSEMOTION:
                PostGameMessage(WM_MOUSEMOVE, 0, MAKELPARAM(event.motion.x, event.motion.y));
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

// Process game messages
bool ProcessGameMessages() {
    std::lock_guard<std::mutex> lock(g_messageMutex);

    while (!g_messageQueue.empty()) {
        GameMessage msg = g_messageQueue.front();
        g_messageQueue.pop();

        // Handle game-specific messages here
        // This will be expanded to handle FM_* messages
        switch (msg.message) {
            case WM_QUIT:
                return false;
            default:
                // Message not handled
                break;
        }
    }
    return true;
}

static void render_frame(void) {
    // Clear only color buffer for simple test
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw simple test triangle using immediate mode (most compatible)
    glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 0.0f, 0.0f);   // Red
        glVertex2f(-0.5f, -0.5f);
        glColor3f(0.0f, 1.0f, 0.0f);   // Green
        glVertex2f(0.5f, -0.5f);
        glColor3f(0.0f, 0.0f, 1.0f);   // Blue
        glVertex2f(0.0f, 0.5f);
    glEnd();

    // Ensure rendering is complete before swap
    glFlush();

    SDL_GL_SwapWindow(g_SDLWindow);
}

static void main_loop(void) {
    printf("\n========================================\n");
    printf("Entering main loop\n");
    printf("  ESC = quit, Click X to close\n");
    printf("========================================\n\n");

    Uint32 frameCount = 0;
    Uint32 lastFPSTime = SDL_GetTicks();
    const Uint32 targetFrameTime = 16;  // ~60 FPS cap
    int fpsReportCounter = 0;

    while (g_running) {
        Uint32 frameStart = SDL_GetTicks();

        // Handle SDL events and convert to game messages
        handle_sdl_events();

        // Process game message queue
        if (!ProcessGameMessages()) {
            break;
        }

        // Render frame
        render_frame();
        frameCount++;

        // FPS counter - only print every 5 seconds to reduce spam
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastFPSTime >= 5000) {
            fpsReportCounter++;
            // Only print periodically, don't use \r which causes terminal issues
            if (fpsReportCounter <= 3) {
                printf("FPS: %u (avg over 5 sec)\n", frameCount / 5);
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

    printf("\nExiting main loop...\n");
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

    // Initialize game core systems
    if (!init_game_core()) {
        fprintf(stderr, "Warning: Some game systems failed to initialize\n");
        // Continue anyway to show the window
    }

    printf("\n========================================\n");
    printf("Initialization complete!\n");
    printf("========================================\n");

    // Run main loop
    main_loop();

    // Cleanup
    cleanup();

    printf("Goodbye!\n");
    return 0;
}

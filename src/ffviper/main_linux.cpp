// FFViper Linux main entry point
// This initializes the game data directory and attempts basic initialization

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

// FreeFalcon headers
#include "f4find.h"
#include "codelib/resources/reslib/src/resmgr.h"

// Default data directory - can be overridden with -d flag
#define DEFAULT_DATA_DIR "/home/g/ese/SAT/WP/drive_c/FreeFalcon6"

// External globals from falclib
extern char FalconDataDirectory[];
extern char FalconCampaignSaveDirectory[];
extern char FalconCampUserSaveDirectory[];
extern char FalconTerrainDataDir[];
extern char FalconMiscTexDataDir[];
extern char FalconPictureDirectory[];

static void print_usage(const char* progname) {
    printf("Usage: %s [options]\n", progname);
    printf("Options:\n");
    printf("  -d <path>    Set game data directory\n");
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

    // Set up derived paths
    snprintf(FalconCampaignSaveDirectory, _MAX_PATH, "%s/campaign/save", FalconDataDirectory);
    snprintf(FalconCampUserSaveDirectory, _MAX_PATH, "%s/campaign/save", FalconDataDirectory);
    snprintf(FalconTerrainDataDir, _MAX_PATH, "%s/terrdata", FalconDataDirectory);
    snprintf(FalconMiscTexDataDir, _MAX_PATH, "%s/terrdata/misctex", FalconDataDirectory);
    snprintf(FalconPictureDirectory, _MAX_PATH, "%s/pictures", FalconDataDirectory);

    return true;
}

static bool init_resource_manager(void) {
    printf("Initializing resource manager...\n");

    // Initialize the resource manager with the data directory
    if (!ResInit(FalconDataDirectory)) {
        fprintf(stderr, "Warning: Resource manager initialization returned false\n");
        // Continue anyway - some things may still work
    }

    // Try to attach the main zip file
    char zipPath[_MAX_PATH];
    snprintf(zipPath, _MAX_PATH, "%s/Zips/art.zip", FalconDataDirectory);

    if (access(zipPath, R_OK) == 0) {
        printf("Found art.zip at: %s\n", zipPath);
    } else {
        printf("Note: art.zip not found (may use loose files)\n");
    }

    return true;
}

int main(int argc, char** argv) {
    const char* dataDir = DEFAULT_DATA_DIR;

    printf("========================================\n");
    printf("FreeFalcon Linux Port\n");
    printf("Build: %s %s\n", __DATE__, __TIME__);
    printf("========================================\n\n");

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            dataDir = argv[++i];
        } else if (strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }

    // Initialize data directory
    if (!init_data_directory(dataDir)) {
        fprintf(stderr, "\nFailed to initialize data directory.\n");
        fprintf(stderr, "Make sure the game data is at: %s\n", dataDir);
        fprintf(stderr, "Or specify a different path with: %s -d /path/to/FreeFalcon6\n", argv[0]);
        return 1;
    }

    // Change to data directory
    if (chdir(FalconDataDirectory) != 0) {
        fprintf(stderr, "Warning: Could not change to data directory\n");
    }

    // Initialize resource manager
    if (!init_resource_manager()) {
        fprintf(stderr, "Warning: Resource manager initialization issues\n");
    }

    // Check for key data files
    printf("\nChecking for key data files:\n");

    const char* checkFiles[] = {
        "ffviper.cfg",
        "theater.lst",
        "TacRefDB.bin",
        "terrdata",
        "art",
        "sounds",
        "campaign",
        NULL
    };

    char checkPath[_MAX_PATH];
    for (int i = 0; checkFiles[i] != NULL; i++) {
        snprintf(checkPath, _MAX_PATH, "%s/%s", FalconDataDirectory, checkFiles[i]);
        if (access(checkPath, R_OK) == 0) {
            printf("  [OK] %s\n", checkFiles[i]);
        } else {
            printf("  [MISSING] %s\n", checkFiles[i]);
        }
    }

    printf("\n");
    printf("========================================\n");
    printf("Initialization complete.\n");
    printf("========================================\n");
    printf("\n");
    printf("Next steps to make the game playable:\n");
    printf("  1. Implement SDL2 window creation\n");
    printf("  2. Initialize OpenGL rendering context\n");
    printf("  3. Set up OpenAL for audio\n");
    printf("  4. Connect input handling\n");
    printf("  5. Call into game main loop\n");
    printf("\n");

    return 0;
}

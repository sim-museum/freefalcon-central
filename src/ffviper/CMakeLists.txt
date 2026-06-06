
# FF_LINUX: stamp the build with the git hash so the window title shows
# exactly which version is running (avoids version-skew confusion)
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE FF_GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
if(NOT FF_GIT_HASH)
    set(FF_GIT_HASH "unknown")
endif()
add_compile_definitions(FF_GIT_HASH="${FF_GIT_HASH}")
# FFViper - Main executable
# Note: The main source files are in ui library (winmain.cpp, f4config.cpp)
# This creates the executable by linking all the libraries together

# Create the main entry point
if(FF_PLATFORM_LINUX)
    # Linux main entry point
    set(FFVIPER_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/main_linux.cpp)
else()
    # Windows uses WinMain from ui library
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/main_stub.cpp
"// FFViper main stub - actual main is in ui library (winmain.cpp)
// This file exists to create the executable target
extern int main_entry();
int main(int argc, char** argv) { (void)argc; (void)argv; return 0; }
")
    set(FFVIPER_MAIN ${CMAKE_CURRENT_BINARY_DIR}/main_stub.cpp)
endif()

add_executable(FFViper ${FFVIPER_MAIN})

# Link all the libraries
target_link_libraries(FFViper
    # UI contains the main entry point
    ui

    # Core libraries
    falclib
    vu2

    # Graphics
    3dlib
    bsplib
    ddstuff
    dxengine
    nvdxt
    objects
    renderer
    terrain
    texture
    gfx_utils
    weather

    # Sound
    falcsnd

    # Campaign
    camplib
    camptask
    campui
    campupd

    # Simulation
    aircraft
    airframe
    bomb
    cockpit
    digi
    displays
    eject
    faults
    fcc
    feature
    ground
    guns
    hdigi
    helo
    icp
    irst
    missile
    navaids
    otwdrive
    radar
    rwr
    simdrive
    siminput
    simlib
    simloop
    visual

    # UI components
    ui95
    ui95_ext

    # Other
    acmi
    comms
    crashhandler
    lists
    resmgr

    # System libraries (Linux)
    Threads::Threads
)

# Windows-only libraries
if(WIN32)
    target_link_libraries(FFViper
        camptool
        movie
        tools
    )
endif()

if(FF_PLATFORM_LINUX)
    # Circular dependencies between static libraries require the whole-archive approach
    # Use -Wl,--whole-archive and -Wl,--no-whole-archive to force inclusion of all symbols
    target_link_libraries(FFViper
        # Linker group start
        "-Wl,--start-group"
        # Compat must come first to provide stubs
        compat
        # UI
        ui
        ui95
        ui95_ext
        # Graphics
        objects
        renderer
        3dlib
        bsplib
        dxengine
        ddstuff
        texture
        terrain
        weather
        gfx_utils
        nvdxt
        # Core
        falclib
        vu2
        falcsnd
        # Campaign
        camplib
        camptask
        campui
        campupd
        comms
        # Simulation
        simlib
        otwdrive
        simdrive
        siminput
        simloop
        cockpit
        displays
        aircraft
        airframe
        ground
        helo
        feature
        bomb
        digi
        hdigi
        fcc
        guns
        missile
        radar
        rwr
        irst
        visual
        icp
        navaids
        eject
        faults
        # Other
        acmi
        crashhandler
        lists
        resmgr
        # Linker group end
        "-Wl,--end-group"
        # System libraries come after the group
        dl
        m
    )

    if(SDL2_FOUND)
        target_link_libraries(FFViper ${SDL2_LIBRARIES})
    endif()
    if(OPENGL_FOUND)
        target_link_libraries(FFViper ${OPENGL_LIBRARIES})
    endif()
    if(GLEW_FOUND)
        target_link_libraries(FFViper ${GLEW_LIBRARIES})
    endif()
    if(OPENAL_FOUND)
        target_link_libraries(FFViper ${OPENAL_LIBRARY})
    endif()
endif()

# Include directories
target_include_directories(FFViper PRIVATE
    ${FF_SRC_DIR}
    ${FF_SRC_DIR}/include
    ${FF_SRC_DIR}/falclib/include
    ${FF_SRC_DIR}/codelib/include
    ${FF_SRC_DIR}/vu2/include
    ${FF_SRC_DIR}/sim/include
    ${FF_SRC_DIR}/campaign/include
    ${FF_SRC_DIR}/ui/include
    ${FF_SRC_DIR}/graphics/include
    ${FF_SRC_DIR}/ui95
    ${FF_SRC_DIR}/acmi/src/include
)

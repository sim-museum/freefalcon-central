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
    target_link_libraries(FFViper dl m)

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
    ${FF_SRC_DIR}/falclib/include
)

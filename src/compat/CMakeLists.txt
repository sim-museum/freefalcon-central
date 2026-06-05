# compat - Linux compatibility layer (Windows API emulation)
add_library(compat STATIC
    linux_stubs_c.cpp
    linux_stubs.cpp
    d3d_gl.cpp
    openal_dsound.cpp
    win_only_stubs.cpp
    # FF_LINUX: DirectInput compatibility stubs (input handled by SDL2)
    dinput_stubs.cpp
    # FF_LINUX: LZSS compression used by campaign save files
    ${CMAKE_SOURCE_DIR}/src/utils/lzss.cpp
)

target_include_directories(compat PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src   # For utils/lzss.h
)

target_include_directories(compat PRIVATE
    ${FF_SRC_DIR}
    ${FF_SRC_DIR}/include
    ${FF_SRC_DIR}/falclib/include
    ${FF_SRC_DIR}/codelib/include
)

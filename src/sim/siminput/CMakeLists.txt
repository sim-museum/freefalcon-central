# siminput library (generated from siminput.vcxproj for Linux port)

set(SIMINPUT_SOURCES
    ascii.cpp
    commands.cpp
    findfunc.cpp
    mouselook.cpp
    radiocmds.cpp
    setupinp.cpp
    sicursor.cpp
    sidevice.cpp
    sierror.cpp
    sijoy.cpp
    sikeybd.cpp
    siloop.cpp
    simouse.cpp
    trackir.cpp
    wingcmds.cpp
)

add_library(siminput STATIC ${SIMINPUT_SOURCES})

target_include_directories(siminput PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
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

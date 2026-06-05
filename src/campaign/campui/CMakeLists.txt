# campui library (generated from campui.vcxproj for Linux port)

set(CAMPUI_SOURCES
    acselect.cpp
    brief.cpp
    campjoin.cpp
    campmap.cpp
    campstr.cpp
    cuievent.cpp
    evtparse.cpp
    misseval.cpp
    options.cpp
    pilot.cpp
    squadui.cpp
)

add_library(campui STATIC ${CAMPUI_SOURCES})

target_include_directories(campui PRIVATE
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

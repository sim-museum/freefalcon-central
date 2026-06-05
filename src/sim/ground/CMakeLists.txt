# ground library (generated from ground.vcxproj for Linux port)

set(GROUND_SOURCES
    campgrnd.cpp
    gndai.cpp
    gndapi.cpp
    grndmain.cpp
    target.cpp
    weapon.cpp
)

add_library(ground STATIC ${GROUND_SOURCES})

target_include_directories(ground PRIVATE
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

# texture library (generated from texture.vcxproj for Linux port)

set(TEXTURE_SOURCES
    fartex.cpp
    palette.cpp
    terrtex.cpp
    tex.cpp
)

add_library(texture STATIC ${TEXTURE_SOURCES})

target_include_directories(texture PRIVATE
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

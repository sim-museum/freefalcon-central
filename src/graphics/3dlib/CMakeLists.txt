# 3dlib library (generated from 3dlib.vcxproj for Linux port)

set(3DLIB_SOURCES
    alloc.c
    context.cpp
    fileio.cpp
    filemem.cpp
    image.cpp
    math.cpp
    radix.c
)

add_library(3dlib STATIC ${3DLIB_SOURCES})

target_include_directories(3dlib PRIVATE
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

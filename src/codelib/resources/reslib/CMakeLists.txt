# resmgr library (resource manager with zip support)
set(RESMGR_SOURCES
    src/resmgr.c
    src/unzip.c
    src/inflate.c
)

add_library(resmgr STATIC ${RESMGR_SOURCES})

target_include_directories(resmgr PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${FF_SRC_DIR}
    ${FF_SRC_DIR}/include
    ${FF_SRC_DIR}/falclib/include
    ${FF_SRC_DIR}/codelib/include
    ${FF_SRC_DIR}/codelib/tools/lists
)

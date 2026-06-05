# renderer library (generated from renderer.vcxproj for Linux port)

set(RENDERER_SOURCES
    canvas3d.cpp
    clip2d.cpp
    clip3d.cpp
    display.cpp
    gmcomposit.cpp
    gmradar.cpp
    mono2d.cpp
    otw.cpp
    otwcull.cpp
    otwdraw.cpp
    otwsky.cpp
    render2d.cpp
    render3d.cpp
    renderir.cpp
    rendernvg.cpp
    rendertv.cpp
    renderwire.cpp
    rviewpnt.cpp
)

add_library(renderer STATIC ${RENDERER_SOURCES})

target_include_directories(renderer PRIVATE
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

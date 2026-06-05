# objects library (generated from objects.vcxproj for Linux port)

set(OBJECTS_SOURCES
    draw2d.cpp
    drawbldg.cpp
    drawbrdg.cpp
    drawbsp.cpp
    drawcld.cpp
    drawgrnd.cpp
    drawguys.cpp
    drawovc.cpp
    drawparticlesys.cpp
    drawplat.cpp
    drawpnt.cpp
    drawpole.cpp
    drawpuff.cpp
    drawrdbd.cpp
    drawsgmt.cpp
    drawshdw.cpp
    drawtrcr.cpp
    drawtrcr_dx.cpp
    objlist.cpp
)

add_library(objects STATIC ${OBJECTS_SOURCES})

target_include_directories(objects PRIVATE
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

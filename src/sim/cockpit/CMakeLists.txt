# cockpit library (generated from cockpit.vcxproj for Linux port)

set(COCKPIT_SOURCES
    button.cpp
    cbbuttons.cpp
    cbdials.cpp
    cbdigits.cpp
    cbindicators.cpp
    cblights.cpp
    cbmachasi.cpp
    cbtext.cpp
    cpadi.cpp
    cpcb.cpp
    cpchev.cpp
    cpcursor.cpp
    cpded.cpp
    cpdial.cpp
    cpdigits.cpp
    cphsi.cpp
    cpindicator.cpp
    cpkneeview.cpp
    cplift.cpp
    cplight.cpp
    cpmachasi.cpp
    cpmanager.cpp
    cpmirror.cpp
    cpmisc.cpp
    cpobject.cpp
    cppanel.cpp
    cpsound.cpp
    cpsurface.cpp
    cptext.cpp
    cpvbounds.cpp
    kneeboard.cpp
    popcb.cpp
    popmenu.cpp
    stdafx.cpp
)

add_library(cockpit STATIC ${COCKPIT_SOURCES})

target_include_directories(cockpit PRIVATE
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

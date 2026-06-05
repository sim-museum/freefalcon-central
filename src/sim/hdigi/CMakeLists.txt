# hdigi library (generated from hdigi.vcxproj for Linux port)

set(HDIGI_SOURCES
    h_actions.cpp
    h_cavoid.cpp
    h_digimain.cpp
    h_dlogic.cpp
    h_engage.cpp
    h_flitlead.cpp
    h_gengage.cpp
    h_gunsjink.cpp
    h_landme.cpp
    h_mdefeat.cpp
    h_mengage.cpp
    h_mnvers.cpp
    h_randp.cpp
    h_sfusion.cpp
    h_waypoint.cpp
    h_wingman.cpp
)

add_library(hdigi STATIC ${HDIGI_SOURCES})

target_include_directories(hdigi PRIVATE
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

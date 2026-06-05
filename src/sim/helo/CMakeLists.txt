# helo library (generated from helo.vcxproj for Linux port)

set(HELO_SOURCES
    h_damage.cpp
    h_doweapon.cpp
    h_inputs.cpp
    helimain.cpp
    helimm.cpp
)

add_library(helo STATIC ${HELO_SOURCES})

target_include_directories(helo PRIVATE
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

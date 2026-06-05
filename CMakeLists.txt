# FreeFalcon - Linux port top-level build
cmake_minimum_required(VERSION 3.16)
project(FreeFalcon C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Platform detection
if(UNIX AND NOT APPLE)
    set(FF_PLATFORM_LINUX TRUE)
endif()

set(FF_SRC_DIR ${CMAKE_SOURCE_DIR}/src)

if(FF_PLATFORM_LINUX)
    add_compile_definitions(FF_LINUX _DEBUG NOMINMAX
        # The RV "new" particle system: without it PPN[] stores indices that
        # pointer-consuming code dereferences (missile-impact crash).
        USE_NEW_PS
        stricmp=strcasecmp strnicmp=strncasecmp strcmpi=strcasecmp)

    # The codebase is legacy 32-bit Windows C++; relax modern strictness
    add_compile_options(
        -fpermissive
        -fno-strict-aliasing
        -fcommon
        -w
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-write-strings>
    )

    # Compat layer provides windows.h, io.h, ddraw.h, etc. for all targets
    include_directories(${FF_SRC_DIR}/compat)

    find_package(Threads REQUIRED)

    # Locally-extracted dev packages (extern/) - runtime libs are system-installed,
    # except GLEW whose runtime also lives in extern (rpath'ed below).
    set(FF_EXTERN ${CMAKE_SOURCE_DIR}/extern/usr)
    set(FF_EXTERN_LIB ${FF_EXTERN}/lib/x86_64-linux-gnu)
    include_directories(${FF_EXTERN}/include ${FF_EXTERN}/include/x86_64-linux-gnu)
    link_directories(${FF_EXTERN_LIB})
    set(CMAKE_BUILD_RPATH ${FF_EXTERN_LIB})

    set(SDL2_FOUND TRUE)
    set(SDL2_LIBRARIES SDL2)
    set(OPENGL_FOUND TRUE)
    set(OPENGL_LIBRARIES GL)
    set(GLEW_FOUND TRUE)
    set(GLEW_LIBRARIES GLEW)
    set(OPENAL_FOUND TRUE)
    set(OPENAL_LIBRARY openal)
endif()

add_subdirectory(src)

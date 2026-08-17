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
        stricmp=strcasecmp strnicmp=strncasecmp strcmpi=strcasecmp
        # pthread_timedjoin_np (compat_winbase.h's timed WaitForSingleObject) is
        # a GNU extension, declared only when _GNU_SOURCE is set BEFORE the first
        # libc header. g++ defines it for C++ automatically, but the C sources
        # here (reslib) do not get it, so they were calling it with no prototype
        # at all. It has to be a compile definition rather than a #define in the
        # header, which by then is far too late.
        _GNU_SOURCE
        # _strdup was left undeclared, so C assumed it returns int and the
        # returned char* would be truncated on 64-bit before being stored.
        # Currently dormant (the only caller is dead-stripped at -O3, and the
        # symbol resolves nowhere -- it is not in glibc), but it is a link error
        # waiting for anyone who enables the profiler display.
        _strdup=strdup)

    # The codebase is legacy 32-bit Windows C++; relax modern strictness
    add_compile_options(
        -fpermissive
        -fno-strict-aliasing
        -fcommon
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-write-strings>
    )

    # FF_WARN=ON turns on a hand-picked set of diagnostics instead of the blanket
    # -w. Deliberately NOT the default: this is 800k lines of legacy code and a
    # full -Wall is unreadable noise. Every warning listed here detects a bug
    # class that has actually bitten this port and had to be found by hand:
    #
    #   sizeof-pointer-memaccess  memset(p, 0, sizeof(p)) on a new[] array (BOMB-1)
    #   mismatched-new-delete     new[] freed with scalar delete   (MSG-1, 21 of them)
    #   format / format-overflow  sprintf into fixed buffers from file data
    #   return-type               falling off a non-void function is UB
    #   uninitialized             the WAVEFORMATEX-from-a-failed-parse shape (SND-1)
    #   memset-elt-size / alloc-size-larger-than / free-nonheap-object
    #
    # Usage:  cmake -DFF_WARN=ON -B build-warn && ninja -C build-warn 2>&1 | tee /tmp/warn.log
    option(FF_WARN "Enable targeted bug-class diagnostics instead of -w" OFF)

    if(FF_WARN)
        add_compile_options(
            -Wsizeof-pointer-memaccess
            -Wmismatched-new-delete
            -Wformat=2
            -Wformat-overflow=2
            -Wreturn-type
            -Wuninitialized
            -Wmemset-elt-size
            -Walloc-size-larger-than=1073741824
            -Wfree-nonheap-object
            -Wno-format-nonliteral
            -Wno-format-y2k
        )
    else()
        add_compile_options(-w)
    endif()

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

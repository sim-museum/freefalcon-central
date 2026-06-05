/*
 * FreeFalcon Linux Port - C-linkage misc stubs
 *
 * Symbols required at link time that have no Linux equivalent or
 * are satisfied trivially. Extend as the linker demands.
 */

#ifdef FF_LINUX

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "compat_types.h"

/* Global ShiAssert support flags are defined in main_linux.cpp */

#endif /* FF_LINUX */

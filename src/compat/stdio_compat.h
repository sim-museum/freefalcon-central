/* FreeFalcon Linux Port - case-insensitive file helpers */
#ifndef FF_COMPAT_STDIO_COMPAT_H
#define FF_COMPAT_STDIO_COMPAT_H
#ifdef FF_LINUX
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Implemented in linux_stubs.cpp - case-insensitive path resolution */
FILE *fopen_nocase(const char *filepath, const char *mode);
int   open_nocase(const char *filepath, int flags, int mode);
#ifdef __cplusplus
}
#endif
#endif /* FF_LINUX */
#endif

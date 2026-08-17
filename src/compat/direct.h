/* FreeFalcon Linux Port - direct.h compatibility */
#ifndef FF_COMPAT_DIRECT_H
#define FF_COMPAT_DIRECT_H
#ifdef FF_LINUX
#include <unistd.h>
#include <sys/stat.h>
#define _getcwd getcwd
#define _chdir  chdir
#define _rmdir  rmdir
static inline int _mkdir(const char *path) { return mkdir(path, 0755); }
static inline int _chdrive(int drive) { (void)drive; return 0; }
static inline int _getdrive(void) { return 3; /* C: */ }

/* FF_LINUX: _getdcwd is defined in win_only_stubs.cpp but was not declared
 * anywhere, so resmgr.c (a C translation unit) fell back to the implicit int
 * return. It actually returns char*, and an implicit int return TRUNCATES a
 * 64-bit pointer -- the same trap the _strdup note in CMakeLists.txt describes.
 * Harmless at today's only call site, which discards the result, but it should
 * not be left waiting for the next caller. */
#ifdef __cplusplus
extern "C"
#endif
char *_getdcwd(int drive, char *buffer, int maxlen);
#endif
#endif

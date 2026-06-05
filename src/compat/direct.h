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
#endif
#endif

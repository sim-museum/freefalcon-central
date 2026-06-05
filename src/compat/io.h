/* FreeFalcon Linux Port - io.h compatibility (MSVC low-level I/O) */
#ifndef FF_COMPAT_IO_H
#define FF_COMPAT_IO_H
#ifdef FF_LINUX

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdio.h>

/* O_BINARY / O_TEXT don't exist on Linux */
#ifndef O_BINARY
#define O_BINARY 0
#endif
#ifndef O_TEXT
#define O_TEXT 0
#endif
#define _O_BINARY     O_BINARY
#define _O_TEXT       O_TEXT
#define _O_RDONLY     O_RDONLY
#define _O_WRONLY     O_WRONLY
#define _O_RDWR       O_RDWR
#define _O_CREAT      O_CREAT
#define _O_TRUNC      O_TRUNC
#define _O_APPEND     O_APPEND
#define _O_EXCL       O_EXCL
#define _O_SEQUENTIAL 0
#define _O_RANDOM     0

#define _S_IREAD  S_IRUSR
#define _S_IWRITE S_IWUSR
#define _S_IFDIR  S_IFDIR
#define _S_IFREG  S_IFREG

#define _open  open
#define _close close
#define _read  read
#define _write write
#define _lseek lseek
#define _tell(fd) lseek((fd), 0, SEEK_CUR)
#define tell(fd)  lseek((fd), 0, SEEK_CUR)
#define _access access
#define _unlink unlink
#define _commit fsync
#define _fileno fileno
#define _isatty isatty
#define _dup    dup
#define _dup2   dup2
#define _chsize ftruncate

static inline long _filelength(int fd) {
    struct stat st;
    if (fstat(fd, &st) != 0) return -1;
    return (long)st.st_size;
}

static inline int eof(int fd) {
    off_t cur = lseek(fd, 0, SEEK_CUR);
    off_t end = lseek(fd, 0, SEEK_END);
    lseek(fd, cur, SEEK_SET);
    return cur >= end;
}
static inline int _eof(int fd) {
    off_t cur = lseek(fd, 0, SEEK_CUR);
    off_t end = lseek(fd, 0, SEEK_END);
    lseek(fd, cur, SEEK_SET);
    return cur >= end;
}

static inline int _setmode(int fd, int mode) { (void)fd; (void)mode; return O_BINARY; }

/* _findfirst family */
#define _A_NORMAL 0x00
#define _A_RDONLY 0x01
#define _A_HIDDEN 0x02
#define _A_SYSTEM 0x04
#define _A_SUBDIR 0x10
#define _A_ARCH   0x20

struct _finddata_t {
    unsigned   attrib;
    long       time_create;
    long       time_access;
    long       time_write;
    unsigned long size;
    char       name[260];
};

#ifdef __cplusplus
extern "C" {
#endif
/* Implemented in linux_stubs.cpp */
intptr_t _findfirst(const char *filespec, struct _finddata_t *fileinfo);
int _findnext(intptr_t handle, struct _finddata_t *fileinfo);
int _findclose(intptr_t handle);
void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext);
void _makepath(char *path, const char *drive, const char *dir, const char *fname, const char *ext);
char *_fullpath(char *absPath, const char *relPath, size_t maxLength);
#ifdef __cplusplus
}
#endif

#endif /* FF_LINUX */
#endif /* FF_COMPAT_IO_H */

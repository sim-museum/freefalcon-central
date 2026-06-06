/*
 * FreeFalcon Linux Port - winbase.h compatibility (kernel32 emulation)
 *
 * Threads, synchronization, file I/O, timing and misc kernel APIs
 * implemented over POSIX. HANDLEs are tagged heap structs so that
 * CloseHandle / WaitForSingleObject can dispatch on the handle kind.
 */

#ifndef FF_COMPAT_WINBASE_H
#define FF_COMPAT_WINBASE_H

#ifdef FF_LINUX

#include "compat_types.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
#include <ctype.h>
#include <stdarg.h>

/* Case-insensitive open helpers (linux_stubs.cpp) */
#ifdef __cplusplus
extern "C" {
#endif
FILE *fopen_nocase(const char *filepath, const char *mode);
int   open_nocase(const char *filepath, int flags, int mode);
#ifdef __cplusplus
}
#endif

/* ============================================================
 * Tagged handle structs
 * ============================================================ */
#define FF_HANDLE_TYPE_EVENT            0x46464531 /* 'FFE1' */
#define FF_HANDLE_TYPE_MUTEX            0x46464D31 /* 'FFM1' */
#define FF_HANDLE_TYPE_THREAD           0x46465431 /* 'FFT1' */
#define FF_HANDLE_TYPE_DETACHED_THREAD  0x46465432 /* 'FFT2' */
#define FF_HANDLE_TYPE_FILE             0x46464631 /* 'FFF1' */
#define FF_HANDLE_TYPE_FILEMAP          0x46464D50 /* 'FFMP' */

typedef struct {
    unsigned int    type;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    int             signaled;
    int             manualReset;
} FF_EVENT_HANDLE;

typedef struct {
    unsigned int    type;
    pthread_mutex_t mutex;
} FF_MUTEX_HANDLE;

typedef struct {
    unsigned int type;
    pthread_t    thread;
    int          joined;
} FF_THREAD_HANDLE;

typedef struct {
    unsigned int type;
    pthread_t    thread;
} FF_DETACHED_THREAD_HANDLE;

typedef struct {
    unsigned int type;
    int          fd;
} FF_FILE_HANDLE;

typedef struct {
    unsigned int type;
    int          fd;     /* dup'ed fd for mapping */
    size_t       size;
} FF_FILEMAP_HANDLE;

/* ============================================================
 * Wait constants
 * ============================================================ */
#define INFINITE        0xFFFFFFFF
#define WAIT_OBJECT_0   0x00000000
#define WAIT_ABANDONED  0x00000080
#define WAIT_TIMEOUT    0x00000102
#define WAIT_FAILED     0xFFFFFFFF

#define STILL_ACTIVE    259

/* ============================================================
 * Sleep / time
 * ============================================================ */
static inline void Sleep(DWORD dwMilliseconds) {
    struct timespec ts;
    ts.tv_sec = dwMilliseconds / 1000;
    ts.tv_nsec = (long)(dwMilliseconds % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}
static inline DWORD SleepEx(DWORD ms, BOOL bAlertable) { (void)bAlertable; Sleep(ms); return 0; }

static inline DWORD GetTickCount(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (DWORD)((uint64_t)ts.tv_sec * 1000u + (uint64_t)ts.tv_nsec / 1000000u);
}

static inline BOOL QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    if (lpPerformanceCount)
        lpPerformanceCount->QuadPart = (LONGLONG)ts.tv_sec * 1000000000LL + ts.tv_nsec;
    return TRUE;
}
static inline BOOL QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency) {
    if (lpFrequency) lpFrequency->QuadPart = 1000000000LL;
    return TRUE;
}

static inline void GetLocalTime(LPSYSTEMTIME lpSystemTime) {
    struct timeval tv;
    struct tm tmv;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &tmv);
    if (lpSystemTime) {
        lpSystemTime->wYear = (WORD)(tmv.tm_year + 1900);
        lpSystemTime->wMonth = (WORD)(tmv.tm_mon + 1);
        lpSystemTime->wDayOfWeek = (WORD)tmv.tm_wday;
        lpSystemTime->wDay = (WORD)tmv.tm_mday;
        lpSystemTime->wHour = (WORD)tmv.tm_hour;
        lpSystemTime->wMinute = (WORD)tmv.tm_min;
        lpSystemTime->wSecond = (WORD)tmv.tm_sec;
        lpSystemTime->wMilliseconds = (WORD)(tv.tv_usec / 1000);
    }
}
static inline void GetSystemTime(LPSYSTEMTIME lpSystemTime) {
    struct timeval tv;
    struct tm tmv;
    gettimeofday(&tv, NULL);
    gmtime_r(&tv.tv_sec, &tmv);
    if (lpSystemTime) {
        lpSystemTime->wYear = (WORD)(tmv.tm_year + 1900);
        lpSystemTime->wMonth = (WORD)(tmv.tm_mon + 1);
        lpSystemTime->wDayOfWeek = (WORD)tmv.tm_wday;
        lpSystemTime->wDay = (WORD)tmv.tm_mday;
        lpSystemTime->wHour = (WORD)tmv.tm_hour;
        lpSystemTime->wMinute = (WORD)tmv.tm_min;
        lpSystemTime->wSecond = (WORD)tmv.tm_sec;
        lpSystemTime->wMilliseconds = (WORD)(tv.tv_usec / 1000);
    }
}
static inline void GetSystemTimeAsFileTime(LPFILETIME lpft) {
    /* 100ns units since 1601-01-01 */
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t t = ((uint64_t)tv.tv_sec + 11644473600ULL) * 10000000ULL + (uint64_t)tv.tv_usec * 10ULL;
    if (lpft) {
        lpft->dwLowDateTime = (DWORD)(t & 0xFFFFFFFFu);
        lpft->dwHighDateTime = (DWORD)(t >> 32);
    }
}

/* mmsystem-style timers (also exposed via mmsystem.h) */
static inline DWORD timeGetTime(void) { return GetTickCount(); }
static inline UINT timeBeginPeriod(UINT uPeriod) { (void)uPeriod; return 0; }
static inline UINT timeEndPeriod(UINT uPeriod) { (void)uPeriod; return 0; }

/* ============================================================
 * Critical sections (recursive mutexes)
 * ============================================================ */
typedef struct _RTL_CRITICAL_SECTION {
    pthread_mutex_t mutex;
    int             initialized;
} CRITICAL_SECTION, *PCRITICAL_SECTION, *LPCRITICAL_SECTION;

static inline void InitializeCriticalSection(LPCRITICAL_SECTION cs) {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&cs->mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    cs->initialized = 1;
}
static inline BOOL InitializeCriticalSectionAndSpinCount(LPCRITICAL_SECTION cs, DWORD dwSpinCount) {
    (void)dwSpinCount;
    InitializeCriticalSection(cs);
    return TRUE;
}
static inline void EnterCriticalSection(LPCRITICAL_SECTION cs) {
    if (!cs->initialized) InitializeCriticalSection(cs);
    pthread_mutex_lock(&cs->mutex);
}
static inline BOOL TryEnterCriticalSection(LPCRITICAL_SECTION cs) {
    if (!cs->initialized) InitializeCriticalSection(cs);
    return pthread_mutex_trylock(&cs->mutex) == 0;
}
static inline void LeaveCriticalSection(LPCRITICAL_SECTION cs) {
    pthread_mutex_unlock(&cs->mutex);
}
static inline void DeleteCriticalSection(LPCRITICAL_SECTION cs) {
    if (cs->initialized) {
        pthread_mutex_destroy(&cs->mutex);
        cs->initialized = 0;
    }
}

/* ============================================================
 * Interlocked operations
 * ============================================================ */
static inline LONG InterlockedIncrement(volatile LONG *Addend) {
    return __sync_add_and_fetch(Addend, 1);
}
static inline LONG InterlockedDecrement(volatile LONG *Addend) {
    return __sync_sub_and_fetch(Addend, 1);
}
static inline LONG InterlockedExchange(volatile LONG *Target, LONG Value) {
    return __sync_lock_test_and_set(Target, Value);
}
static inline LONG InterlockedExchangeAdd(volatile LONG *Addend, LONG Value) {
    return __sync_fetch_and_add(Addend, Value);
}
static inline LONG InterlockedCompareExchange(volatile LONG *Destination, LONG Exchange, LONG Comperand) {
    return __sync_val_compare_and_swap(Destination, Comperand, Exchange);
}

/* ============================================================
 * Events
 * ============================================================ */
typedef struct _SECURITY_ATTRIBUTES {
    DWORD  nLength;
    LPVOID lpSecurityDescriptor;
    BOOL   bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

static inline HANDLE CreateEventA(LPSECURITY_ATTRIBUTES sa, BOOL bManualReset, BOOL bInitialState, LPCSTR lpName) {
    (void)sa; (void)lpName;
    FF_EVENT_HANDLE *ev = (FF_EVENT_HANDLE *)malloc(sizeof(FF_EVENT_HANDLE));
    if (!ev) return NULL;
    ev->type = FF_HANDLE_TYPE_EVENT;
    pthread_mutex_init(&ev->mutex, NULL);
    pthread_cond_init(&ev->cond, NULL);
    ev->signaled = bInitialState ? 1 : 0;
    ev->manualReset = bManualReset ? 1 : 0;
    return (HANDLE)ev;
}
#define CreateEvent CreateEventA

static inline BOOL SetEvent(HANDLE hEvent) {
    FF_EVENT_HANDLE *ev = (FF_EVENT_HANDLE *)hEvent;
    if (!ev || ev->type != FF_HANDLE_TYPE_EVENT) return FALSE;
    pthread_mutex_lock(&ev->mutex);
    ev->signaled = 1;
    pthread_cond_broadcast(&ev->cond);
    pthread_mutex_unlock(&ev->mutex);
    return TRUE;
}
static inline BOOL ResetEvent(HANDLE hEvent) {
    FF_EVENT_HANDLE *ev = (FF_EVENT_HANDLE *)hEvent;
    if (!ev || ev->type != FF_HANDLE_TYPE_EVENT) return FALSE;
    pthread_mutex_lock(&ev->mutex);
    ev->signaled = 0;
    pthread_mutex_unlock(&ev->mutex);
    return TRUE;
}
static inline BOOL PulseEvent(HANDLE hEvent) {
    FF_EVENT_HANDLE *ev = (FF_EVENT_HANDLE *)hEvent;
    if (!ev || ev->type != FF_HANDLE_TYPE_EVENT) return FALSE;
    pthread_mutex_lock(&ev->mutex);
    ev->signaled = 1;
    pthread_cond_broadcast(&ev->cond);
    ev->signaled = 0;
    pthread_mutex_unlock(&ev->mutex);
    return TRUE;
}

/* ============================================================
 * Mutexes
 * ============================================================ */
static inline HANDLE CreateMutexA(LPSECURITY_ATTRIBUTES sa, BOOL bInitialOwner, LPCSTR lpName) {
    (void)sa; (void)lpName;
    FF_MUTEX_HANDLE *m = (FF_MUTEX_HANDLE *)malloc(sizeof(FF_MUTEX_HANDLE));
    if (!m) return NULL;
    m->type = FF_HANDLE_TYPE_MUTEX;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&m->mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    if (bInitialOwner) pthread_mutex_lock(&m->mutex);
    return (HANDLE)m;
}
#define CreateMutex CreateMutexA

static inline BOOL ReleaseMutex(HANDLE hMutex) {
    FF_MUTEX_HANDLE *m = (FF_MUTEX_HANDLE *)hMutex;
    if (!m || m->type != FF_HANDLE_TYPE_MUTEX) return FALSE;
    pthread_mutex_unlock(&m->mutex);
    return TRUE;
}

/* ============================================================
 * Threads
 * ============================================================ */
typedef DWORD (WINAPI *LPTHREAD_START_ROUTINE)(LPVOID lpThreadParameter);

typedef struct {
    LPTHREAD_START_ROUTINE start;
    LPVOID arg;
} FF_THREAD_TRAMPOLINE_ARGS;

static inline void *FF_ThreadTrampoline(void *p) {
    FF_THREAD_TRAMPOLINE_ARGS a = *(FF_THREAD_TRAMPOLINE_ARGS *)p;
    free(p);
    a.start(a.arg);
    return NULL;
}

static inline HANDLE CreateThread(LPSECURITY_ATTRIBUTES sa, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress,
                                  LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId) {
    (void)sa; (void)dwStackSize; (void)dwCreationFlags;
    FF_THREAD_HANDLE *th = (FF_THREAD_HANDLE *)malloc(sizeof(FF_THREAD_HANDLE));
    if (!th) return NULL;
    th->type = FF_HANDLE_TYPE_THREAD;
    th->joined = 0;
    FF_THREAD_TRAMPOLINE_ARGS *ta = (FF_THREAD_TRAMPOLINE_ARGS *)malloc(sizeof(FF_THREAD_TRAMPOLINE_ARGS));
    ta->start = lpStartAddress;
    ta->arg = lpParameter;
    if (pthread_create(&th->thread, NULL, FF_ThreadTrampoline, ta) != 0) {
        free(ta);
        free(th);
        return NULL;
    }
    if (lpThreadId) *lpThreadId = (DWORD)(uintptr_t)th->thread;
    return (HANDLE)th;
}

static inline DWORD GetCurrentThreadId(void) { return (DWORD)(uintptr_t)pthread_self(); }
static inline DWORD GetCurrentProcessId(void) { return (DWORD)getpid(); }
static inline HANDLE GetCurrentThread(void) { return (HANDLE)(uintptr_t)-2; }
static inline HANDLE GetCurrentProcess(void) { return (HANDLE)(uintptr_t)-1; }
static inline void ExitThread(DWORD dwExitCode) { (void)dwExitCode; pthread_exit(NULL); }
static inline BOOL TerminateThread(HANDLE hThread, DWORD dwExitCode) { (void)hThread; (void)dwExitCode; return FALSE; }
static inline DWORD SuspendThread(HANDLE hThread) { (void)hThread; return (DWORD)-1; }
static inline DWORD ResumeThread(HANDLE hThread) { (void)hThread; return 0; }
static inline BOOL GetExitCodeThread(HANDLE hThread, LPDWORD lpExitCode) {
    (void)hThread;
    if (lpExitCode) *lpExitCode = 0;
    return TRUE;
}

#define THREAD_PRIORITY_IDLE          (-15)
#define THREAD_PRIORITY_LOWEST        (-2)
#define THREAD_PRIORITY_BELOW_NORMAL  (-1)
#define THREAD_PRIORITY_NORMAL        0
#define THREAD_PRIORITY_ABOVE_NORMAL  1
#define THREAD_PRIORITY_HIGHEST       2
#define THREAD_PRIORITY_TIME_CRITICAL 15

static inline BOOL SetThreadPriority(HANDLE hThread, int nPriority) { (void)hThread; (void)nPriority; return TRUE; }
static inline int GetThreadPriority(HANDLE hThread) { (void)hThread; return THREAD_PRIORITY_NORMAL; }
static inline DWORD_PTR SetThreadAffinityMask(HANDLE hThread, DWORD_PTR mask) { (void)hThread; (void)mask; return 1; }

#define CREATE_SUSPENDED 0x00000004

/* ============================================================
 * WaitForSingleObject / WaitForMultipleObjects
 * ============================================================ */
static inline DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) {
    if (!hHandle) return WAIT_FAILED;
    unsigned int type = *(unsigned int *)hHandle;

    if (type == FF_HANDLE_TYPE_EVENT) {
        FF_EVENT_HANDLE *ev = (FF_EVENT_HANDLE *)hHandle;
        pthread_mutex_lock(&ev->mutex);
        if (dwMilliseconds == INFINITE) {
            while (!ev->signaled)
                pthread_cond_wait(&ev->cond, &ev->mutex);
        } else {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += dwMilliseconds / 1000;
            ts.tv_nsec += (long)(dwMilliseconds % 1000) * 1000000L;
            if (ts.tv_nsec >= 1000000000L) { ts.tv_sec++; ts.tv_nsec -= 1000000000L; }
            while (!ev->signaled) {
                if (pthread_cond_timedwait(&ev->cond, &ev->mutex, &ts) == ETIMEDOUT) {
                    pthread_mutex_unlock(&ev->mutex);
                    return WAIT_TIMEOUT;
                }
            }
        }
        if (!ev->manualReset)
            ev->signaled = 0;
        pthread_mutex_unlock(&ev->mutex);
        return WAIT_OBJECT_0;
    }

    if (type == FF_HANDLE_TYPE_MUTEX) {
        FF_MUTEX_HANDLE *m = (FF_MUTEX_HANDLE *)hHandle;
        if (dwMilliseconds == INFINITE) {
            pthread_mutex_lock(&m->mutex);
            return WAIT_OBJECT_0;
        }
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += dwMilliseconds / 1000;
        ts.tv_nsec += (long)(dwMilliseconds % 1000) * 1000000L;
        if (ts.tv_nsec >= 1000000000L) { ts.tv_sec++; ts.tv_nsec -= 1000000000L; }
        if (pthread_mutex_timedlock(&m->mutex, &ts) == 0) return WAIT_OBJECT_0;
        return WAIT_TIMEOUT;
    }

    if (type == FF_HANDLE_TYPE_THREAD) {
        FF_THREAD_HANDLE *th = (FF_THREAD_HANDLE *)hHandle;
        /* Note: timed join ignored - join fully (the game waits for thread exit) */
        if (!th->joined) {
            pthread_join(th->thread, NULL);
            th->joined = 1;
        }
        return WAIT_OBJECT_0;
    }

    return WAIT_FAILED;
}

static inline DWORD WaitForMultipleObjects(DWORD nCount, const HANDLE *lpHandles, BOOL bWaitAll, DWORD dwMilliseconds) {
    /* Simple implementation: poll each handle */
    if (bWaitAll) {
        for (DWORD i = 0; i < nCount; i++)
            WaitForSingleObject(lpHandles[i], dwMilliseconds);
        return WAIT_OBJECT_0;
    }
    DWORD waited = 0;
    for (;;) {
        for (DWORD i = 0; i < nCount; i++) {
            if (WaitForSingleObject(lpHandles[i], 0) == WAIT_OBJECT_0)
                return WAIT_OBJECT_0 + i;
        }
        if (dwMilliseconds != INFINITE) {
            if (waited >= dwMilliseconds) return WAIT_TIMEOUT;
            waited += 5;
        }
        Sleep(5);
    }
}

/* ============================================================
 * CloseHandle - dispatches on handle tag
 * ============================================================ */
static inline BOOL CloseHandle(HANDLE hObject) {
    if (!hObject || hObject == INVALID_HANDLE_VALUE) return FALSE;
    unsigned int type = *(unsigned int *)hObject;
    switch (type) {
        case FF_HANDLE_TYPE_EVENT: {
            FF_EVENT_HANDLE *ev = (FF_EVENT_HANDLE *)hObject;
            pthread_mutex_destroy(&ev->mutex);
            pthread_cond_destroy(&ev->cond);
            free(ev);
            return TRUE;
        }
        case FF_HANDLE_TYPE_MUTEX: {
            FF_MUTEX_HANDLE *m = (FF_MUTEX_HANDLE *)hObject;
            pthread_mutex_destroy(&m->mutex);
            free(m);
            return TRUE;
        }
        case FF_HANDLE_TYPE_THREAD: {
            FF_THREAD_HANDLE *th = (FF_THREAD_HANDLE *)hObject;
            if (!th->joined)
                pthread_detach(th->thread);
            free(th);
            return TRUE;
        }
        case FF_HANDLE_TYPE_DETACHED_THREAD: {
            /* From _beginthreadex - thread already detached */
            free(hObject);
            return TRUE;
        }
        case FF_HANDLE_TYPE_FILE: {
            FF_FILE_HANDLE *f = (FF_FILE_HANDLE *)hObject;
            close(f->fd);
            free(f);
            return TRUE;
        }
        case FF_HANDLE_TYPE_FILEMAP: {
            FF_FILEMAP_HANDLE *fm = (FF_FILEMAP_HANDLE *)hObject;
            if (fm->fd >= 0) close(fm->fd);
            free(fm);
            return TRUE;
        }
        default:
            /* Unknown handle - don't free blindly */
            return FALSE;
    }
}

/* ============================================================
 * File I/O (HANDLE based)
 * ============================================================ */
#define GENERIC_READ    0x80000000
#define GENERIC_WRITE   0x40000000
#define GENERIC_ALL     0x10000000

#define FILE_SHARE_READ   0x00000001
#define FILE_SHARE_WRITE  0x00000002
#define FILE_SHARE_DELETE 0x00000004

#define CREATE_NEW        1
#define CREATE_ALWAYS     2
#define OPEN_EXISTING     3
#define OPEN_ALWAYS       4
#define TRUNCATE_EXISTING 5

#define FILE_ATTRIBUTE_READONLY  0x00000001
#define FILE_ATTRIBUTE_HIDDEN    0x00000002
#define FILE_ATTRIBUTE_SYSTEM    0x00000004
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#define FILE_ATTRIBUTE_ARCHIVE   0x00000020
#define FILE_ATTRIBUTE_NORMAL    0x00000080
#define FILE_FLAG_SEQUENTIAL_SCAN 0x08000000
#define FILE_FLAG_RANDOM_ACCESS   0x10000000
#define FILE_FLAG_WRITE_THROUGH   0x80000000
#define INVALID_FILE_ATTRIBUTES   ((DWORD)-1)

#define FILE_BEGIN   0
#define FILE_CURRENT 1
#define FILE_END     2
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#define INVALID_FILE_SIZE        ((DWORD)0xFFFFFFFF)

typedef struct _OVERLAPPED {
    ULONG_PTR Internal;
    ULONG_PTR InternalHigh;
    union {
        struct {
            DWORD Offset;
            DWORD OffsetHigh;
        };
        PVOID Pointer;
    };
    HANDLE hEvent;
} OVERLAPPED, *LPOVERLAPPED;

static inline HANDLE CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
                                 LPSECURITY_ATTRIBUTES sa, DWORD dwCreationDisposition,
                                 DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
    (void)dwShareMode; (void)sa; (void)dwFlagsAndAttributes; (void)hTemplateFile;
    int flags = 0;
    if ((dwDesiredAccess & GENERIC_READ) && (dwDesiredAccess & GENERIC_WRITE)) flags = O_RDWR;
    else if (dwDesiredAccess & GENERIC_WRITE) flags = O_WRONLY;
    else flags = O_RDONLY;

    switch (dwCreationDisposition) {
        case CREATE_NEW:        flags |= O_CREAT | O_EXCL; break;
        case CREATE_ALWAYS:     flags |= O_CREAT | O_TRUNC; break;
        case OPEN_EXISTING:     break;
        case OPEN_ALWAYS:       flags |= O_CREAT; break;
        case TRUNCATE_EXISTING: flags |= O_TRUNC; break;
    }

    /* Convert backslashes and use case-insensitive lookup */
    char path[1024];
    size_t i = 0;
    for (; lpFileName[i] && i < sizeof(path) - 1; i++)
        path[i] = (lpFileName[i] == '\\') ? '/' : lpFileName[i];
    path[i] = '\0';

    int fd = open_nocase(path, flags, 0644);
    if (fd < 0) return INVALID_HANDLE_VALUE;

    FF_FILE_HANDLE *f = (FF_FILE_HANDLE *)malloc(sizeof(FF_FILE_HANDLE));
    f->type = FF_HANDLE_TYPE_FILE;
    f->fd = fd;
    return (HANDLE)f;
}
#define CreateFile CreateFileA

static inline int FF_HandleToFd(HANDLE h) {
    if (!h || h == INVALID_HANDLE_VALUE) return -1;
    FF_FILE_HANDLE *f = (FF_FILE_HANDLE *)h;
    if (f->type != FF_HANDLE_TYPE_FILE) return -1;
    return f->fd;
}

static inline BOOL ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
                            LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {
    (void)lpOverlapped;
    int fd = FF_HandleToFd(hFile);
    if (fd < 0) return FALSE;
    ssize_t n = read(fd, lpBuffer, nNumberOfBytesToRead);
    if (n < 0) {
        if (lpNumberOfBytesRead) *lpNumberOfBytesRead = 0;
        return FALSE;
    }
    if (lpNumberOfBytesRead) *lpNumberOfBytesRead = (DWORD)n;
    return TRUE;
}

static inline BOOL WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,
                             LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) {
    (void)lpOverlapped;
    int fd = FF_HandleToFd(hFile);
    if (fd < 0) return FALSE;
    ssize_t n = write(fd, lpBuffer, nNumberOfBytesToWrite);
    if (n < 0) {
        if (lpNumberOfBytesWritten) *lpNumberOfBytesWritten = 0;
        return FALSE;
    }
    if (lpNumberOfBytesWritten) *lpNumberOfBytesWritten = (DWORD)n;
    return TRUE;
}

static inline DWORD SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod) {
    int fd = FF_HandleToFd(hFile);
    if (fd < 0) return INVALID_SET_FILE_POINTER;
    off_t offset = lDistanceToMove;
    if (lpDistanceToMoveHigh)
        offset |= ((off_t)*lpDistanceToMoveHigh) << 32;
    int whence = (dwMoveMethod == FILE_BEGIN) ? SEEK_SET : (dwMoveMethod == FILE_CURRENT) ? SEEK_CUR : SEEK_END;
    off_t result = lseek(fd, offset, whence);
    if (result < 0) return INVALID_SET_FILE_POINTER;
    if (lpDistanceToMoveHigh) *lpDistanceToMoveHigh = (LONG)(result >> 32);
    return (DWORD)(result & 0xFFFFFFFF);
}

static inline DWORD GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh) {
    int fd = FF_HandleToFd(hFile);
    if (fd < 0) return INVALID_FILE_SIZE;
    struct stat st;
    if (fstat(fd, &st) != 0) return INVALID_FILE_SIZE;
    if (lpFileSizeHigh) *lpFileSizeHigh = (DWORD)((uint64_t)st.st_size >> 32);
    return (DWORD)(st.st_size & 0xFFFFFFFF);
}

static inline BOOL FlushFileBuffers(HANDLE hFile) {
    int fd = FF_HandleToFd(hFile);
    if (fd < 0) return FALSE;
    return fsync(fd) == 0;
}

static inline BOOL GetFileTime(HANDLE hFile, LPFILETIME c, LPFILETIME a, LPFILETIME w) {
    int fd = FF_HandleToFd(hFile);
    if (fd < 0) return FALSE;
    struct stat st;
    if (fstat(fd, &st) != 0) return FALSE;
    uint64_t t = ((uint64_t)st.st_mtime + 11644473600ULL) * 10000000ULL;
    if (c) { c->dwLowDateTime = (DWORD)t; c->dwHighDateTime = (DWORD)(t >> 32); }
    if (a) { a->dwLowDateTime = (DWORD)t; a->dwHighDateTime = (DWORD)(t >> 32); }
    if (w) { w->dwLowDateTime = (DWORD)t; w->dwHighDateTime = (DWORD)(t >> 32); }
    return TRUE;
}

/* ============================================================
 * File mapping
 * ============================================================ */
#define PAGE_NOACCESS  0x01
#define PAGE_READONLY  0x02
#define PAGE_READWRITE 0x04
#define PAGE_EXECUTE_READ 0x20
#define FILE_MAP_COPY  0x0001
#define FILE_MAP_WRITE 0x0002
#define FILE_MAP_READ  0x0004
#define FILE_MAP_ALL_ACCESS 0x000F001F
#define MEM_COMMIT  0x1000
#define MEM_RESERVE 0x2000
#define MEM_RELEASE 0x8000

static inline HANDLE CreateFileMappingA(HANDLE hFile, LPSECURITY_ATTRIBUTES sa, DWORD flProtect,
                                        DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName) {
    (void)sa; (void)flProtect; (void)lpName;
    int fd = FF_HandleToFd(hFile);
    if (fd < 0) return NULL;
    FF_FILEMAP_HANDLE *fm = (FF_FILEMAP_HANDLE *)malloc(sizeof(FF_FILEMAP_HANDLE));
    fm->type = FF_HANDLE_TYPE_FILEMAP;
    fm->fd = dup(fd);
    uint64_t maxSize = ((uint64_t)dwMaximumSizeHigh << 32) | dwMaximumSizeLow;
    if (maxSize == 0) {
        struct stat st;
        fstat(fd, &st);
        maxSize = (uint64_t)st.st_size;
    }
    fm->size = (size_t)maxSize;
    return (HANDLE)fm;
}
#define CreateFileMapping CreateFileMappingA

static inline LPVOID MapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess,
                                   DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap) {
    FF_FILEMAP_HANDLE *fm = (FF_FILEMAP_HANDLE *)hFileMappingObject;
    if (!fm || fm->type != FF_HANDLE_TYPE_FILEMAP) return NULL;
    int prot = PROT_READ;
    int mflags = MAP_PRIVATE;
    if (dwDesiredAccess & FILE_MAP_WRITE) { prot |= PROT_WRITE; mflags = MAP_SHARED; }
    if (dwDesiredAccess & FILE_MAP_COPY) { prot |= PROT_WRITE; mflags = MAP_PRIVATE; }
    off_t offset = ((off_t)dwFileOffsetHigh << 32) | dwFileOffsetLow;
    size_t len = dwNumberOfBytesToMap ? dwNumberOfBytesToMap : fm->size;
    void *p = mmap(NULL, len, prot, mflags, fm->fd, offset);
    return (p == MAP_FAILED) ? NULL : p;
}

static inline BOOL UnmapViewOfFile(LPCVOID lpBaseAddress) {
    /* Note: we don't track sizes per-view; use a large length-free munmap workaround.
     * Most callers map the whole file once. We track via /proc would be overkill -
     * munmap with 0 length fails, so this leaks the mapping if size unknown.
     * FileMemMap in the game stores the size and calls UnmapViewOfFile once at
     * teardown; leaking until exit is acceptable. Attempt msync only. */
    (void)lpBaseAddress;
    return TRUE;
}
static inline BOOL FlushViewOfFile(LPCVOID lpBaseAddress, SIZE_T dwNumberOfBytesToFlush) {
    (void)lpBaseAddress; (void)dwNumberOfBytesToFlush;
    return TRUE;
}

static inline LPVOID VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect) {
    (void)lpAddress; (void)flAllocationType; (void)flProtect;
    return calloc(1, dwSize);
}
static inline BOOL VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType) {
    (void)dwSize; (void)dwFreeType;
    free(lpAddress);
    return TRUE;
}

/* ============================================================
 * Directory / file management
 * ============================================================ */
static inline BOOL DeleteFileA(LPCSTR lpFileName) {
    char path[1024];
    size_t i = 0;
    for (; lpFileName[i] && i < sizeof(path) - 1; i++)
        path[i] = (lpFileName[i] == '\\') ? '/' : lpFileName[i];
    path[i] = '\0';
    return unlink(path) == 0;
}
#define DeleteFile DeleteFileA

static inline BOOL CopyFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, BOOL bFailIfExists) {
    FILE *src = fopen_nocase(lpExistingFileName, "rb");
    if (!src) return FALSE;
    if (bFailIfExists) {
        FILE *t = fopen(lpNewFileName, "rb");
        if (t) { fclose(t); fclose(src); return FALSE; }
    }
    FILE *dst = fopen(lpNewFileName, "wb");
    if (!dst) { fclose(src); return FALSE; }
    char buf[65536];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0)
        fwrite(buf, 1, n, dst);
    fclose(src);
    fclose(dst);
    return TRUE;
}
#define CopyFile CopyFileA

static inline BOOL CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES sa) {
    (void)sa;
    return mkdir(lpPathName, 0755) == 0;
}
#define CreateDirectory CreateDirectoryA

static inline BOOL RemoveDirectoryA(LPCSTR lpPathName) { return rmdir(lpPathName) == 0; }
#define RemoveDirectory RemoveDirectoryA

static inline BOOL SetCurrentDirectoryA(LPCSTR lpPathName) { return chdir(lpPathName) == 0; }
#define SetCurrentDirectory SetCurrentDirectoryA

static inline DWORD GetCurrentDirectoryA(DWORD nBufferLength, LPSTR lpBuffer) {
    if (getcwd(lpBuffer, nBufferLength) == NULL) return 0;
    return (DWORD)strlen(lpBuffer);
}
#define GetCurrentDirectory GetCurrentDirectoryA

static inline DWORD GetFileAttributesA(LPCSTR lpFileName) {
    struct stat st;
    char path[1024];
    size_t i = 0;
    for (; lpFileName[i] && i < sizeof(path) - 1; i++)
        path[i] = (lpFileName[i] == '\\') ? '/' : lpFileName[i];
    path[i] = '\0';
    if (stat(path, &st) != 0) return INVALID_FILE_ATTRIBUTES;
    DWORD attrs = 0;
    if (S_ISDIR(st.st_mode)) attrs |= FILE_ATTRIBUTE_DIRECTORY;
    if (attrs == 0) attrs = FILE_ATTRIBUTE_NORMAL;
    return attrs;
}
#define GetFileAttributes GetFileAttributesA

static inline DWORD GetTempPathA(DWORD nBufferLength, LPSTR lpBuffer) {
    const char *tmp = getenv("TMPDIR");
    if (!tmp) tmp = "/tmp/";
    strncpy(lpBuffer, tmp, nBufferLength - 1);
    lpBuffer[nBufferLength - 1] = '\0';
    size_t len = strlen(lpBuffer);
    if (len && lpBuffer[len - 1] != '/' && len + 1 < nBufferLength) {
        lpBuffer[len] = '/';
        lpBuffer[len + 1] = '\0';
        len++;
    }
    return (DWORD)len;
}
#define GetTempPath GetTempPathA

static inline DWORD GetFullPathNameA(LPCSTR lpFileName, DWORD nBufferLength, LPSTR lpBuffer, LPSTR *lpFilePart) {
    char resolved[4096];
    if (realpath(lpFileName, resolved) == NULL) {
        /* Fall back to as-is */
        strncpy(lpBuffer, lpFileName, nBufferLength - 1);
        lpBuffer[nBufferLength - 1] = '\0';
    } else {
        strncpy(lpBuffer, resolved, nBufferLength - 1);
        lpBuffer[nBufferLength - 1] = '\0';
    }
    if (lpFilePart) {
        char *slash = strrchr(lpBuffer, '/');
        *lpFilePart = slash ? slash + 1 : lpBuffer;
    }
    return (DWORD)strlen(lpBuffer);
}
#define GetFullPathName GetFullPathNameA

/* ============================================================
 * FindFirstFile / FindNextFile
 * ============================================================ */
typedef struct _WIN32_FIND_DATAA {
    DWORD    dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD    nFileSizeHigh;
    DWORD    nFileSizeLow;
    DWORD    dwReserved0;
    DWORD    dwReserved1;
    CHAR     cFileName[MAX_PATH];
    CHAR     cAlternateFileName[14];
} WIN32_FIND_DATAA, *PWIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;
typedef WIN32_FIND_DATAA WIN32_FIND_DATA, *PWIN32_FIND_DATA, *LPWIN32_FIND_DATA;

#ifdef __cplusplus
extern "C" {
#endif
/* Implemented in linux_stubs.cpp (glob-based with pattern matching) */
HANDLE FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData);
BOOL FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData);
BOOL FindClose(HANDLE hFindFile);
#ifdef __cplusplus
}
#endif
#define FindFirstFile FindFirstFileA
#define FindNextFile FindNextFileA

/* ============================================================
 * Module / library
 * ============================================================ */
typedef void (*FARPROC)(void);
typedef void (*PROC)(void);

static inline HMODULE LoadLibraryA(LPCSTR lpLibFileName) {
    (void)lpLibFileName;
    return NULL; /* No Windows DLLs on Linux */
}
#define LoadLibrary LoadLibraryA
static inline BOOL FreeLibrary(HMODULE hLibModule) { (void)hLibModule; return TRUE; }
static inline FARPROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName) {
    (void)hModule; (void)lpProcName;
    return NULL;
}
static inline HMODULE GetModuleHandleA(LPCSTR lpModuleName) {
    (void)lpModuleName;
    return (HMODULE)(uintptr_t)1; /* Non-NULL dummy */
}
#define GetModuleHandle GetModuleHandleA
static inline DWORD GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize) {
    (void)hModule;
    ssize_t n = readlink("/proc/self/exe", lpFilename, nSize - 1);
    if (n < 0) {
        lpFilename[0] = '\0';
        return 0;
    }
    lpFilename[n] = '\0';
    return (DWORD)n;
}
#define GetModuleFileName GetModuleFileNameA

/* ============================================================
 * Debug / error
 * ============================================================ */
static inline void OutputDebugStringA(LPCSTR lpOutputString) {
    if (lpOutputString) fprintf(stderr, "%s", lpOutputString);
}
#define OutputDebugString OutputDebugStringA

static inline void DebugBreak(void) {
    /* Don't actually trap in release-style runs */
}

static inline DWORD GetLastError(void) { return (DWORD)errno; }
static inline void SetLastError(DWORD dwErrCode) { errno = (int)dwErrCode; }

#define ERROR_SUCCESS          0
#define ERROR_FILE_NOT_FOUND   2
#define ERROR_PATH_NOT_FOUND   3
#define ERROR_ACCESS_DENIED    5
#define ERROR_INVALID_HANDLE   6
#define ERROR_NOT_ENOUGH_MEMORY 8
#define ERROR_NO_MORE_FILES    18
#define ERROR_ALREADY_EXISTS   183
#define ERROR_IO_PENDING       997
#define NO_ERROR               0

#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0x00000100
#define FORMAT_MESSAGE_IGNORE_INSERTS  0x00000200
#define FORMAT_MESSAGE_FROM_STRING     0x00000400
#define FORMAT_MESSAGE_FROM_SYSTEM     0x00001000
#define FORMAT_MESSAGE_ARGUMENT_ARRAY  0x00002000

static inline DWORD FormatMessageA(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId,
                                   LPSTR lpBuffer, DWORD nSize, va_list *Arguments) {
    (void)dwFlags; (void)lpSource; (void)dwLanguageId; (void)Arguments;
    if (lpBuffer && nSize > 0) {
        snprintf(lpBuffer, nSize, "Error %u: %s", dwMessageId, strerror((int)dwMessageId));
        return (DWORD)strlen(lpBuffer);
    }
    return 0;
}
#define FormatMessage FormatMessageA

/* ============================================================
 * String helpers
 * ============================================================ */
static inline int lstrlenA(LPCSTR lpString) { return lpString ? (int)strlen(lpString) : 0; }
#define lstrlen lstrlenA
static inline LPSTR lstrcpyA(LPSTR d, LPCSTR s) { return strcpy(d, s); }
#define lstrcpy lstrcpyA
static inline LPSTR lstrcpynA(LPSTR d, LPCSTR s, int n) {
    strncpy(d, s, n - 1);
    d[n - 1] = '\0';
    return d;
}
#define lstrcpyn lstrcpynA
static inline LPSTR lstrcatA(LPSTR d, LPCSTR s) { return strcat(d, s); }
#define lstrcat lstrcatA
static inline int lstrcmpA(LPCSTR a, LPCSTR b) { return strcmp(a, b); }
#define lstrcmp lstrcmpA
static inline int lstrcmpiA(LPCSTR a, LPCSTR b) { return strcasecmp(a, b); }
#define lstrcmpi lstrcmpiA

#define wsprintfA sprintf
#define wsprintf  sprintf
#define wvsprintfA vsprintf
#define wvsprintf  vsprintf

static inline int MulDiv(int nNumber, int nNumerator, int nDenominator) {
    if (nDenominator == 0) return -1;
    return (int)(((long long)nNumber * nNumerator + nDenominator / 2) / nDenominator);
}

/* ============================================================
 * Heap / memory
 * ============================================================ */
static inline HANDLE GetProcessHeap(void) { return (HANDLE)(uintptr_t)1; }
static inline LPVOID HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes) {
    (void)hHeap;
    return (dwFlags & 0x8) ? calloc(1, dwBytes) : malloc(dwBytes); /* 0x8 = HEAP_ZERO_MEMORY */
}
static inline BOOL HeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem) {
    (void)hHeap; (void)dwFlags;
    free(lpMem);
    return TRUE;
}
#define HEAP_ZERO_MEMORY 0x00000008
#define HEAP_NO_SERIALIZE 0x00000001
#define HEAP_GENERATE_EXCEPTIONS 0x00000004
static inline HANDLE HeapCreate(DWORD flOptions, SIZE_T dwInitialSize, SIZE_T dwMaximumSize) {
    (void)flOptions; (void)dwInitialSize; (void)dwMaximumSize;
    return (HANDLE)(uintptr_t)1;
}
static inline BOOL HeapDestroy(HANDLE hHeap) { (void)hHeap; return TRUE; }
static inline LPVOID HeapReAlloc(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem, SIZE_T dwBytes) {
    (void)hHeap; (void)dwFlags;
    return realloc(lpMem, dwBytes);
}

#define GMEM_FIXED    0x0000
#define GMEM_MOVEABLE 0x0002
#define GMEM_ZEROINIT 0x0040
#define GPTR          (GMEM_FIXED | GMEM_ZEROINIT)
#define GHND          (GMEM_MOVEABLE | GMEM_ZEROINIT)
#define LMEM_FIXED    0x0000
#define LMEM_ZEROINIT 0x0040
#define LPTR          (LMEM_FIXED | LMEM_ZEROINIT)

static inline HGLOBAL GlobalAlloc(UINT uFlags, SIZE_T dwBytes) {
    return (HGLOBAL)((uFlags & GMEM_ZEROINIT) ? calloc(1, dwBytes) : malloc(dwBytes));
}
static inline HGLOBAL GlobalFree(HGLOBAL hMem) { free(hMem); return NULL; }
static inline LPVOID GlobalLock(HGLOBAL hMem) { return (LPVOID)hMem; }
static inline BOOL GlobalUnlock(HGLOBAL hMem) { (void)hMem; return TRUE; }
static inline HLOCAL LocalAlloc(UINT uFlags, SIZE_T uBytes) {
    return (HLOCAL)((uFlags & LMEM_ZEROINIT) ? calloc(1, uBytes) : malloc(uBytes));
}
static inline HLOCAL LocalFree(HLOCAL hMem) { free(hMem); return NULL; }

typedef struct _MEMORYSTATUS {
    DWORD dwLength;
    DWORD dwMemoryLoad;
    SIZE_T dwTotalPhys;
    SIZE_T dwAvailPhys;
    SIZE_T dwTotalPageFile;
    SIZE_T dwAvailPageFile;
    SIZE_T dwTotalVirtual;
    SIZE_T dwAvailVirtual;
} MEMORYSTATUS, *LPMEMORYSTATUS;

static inline void GlobalMemoryStatus(LPMEMORYSTATUS lpBuffer) {
    if (lpBuffer) {
        memset(lpBuffer, 0, sizeof(*lpBuffer));
        lpBuffer->dwLength = sizeof(*lpBuffer);
        lpBuffer->dwTotalPhys = 2u * 1024u * 1024u * 1024u - 1; /* pretend 2GB */
        lpBuffer->dwAvailPhys = 1u * 1024u * 1024u * 1024u;
        lpBuffer->dwTotalVirtual = 2u * 1024u * 1024u * 1024u - 1;
        lpBuffer->dwAvailVirtual = 1u * 1024u * 1024u * 1024u;
    }
}

/* ============================================================
 * Registry stubs (always fail - no registry on Linux)
 * ============================================================ */
#define HKEY_CLASSES_ROOT  ((HKEY)(uintptr_t)0x80000000)
#define HKEY_CURRENT_USER  ((HKEY)(uintptr_t)0x80000001)
#define HKEY_LOCAL_MACHINE ((HKEY)(uintptr_t)0x80000002)
#define HKEY_USERS         ((HKEY)(uintptr_t)0x80000003)

#define KEY_QUERY_VALUE   0x0001
#define KEY_SET_VALUE     0x0002
#define KEY_CREATE_SUB_KEY 0x0004
#define KEY_READ          0x20019
#define KEY_WRITE         0x20006
#define KEY_ALL_ACCESS    0xF003F

#define REG_NONE      0
#define REG_SZ        1
#define REG_EXPAND_SZ 2
#define REG_BINARY    3
#define REG_DWORD     4

#define REG_OPTION_NON_VOLATILE 0

typedef DWORD REGSAM;

static inline LONG RegOpenKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult) {
    (void)hKey; (void)lpSubKey; (void)ulOptions; (void)samDesired;
    if (phkResult) *phkResult = NULL;
    return ERROR_FILE_NOT_FOUND;
}
#define RegOpenKeyEx RegOpenKeyExA
static inline LONG RegOpenKeyA(HKEY hKey, LPCSTR lpSubKey, PHKEY phkResult) {
    return RegOpenKeyExA(hKey, lpSubKey, 0, 0, phkResult);
}
#define RegOpenKey RegOpenKeyA
static inline LONG RegCreateKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD r, LPSTR cls, DWORD opt, REGSAM sam,
                                   LPSECURITY_ATTRIBUTES sa, PHKEY phkResult, LPDWORD disp) {
    (void)hKey; (void)lpSubKey; (void)r; (void)cls; (void)opt; (void)sam; (void)sa; (void)disp;
    if (phkResult) *phkResult = NULL;
    return ERROR_ACCESS_DENIED;
}
#define RegCreateKeyEx RegCreateKeyExA
static inline LONG RegQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD r, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) {
    (void)hKey; (void)lpValueName; (void)r; (void)lpType; (void)lpData; (void)lpcbData;
    return ERROR_FILE_NOT_FOUND;
}
#define RegQueryValueEx RegQueryValueExA
static inline LONG RegSetValueExA(HKEY hKey, LPCSTR lpValueName, DWORD r, DWORD dwType, const BYTE *lpData, DWORD cbData) {
    (void)hKey; (void)lpValueName; (void)r; (void)dwType; (void)lpData; (void)cbData;
    return ERROR_ACCESS_DENIED;
}
#define RegSetValueEx RegSetValueExA
static inline LONG RegCloseKey(HKEY hKey) { (void)hKey; return ERROR_SUCCESS; }
static inline LONG RegDeleteKeyA(HKEY hKey, LPCSTR lpSubKey) { (void)hKey; (void)lpSubKey; return ERROR_SUCCESS; }
#define RegDeleteKey RegDeleteKeyA
static inline LONG RegEnumKeyA(HKEY hKey, DWORD dwIndex, LPSTR lpName, DWORD cchName) {
    (void)hKey; (void)dwIndex; (void)lpName; (void)cchName;
    return ERROR_NO_MORE_FILES;
}
#define RegEnumKey RegEnumKeyA

/* Drive types */
#define DRIVE_UNKNOWN     0
#define DRIVE_NO_ROOT_DIR 1
#define DRIVE_REMOVABLE   2
#define DRIVE_FIXED       3
#define DRIVE_REMOTE      4
#define DRIVE_CDROM       5
#define DRIVE_RAMDISK     6
static inline UINT GetDriveTypeA(LPCSTR lpRootPathName) { (void)lpRootPathName; return DRIVE_FIXED; }
#define GetDriveType GetDriveTypeA
static inline DWORD GetLogicalDrives(void) { return 0x4; /* C: */ }
static inline DWORD GetLogicalDriveStringsA(DWORD nBufferLength, LPSTR lpBuffer) {
    if (lpBuffer && nBufferLength >= 5) {
        memcpy(lpBuffer, "C:\\\0\0", 5);
        return 4;
    }
    return 0;
}
#define GetLogicalDriveStrings GetLogicalDriveStringsA
static inline BOOL GetDiskFreeSpaceA(LPCSTR root, LPDWORD spc, LPDWORD bps, LPDWORD freec, LPDWORD totalc) {
    (void)root;
    if (spc) *spc = 8;
    if (bps) *bps = 512;
    if (freec) *freec = 0x100000;
    if (totalc) *totalc = 0x200000;
    return TRUE;
}
#define GetDiskFreeSpace GetDiskFreeSpaceA

/* ============================================================
 * Version / system info
 * ============================================================ */
typedef struct _OSVERSIONINFOA {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    CHAR  szCSDVersion[128];
} OSVERSIONINFOA, *POSVERSIONINFOA, *LPOSVERSIONINFOA;
typedef OSVERSIONINFOA OSVERSIONINFO, *LPOSVERSIONINFO;

#define VER_PLATFORM_WIN32_NT      2
#define VER_PLATFORM_WIN32_WINDOWS 1

static inline BOOL GetVersionExA(LPOSVERSIONINFOA lpVersionInformation) {
    if (lpVersionInformation) {
        lpVersionInformation->dwMajorVersion = 5;
        lpVersionInformation->dwMinorVersion = 1;
        lpVersionInformation->dwBuildNumber = 2600;
        lpVersionInformation->dwPlatformId = VER_PLATFORM_WIN32_NT;
        lpVersionInformation->szCSDVersion[0] = '\0';
    }
    return TRUE;
}
#define GetVersionEx GetVersionExA
static inline DWORD GetVersion(void) { return 0x0105; }

static inline BOOL GetUserNameA(LPSTR lpBuffer, LPDWORD pcbBuffer) {
    const char *user = getenv("USER");
    if (!user) user = "player";
    if (lpBuffer && pcbBuffer && *pcbBuffer > strlen(user)) {
        strcpy(lpBuffer, user);
        *pcbBuffer = (DWORD)strlen(user) + 1;
        return TRUE;
    }
    return FALSE;
}
#define GetUserName GetUserNameA

static inline BOOL GetComputerNameA(LPSTR lpBuffer, LPDWORD nSize) {
    if (lpBuffer && nSize && gethostname(lpBuffer, *nSize) == 0) {
        *nSize = (DWORD)strlen(lpBuffer);
        return TRUE;
    }
    return FALSE;
}
#define GetComputerName GetComputerNameA

typedef struct _SYSTEM_INFO {
    DWORD dwOemId;
    DWORD dwPageSize;
    LPVOID lpMinimumApplicationAddress;
    LPVOID lpMaximumApplicationAddress;
    DWORD_PTR dwActiveProcessorMask;
    DWORD dwNumberOfProcessors;
    DWORD dwProcessorType;
    DWORD dwAllocationGranularity;
    WORD wProcessorLevel;
    WORD wProcessorRevision;
} SYSTEM_INFO, *LPSYSTEM_INFO;

static inline void GetSystemInfo(LPSYSTEM_INFO lpSystemInfo) {
    if (lpSystemInfo) {
        memset(lpSystemInfo, 0, sizeof(*lpSystemInfo));
        lpSystemInfo->dwPageSize = 4096;
        lpSystemInfo->dwNumberOfProcessors = (DWORD)sysconf(_SC_NPROCESSORS_ONLN);
    }
}

static inline void ExitProcess(UINT uExitCode) { exit((int)uExitCode); }
static inline BOOL TerminateProcess(HANDLE hProcess, UINT uExitCode) {
    (void)hProcess;
    exit((int)uExitCode);
    return TRUE;
}

/* Process priority */
#define NORMAL_PRIORITY_CLASS   0x20
#define HIGH_PRIORITY_CLASS     0x80
#define REALTIME_PRIORITY_CLASS 0x100
#define IDLE_PRIORITY_CLASS     0x40
static inline BOOL SetPriorityClass(HANDLE hProcess, DWORD dwPriorityClass) {
    (void)hProcess; (void)dwPriorityClass;
    return TRUE;
}
static inline DWORD GetPriorityClass(HANDLE hProcess) { (void)hProcess; return NORMAL_PRIORITY_CLASS; }

#define MAXIMUM_WAIT_OBJECTS 64
#define SEC_COMMIT  0x8000000
#define SEC_RESERVE 0x4000000

static inline LPVOID MapViewOfFileEx(HANDLE h, DWORD acc, DWORD offHigh, DWORD offLow, SIZE_T bytes, LPVOID base) {
    (void)base;
    return MapViewOfFile(h, acc, offHigh, offLow, bytes);
}

/* Pointer validation stubs - assume valid */
static inline BOOL IsBadReadPtr(const void *lp, UINT_PTR ucb) { (void)ucb; return lp == NULL; }
static inline BOOL IsBadWritePtr(LPVOID lp, UINT_PTR ucb) { (void)ucb; return lp == NULL; }
static inline BOOL IsBadCodePtr(FARPROC lpfn) { return lpfn == NULL; }
static inline BOOL IsBadStringPtrA(LPCSTR lpsz, UINT_PTR ucchMax) { (void)ucchMax; return lpsz == NULL; }
#define IsBadStringPtr IsBadStringPtrA

/* Code page conversion */
#define CP_ACP  0
#define CP_OEMCP 1
#define CP_UTF8 65001
#define MB_PRECOMPOSED 0x1
static inline int MultiByteToWideChar(UINT cp, DWORD flags, LPCSTR str, int cbMultiByte, LPWSTR wstr, int cchWideChar) {
    (void)cp; (void)flags;
    int len = (cbMultiByte < 0) ? (int)strlen(str) + 1 : cbMultiByte;
    if (!wstr || cchWideChar == 0) return len;
    int n = (len < cchWideChar) ? len : cchWideChar;
    for (int i = 0; i < n; i++) wstr[i] = (WCHAR)(unsigned char)str[i];
    return n;
}
static inline int WideCharToMultiByte(UINT cp, DWORD flags, LPCWSTR wstr, int cchWideChar, LPSTR str, int cbMultiByte, LPCSTR defChar, LPBOOL usedDef) {
    (void)cp; (void)flags; (void)defChar; (void)usedDef;
    int len = cchWideChar;
    if (len < 0) {
        len = 0;
        while (wstr[len]) len++;
        len++;
    }
    if (!str || cbMultiByte == 0) return len;
    int n = (len < cbMultiByte) ? len : cbMultiByte;
    for (int i = 0; i < n; i++) str[i] = (char)wstr[i];
    return n;
}

static inline BOOL FileTimeToLocalFileTime(const FILETIME *lpFileTime, LPFILETIME lpLocalFileTime) {
    if (lpLocalFileTime && lpFileTime) *lpLocalFileTime = *lpFileTime;
    return TRUE;
}
static inline BOOL FileTimeToDosDateTime(const FILETIME *lpFileTime, LPWORD lpFatDate, LPWORD lpFatTime) {
    if (!lpFileTime) return FALSE;
    uint64_t t = ((uint64_t)lpFileTime->dwHighDateTime << 32) | lpFileTime->dwLowDateTime;
    time_t unix_t = (time_t)(t / 10000000ULL - 11644473600ULL);
    struct tm tmv;
    localtime_r(&unix_t, &tmv);
    if (lpFatDate) *lpFatDate = (WORD)(((tmv.tm_year - 80) << 9) | ((tmv.tm_mon + 1) << 5) | tmv.tm_mday);
    if (lpFatTime) *lpFatTime = (WORD)((tmv.tm_hour << 11) | (tmv.tm_min << 5) | (tmv.tm_sec / 2));
    return TRUE;
}
static inline BOOL FileTimeToSystemTime(const FILETIME *ft, LPSYSTEMTIME st) {
    if (!ft || !st) return FALSE;
    uint64_t t = ((uint64_t)ft->dwHighDateTime << 32) | ft->dwLowDateTime;
    time_t unix_t = (time_t)(t / 10000000ULL - 11644473600ULL);
    struct tm tmv;
    gmtime_r(&unix_t, &tmv);
    st->wYear = (WORD)(tmv.tm_year + 1900);
    st->wMonth = (WORD)(tmv.tm_mon + 1);
    st->wDayOfWeek = (WORD)tmv.tm_wday;
    st->wDay = (WORD)tmv.tm_mday;
    st->wHour = (WORD)tmv.tm_hour;
    st->wMinute = (WORD)tmv.tm_min;
    st->wSecond = (WORD)tmv.tm_sec;
    st->wMilliseconds = 0;
    return TRUE;
}

/* ============================================================
 * Language / locale macros
 * ============================================================ */
#define LANG_NEUTRAL    0x00
#define LANG_ENGLISH    0x09
#define SUBLANG_DEFAULT 0x01
#define SUBLANG_NEUTRAL 0x00
#define MAKELANGID(p, s) ((((WORD)(s)) << 10) | (WORD)(p))
#define PRIMARYLANGID(lgid) ((WORD)(lgid) & 0x3ff)
#define SUBLANGID(lgid)     ((WORD)(lgid) >> 10)

/* ============================================================
 * Version info stubs
 * ============================================================ */
static inline DWORD GetFileVersionInfoSizeA(LPCSTR f, LPDWORD h) { (void)f; if (h) *h = 0; return 0; }
#define GetFileVersionInfoSize GetFileVersionInfoSizeA
static inline BOOL GetFileVersionInfoA(LPCSTR f, DWORD h, DWORD len, LPVOID data) {
    (void)f; (void)h; (void)len; (void)data;
    return FALSE;
}
#define GetFileVersionInfo GetFileVersionInfoA
static inline BOOL VerQueryValueA(LPCVOID block, LPCSTR sub, LPVOID *buf, PUINT len) {
    (void)block; (void)sub;
    if (buf) *buf = NULL;
    if (len) *len = 0;
    return FALSE;
}
#define VerQueryValue VerQueryValueA

/* ============================================================
 * Private profile (.ini) functions - real implementation.
 * Backend FF_IniGetValue() lives in linux_stubs.cpp (case-insensitive
 * section/key match, whitespace/CR trim, quote strip). These were
 * previously default-returning stubs, which zeroed every .ini-loaded
 * tuning value (campaign AI inputs, rules, FF effects, ...).
 * ============================================================ */
#ifdef __cplusplus
extern "C"
#else
extern
#endif
int FF_IniGetValue(const char *section, const char *key,
                   char *out, unsigned size, const char *file);

static inline UINT GetPrivateProfileIntA(LPCSTR app, LPCSTR key, INT def, LPCSTR file) {
    char val[64];
    if (FF_IniGetValue(app, key, val, sizeof(val), file))
        return (UINT)strtol(val, NULL, 0);
    return (UINT)def;
}
#define GetPrivateProfileInt GetPrivateProfileIntA
static inline DWORD GetPrivateProfileStringA(LPCSTR app, LPCSTR key, LPCSTR def, LPSTR ret, DWORD size, LPCSTR file) {
    if (!ret || !size)
        return 0;
    if (FF_IniGetValue(app, key, ret, size, file))
        return (DWORD)strlen(ret);
    strncpy(ret, def ? def : "", size - 1);
    ret[size - 1] = '\0';
    return (DWORD)strlen(ret);
}
#define GetPrivateProfileString GetPrivateProfileStringA
static inline BOOL WritePrivateProfileStringA(LPCSTR app, LPCSTR key, LPCSTR str, LPCSTR file) {
    (void)app; (void)key; (void)str; (void)file;
    return TRUE;
}
#define WritePrivateProfileString WritePrivateProfileStringA
static inline UINT GetProfileIntA(LPCSTR app, LPCSTR key, INT def) { (void)app; (void)key; return (UINT)def; }
#define GetProfileInt GetProfileIntA

#endif /* FF_LINUX */
#endif /* FF_COMPAT_WINBASE_H */

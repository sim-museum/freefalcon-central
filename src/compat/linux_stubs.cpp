/*
 * FreeFalcon Linux Port - Linux stub/helper implementations
 *
 * Case-insensitive file lookup, find-file emulation, path helpers,
 * D3DX surface format mapping, and DirectDraw factory entry points.
 */

#ifdef FF_LINUX

#define FF_NO_FOPEN_REDIRECT  /* this file implements fopen_nocase itself */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cerrno>

#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fnmatch.h>

#include <SDL2/SDL.h>

#include "compat_types.h"
#include "compat_winbase.h"
#include "ddraw.h"
#include "d3d.h"
#include "d3dxcore.h"

/* ============================================================
 * Case-insensitive path resolution
 *
 * Resolves each path component case-insensitively against the
 * actual directory contents. Backslashes are converted to '/'.
 * ============================================================ */
static int resolve_nocase_uncached(const char *filepath, char *resolved, size_t resolvedSize) {
    char work[2048];
    size_t i = 0;
    for (; filepath[i] && i < sizeof(work) - 1; i++)
        work[i] = (filepath[i] == '\\') ? '/' : filepath[i];
    work[i] = '\0';

    /* Fast path: exact match exists */
    if (access(work, F_OK) == 0) {
        strncpy(resolved, work, resolvedSize - 1);
        resolved[resolvedSize - 1] = '\0';
        return 0;
    }

    /* Component-wise case-insensitive walk */
    char out[2048];
    const char *p = work;
    if (*p == '/') {
        out[0] = '/';
        out[1] = '\0';
        p++;
    } else {
        out[0] = '\0';
    }

    char comp[512];
    while (*p) {
        /* Extract next component */
        size_t ci = 0;
        while (*p && *p != '/' && ci < sizeof(comp) - 1)
            comp[ci++] = *p++;
        comp[ci] = '\0';
        while (*p == '/') p++;

        if (ci == 0) continue;

        /* Try exact */
        char candidate[2048];
        snprintf(candidate, sizeof(candidate), "%s%s%s", out, (out[0] && out[strlen(out) - 1] != '/') ? "/" : "", comp);
        if (access(candidate, F_OK) == 0) {
            strncpy(out, candidate, sizeof(out) - 1);
            out[sizeof(out) - 1] = '\0';
            continue;
        }

        /* Scan directory for case-insensitive match */
        const char *dirPath = out[0] ? out : ".";
        DIR *d = opendir(dirPath);
        if (!d)
            return -1;
        struct dirent *e;
        int found = 0;
        while ((e = readdir(d)) != NULL) {
            if (strcasecmp(e->d_name, comp) == 0) {
                snprintf(candidate, sizeof(candidate), "%s%s%s", out, (out[0] && out[strlen(out) - 1] != '/') ? "/" : "", e->d_name);
                strncpy(out, candidate, sizeof(out) - 1);
                out[sizeof(out) - 1] = '\0';
                found = 1;
                break;
            }
        }
        closedir(d);
        if (!found) {
            /* Component not found - return remaining path as-is (caller may be creating it) */
            snprintf(candidate, sizeof(candidate), "%s%s%s", out, (out[0] && out[strlen(out) - 1] != '/') ? "/" : "", comp);
            if (*p) {
                /* Still more components - give up resolving, append rest */
                strncat(candidate, "/", sizeof(candidate) - strlen(candidate) - 1);
                strncat(candidate, p, sizeof(candidate) - strlen(candidate) - 1);
            }
            strncpy(resolved, candidate, resolvedSize - 1);
            resolved[resolvedSize - 1] = '\0';
            return -1;
        }
    }

    strncpy(resolved, out, resolvedSize - 1);
    resolved[resolvedSize - 1] = '\0';
    return 0;
}

/* ============================================================
 * FF_LINUX: resolve_nocase() result cache
 *
 * Every texture / terrain / model / resource load goes through
 * resolve_nocase(), and each miss of the exact-case fast path
 * re-opendir()s *every* component of the path. That is squarely on
 * the hot loading path, so the resolved names are cached.
 *
 * Correctness rules (the game creates and writes files: campaign
 * saves, logbook, debrief, ACMI):
 *   1. ONLY successful resolutions are cached. A negative result is
 *      never remembered, so a file that appears later (because the
 *      game just created it) is always found.
 *   2. Every cache hit is validated with a single access(F_OK) on the
 *      cached target. If it has been deleted or renamed the entry is
 *      dropped and the full walk re-runs. One syscall still beats N
 *      directory scans.
 *   3. Any create/write open (fopen_nocase with w/a/+, open_nocase
 *      with O_CREAT/O_TRUNC/write access) flushes the whole cache, so
 *      a save that changes what exists on disk can never be served
 *      from a stale entry. Writes are rare; reads are the hot path.
 *
 * FF_NO_PATHCACHE=1   - bypass the cache entirely (revert switch).
 * FF_TRACE_PATHCACHE=1- hit/miss/validate-fail/flush trace + periodic
 *                       summary on stderr.
 * ============================================================ */
#include <string>
#include <unordered_map>
#include <mutex>

#define FF_PATHCACHE_MAX 20000

static std::mutex                                    g_pcMutex;
static std::unordered_map<std::string, std::string> *g_pcMap = NULL;
static unsigned long g_pcHits = 0, g_pcMisses = 0, g_pcStale = 0, g_pcFlushes = 0;

static int pc_disabled(void) {
    static int v = -1;
    if (v < 0) v = getenv("FF_NO_PATHCACHE") ? 1 : 0;
    return v;
}
static int pc_trace(void) {
    static int v = -1;
    if (v < 0) v = getenv("FF_TRACE_PATHCACHE") ? 1 : 0;
    return v;
}

static void pc_report(void) {
    if (!pc_trace()) return;
    unsigned long total = g_pcHits + g_pcMisses;
    if (total && (total % 1000) == 0)
        fprintf(stderr, "[PATHCACHE] lookups=%lu hits=%lu (%.1f%%) misses=%lu stale=%lu flushes=%lu entries=%zu\n",
                total, g_pcHits, 100.0 * (double)g_pcHits / (double)total,
                g_pcMisses, g_pcStale, g_pcFlushes, g_pcMap ? g_pcMap->size() : (size_t)0);
}

/* Drop everything. Called whenever the caller is about to create or
   write a file, i.e. whenever the on-disk name set may change. */
static void pc_flush(const char *why) {
    if (pc_disabled()) return;
    std::lock_guard<std::mutex> lk(g_pcMutex);
    if (g_pcMap && !g_pcMap->empty()) {
        if (pc_trace())
            fprintf(stderr, "[PATHCACHE] flush (%s): dropped %zu entries\n", why, g_pcMap->size());
        g_pcMap->clear();
        g_pcFlushes++;
    }
}

/* Decide whether a create/write open really needs a flush.
   Writing to a file that ALREADY exists cannot change the set of names on
   disk, so every cached resolution stays valid - no flush needed. Only a
   write that may CREATE a new name invalidates the cache. (Without this the
   port's per-frame diagnostic log opens flushed the cache thousands of times
   per mission and the hit rate collapsed to ~20%.) Deletions/renames are
   covered separately by the per-hit access() validation. */
static void pc_flush_for_write(const char *filepath, const char *why) {
    if (pc_disabled() || !filepath) return;
    char conv[2048];
    size_t i = 0;
    for (; filepath[i] && i < sizeof(conv) - 1; i++)
        conv[i] = (filepath[i] == '\\') ? '/' : filepath[i];
    conv[i] = '\0';
    if (access(conv, F_OK) == 0) return;             /* plain overwrite */
    {
        std::lock_guard<std::mutex> lk(g_pcMutex);
        if (g_pcMap) {
            std::unordered_map<std::string, std::string>::iterator it = g_pcMap->find(filepath);
            if (it != g_pcMap->end() && access(it->second.c_str(), F_OK) == 0)
                return;                              /* overwrite of a case-variant */
        }
    }
    pc_flush(why);                                   /* may create a new name */
}

static int resolve_nocase(const char *filepath, char *resolved, size_t resolvedSize) {
    if (pc_disabled() || !filepath || !*filepath)
        return resolve_nocase_uncached(filepath, resolved, resolvedSize);

    {
        std::lock_guard<std::mutex> lk(g_pcMutex);
        if (!g_pcMap) g_pcMap = new std::unordered_map<std::string, std::string>();
        std::unordered_map<std::string, std::string>::iterator it = g_pcMap->find(filepath);
        if (it != g_pcMap->end()) {
            /* Validate: the target must still exist (rule 2). */
            if (access(it->second.c_str(), F_OK) == 0) {
                strncpy(resolved, it->second.c_str(), resolvedSize - 1);
                resolved[resolvedSize - 1] = '\0';
                g_pcHits++;
                if (pc_trace()) {
                    fprintf(stderr, "[PATHCACHE] hit  %s -> %s\n", filepath, it->second.c_str());
                    pc_report();
                }
                return 0;
            }
            g_pcStale++;
            if (pc_trace())
                fprintf(stderr, "[PATHCACHE] stale %s -> %s (gone, re-resolving)\n",
                        filepath, it->second.c_str());
            g_pcMap->erase(it);
        }
    }

    int rc = resolve_nocase_uncached(filepath, resolved, resolvedSize);

    {
        std::lock_guard<std::mutex> lk(g_pcMutex);
        g_pcMisses++;
        /* Rule 1: cache successes only. */
        if (rc == 0 && g_pcMap) {
            if (g_pcMap->size() >= FF_PATHCACHE_MAX) {
                if (pc_trace())
                    fprintf(stderr, "[PATHCACHE] cap %d reached, clearing\n", FF_PATHCACHE_MAX);
                g_pcMap->clear();
            }
            (*g_pcMap)[filepath] = resolved;
        }
        if (pc_trace()) {
            fprintf(stderr, "[PATHCACHE] miss %s -> %s (rc=%d)\n", filepath, resolved, rc);
            pc_report();
        }
    }
    return rc;
}

extern "C" FILE *fopen_nocase(const char *filepath, const char *mode) {
    if (!filepath || !mode)
        return NULL;
    /* FF_LINUX: a write/append/update open can create a file or change what
       exists on disk -> drop the resolve cache (rule 3 above). */
    if (strchr(mode, 'w') || strchr(mode, 'a') || strchr(mode, '+'))
        pc_flush_for_write(filepath, "fopen write");
    char resolved[2048];
    if (resolve_nocase(filepath, resolved, sizeof(resolved)) == 0) {
        /* Windows fopen() fails on directories; Linux succeeds. Match Windows. */
        struct stat st;
        if (stat(resolved, &st) == 0 && S_ISDIR(st.st_mode))
            return NULL;
        return fopen(resolved, mode);
    }
    /* Writing: create with requested (slash-converted) name */
    if (strchr(mode, 'w') || strchr(mode, 'a'))
        return fopen(resolved, mode);
    return NULL;
}

extern "C" int open_nocase(const char *filepath, int flags, int mode) {
    if (!filepath)
        return -1;
    /* FF_LINUX: same rule as fopen_nocase - any create/truncate/write open
       may change the on-disk name set, so drop the resolve cache. */
    if ((flags & (O_CREAT | O_TRUNC)) || (flags & O_ACCMODE) == O_WRONLY || (flags & O_ACCMODE) == O_RDWR)
        pc_flush_for_write(filepath, "open write");
    char resolved[2048];
    if (resolve_nocase(filepath, resolved, sizeof(resolved)) == 0)
        return open(resolved, flags, mode);
    if (flags & O_CREAT)
        return open(resolved, flags, mode);
    return -1;
}

/* ============================================================
 * FindFirstFile / FindNextFile (Win32 pattern enumeration)
 * ============================================================ */
struct FF_FIND_CONTEXT {
    DIR *dir;
    char dirPath[1024];
    char pattern[512];
};

static void fill_find_data(FF_FIND_CONTEXT *ctx, const char *name, LPWIN32_FIND_DATAA out) {
    memset(out, 0, sizeof(*out));
    strncpy(out->cFileName, name, MAX_PATH - 1);
    char full[2048];
    snprintf(full, sizeof(full), "%s/%s", ctx->dirPath, name);
    struct stat st;
    if (stat(full, &st) == 0) {
        out->dwFileAttributes = S_ISDIR(st.st_mode) ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
        out->nFileSizeLow = (DWORD)(st.st_size & 0xFFFFFFFF);
        out->nFileSizeHigh = (DWORD)((uint64_t)st.st_size >> 32);
        uint64_t t = ((uint64_t)st.st_mtime + 11644473600ULL) * 10000000ULL;
        out->ftLastWriteTime.dwLowDateTime = (DWORD)t;
        out->ftLastWriteTime.dwHighDateTime = (DWORD)(t >> 32);
        out->ftCreationTime = out->ftLastWriteTime;
        out->ftLastAccessTime = out->ftLastWriteTime;
    } else {
        out->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
    }
}

extern "C" HANDLE FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData) {
    if (!lpFileName)
        return INVALID_HANDLE_VALUE;

    /* Convert backslashes, split into dir + pattern */
    char work[1536];
    size_t i = 0;
    for (; lpFileName[i] && i < sizeof(work) - 1; i++)
        work[i] = (lpFileName[i] == '\\') ? '/' : lpFileName[i];
    work[i] = '\0';

    FF_FIND_CONTEXT *ctx = new FF_FIND_CONTEXT;
    char *slash = strrchr(work, '/');
    if (slash) {
        *slash = '\0';
        /* Resolve directory case-insensitively */
        char resolvedDir[1024];
        if (resolve_nocase(work, resolvedDir, sizeof(resolvedDir)) == 0)
            strncpy(ctx->dirPath, resolvedDir, sizeof(ctx->dirPath) - 1);
        else
            strncpy(ctx->dirPath, work, sizeof(ctx->dirPath) - 1);
        ctx->dirPath[sizeof(ctx->dirPath) - 1] = '\0';
        strncpy(ctx->pattern, slash + 1, sizeof(ctx->pattern) - 1);
    } else {
        strcpy(ctx->dirPath, ".");
        strncpy(ctx->pattern, work, sizeof(ctx->pattern) - 1);
    }
    ctx->pattern[sizeof(ctx->pattern) - 1] = '\0';

    ctx->dir = opendir(ctx->dirPath);
    if (!ctx->dir) {
        delete ctx;
        return INVALID_HANDLE_VALUE;
    }

    struct dirent *e;
    while ((e = readdir(ctx->dir)) != NULL) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
            continue;
        if (fnmatch(ctx->pattern, e->d_name, FNM_CASEFOLD) == 0) {
            fill_find_data(ctx, e->d_name, lpFindFileData);
            return (HANDLE)ctx;
        }
    }
    closedir(ctx->dir);
    delete ctx;
    return INVALID_HANDLE_VALUE;
}

extern "C" BOOL FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData) {
    if (!hFindFile || hFindFile == INVALID_HANDLE_VALUE)
        return FALSE;
    FF_FIND_CONTEXT *ctx = (FF_FIND_CONTEXT *)hFindFile;
    struct dirent *e;
    while ((e = readdir(ctx->dir)) != NULL) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
            continue;
        if (fnmatch(ctx->pattern, e->d_name, FNM_CASEFOLD) == 0) {
            fill_find_data(ctx, e->d_name, lpFindFileData);
            return TRUE;
        }
    }
    return FALSE;
}

extern "C" BOOL FindClose(HANDLE hFindFile) {
    if (!hFindFile || hFindFile == INVALID_HANDLE_VALUE)
        return FALSE;
    FF_FIND_CONTEXT *ctx = (FF_FIND_CONTEXT *)hFindFile;
    closedir(ctx->dir);
    delete ctx;
    return TRUE;
}

/* ============================================================
 * MSVC _findfirst family (struct _finddata_t from io.h)
 * ============================================================ */
#include "io.h"

struct FF_CRT_FIND_CONTEXT {
    DIR *dir;
    char dirPath[1024];
    char pattern[512];
};

static void fill_finddata_t(FF_CRT_FIND_CONTEXT *ctx, const char *name, struct _finddata_t *out) {
    memset(out, 0, sizeof(*out));
    strncpy(out->name, name, sizeof(out->name) - 1);
    char full[2048];
    snprintf(full, sizeof(full), "%s/%s", ctx->dirPath, name);
    struct stat st;
    if (stat(full, &st) == 0) {
        out->attrib = S_ISDIR(st.st_mode) ? _A_SUBDIR : _A_NORMAL;
        out->size = (unsigned long)st.st_size;
        out->time_write = (long)st.st_mtime;
        out->time_access = (long)st.st_atime;
        out->time_create = (long)st.st_ctime;
    }
}

extern "C" intptr_t _findfirst(const char *filespec, struct _finddata_t *fileinfo) {
    if (!filespec)
        return -1;
    char work[1536];
    size_t i = 0;
    for (; filespec[i] && i < sizeof(work) - 1; i++)
        work[i] = (filespec[i] == '\\') ? '/' : filespec[i];
    work[i] = '\0';

    FF_CRT_FIND_CONTEXT *ctx = new FF_CRT_FIND_CONTEXT;
    char *slash = strrchr(work, '/');
    if (slash) {
        *slash = '\0';
        char resolvedDir[1024];
        if (resolve_nocase(work, resolvedDir, sizeof(resolvedDir)) == 0)
            strncpy(ctx->dirPath, resolvedDir, sizeof(ctx->dirPath) - 1);
        else
            strncpy(ctx->dirPath, work, sizeof(ctx->dirPath) - 1);
        ctx->dirPath[sizeof(ctx->dirPath) - 1] = '\0';
        strncpy(ctx->pattern, slash + 1, sizeof(ctx->pattern) - 1);
    } else {
        strcpy(ctx->dirPath, ".");
        strncpy(ctx->pattern, work, sizeof(ctx->pattern) - 1);
    }
    ctx->pattern[sizeof(ctx->pattern) - 1] = '\0';

    ctx->dir = opendir(ctx->dirPath);
    if (!ctx->dir) {
        delete ctx;
        return -1;
    }
    struct dirent *e;
    while ((e = readdir(ctx->dir)) != NULL) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
            continue;
        if (fnmatch(ctx->pattern, e->d_name, FNM_CASEFOLD) == 0) {
            fill_finddata_t(ctx, e->d_name, fileinfo);
            return (intptr_t)ctx;
        }
    }
    closedir(ctx->dir);
    delete ctx;
    return -1;
}

extern "C" int _findnext(intptr_t handle, struct _finddata_t *fileinfo) {
    if (handle == -1 || handle == 0)
        return -1;
    FF_CRT_FIND_CONTEXT *ctx = (FF_CRT_FIND_CONTEXT *)handle;
    struct dirent *e;
    while ((e = readdir(ctx->dir)) != NULL) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
            continue;
        if (fnmatch(ctx->pattern, e->d_name, FNM_CASEFOLD) == 0) {
            fill_finddata_t(ctx, e->d_name, fileinfo);
            return 0;
        }
    }
    return -1;
}

extern "C" int _findclose(intptr_t handle) {
    if (handle == -1 || handle == 0)
        return -1;
    FF_CRT_FIND_CONTEXT *ctx = (FF_CRT_FIND_CONTEXT *)handle;
    closedir(ctx->dir);
    delete ctx;
    return 0;
}

/* ============================================================
 * Path helpers
 * ============================================================ */
extern "C" void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext) {
    if (drive) drive[0] = '\0';
    if (dir) dir[0] = '\0';
    if (fname) fname[0] = '\0';
    if (ext) ext[0] = '\0';
    if (!path) return;

    /* Skip drive letter */
    if (path[0] && path[1] == ':') {
        if (drive) {
            drive[0] = path[0];
            drive[1] = ':';
            drive[2] = '\0';
        }
        path += 2;
    }

    const char *lastSlash = NULL;
    for (const char *p = path; *p; p++)
        if (*p == '/' || *p == '\\')
            lastSlash = p;

    const char *namePart = lastSlash ? lastSlash + 1 : path;
    if (dir && lastSlash) {
        size_t len = (size_t)(lastSlash - path) + 1;
        memcpy(dir, path, len);
        dir[len] = '\0';
    }

    const char *dot = strrchr(namePart, '.');
    if (dot) {
        if (fname) {
            size_t len = (size_t)(dot - namePart);
            memcpy(fname, namePart, len);
            fname[len] = '\0';
        }
        if (ext)
            strcpy(ext, dot);
    } else {
        if (fname)
            strcpy(fname, namePart);
    }
}

extern "C" void _makepath(char *path, const char *drive, const char *dir, const char *fname, const char *ext) {
    if (!path) return;
    path[0] = '\0';
    if (drive && drive[0]) {
        strcat(path, drive);
        if (path[strlen(path) - 1] != ':')
            strcat(path, ":");
    }
    if (dir && dir[0]) {
        strcat(path, dir);
        char last = path[strlen(path) - 1];
        if (last != '/' && last != '\\')
            strcat(path, "/");
    }
    if (fname)
        strcat(path, fname);
    if (ext && ext[0]) {
        if (ext[0] != '.')
            strcat(path, ".");
        strcat(path, ext);
    }
}

extern "C" char *_fullpath(char *absPath, const char *relPath, size_t maxLength) {
    char resolved[4096];
    if (realpath(relPath, resolved)) {
        strncpy(absPath, resolved, maxLength - 1);
        absPath[maxLength - 1] = '\0';
        return absPath;
    }
    strncpy(absPath, relPath, maxLength - 1);
    absPath[maxLength - 1] = '\0';
    return absPath;
}

extern "C" char *_strlwr(char *str) {
    for (char *p = str; *p; p++)
        *p = (char)tolower((unsigned char)*p);
    return str;
}

extern "C" char *_strupr(char *str) {
    for (char *p = str; *p; p++)
        *p = (char)toupper((unsigned char)*p);
    return str;
}

extern "C" char *_itoa(int value, char *str, int radix) {
    if (radix == 16)
        sprintf(str, "%x", value);
    else if (radix == 8)
        sprintf(str, "%o", value);
    else
        sprintf(str, "%d", value);
    return str;
}

extern "C" char *_ltoa(long value, char *str, int radix) {
    if (radix == 16)
        sprintf(str, "%lx", value);
    else if (radix == 8)
        sprintf(str, "%lo", value);
    else
        sprintf(str, "%ld", value);
    return str;
}

extern "C" char *_ultoa(unsigned long value, char *str, int radix) {
    if (radix == 16)
        sprintf(str, "%lx", value);
    else if (radix == 8)
        sprintf(str, "%lo", value);
    else
        sprintf(str, "%lu", value);
    return str;
}

extern "C" char *_gcvt(double value, int digits, char *buffer) {
    sprintf(buffer, "%.*g", digits, value);
    return buffer;
}

extern "C" char *_fcvt(double value, int count, int *dec, int *sign) {
    static char buf[64];
    *sign = value < 0;
    if (value < 0) value = -value;
    snprintf(buf, sizeof(buf), "%.*f", count, value);
    /* strip the decimal point; report its position */
    char *dot = strchr(buf, '.');
    if (dot) {
        *dec = (int)(dot - buf);
        memmove(dot, dot + 1, strlen(dot));
    } else {
        *dec = (int)strlen(buf);
    }
    return buf;
}

extern "C" char *_ecvt(double value, int count, int *dec, int *sign) {
    return _fcvt(value, count, dec, sign);
}

/* ============================================================
 * D3DX surface format mapping
 * ============================================================ */
extern "C" D3DX_SURFACEFORMAT D3DXMakeSurfaceFormat(LPDDPIXELFORMAT pddpf) {
    if (!pddpf)
        return D3DX_SF_UNKNOWN;
    if (pddpf->dwFlags & DDPF_FOURCC) {
        if (pddpf->dwFourCC == MAKEFOURCC('D', 'X', 'T', '1')) return D3DX_SF_DXT1;
        if (pddpf->dwFourCC == MAKEFOURCC('D', 'X', 'T', '3')) return D3DX_SF_DXT3;
        if (pddpf->dwFourCC == MAKEFOURCC('D', 'X', 'T', '5')) return D3DX_SF_DXT5;
        return D3DX_SF_UNKNOWN;
    }
    if (pddpf->dwFlags & DDPF_PALETTEINDEXED8)
        return D3DX_SF_PALETTE8;
    if (pddpf->dwFlags & DDPF_PALETTEINDEXED4)
        return D3DX_SF_PALETTE4;
    if (pddpf->dwFlags & DDPF_RGB) {
        DWORD bits = pddpf->dwRGBBitCount;
        DWORD aMask = (pddpf->dwFlags & DDPF_ALPHAPIXELS) ? pddpf->dwRGBAlphaBitMask : 0;
        if (bits == 32)
            return aMask ? D3DX_SF_A8R8G8B8 : D3DX_SF_X8R8G8B8;
        if (bits == 24)
            return D3DX_SF_R8G8B8;
        if (bits == 16) {
            if (aMask == 0x8000) return D3DX_SF_A1R5G5B5;
            if (aMask == 0xF000) return D3DX_SF_A4R4G4B4;
            if (pddpf->dwGBitMask == 0x07E0) return D3DX_SF_R5G6B5;
            return D3DX_SF_R5G5B5;
        }
        if (bits == 8)
            return D3DX_SF_PALETTE8;
    }
    return D3DX_SF_UNKNOWN;
}

extern "C" HRESULT D3DXMakePixelFormat(D3DX_SURFACEFORMAT fmt, LPDDPIXELFORMAT pddpf) {
    if (!pddpf)
        return E_POINTER;
    memset(pddpf, 0, sizeof(*pddpf));
    pddpf->dwSize = sizeof(DDPIXELFORMAT);
    switch (fmt) {
        case D3DX_SF_A8R8G8B8:
            pddpf->dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
            pddpf->dwRGBBitCount = 32;
            pddpf->dwRBitMask = 0x00FF0000;
            pddpf->dwGBitMask = 0x0000FF00;
            pddpf->dwBBitMask = 0x000000FF;
            pddpf->dwRGBAlphaBitMask = 0xFF000000;
            break;
        case D3DX_SF_X8R8G8B8:
            pddpf->dwFlags = DDPF_RGB;
            pddpf->dwRGBBitCount = 32;
            pddpf->dwRBitMask = 0x00FF0000;
            pddpf->dwGBitMask = 0x0000FF00;
            pddpf->dwBBitMask = 0x000000FF;
            break;
        case D3DX_SF_R5G6B5:
            pddpf->dwFlags = DDPF_RGB;
            pddpf->dwRGBBitCount = 16;
            pddpf->dwRBitMask = 0xF800;
            pddpf->dwGBitMask = 0x07E0;
            pddpf->dwBBitMask = 0x001F;
            break;
        case D3DX_SF_A4R4G4B4:
            pddpf->dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
            pddpf->dwRGBBitCount = 16;
            pddpf->dwRBitMask = 0x0F00;
            pddpf->dwGBitMask = 0x00F0;
            pddpf->dwBBitMask = 0x000F;
            pddpf->dwRGBAlphaBitMask = 0xF000;
            break;
        case D3DX_SF_A1R5G5B5:
            pddpf->dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
            pddpf->dwRGBBitCount = 16;
            pddpf->dwRBitMask = 0x7C00;
            pddpf->dwGBitMask = 0x03E0;
            pddpf->dwBBitMask = 0x001F;
            pddpf->dwRGBAlphaBitMask = 0x8000;
            break;
        case D3DX_SF_PALETTE8:
            pddpf->dwFlags = DDPF_PALETTEINDEXED8 | DDPF_RGB;
            pddpf->dwRGBBitCount = 8;
            break;
        default:
            return E_FAIL;
    }
    return S_OK;
}

extern "C" unsigned int vuxGameTime_ff_probe(void) {
    extern unsigned int vuxGameTime;  /* VU_TIME (uint32_t) */
    return vuxGameTime;
}

extern "C" HRESULT D3DXInitialize(void) { return S_OK; }
extern "C" HRESULT D3DXUninitialize(void) { return S_OK; }

/* ============================================================
 * DirectDraw entry points
 * ============================================================ */
extern "C" HRESULT DirectDrawCreateEx(GUID *lpGuid, LPVOID *lplpDD, REFIID iid, IUnknown *pUnkOuter) {
    (void)lpGuid; (void)iid; (void)pUnkOuter;
    if (!lplpDD)
        return DDERR_INVALIDPARAMS;
    *lplpDD = (LPVOID)FF_CreateDirectDraw7();
    return *lplpDD ? DD_OK : DDERR_GENERIC;
}

extern "C" HRESULT DirectDrawCreate(GUID *lpGUID, LPDIRECTDRAW *lplpDD, IUnknown *pUnkOuter) {
    (void)lpGUID; (void)pUnkOuter;
    if (!lplpDD)
        return DDERR_INVALIDPARAMS;
    *lplpDD = (LPDIRECTDRAW)FF_CreateDirectDraw7();
    return *lplpDD ? DD_OK : DDERR_GENERIC;
}

extern "C" HRESULT DirectDrawEnumerateA(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext) {
    if (lpCallback)
        lpCallback(NULL, (LPSTR)"OpenGL Display Driver", (LPSTR)"display", lpContext);
    return DD_OK;
}

extern "C" HRESULT DirectDrawEnumerateExA(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags) {
    (void)dwFlags;
    if (lpCallback)
        lpCallback(NULL, (LPSTR)"OpenGL Display Driver", (LPSTR)"display", lpContext, NULL);
    return DD_OK;
}

/* ============================================================
 * Real GetPrivateProfileString/Int backend.
 *
 * The compat-header versions were stubs that returned the supplied
 * default, which silently zeroed EVERY .ini-loaded tuning value: all
 * campaign AI inputs (Falcon4.AII via ReadCampAIInputs -
 * REAGREGATION_RATIO=0 caused the constant aggregation flap, issue #5),
 * game rules, force-feedback effects, cockpit settings, etc.
 *
 * Windows semantics implemented:
 *  - section/key matching is case-insensitive
 *  - whitespace around key and value is trimmed
 *  - surrounding single/double quotes on the value are stripped
 *  - ';' starts a comment line
 *  - CRLF files: trailing \r is stripped (game data is Windows text)
 * ============================================================ */

static char *ff_ini_trim(char *s) {
    while (*s == ' ' || *s == '\t') s++;
    char *end = s + strlen(s);
    while (end > s && (end[-1] == ' ' || end[-1] == '\t' ||
                       end[-1] == '\r' || end[-1] == '\n'))
        *--end = '\0';
    return s;
}

/* Find [section] key= in file; copy trimmed value into out (size bytes).
 * Returns 1 if found, 0 if not. */
extern "C" int FF_IniGetValue(const char *section, const char *key,
                              char *out, unsigned size, const char *file) {
    if (!section || !key || !out || !size || !file)
        return 0;

    FILE *fp = fopen_nocase(file, "r");
    if (!fp)
        return 0;

    char line[512];
    int inSection = 0;
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        char *s = ff_ini_trim(line);

        if (*s == '\0' || *s == ';')
            continue;

        if (*s == '[') {
            char *close = strchr(s, ']');
            if (close) {
                *close = '\0';
                inSection = (strcasecmp(s + 1, section) == 0);
            }
            continue;
        }

        if (!inSection)
            continue;

        char *eq = strchr(s, '=');
        if (!eq)
            continue;

        *eq = '\0';
        char *k = ff_ini_trim(s);
        if (strcasecmp(k, key) != 0)
            continue;

        char *v = ff_ini_trim(eq + 1);

        /* Windows strips matching surrounding quotes */
        size_t vlen = strlen(v);
        if (vlen >= 2 && ((v[0] == '"' && v[vlen - 1] == '"') ||
                          (v[0] == '\'' && v[vlen - 1] == '\''))) {
            v[vlen - 1] = '\0';
            v++;
        }

        strncpy(out, v, size - 1);
        out[size - 1] = '\0';
        found = 1;
        break;
    }

    fclose(fp);
    return found;
}

/* ============================================================
 * A small file-backed registry
 *
 * The Reg* entry points used to be stubs that reported failure for
 * every call. That is not as harmless as it looks: the game stores
 * the current pilot's callsign under HKLM, and UI_Init() treats "no
 * pilot could be loaded" as "first ever run", which re-runs
 * LogBook/PlayerOptions/DisplayOptions Initialize(). Those defaults
 * were then written back over the player's config on exit, so no
 * Setup change -- resolution included -- could survive a restart.
 *
 * Values live in config/registry.ini, alongside the other config
 * files. The game chdir()s to FalconDataDirectory during startup, so
 * a relative path keeps this free of any dependency on falclib.
 *
 * Two deliberate departures from the Win32 API:
 *
 *  - Opening a key for write access creates it. On Windows the
 *    installer creates HKLM\Software\MicroProse\Falcon\4.0; there is
 *    no installer here, and every writer in this codebase opens the
 *    key and gives up if the open fails, so nothing could ever have
 *    created it.
 *
 *  - REG_SZ values are stored with a terminating NUL even when the
 *    caller's cbData excludes it (they all pass strlen()). Callers
 *    such as TheaterList::GetCurrentTheater() read straight into an
 *    uninitialised stack buffer and would otherwise be left holding an
 *    unterminated string.
 * ============================================================ */

#include <map>
#include <string>
#include <vector>

namespace {

typedef std::vector<unsigned char> RegBlob;

struct RegValue {
    DWORD   type;
    RegBlob data;
};

typedef std::map<std::string, RegValue> RegValues;   /* value name -> value  */
typedef std::map<std::string, RegValues> RegStore;   /* key path   -> values */

RegStore  g_regStore;
bool      g_regLoaded = false;

/* Interned so an HKEY stays valid until exit and RegCloseKey can be a no-op. */
std::map<std::string, std::string *> g_regHandles;

const char *REG_STORE_PATH = "config/registry.ini";

const char *RegRootName(HKEY h) {
    switch ((unsigned long)(uintptr_t)h) {
        case 0x80000000UL: return "HKCR";
        case 0x80000001UL: return "HKCU";
        case 0x80000002UL: return "HKLM";
        case 0x80000003UL: return "HKU";
        default:           return NULL;
    }
}

/* A predefined root, or a handle we previously handed out. */
bool RegPathOf(HKEY h, std::string &out) {
    const char *root = RegRootName(h);

    if (root) {
        out = root;
        return true;
    }

    if (!h) return false;

    /* Only trust handles we minted ourselves. */
    for (std::map<std::string, std::string *>::iterator it = g_regHandles.begin();
         it != g_regHandles.end(); ++it) {
        if (it->second == (std::string *)h) {
            out = *it->second;
            return true;
        }
    }

    return false;
}

HKEY RegHandleFor(const std::string &path) {
    std::map<std::string, std::string *>::iterator it = g_regHandles.find(path);

    if (it == g_regHandles.end())
        it = g_regHandles.insert(std::make_pair(path, new std::string(path))).first;

    return (HKEY)it->second;
}

int RegHexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

void RegLoadStore() {
    if (g_regLoaded) return;

    g_regLoaded = true;

    FILE *fp = fopen_nocase(REG_STORE_PATH, "r");

    if (!fp) return;

    char line[8192];
    std::string section;

    while (fgets(line, sizeof(line), fp)) {
        char *s = line;

        while (*s == ' ' || *s == '\t') s++;

        size_t n = strlen(s);

        while (n && (s[n - 1] == '\n' || s[n - 1] == '\r' || s[n - 1] == ' ' || s[n - 1] == '\t'))
            s[--n] = '\0';

        if (!*s || *s == ';' || *s == '#') continue;

        if (*s == '[') {
            char *end = strchr(s, ']');

            if (end) {
                *end = '\0';
                section = s + 1;
            }

            continue;
        }

        char *eq = strchr(s, '=');

        if (!eq || section.empty()) continue;

        *eq = '\0';
        std::string name(s);

        char *v = eq + 1;
        char *comma = strchr(v, ',');

        if (!comma) continue;

        *comma = '\0';
        RegValue val;
        val.type = (DWORD)strtoul(v, NULL, 10);

        for (char *h = comma + 1; h[0] && h[1]; h += 2) {
            int hi = RegHexVal(h[0]), lo = RegHexVal(h[1]);

            if (hi < 0 || lo < 0) break;

            val.data.push_back((unsigned char)((hi << 4) | lo));
        }

        g_regStore[section][name] = val;
    }

    fclose(fp);
}

void RegSaveStore() {
    /* Written whole each time: this holds a handful of values. */
    FILE *fp = fopen_nocase(REG_STORE_PATH, "w");

    if (!fp) return;

    fprintf(fp, "; FreeFalcon Linux port - registry substitute. Generated file.\n");

    for (RegStore::iterator k = g_regStore.begin(); k != g_regStore.end(); ++k) {
        fprintf(fp, "\n[%s]\n", k->first.c_str());

        for (RegValues::iterator v = k->second.begin(); v != k->second.end(); ++v) {
            fprintf(fp, "%s=%lu,", v->first.c_str(), (unsigned long)v->second.type);

            for (size_t i = 0; i < v->second.data.size(); i++)
                fprintf(fp, "%02X", v->second.data[i]);

            fprintf(fp, "\n");
        }
    }

    fclose(fp);
}

} /* namespace */

extern "C" LONG FF_RegOpenKey(HKEY parent, const char *subkey, REGSAM sam, PHKEY out) {
    if (out) *out = NULL;

    RegLoadStore();

    std::string base;

    if (!RegPathOf(parent, base)) return ERROR_FILE_NOT_FOUND;

    std::string path = base;

    if (subkey && *subkey) {
        path += "\\";
        path += subkey;
    }

    const bool wantWrite = (sam & (KEY_WRITE | KEY_ALL_ACCESS)) != 0;

    if (g_regStore.find(path) == g_regStore.end()) {
        if (!wantWrite) return ERROR_FILE_NOT_FOUND;

        g_regStore[path];  /* create empty -- see the note above */
    }

    if (out) *out = RegHandleFor(path);

    return ERROR_SUCCESS;
}

extern "C" LONG FF_RegQueryValue(HKEY key, const char *name, LPDWORD type,
                                 LPBYTE data, LPDWORD size) {
    RegLoadStore();

    std::string path;

    if (!name || !RegPathOf(key, path)) return ERROR_FILE_NOT_FOUND;

    RegStore::iterator k = g_regStore.find(path);

    if (k == g_regStore.end()) return ERROR_FILE_NOT_FOUND;

    RegValues::iterator v = k->second.find(name);

    if (v == k->second.end()) return ERROR_FILE_NOT_FOUND;

    if (type) *type = v->second.type;

    const DWORD stored = (DWORD)v->second.data.size();

    if (!data) {
        if (size) *size = stored;

        return ERROR_SUCCESS;
    }

    if (!size) return ERROR_INVALID_PARAMETER;

    if (*size < stored) {
        *size = stored;
        return ERROR_MORE_DATA;
    }

    if (stored) memcpy(data, &v->second.data[0], stored);

    *size = stored;
    return ERROR_SUCCESS;
}

/* ------------------------------------------------------------
 * SetWindowPos
 *
 * Another stub that returned TRUE without doing anything. The game
 * resizes its window when it switches between the UI (1024x768 art)
 * and the sim (whatever resolution the player chose in Setup), so
 * with this inert the sim was rendered into a device of the chosen
 * size while the window stayed at the UI's.
 *
 * Only the main SDL window is touched -- the rest of the HWNDs in
 * this port are opaque non-window handles and must not be passed to
 * SDL.
 * ------------------------------------------------------------ */
/* Desktop size. Returning 0 here is what made the windowed-mode clamp in
 * FalconDisplayConfiguration::EnterMode compute a negative window size for any
 * resolution. Only the screen metrics are answered; every other index keeps the
 * old zero, since nothing in FFViper asks for them. */
extern "C" int FF_GetSystemMetrics(int nIndex) {
    if (nIndex == 0 || nIndex == 1 || nIndex == 16 || nIndex == 17) {
        SDL_DisplayMode dm;

        if (SDL_WasInit(SDL_INIT_VIDEO) && SDL_GetDesktopDisplayMode(0, &dm) == 0)
            return (nIndex == 0 || nIndex == 16) ? dm.w : dm.h;
    }

    return 0;
}

extern "C" BOOL FF_SetWindowPos(HWND hWnd, int X, int Y, int cx, int cy, UINT uFlags) {
    extern SDL_Window *g_SDLWindow;

    if (!g_SDLWindow || hWnd != (HWND)g_SDLWindow) return TRUE;

    if (!(uFlags & 0x0001 /* SWP_NOSIZE */) && cx > 0 && cy > 0)
        SDL_SetWindowSize(g_SDLWindow, cx, cy);

    if (!(uFlags & 0x0002 /* SWP_NOMOVE */))
        SDL_SetWindowPosition(g_SDLWindow, X, Y);

    return TRUE;
}

extern "C" LONG FF_RegSetValue(HKEY key, const char *name, DWORD type,
                               const BYTE *data, DWORD size) {
    RegLoadStore();

    std::string path;

    if (!name || !RegPathOf(key, path)) return ERROR_FILE_NOT_FOUND;

    RegValue val;
    val.type = type;

    if (data && size) val.data.assign(data, data + size);

    /* See the REG_SZ note above. */
    if (type == REG_SZ && (val.data.empty() || val.data.back() != 0))
        val.data.push_back(0);

    g_regStore[path][name] = val;
    RegSaveStore();

    return ERROR_SUCCESS;
}

#endif /* FF_LINUX */

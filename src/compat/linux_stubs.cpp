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
static int resolve_nocase(const char *filepath, char *resolved, size_t resolvedSize) {
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

extern "C" FILE *fopen_nocase(const char *filepath, const char *mode) {
    if (!filepath || !mode)
        return NULL;
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

#endif /* FF_LINUX */

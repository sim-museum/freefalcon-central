/*
 * FreeFalcon Linux Port - Windows base types
 *
 * Fixed-width mappings of the Win32 base types for 64-bit Linux.
 * IMPORTANT: DWORD/LONG/ULONG stay 32-bit (as on Windows) so that
 * binary file formats and struct layouts match the original game data.
 */

#ifndef FF_COMPAT_TYPES_H
#define FF_COMPAT_TYPES_H

#ifdef FF_LINUX

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

/* ============================================================
 * Calling conventions - all empty on Linux (System V ABI)
 * ============================================================ */
#ifndef __cdecl
#define __cdecl
#endif
#ifndef __stdcall
#define __stdcall
#endif
#ifndef __fastcall
#define __fastcall
#endif
#ifndef CALLBACK
#define CALLBACK
#endif
#ifndef WINAPI
#define WINAPI
#endif
#ifndef WINAPIV
#define WINAPIV
#endif
#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef PASCAL
#define PASCAL
#endif
#ifndef FAR
#define FAR
#endif
#ifndef NEAR
#define NEAR
#endif
#ifndef _cdecl
#define _cdecl
#endif
#ifndef _stdcall
#define _stdcall
#endif
#ifndef _fastcall
#define _fastcall
#endif
#ifndef _pascal
#define _pascal
#endif
#ifndef __forceinline
#define __forceinline inline
#endif
#ifndef __declspec
#define __declspec(x)
#endif
#ifndef _declspec
#define _declspec(x)
#endif

/* MSVC integer suffix types */
typedef int64_t  __int64;
typedef int32_t  __int32;
typedef int16_t  __int16;
typedef int8_t   __int8;

/* ============================================================
 * Base integer types (Windows sizes!)
 * ============================================================ */
typedef int                 BOOL;
typedef unsigned char       BOOLEAN;
typedef BOOL               *PBOOL, *LPBOOL;
typedef unsigned char       BYTE, *PBYTE, *LPBYTE;
typedef unsigned char       UCHAR, *PUCHAR;
typedef char                CHAR, *PCHAR;
typedef unsigned short      WORD, *PWORD, *LPWORD;
typedef short               SHORT, *PSHORT;
typedef unsigned short      USHORT, *PUSHORT;
typedef uint32_t            DWORD, *PDWORD, *LPDWORD;
typedef int32_t             LONG, *PLONG, *LPLONG;
typedef uint32_t            ULONG, *PULONG;
typedef int                 INT, *PINT, *LPINT;
typedef unsigned int        UINT, *PUINT;
typedef float               FLOAT, *PFLOAT;
typedef double              DOUBLE;
typedef void               *PVOID, *LPVOID;
typedef const void         *LPCVOID;
typedef int64_t             LONGLONG;
typedef uint64_t            ULONGLONG, DWORDLONG;
typedef uint64_t            DWORD64, ULONG64;
typedef int64_t             LONG64, INT64;
typedef uint64_t            UINT64;
typedef uint32_t            UINT32;
typedef int32_t             INT32;
typedef int16_t             INT16;
typedef uint16_t            UINT16;
typedef int8_t              INT8;
typedef uint8_t             UINT8;
/* WCHAR maps to native wchar_t (32-bit) so wcslen/wcscpy/L"" work.
 * The game does not persist WCHAR in binary files. */
#include <wchar.h>
typedef wchar_t             WCHAR, *PWCHAR;

/* Pointer-sized integers */
typedef intptr_t            INT_PTR, *PINT_PTR;
typedef uintptr_t           UINT_PTR, *PUINT_PTR;
typedef intptr_t            LONG_PTR, *PLONG_PTR;
typedef uintptr_t           ULONG_PTR, *PULONG_PTR;
typedef uintptr_t           DWORD_PTR, *PDWORD_PTR;
typedef size_t              SIZE_T;
typedef ptrdiff_t           SSIZE_T;

/* String types */
typedef char               *PSTR, *LPSTR;
typedef const char         *PCSTR, *LPCSTR;
typedef WCHAR              *PWSTR, *LPWSTR;
typedef const WCHAR        *PCWSTR, *LPCWSTR;
typedef char                TCHAR, *PTCHAR, *PTSTR, *LPTSTR;
typedef const char         *PCTSTR, *LPCTSTR;

/* Message-loop types */
typedef UINT_PTR            WPARAM;
typedef LONG_PTR            LPARAM;
typedef LONG_PTR            LRESULT;

typedef int32_t             HRESULT;
typedef WORD                ATOM;
typedef DWORD               COLORREF, *LPCOLORREF;
typedef LONG               *LPLONG_T;

/* Identity macros: legacy headers guard their own typedefs with
 * "#ifndef ULONG" etc., which only works if these are macros. */
#define BYTE   BYTE
#define ULONG  ULONG
#define SHORT  SHORT
#define LONG   LONG
#define DWORD  DWORD
#define WORD   WORD
#define UINT   UINT
#define BOOL   BOOL
#define CHAR   CHAR
#define UCHAR  UCHAR
#define USHORT USHORT
#define FLOAT  FLOAT

#ifndef VOID
#define VOID void
#endif

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef CONST
#define CONST const
#endif

/* ============================================================
 * Handle types - all opaque pointers
 * ============================================================ */
typedef void *HANDLE, **PHANDLE, **LPHANDLE;
#define DECLARE_HANDLE(name) typedef void *name
DECLARE_HANDLE(HWND);
DECLARE_HANDLE(HINSTANCE);
typedef HINSTANCE HMODULE;
DECLARE_HANDLE(HICON);
typedef HICON HCURSOR;
DECLARE_HANDLE(HBRUSH);
DECLARE_HANDLE(HMENU);
DECLARE_HANDLE(HACCEL);
DECLARE_HANDLE(HDC);
DECLARE_HANDLE(HBITMAP);
DECLARE_HANDLE(HFONT);
DECLARE_HANDLE(HPEN);
DECLARE_HANDLE(HRGN);
DECLARE_HANDLE(HPALETTE);
DECLARE_HANDLE(HGDIOBJ);
DECLARE_HANDLE(HKEY);
typedef HKEY *PHKEY;
DECLARE_HANDLE(HGLOBAL);
DECLARE_HANDLE(HLOCAL);
DECLARE_HANDLE(HHOOK);
DECLARE_HANDLE(HMONITOR);
DECLARE_HANDLE(HDROP);
DECLARE_HANDLE(HRSRC);
DECLARE_HANDLE(HMMIO);
DECLARE_HANDLE(HWAVEOUT);
DECLARE_HANDLE(HWAVEIN);

#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)

/* ============================================================
 * Geometry structs
 * ============================================================ */
typedef struct tagPOINT {
    LONG x;
    LONG y;
} POINT, *PPOINT, *LPPOINT;

typedef struct tagPOINTS {
    SHORT x;
    SHORT y;
} POINTS, *PPOINTS, *LPPOINTS;

typedef struct tagRECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT, *PRECT, *LPRECT;
typedef const RECT *LPCRECT;

typedef struct tagSIZE {
    LONG cx;
    LONG cy;
} SIZE, *PSIZE, *LPSIZE;

/* ============================================================
 * Time / misc structs
 * ============================================================ */
typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG  HighPart;
    };
    struct {
        DWORD LowPart;
        LONG  HighPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef union _ULARGE_INTEGER {
    struct {
        DWORD LowPart;
        DWORD HighPart;
    };
    ULONGLONG QuadPart;
} ULARGE_INTEGER, *PULARGE_INTEGER;

typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

/* ============================================================
 * GUID
 * ============================================================ */
#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID {
    DWORD Data1;
    WORD  Data2;
    WORD  Data3;
    BYTE  Data4[8];
} GUID;
#endif

typedef GUID IID, CLSID, FMTID;
typedef GUID *LPGUID, *LPIID, *LPCLSID;
typedef const GUID *LPCGUID;

#ifdef __cplusplus
#define REFGUID  const GUID &
#define REFIID   const IID &
#define REFCLSID const CLSID &
#else
#define REFGUID  const GUID *
#define REFIID   const IID *
#define REFCLSID const CLSID *
#endif

/* ============================================================
 * Common macros
 * ============================================================ */
#ifndef MAKEWORD
#define MAKEWORD(a, b)   ((WORD)(((BYTE)((DWORD_PTR)(a) & 0xff)) | ((WORD)((BYTE)((DWORD_PTR)(b) & 0xff))) << 8))
#endif
#ifndef MAKELONG
#define MAKELONG(a, b)   ((LONG)(((WORD)((DWORD_PTR)(a) & 0xffff)) | ((DWORD)((WORD)((DWORD_PTR)(b) & 0xffff))) << 16))
#endif
#ifndef LOWORD
#define LOWORD(l)        ((WORD)((DWORD_PTR)(l) & 0xffff))
#endif
#ifndef HIWORD
#define HIWORD(l)        ((WORD)((DWORD_PTR)(l) >> 16))
#endif
#ifndef LOBYTE
#define LOBYTE(w)        ((BYTE)((DWORD_PTR)(w) & 0xff))
#endif
#ifndef HIBYTE
#define HIBYTE(w)        ((BYTE)((DWORD_PTR)(w) >> 8))
#endif

/*
 * min/max macros: legacy game code relies on these, but they poison
 * C++ standard headers (std::numeric_limits<T>::min() etc.).
 * Pre-include the common std headers BEFORE defining the macros so their
 * include guards make later inclusions no-ops.
 */
#ifdef __cplusplus
#include <cmath>
#include <climits>
#include <limits>
#include <algorithm>
#include <utility>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <queue>
#include <deque>
#include <stack>
#include <memory>
#include <functional>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <random>
#include <stdexcept>
#include <typeinfo>
#include <iterator>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <numeric>
#include <valarray>
#include <complex>
#include <regex>
#include <array>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define ZeroMemory(dest, len)        memset((dest), 0, (len))
#define CopyMemory(dest, src, len)   memcpy((dest), (src), (len))
#define MoveMemory(dest, src, len)   memmove((dest), (src), (len))
#define FillMemory(dest, len, fill)  memset((dest), (fill), (len))

#define UNREFERENCED_PARAMETER(p) (void)(p)

/* ============================================================
 * MSVC CRT string function aliases
 * ============================================================ */
#include <strings.h>
#ifndef stricmp
#define stricmp    strcasecmp
#endif
#ifndef strcmpi
#define strcmpi    strcasecmp
#endif
#ifndef strnicmp
#define strnicmp   strncasecmp
#endif
#define _stricmp   strcasecmp
#define _strcmpi   strcasecmp
#define _strnicmp  strncasecmp
#define _snprintf  snprintf
#define _vsnprintf vsnprintf
#define _atoi64    atoll

#ifdef __cplusplus
extern "C" {
#endif
char *_strlwr(char *str);
char *_strupr(char *str);
char *_itoa(int value, char *str, int radix);
char *_ltoa(long value, char *str, int radix);
char *_ultoa(unsigned long value, char *str, int radix);
char *_gcvt(double value, int digits, char *buffer);
char *_fcvt(double value, int count, int *dec, int *sign);
char *_ecvt(double value, int count, int *dec, int *sign);
#ifdef __cplusplus
}
#endif
#define strlwr _strlwr
#define strupr _strupr
#define ff_max(a, b) (((a) > (b)) ? (a) : (b))
#define ff_min(a, b) (((a) < (b)) ? (a) : (b))

/* MSVC SSE alignment macro */
#ifndef _MM_ALIGN16
#define _MM_ALIGN16 __attribute__((aligned(16)))
#endif

/* MSVC math aliases */
#include <math.h>
#define _isnan(x)     isnan(x)
#define _finite(x)    isfinite(x)
#define _copysign(x, y) copysign((x), (y))
#define _hypot        hypot
#define _j0 j0
#define _j1 j1

/* MSVC ctype aliases */
#include <ctype.h>
#define _istlower(c) islower(c)
#define _istupper(c) isupper(c)
#define _istalpha(c) isalpha(c)
#define _istdigit(c) isdigit(c)
#define _istspace(c) isspace(c)

#ifndef _ASSERTE
#define _ASSERTE(x) ((void)0)
#endif

/* ============================================================
 * Global case-insensitive fopen redirect.
 * Game data comes from Windows where paths are case-insensitive;
 * redirect every fopen() to the resolving wrapper. Compilation
 * units that need the real fopen define FF_NO_FOPEN_REDIRECT.
 * ============================================================ */
#ifdef __cplusplus
extern "C" {
#endif
FILE *fopen_nocase(const char *filepath, const char *mode);
#ifdef __cplusplus
}
#endif
#ifndef FF_NO_FOPEN_REDIRECT
#define fopen fopen_nocase
#endif
#define itoa   _itoa
#define ltoa   _ltoa
#define ultoa  _ultoa

#ifndef _MAX_PATH
#define _MAX_PATH 260
#endif
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#ifndef _MAX_DIR
#define _MAX_DIR 256
#endif
#ifndef _MAX_FNAME
#define _MAX_FNAME 256
#endif
#ifndef _MAX_EXT
#define _MAX_EXT 256
#endif
#ifndef _MAX_DRIVE
#define _MAX_DRIVE 3
#endif

/* Text macro (ANSI build) */
#ifndef TEXT
#define TEXT(s) s
#endif
#ifndef _T
#define _T(s) s
#endif

#endif /* FF_LINUX */
#endif /* FF_COMPAT_TYPES_H */

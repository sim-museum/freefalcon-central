/* FreeFalcon Linux Port - tchar.h compatibility (ANSI build) */
#ifndef FF_COMPAT_TCHAR_H
#define FF_COMPAT_TCHAR_H
#ifdef FF_LINUX
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

typedef char _TCHAR;
#ifndef _T
#define _T(x) x
#endif
#ifndef TEXT
#define TEXT(x) x
#endif

#define _tcscpy   strcpy
#define _tcsncpy  strncpy
#define _tcscat   strcat
#define _tcsncat  strncat
#define _tcslen   strlen
#define _tcsclen  strlen
#define _tcscmp   strcmp
#define _tcsncmp  strncmp
#define _tcsicmp  strcasecmp
#define _tcsnicmp strncasecmp
#define _tcschr   strchr
#define _tcsrchr  strrchr
#define _tcsstr   strstr
#define _tcstok   strtok
#define _tcstol   strtol
#define _tcstoul  strtoul
#define _tcstod   strtod
#define _stprintf sprintf
#define _sntprintf snprintf
#define _vstprintf vsprintf
#define _vsntprintf vsnprintf
#define _stscanf  sscanf
#define _tfopen   fopen
#define _ttoi     atoi
#define _ttol     atol
#define _itot     _itoa
#define _tprintf  printf
#define _ftprintf fprintf
#define _fgetts   fgets
#define _fputts   fputs
#define _totupper toupper
#define _totlower tolower
#define _istdigit isdigit
#define _istalpha isalpha
#define _istspace isspace
#define _tcsspnp(s, c) ({ const char *_p = (s) + strspn((s), (c)); (*_p) ? (char *)_p : (char *)0; })
#define _tcsncicmp strncasecmp
#define _tcsdup strdup
#define _trename rename
#define _tcscspn strcspn
#define _tcsspn  strspn
#define _istalnum isalnum
#define _tremove remove
#define _tunlink unlink

#endif
#endif

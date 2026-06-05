/*
 * FreeFalcon Linux Port - windows.h umbrella header
 */
#ifndef FF_COMPAT_WINDOWS_H
#define FF_COMPAT_WINDOWS_H

#ifdef FF_LINUX

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>

#define _INC_WINDOWS      /* MSVC windows.h guard - legacy code checks this */
#define _WINDOWS_
#define __WINDOWS_H

#include "compat_types.h"
#include "compat_winbase.h"
#include "compat_wingdi.h"
#include "compat_winuser.h"

/* Legacy user32 helpers */
static inline LONG GetMessageTime(void) { return (LONG)GetTickCount(); }
#define GetCurrentTime() GetTickCount()

#ifndef WINVER
#define WINVER 0x0500
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#endif /* FF_LINUX */
#endif /* FF_COMPAT_WINDOWS_H */

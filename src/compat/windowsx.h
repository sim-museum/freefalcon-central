/* FreeFalcon Linux Port - windowsx.h compatibility */
#ifndef FF_COMPAT_WINDOWSX_H
#define FF_COMPAT_WINDOWSX_H
#ifdef FF_LINUX
#include "windows.h"
#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif
#define GlobalAllocPtr(flags, cb) (GlobalAlloc((flags), (cb)))
#define GlobalFreePtr(lp)         (GlobalFree((HGLOBAL)(lp)))
#define Edit_SetText(hwnd, str)   SetWindowTextA((hwnd), (str))
#define Edit_GetText(hwnd, str, n) GetWindowTextA((hwnd), (str), (n))
#define Button_SetCheck(hwnd, c)  ((void)0)
#define Button_GetCheck(hwnd)     0
#define ComboBox_AddString(h, s)  0
#define ComboBox_SetCurSel(h, i)  0
#define ComboBox_GetCurSel(h)     0
#define ListBox_AddString(h, s)   0
#endif
#endif

/* FreeFalcon Linux Port - commctrl.h stub */
#ifndef FF_COMPAT_COMMCTRL_H
#define FF_COMPAT_COMMCTRL_H
#ifdef FF_LINUX
#include "compat_types.h"

#define WM_USER_CC 0x0400
#define TBM_GETPOS       (WM_USER_CC)
#define TBM_GETRANGEMIN  (WM_USER_CC + 1)
#define TBM_GETRANGEMAX  (WM_USER_CC + 2)
#define TBM_SETPOS       (WM_USER_CC + 5)
#define TBM_SETRANGE     (WM_USER_CC + 6)
#define TBM_SETRANGEMIN  (WM_USER_CC + 7)
#define TBM_SETRANGEMAX  (WM_USER_CC + 8)
#define TBM_SETPAGESIZE  (WM_USER_CC + 21)
#define TBM_SETTICFREQ   (WM_USER_CC + 20)

static inline void InitCommonControls(void) {}

#endif
#endif

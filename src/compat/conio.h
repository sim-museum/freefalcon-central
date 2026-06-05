/* FreeFalcon Linux Port - conio.h compatibility */
#ifndef FF_COMPAT_CONIO_H
#define FF_COMPAT_CONIO_H
#ifdef FF_LINUX
#include <stdio.h>
static inline int _kbhit(void) { return 0; }
static inline int _getch(void) { return getchar(); }
static inline int _putch(int c) { return putchar(c); }
#define kbhit _kbhit
#define getch _getch
static inline int _outp(unsigned short port, int v) { (void)port; (void)v; return v; }
static inline unsigned short _outpw(unsigned short port, unsigned short v) { (void)port; (void)v; return v; }
static inline int _inp(unsigned short port) { (void)port; return 0; }
#define outp  _outp
#define outpw _outpw
#define inp   _inp
#endif
#endif

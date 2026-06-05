/* FreeFalcon Linux Port - winsock.h compatibility (BSD sockets shim) */
#ifndef FF_COMPAT_WINSOCK_H
#define FF_COMPAT_WINSOCK_H
#ifdef FF_LINUX

#include "compat_types.h"
#include "compat_winbase.h"
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)

typedef struct sockaddr     SOCKADDR, *PSOCKADDR, *LPSOCKADDR;
typedef struct sockaddr_in  SOCKADDR_IN, *PSOCKADDR_IN, *LPSOCKADDR_IN;
typedef struct in_addr      IN_ADDR, *PIN_ADDR, *LPIN_ADDR;
typedef struct hostent      HOSTENT, *PHOSTENT, *LPHOSTENT;
typedef struct timeval      TIMEVAL, *PTIMEVAL, *LPTIMEVAL;
typedef fd_set FD_SET;

#define WSADESCRIPTION_LEN 256
#define WSASYS_STATUS_LEN  128

typedef struct WSAData {
    WORD  wVersion;
    WORD  wHighVersion;
    char  szDescription[WSADESCRIPTION_LEN + 1];
    char  szSystemStatus[WSASYS_STATUS_LEN + 1];
    unsigned short iMaxSockets;
    unsigned short iMaxUdpDg;
    char *lpVendorInfo;
} WSADATA, *LPWSADATA;

static inline int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData) {
    if (lpWSAData) {
        memset(lpWSAData, 0, sizeof(*lpWSAData));
        lpWSAData->wVersion = wVersionRequested;
        lpWSAData->wHighVersion = wVersionRequested;
    }
    return 0;
}
static inline int WSACleanup(void) { return 0; }
static inline int WSAGetLastError(void) { return errno; }
static inline void WSASetLastError(int iError) { errno = iError; }

#define closesocket close
static inline int ioctlsocket(SOCKET s, long cmd, u_long *argp) {
    return ioctl(s, (unsigned long)cmd, argp);
}

/* Winsock error codes -> errno */
#define WSABASEERR        10000
#define WSAEINTR          EINTR
#define WSAEBADF          EBADF
#define WSAEACCES         EACCES
#define WSAEFAULT         EFAULT
#define WSAEINVAL         EINVAL
#define WSAEMFILE         EMFILE
#define WSAEWOULDBLOCK    EWOULDBLOCK
#define WSAEINPROGRESS    EINPROGRESS
#define WSAEALREADY       EALREADY
#define WSAENOTSOCK       ENOTSOCK
#define WSAEDESTADDRREQ   EDESTADDRREQ
#define WSAEMSGSIZE       EMSGSIZE
#define WSAEPROTOTYPE     EPROTOTYPE
#define WSAENOPROTOOPT    ENOPROTOOPT
#define WSAEPROTONOSUPPORT EPROTONOSUPPORT
#define WSAEOPNOTSUPP     EOPNOTSUPP
#define WSAEAFNOSUPPORT   EAFNOSUPPORT
#define WSAEADDRINUSE     EADDRINUSE
#define WSAEADDRNOTAVAIL  EADDRNOTAVAIL
#define WSAENETDOWN       ENETDOWN
#define WSAENETUNREACH    ENETUNREACH
#define WSAENETRESET      ENETRESET
#define WSAECONNABORTED   ECONNABORTED
#define WSAECONNRESET     ECONNRESET
#define WSAENOBUFS        ENOBUFS
#define WSAEISCONN        EISCONN
#define WSAENOTCONN       ENOTCONN
#define WSAESHUTDOWN      ESHUTDOWN
#define WSAETIMEDOUT      ETIMEDOUT
#define WSAECONNREFUSED   ECONNREFUSED
#define WSAEHOSTDOWN      EHOSTDOWN
#define WSAEHOSTUNREACH   EHOSTUNREACH
#define WSANOTINITIALISED 10093
#define WSAHOST_NOT_FOUND 11001

/* comms C files use these without including winbase */
#ifndef THREAD_PRIORITY_NORMAL
#define THREAD_PRIORITY_IDLE          (-15)
#define THREAD_PRIORITY_LOWEST        (-2)
#define THREAD_PRIORITY_BELOW_NORMAL  (-1)
#define THREAD_PRIORITY_NORMAL        0
#define THREAD_PRIORITY_ABOVE_NORMAL  1
#define THREAD_PRIORITY_HIGHEST       2
#define THREAD_PRIORITY_TIME_CRITICAL 15
#endif

#ifndef SD_RECEIVE
#define SD_RECEIVE 0
#define SD_SEND    1
#define SD_BOTH    2
#endif

#endif /* FF_LINUX */
#endif

/*
 * FreeFalcon Linux Port - dplay8.h compatibility (DirectPlay8)
 *
 * Compile-only stub for the DirectPlay8 networking API. DirectPlay does
 * not exist on Linux; the voice / multiplayer code that uses it is never
 * reached at runtime because CoCreateInstance() returns E_NOINTERFACE
 * (see compat/objbase.h), so every g_pDPClient / g_pDPServer / address
 * pointer stays NULL and the guarded call sites bail out.
 *
 * These declarations exist purely so that the sources that #include
 * <dplay8.h> (src/falclib/f4comms.cpp via voicecomunication/voicecom.h,
 * and src/voicecomunication/voice.cpp) parse and type-check.
 *
 * COM interfaces follow the project's lpVtbl + inline C++ wrapper layout
 * (mirroring compat/ddraw.h). Because the objects are never created, the
 * vtables are never populated and the wrappers are never executed.
 *
 * WCHAR note: this port defines WCHAR as `unsigned short` (compat_types.h)
 * while the compiler's wide literals (L"...") are 4-byte wchar_t. The
 * DirectPlay code stores WCHAR buffers in the wide-string struct members,
 * so those members are declared `WCHAR*`. The AddComponent /
 * GetComponentByName name parameters, however, only ever receive the
 * DPNA_KEY_* macros (which expand to L"..." literals), so those two
 * parameters are declared `const wchar_t*` instead. The remaining
 * WCHAR/wchar_t mismatches in voicecomunication/voice.cpp are intrinsic
 * to that file's own mixed use of WCHAR buffers with wcscpy/wcslen/L"..."
 * and are out of scope here (voice.cpp is not compiled for f4comms.cpp).
 */

#ifndef FF_COMPAT_DPLAY8_H
#define FF_COMPAT_DPLAY8_H

#ifdef FF_LINUX

#include "compat_types.h"
#include "objbase.h"

#ifdef __cplusplus
extern "C++" {
#endif

/* ============================================================
 * Basic typedefs
 * ============================================================ */
typedef DWORD    DPNID;
typedef DWORD    DPNHANDLE;

/* Message handler callback: HRESULT WINAPI (PVOID, DWORD, PVOID) */
typedef HRESULT (WINAPI *PFNDPNMESSAGEHANDLER)(PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer);

/* ============================================================
 * Forward declarations / pointer typedefs
 * ============================================================ */
struct IDirectPlay8Client;
struct IDirectPlay8Server;
struct IDirectPlay8Peer;
struct IDirectPlay8Address;

typedef struct IDirectPlay8Client  *PDIRECTPLAY8CLIENT,  *LPDIRECTPLAY8CLIENT;
typedef struct IDirectPlay8Server  *PDIRECTPLAY8SERVER,  *LPDIRECTPLAY8SERVER;
typedef struct IDirectPlay8Peer    *PDIRECTPLAY8PEER,    *LPDIRECTPLAY8PEER;
typedef struct IDirectPlay8Address *PDIRECTPLAY8ADDRESS, *LPDIRECTPLAY8ADDRESS;

/* ============================================================
 * Error / result codes
 * ============================================================ */
#define _DPN_FACILITY   0x015
#define MAKE_DPNHRESULT(code) MAKE_HRESULT(1, _DPN_FACILITY, code)

#define DPN_OK                          S_OK
#define DPNSUCCESS_PENDING              MAKE_HRESULT(0, _DPN_FACILITY, 0x0001)

#define DPNERR_GENERIC                  E_FAIL
#define DPNERR_INVALIDPARAM             E_INVALIDARG
#define DPNERR_INVALIDPOINTER           E_POINTER
#define DPNERR_OUTOFMEMORY              E_OUTOFMEMORY
#define DPNERR_BUFFERTOOSMALL           MAKE_DPNHRESULT(0x0010)
#define DPNERR_NOCONNECTION             MAKE_DPNHRESULT(0x0140)
#define DPNERR_NOTHOST                  MAKE_DPNHRESULT(0x0150)
#define DPNERR_TIMEDOUT                 MAKE_DPNHRESULT(0x0210)
#define DPNERR_CONNECTIONLOST           MAKE_DPNHRESULT(0x0070)
#define DPNERR_HOSTREJECTEDCONNECTION   MAKE_DPNHRESULT(0x00B0)
#define DPNERR_ALREADYINITIALIZED       MAKE_DPNHRESULT(0x0020)
#define DPNERR_UNINITIALIZED            MAKE_DPNHRESULT(0x0250)

/* ============================================================
 * Message IDs (system messages delivered to the message handler)
 * ============================================================ */
#define DPN_MSGID_OFFSET                0xFFFF0000
#define DPN_MSGID_CREATE_PLAYER         (DPN_MSGID_OFFSET | 0x0003)
#define DPN_MSGID_DESTROY_PLAYER        (DPN_MSGID_OFFSET | 0x0004)
#define DPN_MSGID_ENUM_HOSTS_QUERY      (DPN_MSGID_OFFSET | 0x0005)
#define DPN_MSGID_ENUM_HOSTS_RESPONSE   (DPN_MSGID_OFFSET | 0x0006)
#define DPN_MSGID_CREATE_GROUP          (DPN_MSGID_OFFSET | 0x0007)
#define DPN_MSGID_DESTROY_GROUP         (DPN_MSGID_OFFSET | 0x0008)
#define DPN_MSGID_ADD_PLAYER_TO_GROUP   (DPN_MSGID_OFFSET | 0x0009)
#define DPN_MSGID_REMOVE_PLAYER_FROM_GROUP (DPN_MSGID_OFFSET | 0x000A)
#define DPN_MSGID_RECEIVE               (DPN_MSGID_OFFSET | 0x0011)
#define DPN_MSGID_INDICATE_CONNECT      (DPN_MSGID_OFFSET | 0x0013)
#define DPN_MSGID_INDICATED_CONNECT_ABORTED (DPN_MSGID_OFFSET | 0x0014)
#define DPN_MSGID_CONNECT_COMPLETE      (DPN_MSGID_OFFSET | 0x0015)
#define DPN_MSGID_TERMINATE_SESSION     (DPN_MSGID_OFFSET | 0x0016)

/* ============================================================
 * Special player / group IDs
 * ============================================================ */
#define DPNID_ALL_PLAYERS_GROUP         0

/* ============================================================
 * Send flags
 * ============================================================ */
#define DPNSEND_SYNC                    0x00000001
#define DPNSEND_NOCOPY                  0x00000002
#define DPNSEND_NOCOMPLETE              0x00000004
#define DPNSEND_COMPLETEONPROCESS       0x00000008
#define DPNSEND_GUARANTEED              0x00000010
#define DPNSEND_NONSEQUENTIAL           0x00000020
#define DPNSEND_NOLOOPBACK              0x00000040
#define DPNSEND_PRIORITY_LOW            0x00000080
#define DPNSEND_PRIORITY_HIGH           0x00000100

/* ============================================================
 * Connect / enum / host flags
 * ============================================================ */
#define DPNCONNECT_SYNC                 0x00000001
#define DPNCONNECT_OKTOQUERYFORADDRESSING 0x00000002

#define DPNENUMHOSTS_SYNC               0x00000001
#define DPNENUMHOSTS_OKTOQUERYFORADDRESSING 0x00000002
#define DPNENUMHOSTS_NORESPONSEMESSAGE  0x00000004

#define DPNHOST_OKTOQUERYFORADDRESSING  0x00000001

/* ============================================================
 * Session flags (DPN_APPLICATION_DESC::dwFlags)
 * ============================================================ */
#define DPNSESSION_CLIENT_SERVER        0x00000001
#define DPNSESSION_MIGRATE_HOST         0x00000004
#define DPNSESSION_NODPNSVR             0x00000040
#define DPNSESSION_REQUIREPASSWORD      0x00000080

/* ============================================================
 * Info flags (DPN_PLAYER_INFO / DPN_GROUP_INFO::dwInfoFlags)
 * ============================================================ */
#define DPNINFO_NAME                    0x00000001
#define DPNINFO_DATA                    0x00000002

/* Group flags */
#define DPNGROUP_AUTODESTRUCT           0x00000001

/* EnumPlayersAndGroups flags */
#define DPNENUM_PLAYERS                 0x00000001
#define DPNENUM_GROUPS                  0x00000002

/* RegisterLobby flags */
#define DPNLOBBY_REGISTER               0x00000001
#define DPNLOBBY_UNREGISTER             0x00000002

/* ============================================================
 * Address component keys and data types (string constants/values)
 * ============================================================ */
#define DPNA_DATATYPE_STRING            0x00000001
#define DPNA_DATATYPE_DWORD             0x00000002
#define DPNA_DATATYPE_GUID              0x00000003
#define DPNA_DATATYPE_BINARY           0x00000004
#define DPNA_DATATYPE_STRING_ANSI       0x00000005

/* These are normally wide-string literals; declared as WCHAR* literals
 * so AddComponent(... pwszName ...) calls with them type-check. */
#define DPNA_KEY_HOSTNAME               (L"hostname")
#define DPNA_KEY_PORT                   (L"port")
#define DPNA_KEY_DEVICE                 (L"device")
#define DPNA_KEY_NAT_RESOLVER           (L"natresolver")
#define DPNA_KEY_PROVIDER               (L"provider")
#define DPNA_KEY_PROGRAM                (L"program")

/* ============================================================
 * Structures
 * ============================================================ */
typedef struct _DPN_APPLICATION_DESC {
    DWORD          dwSize;
    DWORD          dwFlags;
    GUID           guidInstance;
    GUID           guidApplication;
    DWORD          dwMaxPlayers;
    DWORD          dwCurrentPlayers;
    WCHAR       *pwszSessionName;
    WCHAR       *pwszPassword;
    PVOID          pvReservedData;
    DWORD          dwReservedDataSize;
    PVOID          pvApplicationReservedData;
    DWORD          dwApplicationReservedDataSize;
} DPN_APPLICATION_DESC, *PDPN_APPLICATION_DESC;

typedef struct _BUFFERDESC {
    DWORD          dwBufferSize;
    BYTE          *pBufferData;
} DPN_BUFFER_DESC, *PDPN_BUFFER_DESC, BUFFERDESC, *PBUFFERDESC;
typedef const DPN_BUFFER_DESC *PCDPN_BUFFER_DESC;

typedef struct _DPN_PLAYER_INFO {
    DWORD          dwSize;
    DWORD          dwInfoFlags;
    WCHAR       *pwszName;
    PVOID          pvData;
    DWORD          dwDataSize;
    DWORD          dwPlayerFlags;
} DPN_PLAYER_INFO, *PDPN_PLAYER_INFO;

typedef struct _DPN_GROUP_INFO {
    DWORD          dwSize;
    DWORD          dwInfoFlags;
    WCHAR       *pwszName;
    PVOID          pvData;
    DWORD          dwDataSize;
    DWORD          dwGroupFlags;
} DPN_GROUP_INFO, *PDPN_GROUP_INFO;

typedef struct _DPN_SERVICE_PROVIDER_INFO {
    DWORD          dwFlags;
    GUID           guid;
    WCHAR       *pwszName;
    PVOID          pvReserved;
    DWORD          dwReserved;
} DPN_SERVICE_PROVIDER_INFO, *PDPN_SERVICE_PROVIDER_INFO;

typedef struct _DPN_SP_CAPS {
    DWORD          dwSize;
    DWORD          dwFlags;
    DWORD          dwNumThreads;
    DWORD          dwDefaultEnumCount;
    DWORD          dwDefaultEnumRetryInterval;
    DWORD          dwDefaultEnumTimeout;
    DWORD          dwMaxEnumPayloadSize;
    DWORD          dwBuffersPerThread;
    DWORD          dwSystemBufferSize;
} DPN_SP_CAPS, *PDPN_SP_CAPS;

typedef struct _DPN_CONNECTION_INFO {
    DWORD          dwSize;
    DWORD          dwRoundTripLatencyMS;
    DWORD          dwThroughputBPS;
    DWORD          dwPeakThroughputBPS;
    DWORD          dwBytesSentGuaranteed;
    DWORD          dwPacketsSentGuaranteed;
    DWORD          dwBytesSentNonGuaranteed;
    DWORD          dwPacketsSentNonGuaranteed;
    DWORD          dwBytesRetried;
    DWORD          dwPacketsRetried;
    DWORD          dwBytesDropped;
    DWORD          dwPacketsDropped;
    DWORD          dwMessagesTransmittedHighPriority;
    DWORD          dwMessagesTimedOutHighPriority;
    DWORD          dwMessagesTransmittedNormalPriority;
    DWORD          dwMessagesTimedOutNormalPriority;
    DWORD          dwMessagesTransmittedLowPriority;
    DWORD          dwMessagesTimedOutLowPriority;
    DWORD          dwBytesReceivedGuaranteed;
    DWORD          dwPacketsReceivedGuaranteed;
    DWORD          dwBytesReceivedNonGuaranteed;
    DWORD          dwPacketsReceivedNonGuaranteed;
    DWORD          dwMessagesReceived;
} DPN_CONNECTION_INFO, *PDPN_CONNECTION_INFO;

/* ------------------- message structures ------------------- */
typedef struct _DPNMSG_CREATE_PLAYER {
    DWORD          dwSize;
    DPNID          dpnidPlayer;
    PVOID          pvPlayerContext;
} DPNMSG_CREATE_PLAYER, *PDPNMSG_CREATE_PLAYER;

typedef struct _DPNMSG_DESTROY_PLAYER {
    DWORD          dwSize;
    DPNID          dpnidPlayer;
    PVOID          pvPlayerContext;
    DWORD          dwReason;
} DPNMSG_DESTROY_PLAYER, *PDPNMSG_DESTROY_PLAYER;

typedef struct _DPNMSG_CREATE_GROUP {
    DWORD          dwSize;
    DPNID          dpnidGroup;
    DPNID          dpnidOwner;
    PVOID          pvGroupContext;
    PVOID          pvOwnerContext;
} DPNMSG_CREATE_GROUP, *PDPNMSG_CREATE_GROUP;

typedef struct _DPNMSG_RECEIVE {
    DWORD          dwSize;
    DPNID          dpnidSender;
    PVOID          pvPlayerContext;
    BYTE          *pReceiveData;
    DWORD          dwReceiveDataSize;
    DPNHANDLE      hBufferHandle;
} DPNMSG_RECEIVE, *PDPNMSG_RECEIVE;

typedef struct _DPNMSG_ENUM_HOSTS_RESPONSE {
    DWORD                       dwSize;
    PDIRECTPLAY8ADDRESS         pAddressSender;
    PDIRECTPLAY8ADDRESS         pAddressDevice;
    const DPN_APPLICATION_DESC *pApplicationDescription;
    PVOID                       pvResponseData;
    DWORD                       dwResponseDataSize;
    PVOID                       pvUserContext;
    DWORD                       dwRoundTripLatencyMS;
} DPNMSG_ENUM_HOSTS_RESPONSE, *PDPNMSG_ENUM_HOSTS_RESPONSE;

typedef struct _DPNMSG_INDICATE_CONNECT {
    DWORD                       dwSize;
    PVOID                       pvUserConnectData;
    DWORD                       dwUserConnectDataSize;
    PVOID                       pvReplyData;
    DWORD                       dwReplyDataSize;
    PVOID                       pvReplyContext;
    PVOID                       pvPlayerContext;
    PDIRECTPLAY8ADDRESS         pAddressPlayer;
    PDIRECTPLAY8ADDRESS         pAddressDevice;
} DPNMSG_INDICATE_CONNECT, *PDPNMSG_INDICATE_CONNECT;

/* ============================================================
 * IDirectPlay8Address
 * ============================================================ */
typedef struct IDirectPlay8AddressVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IDirectPlay8Address *This, REFIID riid, LPVOID *ppv);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IDirectPlay8Address *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IDirectPlay8Address *This);
    HRESULT (STDMETHODCALLTYPE *BuildFromURLW)(IDirectPlay8Address *This, WCHAR *pwszSourceURL);
    HRESULT (STDMETHODCALLTYPE *BuildFromURLA)(IDirectPlay8Address *This, CHAR *pszSourceURL);
    HRESULT (STDMETHODCALLTYPE *Duplicate)(IDirectPlay8Address *This, IDirectPlay8Address **ppdpaNewAddress);
    HRESULT (STDMETHODCALLTYPE *SetEqual)(IDirectPlay8Address *This, IDirectPlay8Address *pdpaAddress);
    HRESULT (STDMETHODCALLTYPE *IsEqual)(IDirectPlay8Address *This, IDirectPlay8Address *pdpaAddress);
    HRESULT (STDMETHODCALLTYPE *Clear)(IDirectPlay8Address *This);
    HRESULT (STDMETHODCALLTYPE *GetURLW)(IDirectPlay8Address *This, WCHAR *pwszURL, PDWORD pdwNumChars);
    HRESULT (STDMETHODCALLTYPE *GetURLA)(IDirectPlay8Address *This, CHAR *pszURL, PDWORD pdwNumChars);
    HRESULT (STDMETHODCALLTYPE *GetSP)(IDirectPlay8Address *This, GUID *pguidSP);
    HRESULT (STDMETHODCALLTYPE *GetUserData)(IDirectPlay8Address *This, LPVOID pvUserData, PDWORD pdwBufferSize);
    HRESULT (STDMETHODCALLTYPE *SetSP)(IDirectPlay8Address *This, const GUID *const pguidSP);
    HRESULT (STDMETHODCALLTYPE *SetUserData)(IDirectPlay8Address *This, const void *const pvUserData, const DWORD dwDataSize);
    HRESULT (STDMETHODCALLTYPE *GetNumComponents)(IDirectPlay8Address *This, PDWORD pdwNumComponents);
    HRESULT (STDMETHODCALLTYPE *GetComponentByName)(IDirectPlay8Address *This, const wchar_t *const pwszName, LPVOID pvBuffer, PDWORD pdwBufferSize, PDWORD pdwDataType);
    HRESULT (STDMETHODCALLTYPE *GetComponentByIndex)(IDirectPlay8Address *This, const DWORD dwComponentID, WCHAR *pwszName, PDWORD pdwNameLen, LPVOID pvBuffer, PDWORD pdwBufferSize, PDWORD pdwDataType);
    HRESULT (STDMETHODCALLTYPE *AddComponent)(IDirectPlay8Address *This, const wchar_t *const pwszName, const void *const lpvData, const DWORD dwDataSize, const DWORD dwDataType);
    HRESULT (STDMETHODCALLTYPE *GetDevice)(IDirectPlay8Address *This, GUID *pDevGuid);
    HRESULT (STDMETHODCALLTYPE *SetDevice)(IDirectPlay8Address *This, const GUID *const devGuid);
    HRESULT (STDMETHODCALLTYPE *BuildFromDirectPlayConnectionString)(IDirectPlay8Address *This, WCHAR *pwszSourceString);
} IDirectPlay8AddressVtbl;

struct IDirectPlay8Address {
    IDirectPlay8AddressVtbl *lpVtbl;
#ifdef __cplusplus
    HRESULT QueryInterface(REFIID riid, LPVOID *ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG   AddRef()  { return lpVtbl->AddRef(this); }
    ULONG   Release() { return lpVtbl->Release(this); }
    HRESULT Duplicate(IDirectPlay8Address **a) { return lpVtbl->Duplicate(this, a); }
    HRESULT SetSP(const GUID *const a) { return lpVtbl->SetSP(this, a); }
    HRESULT AddComponent(const wchar_t *const a, const void *const b, const DWORD c, const DWORD d) { return lpVtbl->AddComponent(this, a, b, c, d); }
    HRESULT GetComponentByName(const wchar_t *const a, LPVOID b, PDWORD c, PDWORD d) { return lpVtbl->GetComponentByName(this, a, b, c, d); }
    HRESULT GetURLA(CHAR *a, PDWORD b) { return lpVtbl->GetURLA(this, a, b); }
    HRESULT Clear() { return lpVtbl->Clear(this); }
#endif
};

/* ============================================================
 * IDirectPlay8Client
 * ============================================================ */
typedef struct IDirectPlay8ClientVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IDirectPlay8Client *This, REFIID riid, LPVOID *ppv);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IDirectPlay8Client *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IDirectPlay8Client *This);
    HRESULT (STDMETHODCALLTYPE *Initialize)(IDirectPlay8Client *This, PVOID pvUserContext, PFNDPNMESSAGEHANDLER pfn, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *EnumServiceProviders)(IDirectPlay8Client *This, const GUID *const pguidServiceProvider, const GUID *const pguidApplication, void *pSPInfoBuffer, PDWORD pdwBufferSize, PDWORD pdwNumServiceProviders, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *EnumHosts)(IDirectPlay8Client *This, DPN_APPLICATION_DESC *const pApplicationDesc, IDirectPlay8Address *const pAddrHost, IDirectPlay8Address *const pDeviceInfo, void *const pvUserEnumData, const DWORD dwUserEnumDataSize, const DWORD dwEnumCount, const DWORD dwRetryInterval, const DWORD dwTimeOut, void *const pvUserContext, DPNHANDLE *const pAsyncHandle, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *CancelAsyncOperation)(IDirectPlay8Client *This, const DPNHANDLE hAsyncHandle, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *Connect)(IDirectPlay8Client *This, const DPN_APPLICATION_DESC *const pdnAppDesc, IDirectPlay8Address *const pHostAddr, IDirectPlay8Address *const pDeviceInfo, const void *const pdnSecurity, const void *const pdnCredentials, const void *const pvUserConnectData, const DWORD dwUserConnectDataSize, void *const pvAsyncContext, DPNHANDLE *const phAsyncHandle, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *Send)(IDirectPlay8Client *This, const DPN_BUFFER_DESC *const prgBufferDesc, const DWORD cBufferDesc, const DWORD dwTimeOut, void *const pvAsyncContext, DPNHANDLE *const phAsyncHandle, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetSendQueueInfo)(IDirectPlay8Client *This, DWORD *const pdwNumMsgs, DWORD *const pdwNumBytes, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetApplicationDesc)(IDirectPlay8Client *This, DPN_APPLICATION_DESC *const pAppDescBuffer, DWORD *const pcbDataSize, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *SetClientInfo)(IDirectPlay8Client *This, const DPN_PLAYER_INFO *const pdpnPlayerInfo, void *const pvAsyncContext, DPNHANDLE *const phAsyncHandle, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetServerInfo)(IDirectPlay8Client *This, DPN_PLAYER_INFO *const pdpnPlayerInfo, DWORD *const pdwSize, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetServerAddress)(IDirectPlay8Client *This, IDirectPlay8Address **const pAddress, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *Close)(IDirectPlay8Client *This, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *ReturnBuffer)(IDirectPlay8Client *This, const DPNHANDLE hBufferHandle, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetCaps)(IDirectPlay8Client *This, void *const pdpCaps, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *SetCaps)(IDirectPlay8Client *This, const void *const pdpCaps, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *SetSPCaps)(IDirectPlay8Client *This, const GUID *const pguidSP, const DPN_SP_CAPS *const pdpspCaps, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetSPCaps)(IDirectPlay8Client *This, const GUID *const pguidSP, DPN_SP_CAPS *const pdpspCaps, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetConnectionInfo)(IDirectPlay8Client *This, DPN_CONNECTION_INFO *const pdpConnectionInfo, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *RegisterLobby)(IDirectPlay8Client *This, const DPNHANDLE dpnHandle, void *const pIDP8LobbiedApplication, const DWORD dwFlags);
} IDirectPlay8ClientVtbl;

struct IDirectPlay8Client {
    IDirectPlay8ClientVtbl *lpVtbl;
#ifdef __cplusplus
    HRESULT QueryInterface(REFIID riid, LPVOID *ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG   AddRef()  { return lpVtbl->AddRef(this); }
    ULONG   Release() { return lpVtbl->Release(this); }
    HRESULT Initialize(PVOID a, PFNDPNMESSAGEHANDLER b, DWORD c) { return lpVtbl->Initialize(this, a, b, c); }
    HRESULT EnumServiceProviders(const GUID *const a, const GUID *const b, void *c, PDWORD d, PDWORD e, const DWORD f) { return lpVtbl->EnumServiceProviders(this, a, b, c, d, e, f); }
    HRESULT EnumHosts(DPN_APPLICATION_DESC *const a, IDirectPlay8Address *const b, IDirectPlay8Address *const c, void *const d, const DWORD e, const DWORD f, const DWORD g, const DWORD h, void *const i, DPNHANDLE *const j, const DWORD k) { return lpVtbl->EnumHosts(this, a, b, c, d, e, f, g, h, i, j, k); }
    HRESULT Connect(const DPN_APPLICATION_DESC *const a, IDirectPlay8Address *const b, IDirectPlay8Address *const c, const void *const d, const void *const e, const void *const f, const DWORD g, void *const h, DPNHANDLE *const i, const DWORD j) { return lpVtbl->Connect(this, a, b, c, d, e, f, g, h, i, j); }
    HRESULT Send(const DPN_BUFFER_DESC *const a, const DWORD b, const DWORD c, void *const d, DPNHANDLE *const e, const DWORD f) { return lpVtbl->Send(this, a, b, c, d, e, f); }
    HRESULT GetSendQueueInfo(DWORD *const a, DWORD *const b, const DWORD c) { return lpVtbl->GetSendQueueInfo(this, a, b, c); }
    HRESULT SetClientInfo(const DPN_PLAYER_INFO *const a, void *const b, DPNHANDLE *const c, const DWORD d) { return lpVtbl->SetClientInfo(this, a, b, c, d); }
    HRESULT Close(const DWORD a) { return lpVtbl->Close(this, a); }
    HRESULT RegisterLobby(const DPNHANDLE a, void *const b, const DWORD c) { return lpVtbl->RegisterLobby(this, a, b, c); }
#endif
};

/* ============================================================
 * IDirectPlay8Server
 * ============================================================ */
typedef struct IDirectPlay8ServerVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IDirectPlay8Server *This, REFIID riid, LPVOID *ppv);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IDirectPlay8Server *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IDirectPlay8Server *This);
    HRESULT (STDMETHODCALLTYPE *Initialize)(IDirectPlay8Server *This, PVOID pvUserContext, PFNDPNMESSAGEHANDLER pfn, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *EnumServiceProviders)(IDirectPlay8Server *This, const GUID *const pguidServiceProvider, const GUID *const pguidApplication, void *pSPInfoBuffer, PDWORD pdwBufferSize, PDWORD pdwNumServiceProviders, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *CancelAsyncOperation)(IDirectPlay8Server *This, const DPNHANDLE hAsyncHandle, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetSendQueueInfo)(IDirectPlay8Server *This, const DPNID dpnid, DWORD *const pdwNumMsgs, DWORD *const pdwNumBytes, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetApplicationDesc)(IDirectPlay8Server *This, DPN_APPLICATION_DESC *const pAppDescBuffer, DWORD *const pcbDataSize, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *SetServerInfo)(IDirectPlay8Server *This, const DPN_PLAYER_INFO *const pdpnPlayerInfo, void *const pvAsyncContext, DPNHANDLE *const phAsyncHandle, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetClientInfo)(IDirectPlay8Server *This, const DPNID dpnid, DPN_PLAYER_INFO *const pdpnPlayerInfo, DWORD *const pdwSize, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetClientAddress)(IDirectPlay8Server *This, const DPNID dpnid, IDirectPlay8Address **const pAddress, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetLocalHostAddresses)(IDirectPlay8Server *This, IDirectPlay8Address **const prgpAddress, DWORD *const pcAddress, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *SetApplicationDesc)(IDirectPlay8Server *This, const DPN_APPLICATION_DESC *const pad, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *Host)(IDirectPlay8Server *This, const DPN_APPLICATION_DESC *const pdnAppDesc, IDirectPlay8Address **const prgpDeviceInfo, const DWORD cDeviceInfo, const void *const pdnSecurity, const void *const pdnCredentials, void *const pvPlayerContext, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *SendTo)(IDirectPlay8Server *This, const DPNID dpnid, const DPN_BUFFER_DESC *const prgBufferDesc, const DWORD cBufferDesc, const DWORD dwTimeOut, void *const pvAsyncContext, DPNHANDLE *const phAsyncHandle, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *CreateGroup)(IDirectPlay8Server *This, const DPN_GROUP_INFO *const pdpnGroupInfo, void *const pvGroupContext, void *const pvAsyncContext, DPNHANDLE *const phAsyncHandle, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *DestroyGroup)(IDirectPlay8Server *This, const DPNID idGroup, void *const pvAsyncContext, DPNHANDLE *const phAsyncHandle, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *AddPlayerToGroup)(IDirectPlay8Server *This, const DPNID idGroup, const DPNID idClient, void *const pvAsyncContext, DPNHANDLE *const phAsyncHandle, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *RemovePlayerFromGroup)(IDirectPlay8Server *This, const DPNID idGroup, const DPNID idClient, void *const pvAsyncContext, DPNHANDLE *const phAsyncHandle, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *SetGroupInfo)(IDirectPlay8Server *This, const DPNID dpnid, DPN_GROUP_INFO *const pdpnGroupInfo, void *const pvAsyncContext, DPNHANDLE *const phAsyncHandle, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetGroupInfo)(IDirectPlay8Server *This, const DPNID dpnid, DPN_GROUP_INFO *const pdpnGroupInfo, DWORD *const pdwSize, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *EnumPlayersAndGroups)(IDirectPlay8Server *This, DPNID *const prgdpnid, DWORD *const pcdpnid, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *EnumGroupMembers)(IDirectPlay8Server *This, const DPNID dpnid, DPNID *const prgdpnid, DWORD *const pcdpnid, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *SetClientInfo)(IDirectPlay8Server *This, const DPNID dpnid, DPN_PLAYER_INFO *const pdpnPlayerInfo, void *const pvAsyncContext, DPNHANDLE *const phAsyncHandle, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *Close)(IDirectPlay8Server *This, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *DestroyClient)(IDirectPlay8Server *This, const DPNID dpnidClient, const void *const pvDestroyData, const DWORD dwDestroyDataSize, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *ReturnBuffer)(IDirectPlay8Server *This, const DPNHANDLE hBufferHandle, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetPlayerContext)(IDirectPlay8Server *This, const DPNID dpnid, PVOID *const ppvPlayerContext, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetGroupContext)(IDirectPlay8Server *This, const DPNID dpnid, PVOID *const ppvGroupContext, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetCaps)(IDirectPlay8Server *This, void *const pdpCaps, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *SetCaps)(IDirectPlay8Server *This, const void *const pdpCaps, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *SetSPCaps)(IDirectPlay8Server *This, const GUID *const pguidSP, const DPN_SP_CAPS *const pdpspCaps, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetSPCaps)(IDirectPlay8Server *This, const GUID *const pguidSP, DPN_SP_CAPS *const pdpspCaps, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetConnectionInfo)(IDirectPlay8Server *This, const DPNID dpnid, DPN_CONNECTION_INFO *const pdpConnectionInfo, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *RegisterLobby)(IDirectPlay8Server *This, const DPNHANDLE dpnHandle, void *const pIDP8LobbiedApplication, const DWORD dwFlags);
} IDirectPlay8ServerVtbl;

struct IDirectPlay8Server {
    IDirectPlay8ServerVtbl *lpVtbl;
#ifdef __cplusplus
    HRESULT QueryInterface(REFIID riid, LPVOID *ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG   AddRef()  { return lpVtbl->AddRef(this); }
    ULONG   Release() { return lpVtbl->Release(this); }
    HRESULT Initialize(PVOID a, PFNDPNMESSAGEHANDLER b, DWORD c) { return lpVtbl->Initialize(this, a, b, c); }
    HRESULT EnumServiceProviders(const GUID *const a, const GUID *const b, void *c, PDWORD d, PDWORD e, const DWORD f) { return lpVtbl->EnumServiceProviders(this, a, b, c, d, e, f); }
    HRESULT GetGroupInfo(const DPNID a, DPN_GROUP_INFO *const b, DWORD *const c, const DWORD d) { return lpVtbl->GetGroupInfo(this, a, b, c, d); }
    HRESULT GetClientInfo(const DPNID a, DPN_PLAYER_INFO *const b, DWORD *const c, const DWORD d) { return lpVtbl->GetClientInfo(this, a, b, c, d); }
    HRESULT GetClientAddress(const DPNID a, IDirectPlay8Address **const b, const DWORD c) { return lpVtbl->GetClientAddress(this, a, b, c); }
    HRESULT Host(const DPN_APPLICATION_DESC *const a, IDirectPlay8Address **const b, const DWORD c, const void *const d, const void *const e, void *const f, const DWORD g) { return lpVtbl->Host(this, a, b, c, d, e, f, g); }
    HRESULT SendTo(const DPNID a, const DPN_BUFFER_DESC *const b, const DWORD c, const DWORD d, void *const e, DPNHANDLE *const f, const DWORD g) { return lpVtbl->SendTo(this, a, b, c, d, e, f, g); }
    HRESULT CreateGroup(const DPN_GROUP_INFO *const a, void *const b, void *const c, DPNHANDLE *const d, const DWORD e) { return lpVtbl->CreateGroup(this, a, b, c, d, e); }
    HRESULT AddPlayerToGroup(const DPNID a, const DPNID b, void *const c, DPNHANDLE *const d, const DWORD e) { return lpVtbl->AddPlayerToGroup(this, a, b, c, d, e); }
    HRESULT RemovePlayerFromGroup(const DPNID a, const DPNID b, void *const c, DPNHANDLE *const d, const DWORD e) { return lpVtbl->RemovePlayerFromGroup(this, a, b, c, d, e); }
    HRESULT EnumPlayersAndGroups(DPNID *const a, DWORD *const b, const DWORD c) { return lpVtbl->EnumPlayersAndGroups(this, a, b, c); }
    HRESULT GetSendQueueInfo(const DPNID a, DWORD *const b, DWORD *const c, const DWORD d) { return lpVtbl->GetSendQueueInfo(this, a, b, c, d); }
    HRESULT Close(const DWORD a) { return lpVtbl->Close(this, a); }
    HRESULT DestroyClient(const DPNID a, const void *const b, const DWORD c, const DWORD d) { return lpVtbl->DestroyClient(this, a, b, c, d); }
    HRESULT RegisterLobby(const DPNHANDLE a, void *const b, const DWORD c) { return lpVtbl->RegisterLobby(this, a, b, c); }
#endif
};

/* ============================================================
 * GUIDs - arbitrary distinct values; only their addresses/equality
 * are ever used, never any real COM lookup.
 * ============================================================ */
static const GUID CLSID_DirectPlay8Client  = { 0x743f1dc6, 0x5aba, 0x429f, { 0x8b, 0xdf, 0xc5, 0x4d, 0x03, 0x25, 0x3d, 0xc2 } };
static const GUID CLSID_DirectPlay8Server  = { 0xda825e1b, 0x6830, 0x43d7, { 0x83, 0x5d, 0x0b, 0x5a, 0xd8, 0x29, 0x56, 0xa2 } };
static const GUID CLSID_DirectPlay8Peer    = { 0x286f484d, 0x375e, 0x4458, { 0xa2, 0x72, 0xb1, 0x38, 0xe2, 0xf8, 0x0a, 0x6a } };
static const GUID CLSID_DirectPlay8Address = { 0x934a9523, 0xa3ca, 0x4bc5, { 0xad, 0xa0, 0xd6, 0xd9, 0x5d, 0x97, 0x94, 0x21 } };

static const GUID IID_IDirectPlay8Client   = { 0x5102dacd, 0x241b, 0x11d3, { 0xae, 0xa7, 0x00, 0x60, 0x97, 0xb0, 0x14, 0x11 } };
static const GUID IID_IDirectPlay8Server   = { 0x5102dace, 0x241b, 0x11d3, { 0xae, 0xa7, 0x00, 0x60, 0x97, 0xb0, 0x14, 0x11 } };
static const GUID IID_IDirectPlay8Peer     = { 0x5102dacf, 0x241b, 0x11d3, { 0xae, 0xa7, 0x00, 0x60, 0x97, 0xb0, 0x14, 0x11 } };
static const GUID IID_IDirectPlay8Address  = { 0x83783300, 0x4063, 0x4c8a, { 0x9d, 0xb3, 0x82, 0x83, 0x0a, 0x7f, 0xeb, 0x31 } };

/* TCP/IP service provider */
static const GUID CLSID_DP8SP_TCPIP        = { 0xebfe7ba0, 0x628d, 0x11d2, { 0xae, 0x0f, 0x00, 0x60, 0x97, 0xb0, 0x14, 0x11 } };
static const GUID CLSID_DP8SP_IPX          = { 0x53934290, 0x628d, 0x11d2, { 0xae, 0x0f, 0x00, 0x60, 0x97, 0xb0, 0x14, 0x11 } };
static const GUID CLSID_DP8SP_MODEM        = { 0x6d4a3650, 0x628d, 0x11d2, { 0xae, 0x0f, 0x00, 0x60, 0x97, 0xb0, 0x14, 0x11 } };
static const GUID CLSID_DP8SP_SERIAL       = { 0x743b5d60, 0x628d, 0x11d2, { 0xae, 0x0f, 0x00, 0x60, 0x97, 0xb0, 0x14, 0x11 } };

#ifdef __cplusplus
} /* extern "C++" */
#endif

#endif /* FF_LINUX */
#endif /* FF_COMPAT_DPLAY8_H */

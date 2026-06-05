/*
 * FreeFalcon Linux Port - dplobby8.h compatibility (DirectPlay8 Lobby)
 *
 * Compile-only stub. Included by voicecomunication/voicecom.h (and used by
 * voicecomunication/voice.cpp). The lobby object is never created on Linux
 * so the wrappers are never executed.
 *
 * Interfaces follow the project lpVtbl + inline C++ wrapper layout.
 * Wide-string members are declared WCHAR* to accept L"..." / wcs* results
 * (WCHAR is unsigned short on this port; see dplay8.h note).
 */

#ifndef FF_COMPAT_DPLOBBY8_H
#define FF_COMPAT_DPLOBBY8_H

#ifdef FF_LINUX

#include "compat_types.h"
#include "objbase.h"
#include "dplay8.h"

#ifdef __cplusplus
extern "C++" {
#endif

struct IDirectPlay8LobbiedApplication;
struct IDirectPlay8LobbyClient;

typedef struct IDirectPlay8LobbiedApplication *PDIRECTPLAY8LOBBIEDAPPLICATION, *LPDIRECTPLAY8LOBBIEDAPPLICATION;
typedef struct IDirectPlay8LobbyClient        *PDIRECTPLAY8LOBBYCLIENT,        *LPDIRECTPLAY8LOBBYCLIENT;

/* ============================================================
 * Lobby message IDs
 * ============================================================ */
#define DPL_MSGID_OFFSET                0xFFFF0000
#define DPL_MSGID_RECEIVE               (DPL_MSGID_OFFSET | 0x0001)
#define DPL_MSGID_CONNECT               (DPL_MSGID_OFFSET | 0x0002)
#define DPL_MSGID_DISCONNECT            (DPL_MSGID_OFFSET | 0x0003)
#define DPL_MSGID_SESSION_STATUS        (DPL_MSGID_OFFSET | 0x0004)
#define DPL_MSGID_CONNECTION_SETTINGS   (DPL_MSGID_OFFSET | 0x0005)

/* Connection-settings flags */
#define DPLCONNECTSETTINGS_HOST         0x00000001

/* ============================================================
 * Structures
 * ============================================================ */
typedef struct _DPL_PROGRAM_DESC {
    DWORD     dwSize;
    DWORD     dwFlags;
    GUID      guidApplication;
    WCHAR  *pwszApplicationName;
    WCHAR  *pwszCommandLine;
    WCHAR  *pwszCurrentDirectory;
    WCHAR  *pwszDescription;
    WCHAR  *pwszExecutablePath;
    WCHAR  *pwszExecutableFilename;
} DPL_PROGRAM_DESC, *PDPL_PROGRAM_DESC;

typedef struct _DPL_CONNECTION_SETTINGS {
    DWORD                 dwSize;
    DWORD                 dwFlags;
    DPN_APPLICATION_DESC  dpnAppDesc;
    IDirectPlay8Address  *pdp8HostAddress;
    IDirectPlay8Address **ppdp8DeviceAddresses;
    DWORD                 cNumDeviceAddresses;
    WCHAR              *pwszPlayerName;
} DPL_CONNECTION_SETTINGS, *PDPL_CONNECTION_SETTINGS;

typedef struct _DPL_MESSAGE_CONNECT {
    DWORD                     dwSize;
    DPNHANDLE                 hConnectId;
    PDPL_CONNECTION_SETTINGS  pdplConnectionSettings;
    PVOID                     pvLobbyConnectData;
    DWORD                     dwLobbyConnectDataSize;
    PVOID                     pvConnectionContext;
} DPL_MESSAGE_CONNECT, *PDPL_MESSAGE_CONNECT;

typedef struct _DPL_MESSAGE_DISCONNECT {
    DWORD      dwSize;
    DPNHANDLE  hDisconnectId;
    HRESULT    hrReason;
    PVOID      pvConnectionContext;
} DPL_MESSAGE_DISCONNECT, *PDPL_MESSAGE_DISCONNECT;

/* ============================================================
 * IDirectPlay8LobbiedApplication
 * ============================================================ */
typedef struct IDirectPlay8LobbiedApplicationVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IDirectPlay8LobbiedApplication *This, REFIID riid, LPVOID *ppv);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IDirectPlay8LobbiedApplication *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IDirectPlay8LobbiedApplication *This);
    HRESULT (STDMETHODCALLTYPE *Initialize)(IDirectPlay8LobbiedApplication *This, void *const pvUserContext, PFNDPNMESSAGEHANDLER pfn, DPNHANDLE *const pdpnhConnection, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *RegisterProgram)(IDirectPlay8LobbiedApplication *This, PDPL_PROGRAM_DESC pdplProgramDesc, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *UnRegisterProgram)(IDirectPlay8LobbiedApplication *This, GUID *pguidApplication, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *Send)(IDirectPlay8LobbiedApplication *This, const DPNHANDLE hConnection, BYTE *const pBuffer, const DWORD pBufferSize, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *SetAppAvailable)(IDirectPlay8LobbiedApplication *This, const BOOL fAvailable, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *SetConnectionSettings)(IDirectPlay8LobbiedApplication *This, const DPNHANDLE hTarget, const DPL_CONNECTION_SETTINGS *const pdplSessionInfo, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetConnectionSettings)(IDirectPlay8LobbiedApplication *This, const DPNHANDLE hTarget, DPL_CONNECTION_SETTINGS *const pdplSessionInfo, DWORD *pdwInfoSize, const DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *Close)(IDirectPlay8LobbiedApplication *This, const DWORD dwFlags);
} IDirectPlay8LobbiedApplicationVtbl;

struct IDirectPlay8LobbiedApplication {
    IDirectPlay8LobbiedApplicationVtbl *lpVtbl;
#ifdef __cplusplus
    HRESULT QueryInterface(REFIID riid, LPVOID *ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG   AddRef()  { return lpVtbl->AddRef(this); }
    ULONG   Release() { return lpVtbl->Release(this); }
    HRESULT Initialize(void *const a, PFNDPNMESSAGEHANDLER b, DPNHANDLE *const c, const DWORD d) { return lpVtbl->Initialize(this, a, b, c, d); }
    HRESULT RegisterProgram(PDPL_PROGRAM_DESC a, const DWORD b) { return lpVtbl->RegisterProgram(this, a, b); }
    HRESULT UnRegisterProgram(GUID *a, const DWORD b) { return lpVtbl->UnRegisterProgram(this, a, b); }
    HRESULT Close(const DWORD a) { return lpVtbl->Close(this, a); }
#endif
};

/* ============================================================
 * GUIDs - arbitrary distinct values.
 * ============================================================ */
static const GUID CLSID_DirectPlay8LobbiedApplication = { 0x667955ad, 0x6b3b, 0x43ca, { 0xb9, 0x49, 0xbc, 0x69, 0xb5, 0xba, 0xff, 0x7f } };
static const GUID CLSID_DirectPlay8LobbyClient        = { 0x3b2b6775, 0x70b6, 0x45af, { 0x8d, 0xea, 0xa2, 0x09, 0xc6, 0x95, 0x59, 0xf3 } };

static const GUID IID_IDirectPlay8LobbiedApplication  = { 0x667955ad, 0x6b3b, 0x43ca, { 0xb9, 0x49, 0xbc, 0x69, 0xb5, 0xba, 0xff, 0x7e } };
static const GUID IID_IDirectPlay8LobbyClient         = { 0x819074a3, 0x016c, 0x11d3, { 0xae, 0x14, 0x00, 0x60, 0x97, 0xb0, 0x14, 0x11 } };

#ifdef __cplusplus
} /* extern "C++" */
#endif

#endif /* FF_LINUX */
#endif /* FF_COMPAT_DPLOBBY8_H */

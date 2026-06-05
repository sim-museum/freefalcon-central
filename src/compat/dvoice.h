/*
 * FreeFalcon Linux Port - dvoice.h compatibility (DirectPlay Voice)
 *
 * Compile-only stub for the DirectPlay Voice API. Like DirectPlay8 it does
 * not exist on Linux; the voice objects are never created (CoCreateInstance
 * returns E_NOINTERFACE) so every g_pVoiceClient / g_pVoiceServer pointer
 * stays NULL and the call sites are guarded.
 *
 * Declarations exist so that src/falclib/f4comms.cpp (via
 * voicecomunication/voicecom.h) and src/voicecomunication/voice.cpp parse.
 *
 * Interfaces follow the project lpVtbl + inline C++ wrapper layout.
 *
 * lpds* sound-device members are declared as void* so this header stays
 * independent of dsound.h; the code only ever assigns NULL to them.
 */

#ifndef FF_COMPAT_DVOICE_H
#define FF_COMPAT_DVOICE_H

#ifdef FF_LINUX

#include "compat_types.h"
#include "objbase.h"
#include "dplay8.h"

#ifdef __cplusplus
extern "C++" {
#endif

/* ============================================================
 * Basic typedefs
 * ============================================================ */
/*
 * DVID: on Windows this is DWORD (== unsigned long == unsigned int, all
 * 4 bytes). On 64-bit Linux unsigned long is 8 bytes. The original game
 * code passes `unsigned long*` arrays (g_afreqarrey.Freq) into
 * SetTransmitTargets(PDVID,...), so DVID is declared as `unsigned long`
 * here to keep that code compiling.
 */
typedef unsigned long DVID;
typedef DVID *LPDVID, *PDVID;

struct IDirectPlayVoiceClient;
struct IDirectPlayVoiceServer;
struct IDirectPlayVoiceTest;

typedef struct IDirectPlayVoiceClient *LPDIRECTPLAYVOICECLIENT, *PDIRECTPLAYVOICECLIENT;
typedef struct IDirectPlayVoiceServer *LPDIRECTPLAYVOICESERVER, *PDIRECTPLAYVOICESERVER;
typedef struct IDirectPlayVoiceTest   *LPDIRECTPLAYVOICETEST,   *PDIRECTPLAYVOICETEST;

/* Voice message handler callback */
typedef HRESULT (WINAPI *PDVMESSAGEHANDLER)(PVOID pvUserContext, DWORD dwMessageType, PVOID lpMessage);
typedef PDVMESSAGEHANDLER LPDVMESSAGEHANDLER;

/* ============================================================
 * Result codes
 * ============================================================ */
#define _DVERR_FACILITY 0x015
#define MAKE_DVHRESULT(code) MAKE_HRESULT(1, _DVERR_FACILITY, code)

#define DV_OK                       S_OK
#define DV_FULLDUPLEX               MAKE_HRESULT(0, _DVERR_FACILITY, 0x0005)
#define DV_HALFDUPLEX               MAKE_HRESULT(0, _DVERR_FACILITY, 0x000A)
#define DVERR_GENERIC               E_FAIL
#define DVERR_INVALIDPARAM          E_INVALIDARG
#define DVERR_INVALIDPOINTER        E_POINTER
#define DVERR_OUTOFMEMORY           E_OUTOFMEMORY
#define DVERR_BUFFERTOOSMALL        MAKE_DVHRESULT(0x001E)
#define DVERR_RUNSETUP              MAKE_DVHRESULT(0x0046)
#define DVERR_NOTCONNECTED          MAKE_DVHRESULT(0x002A)
#define DVERR_NOTINITIALIZED        MAKE_DVHRESULT(0x0028)
#define DVERR_SESSIONLOST           MAKE_DVHRESULT(0x0030)
#define DVERR_NOTRANSPORT           MAKE_DVHRESULT(0x0034)

/* ============================================================
 * Client message IDs
 * ============================================================ */
#define DVMSGID_CREATEVOICEPLAYER       0x0001
#define DVMSGID_DELETEVOICEPLAYER       0x0002
#define DVMSGID_SESSIONLOST             0x0003
#define DVMSGID_PLAYERVOICESTART        0x0004
#define DVMSGID_PLAYERVOICESTOP         0x0005
#define DVMSGID_RECORDSTART             0x0006
#define DVMSGID_RECORDSTOP              0x0007
#define DVMSGID_CONNECTRESULT           0x0008
#define DVMSGID_DISCONNECTRESULT        0x0009
#define DVMSGID_INPUTLEVEL              0x000A
#define DVMSGID_OUTPUTLEVEL             0x000B
#define DVMSGID_HOSTMIGRATED            0x000C
#define DVMSGID_SETTARGETS              0x000D
#define DVMSGID_PLAYEROUTPUTLEVEL       0x000E

/* ============================================================
 * Session / config flags and values
 * ============================================================ */
#define DVSESSIONTYPE_PEER              0x00000001
#define DVSESSIONTYPE_MIXING           0x00000002
#define DVSESSIONTYPE_FORWARDING       0x00000003
#define DVSESSIONTYPE_ECHO             0x00000004

#define DVSESSION_SERVERCONTROLTARGET  0x00000001
#define DVSESSION_NOHOSTMIGRATION      0x00000004

#define DVBUFFERQUALITY_DEFAULT        0
#define DVBUFFERQUALITY_MIN            1
#define DVBUFFERQUALITY_MAX            100

#define DVBUFFERAGGRESSIVENESS_DEFAULT 0
#define DVBUFFERAGGRESSIVENESS_MIN     1
#define DVBUFFERAGGRESSIVENESS_MAX     100

#define DVCLIENTCONFIG_AUTORECORDVOLUME    0x00000004
#define DVCLIENTCONFIG_AUTOVOICEACTIVATED  0x00000020
#define DVCLIENTCONFIG_RECORDMUTE          0x00000008
#define DVCLIENTCONFIG_PLAYBACKMUTE        0x00000010
#define DVCLIENTCONFIG_MUTEGLOBAL          0x00000040
#define DVCLIENTCONFIG_ECHOSUPPRESSION     0x08000000

#define DVRECORDVOLUME_LAST            0x00000001
#define DVPLAYBACKVOLUME_DEFAULT       0x00000000
#define DVTHRESHOLD_UNUSED             0xFFFFFFFF
#define DVTHRESHOLD_DEFAULT            0x00000000

#define DVSOUNDCONFIG_AUTOSELECT             0x00000001
#define DVSOUNDCONFIG_HALFDUPLEX             0x00000002
#define DVSOUNDCONFIG_NORECVOLAVAILABLE      0x00000004
#define DVSOUNDCONFIG_SETCONVERSIONQUALITY   0x00000010
#define DVSOUNDCONFIG_STRICTFOCUS            0x00000040
#define DVSOUNDCONFIG_NOFOCUS                0x20000000

#define DVFLAGS_SYNC                   0x00000001
#define DVFLAGS_QUERYONLY              0x00000002
#define DVFLAGS_NOHOSTMIGRATE          0x00000008
#define DVFLAGS_ALLOWBACK              0x00000010

/* Special target IDs */
#define DVID_SYS                       0x00000000
#define DVID_ALLPLAYERS                0x00000000
#define DVID_SERVERPLAYER              0x00000001
#define DVID_REMAINING                 0xFFFFFFFF

/* ============================================================
 * Structures
 * ============================================================ */
typedef struct {
    DWORD   dwSize;
    DWORD   dwFlags;
    DWORD   dwSessionType;
    GUID    guidCT;
    DWORD   dwBufferQuality;
    DWORD   dwBufferAggressiveness;
} DVSESSIONDESC, *LPDVSESSIONDESC, *PDVSESSIONDESC;

typedef struct {
    DWORD   dwSize;
    DWORD   dwFlags;
    LONG    lRecordVolume;
    LONG    lPlaybackVolume;
    DWORD   dwThreshold;
    DWORD   dwBufferQuality;
    DWORD   dwBufferAggressiveness;
    DWORD   dwNotifyPeriod;
} DVCLIENTCONFIG, *LPDVCLIENTCONFIG, *PDVCLIENTCONFIG;

typedef struct {
    DWORD   dwSize;
    DWORD   dwFlags;
    GUID    guidPlaybackDevice;
    void   *lpdsPlaybackDevice;     /* LPDIRECTSOUND  - kept void* */
    GUID    guidCaptureDevice;
    void   *lpdsCaptureDevice;      /* LPDIRECTSOUNDCAPTURE - kept void* */
    HWND    hwndAppWindow;
    void   *lpdsMainBuffer;         /* LPDIRECTSOUNDBUFFER - kept void* */
    DWORD   dwMainBufferFlags;
    DWORD   dwMainBufferPriority;
} DVSOUNDDEVICECONFIG, *LPDVSOUNDDEVICECONFIG, *PDVSOUNDDEVICECONFIG;

typedef struct {
    DWORD    dwSize;
    GUID     guidType;
    WCHAR *lpszName;
    WCHAR *lpszDescription;
    DWORD    dwFlags;
    DWORD    dwMaxBitsPerSecond;
} DVCOMPRESSIONINFO, *LPDVCOMPRESSIONINFO, *PDVCOMPRESSIONINFO;

typedef struct {
    DWORD   dwSize;
    DVID    dvidPlayer;
    DWORD   dwFlags;
} DVCLIENTINFO, *LPDVCLIENTINFO, *PDVCLIENTINFO;

/* ============================================================
 * IDirectPlayVoiceClient
 * ============================================================ */
typedef struct IDirectPlayVoiceClientVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IDirectPlayVoiceClient *This, REFIID riid, LPVOID *ppv);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IDirectPlayVoiceClient *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IDirectPlayVoiceClient *This);
    HRESULT (STDMETHODCALLTYPE *Initialize)(IDirectPlayVoiceClient *This, PVOID lpVoid, PDVMESSAGEHANDLER pMessageHandler, PVOID pUserContext, PDWORD pdwMessageMask, DWORD dwMessageMaskElements);
    HRESULT (STDMETHODCALLTYPE *Connect)(IDirectPlayVoiceClient *This, PDVSOUNDDEVICECONFIG pSoundDeviceConfig, PDVCLIENTCONFIG pdvClientConfig, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *Disconnect)(IDirectPlayVoiceClient *This, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetSessionDesc)(IDirectPlayVoiceClient *This, PDVSESSIONDESC pvSessionDesc);
    HRESULT (STDMETHODCALLTYPE *GetClientConfig)(IDirectPlayVoiceClient *This, PDVCLIENTCONFIG pClientConfig);
    HRESULT (STDMETHODCALLTYPE *SetClientConfig)(IDirectPlayVoiceClient *This, PDVCLIENTCONFIG pClientConfig);
    HRESULT (STDMETHODCALLTYPE *GetCaps)(IDirectPlayVoiceClient *This, PVOID pDVCaps);
    HRESULT (STDMETHODCALLTYPE *GetCompressionTypes)(IDirectPlayVoiceClient *This, PVOID pData, PDWORD pdwDataSize, PDWORD pdwNumElements, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *SetTransmitTargets)(IDirectPlayVoiceClient *This, PDVID pdvIDTargets, DWORD dwNumTargets, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetTransmitTargets)(IDirectPlayVoiceClient *This, PDVID pdvIDTargets, PDWORD pdwNumTargets, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *Create3DSoundBuffer)(IDirectPlayVoiceClient *This, DVID dvID, PVOID lpdsBuffer, DWORD dwPriority, DWORD dwFlags, PVOID lpUserBuffer);
    HRESULT (STDMETHODCALLTYPE *Delete3DSoundBuffer)(IDirectPlayVoiceClient *This, DVID dvID, PVOID lpUserBuffer);
    HRESULT (STDMETHODCALLTYPE *SetNotifyMask)(IDirectPlayVoiceClient *This, PDWORD pdwMessageMask, DWORD dwMessageMaskElements);
    HRESULT (STDMETHODCALLTYPE *GetSoundDeviceConfig)(IDirectPlayVoiceClient *This, PDVSOUNDDEVICECONFIG pSoundDeviceConfig, PDWORD pdwSize);
} IDirectPlayVoiceClientVtbl;

struct IDirectPlayVoiceClient {
    IDirectPlayVoiceClientVtbl *lpVtbl;
#ifdef __cplusplus
    HRESULT QueryInterface(REFIID riid, LPVOID *ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG   AddRef()  { return lpVtbl->AddRef(this); }
    ULONG   Release() { return lpVtbl->Release(this); }
    HRESULT Initialize(PVOID a, PDVMESSAGEHANDLER b, PVOID c, PDWORD d, DWORD e) { return lpVtbl->Initialize(this, a, b, c, d, e); }
    HRESULT Connect(PDVSOUNDDEVICECONFIG a, PDVCLIENTCONFIG b, DWORD c) { return lpVtbl->Connect(this, a, b, c); }
    HRESULT Disconnect(DWORD a) { return lpVtbl->Disconnect(this, a); }
    HRESULT GetClientConfig(PDVCLIENTCONFIG a) { return lpVtbl->GetClientConfig(this, a); }
    HRESULT SetClientConfig(PDVCLIENTCONFIG a) { return lpVtbl->SetClientConfig(this, a); }
    HRESULT GetCompressionTypes(PVOID a, PDWORD b, PDWORD c, DWORD d) { return lpVtbl->GetCompressionTypes(this, a, b, c, d); }
    HRESULT SetTransmitTargets(PDVID a, DWORD b, DWORD c) { return lpVtbl->SetTransmitTargets(this, a, b, c); }
#endif
};

/* ============================================================
 * IDirectPlayVoiceServer
 * ============================================================ */
typedef struct IDirectPlayVoiceServerVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IDirectPlayVoiceServer *This, REFIID riid, LPVOID *ppv);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IDirectPlayVoiceServer *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IDirectPlayVoiceServer *This);
    HRESULT (STDMETHODCALLTYPE *Initialize)(IDirectPlayVoiceServer *This, PVOID lpVoid, PDVMESSAGEHANDLER pMessageHandler, PVOID pUserContext, PDWORD pdwMessageMask, DWORD dwMessageMaskElements);
    HRESULT (STDMETHODCALLTYPE *StartSession)(IDirectPlayVoiceServer *This, PDVSESSIONDESC pSessionDesc, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *StopSession)(IDirectPlayVoiceServer *This, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetSessionDesc)(IDirectPlayVoiceServer *This, PDVSESSIONDESC pvSessionDesc);
    HRESULT (STDMETHODCALLTYPE *SetSessionDesc)(IDirectPlayVoiceServer *This, PDVSESSIONDESC pSessionDesc);
    HRESULT (STDMETHODCALLTYPE *GetCaps)(IDirectPlayVoiceServer *This, PVOID pDVCaps);
    HRESULT (STDMETHODCALLTYPE *GetCompressionTypes)(IDirectPlayVoiceServer *This, PVOID pData, PDWORD pdwDataSize, PDWORD pdwNumElements, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *SetTransmitTargets)(IDirectPlayVoiceServer *This, DVID dvSource, PDVID pdvIDTargets, DWORD dwNumTargets, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetTransmitTargets)(IDirectPlayVoiceServer *This, DVID dvSource, PDVID pdvIDTargets, PDWORD pdwNumTargets, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *SetNotifyMask)(IDirectPlayVoiceServer *This, PDWORD pdwMessageMask, DWORD dwMessageMaskElements);
} IDirectPlayVoiceServerVtbl;

struct IDirectPlayVoiceServer {
    IDirectPlayVoiceServerVtbl *lpVtbl;
#ifdef __cplusplus
    HRESULT QueryInterface(REFIID riid, LPVOID *ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG   AddRef()  { return lpVtbl->AddRef(this); }
    ULONG   Release() { return lpVtbl->Release(this); }
    HRESULT Initialize(PVOID a, PDVMESSAGEHANDLER b, PVOID c, PDWORD d, DWORD e) { return lpVtbl->Initialize(this, a, b, c, d, e); }
    HRESULT StartSession(PDVSESSIONDESC a, DWORD b) { return lpVtbl->StartSession(this, a, b); }
    HRESULT StopSession(DWORD a) { return lpVtbl->StopSession(this, a); }
#endif
};

/* ============================================================
 * IDirectPlayVoiceTest
 * ============================================================ */
typedef struct IDirectPlayVoiceTestVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IDirectPlayVoiceTest *This, REFIID riid, LPVOID *ppv);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IDirectPlayVoiceTest *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IDirectPlayVoiceTest *This);
    HRESULT (STDMETHODCALLTYPE *CheckAudioSetup)(IDirectPlayVoiceTest *This, const GUID *pguidPlaybackDevice, const GUID *pguidCaptureDevice, HWND hwndParent, DWORD dwFlags);
} IDirectPlayVoiceTestVtbl;

struct IDirectPlayVoiceTest {
    IDirectPlayVoiceTestVtbl *lpVtbl;
#ifdef __cplusplus
    HRESULT QueryInterface(REFIID riid, LPVOID *ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG   AddRef()  { return lpVtbl->AddRef(this); }
    ULONG   Release() { return lpVtbl->Release(this); }
    HRESULT CheckAudioSetup(const GUID *a, const GUID *b, HWND c, DWORD d) { return lpVtbl->CheckAudioSetup(this, a, b, c, d); }
#endif
};

/* ============================================================
 * GUIDs - arbitrary distinct values.
 * ============================================================ */
static const GUID CLSID_DirectPlayVoiceClient = { 0xb9f3eb85, 0xb781, 0x4ac1, { 0x80, 0x83, 0xb6, 0xd2, 0x6a, 0x39, 0xc7, 0xb2 } };
static const GUID CLSID_DirectPlayVoiceServer = { 0xd3f5b8e6, 0x9b78, 0x4a4c, { 0x94, 0xed, 0x8c, 0xf3, 0xd9, 0x29, 0x6d, 0x5c } };
static const GUID CLSID_DirectPlayVoiceTest   = { 0x0f0f094b, 0xb01c, 0x4910, { 0xa2, 0xb6, 0xc4, 0x16, 0xb2, 0xa9, 0x10, 0xa7 } };

static const GUID IID_IDirectPlayVoiceClient  = { 0x1dfdc8ea, 0xbcf7, 0x41d6, { 0xb2, 0x95, 0xab, 0x64, 0xb3, 0xb2, 0x39, 0x95 } };
static const GUID IID_IDirectPlayVoiceServer  = { 0xfaa1c173, 0x0468, 0x43b6, { 0x8a, 0x2a, 0xea, 0x8a, 0x4f, 0x20, 0x76, 0xc9 } };
static const GUID IID_IDirectPlayVoiceTest    = { 0xd26af734, 0x208b, 0x41da, { 0x82, 0x24, 0xe0, 0xce, 0x79, 0x81, 0x0b, 0xe1 } };

/* Compression-type GUIDs (selectable codecs). Arbitrary distinct values. */
static const GUID DPVCTGUID_NONE   = { 0x8de12fd4, 0x7cb3, 0x48ce, { 0xa7, 0xe8, 0x9c, 0x47, 0xa2, 0x2e, 0x8a, 0xc5 } };
static const GUID DPVCTGUID_DEFAULT= { 0x8de12fd5, 0x7cb3, 0x48ce, { 0xa7, 0xe8, 0x9c, 0x47, 0xa2, 0x2e, 0x8a, 0xc5 } };
static const GUID DPVCTGUID_SC03   = { 0x7d8d4474, 0x0d6c, 0x4f5c, { 0x96, 0x9a, 0xc6, 0x82, 0x80, 0x9a, 0xb3, 0x16 } };
static const GUID DPVCTGUID_SC06   = { 0x53595470, 0xefa9, 0x4a5c, { 0x97, 0x70, 0x6a, 0x73, 0x6f, 0x0e, 0x6b, 0x8a } };
static const GUID DPVCTGUID_GSM    = { 0x24ed17dc, 0x9f51, 0x4b77, { 0x91, 0xc9, 0x4f, 0x91, 0xe7, 0x97, 0x71, 0x2f } };
static const GUID DPVCTGUID_G723_6 = { 0x53595472, 0xefa9, 0x4a5c, { 0x97, 0x70, 0x6a, 0x73, 0x6f, 0x0e, 0x6b, 0x8a } };
static const GUID DPVCTGUID_G723_5 = { 0x53595473, 0xefa9, 0x4a5c, { 0x97, 0x70, 0x6a, 0x73, 0x6f, 0x0e, 0x6b, 0x8a } };
static const GUID DPVCTGUID_VR12   = { 0x667744ee, 0x4f9b, 0x4d2e, { 0x9f, 0x1c, 0x76, 0x12, 0x10, 0xc8, 0xb2, 0x5d } };
static const GUID DPVCTGUID_TRUESPEECH = { 0x6abca8e9, 0x5e4d, 0x4a85, { 0x9c, 0x0d, 0x0b, 0x9b, 0x8a, 0x0a, 0xb4, 0x88 } };

/* DirectSound default voice device GUIDs (referenced by voice.cpp). */
static const GUID DSDEVID_DefaultVoicePlayback = { 0x79480000, 0x9d39, 0x11d3, { 0x9c, 0x63, 0x00, 0xc0, 0x4f, 0x8e, 0xda, 0x0d } };
static const GUID DSDEVID_DefaultVoiceCapture  = { 0x79480001, 0x9d39, 0x11d3, { 0x9c, 0x63, 0x00, 0xc0, 0x4f, 0x8e, 0xda, 0x0d } };

#ifdef __cplusplus
} /* extern "C++" */
#endif

#endif /* FF_LINUX */
#endif /* FF_COMPAT_DVOICE_H */

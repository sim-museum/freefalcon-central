/*
 * FreeFalcon Linux Port - DirectSound compatibility header
 *
 * Provides the DirectSound COM interfaces (IDirectSound, IDirectSoundBuffer,
 * IDirectSound3DBuffer, IDirectSound3DListener, IDirectSoundNotify) and the
 * associated structs/constants used by the game's sound code in src/falcsnd.
 *
 * The concrete implementation lives in openal_dsound.cpp (OpenAL-backed).
 *
 * Style: mirrors ddraw.h / d3d.h - each interface is a struct holding a
 * vtable pointer plus inline C++ wrapper methods that dispatch through the
 * vtable.  Vtable function-pointer order MUST match wrapper order MUST match
 * the impl vtable initialiser order in openal_dsound.cpp.
 */

#ifndef __DSOUND_INCLUDED__
#define __DSOUND_INCLUDED__

#ifdef FF_LINUX

#include "compat_types.h"
#include "objbase.h"

/* D3DVALUE / D3DVECTOR come from the D3D compat headers */
#include "ddraw.h"
#include "d3dtypes.h"

struct IDirectSoundCapture;
typedef struct IDirectSoundCapture *LPDIRECTSOUNDCAPTURE;
struct IDirectSoundCaptureBuffer;
typedef struct IDirectSoundCaptureBuffer *LPDIRECTSOUNDCAPTUREBUFFER;

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * WAVEFORMATEX / PCMWAVEFORMAT
 * Shared with mmsystem.h via the _WAVEFORMATEX_ guard.
 * pack(1) for binary/file compatibility.
 * ============================================================ */
#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_
#pragma pack(push, 1)
typedef struct tWAVEFORMATEX
{
    WORD  wFormatTag;
    WORD  nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD  nBlockAlign;
    WORD  wBitsPerSample;
    WORD  cbSize;
} WAVEFORMATEX, *PWAVEFORMATEX, *NPWAVEFORMATEX, *LPWAVEFORMATEX;
typedef const WAVEFORMATEX *LPCWAVEFORMATEX;
#pragma pack(pop)
#endif /* _WAVEFORMATEX_ */

#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM 1
#endif

#ifndef _PCMWAVEFORMAT_
#define _PCMWAVEFORMAT_
#pragma pack(push, 1)
typedef struct pcmwaveformat_tag
{
    WAVEFORMATEX wf;
    WORD         wBitsPerSample;
} PCMWAVEFORMAT, *PPCMWAVEFORMAT, *LPPCMWAVEFORMAT;
#pragma pack(pop)
#endif /* _PCMWAVEFORMAT_ */

#ifdef __cplusplus
} /* extern "C" */
#endif

/* ============================================================
 * HRESULT result codes
 * ============================================================ */
#define DS_OK                       ((HRESULT)0)

#define _FACDS                      0x878  /* DirectSound facility code */
#define MAKE_DSHRESULT(code)        MAKE_HRESULT(1, _FACDS, code)

#define DSERR_ALLOCATED             MAKE_DSHRESULT(10)
#define DSERR_CONTROLUNAVAIL        MAKE_DSHRESULT(30)
#define DSERR_INVALIDPARAM          E_INVALIDARG
#define DSERR_INVALIDCALL           MAKE_DSHRESULT(50)
#define DSERR_GENERIC               E_FAIL
#define DSERR_PRIOLEVELNEEDED       MAKE_DSHRESULT(70)
#define DSERR_OUTOFMEMORY           E_OUTOFMEMORY
#define DSERR_BADFORMAT             MAKE_DSHRESULT(100)
#define DSERR_UNSUPPORTED           E_NOTIMPL
#define DSERR_NODRIVER              MAKE_DSHRESULT(120)
#define DSERR_ALREADYINITIALIZED    MAKE_DSHRESULT(130)
#define DSERR_NOAGGREGATION         MAKE_DSHRESULT(140)
#define DSERR_BUFFERLOST            MAKE_DSHRESULT(150)
#define DSERR_OTHERAPPHASPRIO       MAKE_DSHRESULT(160)
#define DSERR_UNINITIALIZED         MAKE_DSHRESULT(170)
#define DSERR_NOINTERFACE           E_NOINTERFACE

/* ============================================================
 * Cooperative levels
 * ============================================================ */
#define DSSCL_NORMAL                1
#define DSSCL_PRIORITY              2
#define DSSCL_EXCLUSIVE             3
#define DSSCL_WRITEPRIMARY          4

/* ============================================================
 * Play flags
 * ============================================================ */
#define DSBPLAY_LOOPING             0x00000001
#define DSBPLAY_LOCHARDWARE         0x00000002
#define DSBPLAY_LOCSOFTWARE         0x00000004

/* ============================================================
 * Buffer status flags
 * ============================================================ */
#define DSBSTATUS_PLAYING           0x00000001
#define DSBSTATUS_BUFFERLOST        0x00000002
#define DSBSTATUS_LOOPING           0x00000004
#define DSBSTATUS_LOCHARDWARE       0x00000008
#define DSBSTATUS_LOCSOFTWARE       0x00000010
#define DSBSTATUS_TERMINATED        0x00000020

/* ============================================================
 * Buffer capability / creation flags
 * ============================================================ */
#define DSBCAPS_PRIMARYBUFFER       0x00000001
#define DSBCAPS_STATIC              0x00000002
#define DSBCAPS_LOCHARDWARE         0x00000004
#define DSBCAPS_LOCSOFTWARE         0x00000008
#define DSBCAPS_CTRL3D              0x00000010
#define DSBCAPS_CTRLFREQUENCY       0x00000020
#define DSBCAPS_CTRLPAN             0x00000040
#define DSBCAPS_CTRLVOLUME          0x00000080
#define DSBCAPS_CTRLPOSITIONNOTIFY  0x00000100
#define DSBCAPS_CTRLFX              0x00000200
#define DSBCAPS_STICKYFOCUS         0x00004000
#define DSBCAPS_GLOBALFOCUS         0x00008000
#define DSBCAPS_GETCURRENTPOSITION2 0x00010000
#define DSBCAPS_MUTE3DATMAXDISTANCE 0x00020000
#define DSBCAPS_LOCDEFER            0x00040000

/* ============================================================
 * Volume / pan / frequency limits
 * ============================================================ */
#define DSBVOLUME_MIN               (-10000)
#define DSBVOLUME_MAX               0

#define DSBPAN_LEFT                 (-10000)
#define DSBPAN_CENTER               0
#define DSBPAN_RIGHT                10000

#define DSBFREQUENCY_MIN            100
#define DSBFREQUENCY_MAX            100000
#define DSBFREQUENCY_ORIGINAL       0

/* ============================================================
 * 3D modes / apply flags
 * ============================================================ */
#define DS3DMODE_NORMAL             0x00000000
#define DS3DMODE_HEADRELATIVE       0x00000001
#define DS3DMODE_DISABLE            0x00000002

#define DS3D_IMMEDIATE              0x00000000
#define DS3D_DEFERRED               0x00000001

/* ============================================================
 * Lock flags
 * ============================================================ */
#define DSBLOCK_FROMWRITECURSOR     0x00000001
#define DSBLOCK_ENTIREBUFFER        0x00000002

/* ============================================================
 * Device capability flags
 * ============================================================ */
#define DSCAPS_PRIMARYMONO          0x00000001
#define DSCAPS_PRIMARYSTEREO        0x00000002
#define DSCAPS_PRIMARY8BIT          0x00000004
#define DSCAPS_PRIMARY16BIT         0x00000008
#define DSCAPS_CONTINUOUSRATE       0x00000010
#define DSCAPS_EMULDRIVER           0x00000020
#define DSCAPS_CERTIFIED            0x00000040
#define DSCAPS_SECONDARYMONO        0x00000100
#define DSCAPS_SECONDARYSTEREO      0x00000200
#define DSCAPS_SECONDARY8BIT        0x00000400
#define DSCAPS_SECONDARY16BIT       0x00000800

/* ============================================================
 * Speaker configuration
 * ============================================================ */
#define DSSPEAKER_HEADPHONE         1
#define DSSPEAKER_MONO              2
#define DSSPEAKER_QUAD              3
#define DSSPEAKER_STEREO            4
#define DSSPEAKER_SURROUND          5
#define DSSPEAKER_5POINT1           6
#define DSSPEAKER_7POINT1           7

#define DSSPEAKER_GEOMETRY_MIN      0x00000005  /*  5 degrees */
#define DSSPEAKER_GEOMETRY_NARROW   0x0000000A  /* 10 degrees */
#define DSSPEAKER_GEOMETRY_WIDE     0x00000014  /* 20 degrees */
#define DSSPEAKER_GEOMETRY_MAX      0x000000B4  /* 180 degrees */

#define DSSPEAKER_COMBINED(c, g)    ((DWORD)(((BYTE)(c)) | ((DWORD)((BYTE)(g))) << 16))
#define DSSPEAKER_CONFIG(a)         ((BYTE)(a))
#define DSSPEAKER_GEOMETRY(a)       ((BYTE)(((DWORD)(a) >> 16) & 0x00FF))

/* ============================================================
 * Forward declarations & typedefs
 * ============================================================ */
struct IDirectSound;
struct IDirectSoundBuffer;
struct IDirectSound3DBuffer;
struct IDirectSound3DListener;
struct IDirectSoundNotify;

typedef struct IDirectSound          *LPDIRECTSOUND;
typedef struct IDirectSoundBuffer    *LPDIRECTSOUNDBUFFER;
typedef struct IDirectSound3DBuffer  *LPDIRECTSOUND3DBUFFER;
typedef struct IDirectSound3DListener *LPDIRECTSOUND3DLISTENER;
typedef struct IDirectSoundNotify    *LPDIRECTSOUNDNOTIFY;

/* ============================================================
 * Structs
 * ============================================================ */
typedef struct _DSBUFFERDESC
{
    DWORD          dwSize;
    DWORD          dwFlags;
    DWORD          dwBufferBytes;
    DWORD          dwReserved;
    LPWAVEFORMATEX lpwfxFormat;
    GUID           guid3DAlgorithm;   /* DX8+ field; harmless on DX7-era code */
} DSBUFFERDESC, *LPDSBUFFERDESC;
typedef const DSBUFFERDESC *LPCDSBUFFERDESC;

typedef struct _DSCAPS
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwMinSecondarySampleRate;
    DWORD dwMaxSecondarySampleRate;
    DWORD dwPrimaryBuffers;
    DWORD dwMaxHwMixingAllBuffers;
    DWORD dwMaxHwMixingStaticBuffers;
    DWORD dwMaxHwMixingStreamingBuffers;
    DWORD dwFreeHwMixingAllBuffers;
    DWORD dwFreeHwMixingStaticBuffers;
    DWORD dwFreeHwMixingStreamingBuffers;
    DWORD dwMaxHw3DAllBuffers;
    DWORD dwMaxHw3DStaticBuffers;
    DWORD dwMaxHw3DStreamingBuffers;
    DWORD dwFreeHw3DAllBuffers;
    DWORD dwFreeHw3DStaticBuffers;
    DWORD dwFreeHw3DStreamingBuffers;
    DWORD dwTotalHwMemBytes;
    DWORD dwFreeHwMemBytes;
    DWORD dwMaxContigFreeHwMemBytes;
    DWORD dwUnlockTransferRateHwBuffers;
    DWORD dwPlayCpuOverheadSwBuffers;
    DWORD dwReserved1;
    DWORD dwReserved2;
} DSCAPS, *LPDSCAPS;

typedef struct _DSBCAPS
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwBufferBytes;
    DWORD dwUnlockTransferRate;
    DWORD dwPlayCpuOverhead;
} DSBCAPS, *LPDSBCAPS;

typedef struct _DS3DBUFFER
{
    DWORD     dwSize;
    D3DVECTOR vPosition;
    D3DVECTOR vVelocity;
    DWORD     dwInsideConeAngle;
    DWORD     dwOutsideConeAngle;
    D3DVECTOR vConeOrientation;
    LONG      lConeOutsideVolume;
    D3DVALUE  flMinDistance;
    D3DVALUE  flMaxDistance;
    DWORD     dwMode;
} DS3DBUFFER, *LPDS3DBUFFER;
typedef const DS3DBUFFER *LPCDS3DBUFFER;

typedef struct _DS3DLISTENER
{
    DWORD     dwSize;
    D3DVECTOR vPosition;
    D3DVECTOR vVelocity;
    D3DVECTOR vOrientFront;
    D3DVECTOR vOrientTop;
    D3DVALUE  flDistanceFactor;
    D3DVALUE  flRolloffFactor;
    D3DVALUE  flDopplerFactor;
} DS3DLISTENER, *LPDS3DLISTENER;
typedef const DS3DLISTENER *LPCDS3DLISTENER;

typedef struct _DSBPOSITIONNOTIFY
{
    DWORD  dwOffset;
    HANDLE hEventNotify;
} DSBPOSITIONNOTIFY, *LPDSBPOSITIONNOTIFY;
typedef const DSBPOSITIONNOTIFY *LPCDSBPOSITIONNOTIFY;

/* Offset value that means "play position reached the end" */
#define DSBPN_OFFSETSTOP            0xFFFFFFFF

#ifdef __cplusplus

/* ============================================================
 * IDirectSound
 * ============================================================ */
typedef struct IDirectSoundVtbl
{
    /* IUnknown */
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IDirectSound *This, REFIID riid, void **ppv);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IDirectSound *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IDirectSound *This);
    /* IDirectSound */
    HRESULT (STDMETHODCALLTYPE *CreateSoundBuffer)(IDirectSound *This, LPCDSBUFFERDESC pcDSBufferDesc, LPDIRECTSOUNDBUFFER *ppDSBuffer, IUnknown *pUnkOuter);
    HRESULT (STDMETHODCALLTYPE *GetCaps)(IDirectSound *This, LPDSCAPS pDSCaps);
    HRESULT (STDMETHODCALLTYPE *DuplicateSoundBuffer)(IDirectSound *This, LPDIRECTSOUNDBUFFER pDSBufferOriginal, LPDIRECTSOUNDBUFFER *ppDSBufferDuplicate);
    HRESULT (STDMETHODCALLTYPE *SetCooperativeLevel)(IDirectSound *This, HWND hwnd, DWORD dwLevel);
    HRESULT (STDMETHODCALLTYPE *Compact)(IDirectSound *This);
    HRESULT (STDMETHODCALLTYPE *GetSpeakerConfig)(IDirectSound *This, LPDWORD pdwSpeakerConfig);
    HRESULT (STDMETHODCALLTYPE *SetSpeakerConfig)(IDirectSound *This, DWORD dwSpeakerConfig);
    HRESULT (STDMETHODCALLTYPE *Initialize)(IDirectSound *This, const GUID *pcGuidDevice);
} IDirectSoundVtbl;

struct IDirectSound
{
    IDirectSoundVtbl *lpVtbl;

    HRESULT QueryInterface(REFIID riid, void **ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG   AddRef()  { return lpVtbl->AddRef(this); }
    ULONG   Release() { return lpVtbl->Release(this); }
    HRESULT CreateSoundBuffer(LPCDSBUFFERDESC d, LPDIRECTSOUNDBUFFER *b, IUnknown *o) { return lpVtbl->CreateSoundBuffer(this, d, b, o); }
    HRESULT GetCaps(LPDSCAPS c) { return lpVtbl->GetCaps(this, c); }
    HRESULT DuplicateSoundBuffer(LPDIRECTSOUNDBUFFER o, LPDIRECTSOUNDBUFFER *d) { return lpVtbl->DuplicateSoundBuffer(this, o, d); }
    HRESULT SetCooperativeLevel(HWND h, DWORD l) { return lpVtbl->SetCooperativeLevel(this, h, l); }
    HRESULT Compact() { return lpVtbl->Compact(this); }
    HRESULT GetSpeakerConfig(LPDWORD c) { return lpVtbl->GetSpeakerConfig(this, c); }
    HRESULT SetSpeakerConfig(DWORD c) { return lpVtbl->SetSpeakerConfig(this, c); }
    HRESULT Initialize(const GUID *g) { return lpVtbl->Initialize(this, g); }
};

/* ============================================================
 * IDirectSoundBuffer
 * ============================================================ */
typedef struct IDirectSoundBufferVtbl
{
    /* IUnknown */
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IDirectSoundBuffer *This, REFIID riid, void **ppv);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IDirectSoundBuffer *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IDirectSoundBuffer *This);
    /* IDirectSoundBuffer */
    HRESULT (STDMETHODCALLTYPE *GetCaps)(IDirectSoundBuffer *This, LPDSBCAPS pDSBufferCaps);
    HRESULT (STDMETHODCALLTYPE *GetCurrentPosition)(IDirectSoundBuffer *This, LPDWORD pdwCurrentPlayCursor, LPDWORD pdwCurrentWriteCursor);
    HRESULT (STDMETHODCALLTYPE *GetFormat)(IDirectSoundBuffer *This, LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten);
    HRESULT (STDMETHODCALLTYPE *GetVolume)(IDirectSoundBuffer *This, LPLONG plVolume);
    HRESULT (STDMETHODCALLTYPE *GetPan)(IDirectSoundBuffer *This, LPLONG plPan);
    HRESULT (STDMETHODCALLTYPE *GetFrequency)(IDirectSoundBuffer *This, LPDWORD pdwFrequency);
    HRESULT (STDMETHODCALLTYPE *GetStatus)(IDirectSoundBuffer *This, LPDWORD pdwStatus);
    HRESULT (STDMETHODCALLTYPE *Initialize)(IDirectSoundBuffer *This, LPDIRECTSOUND pDirectSound, LPCDSBUFFERDESC pcDSBufferDesc);
    HRESULT (STDMETHODCALLTYPE *Lock)(IDirectSoundBuffer *This, DWORD dwOffset, DWORD dwBytes, LPVOID *ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID *ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *Play)(IDirectSoundBuffer *This, DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *SetCurrentPosition)(IDirectSoundBuffer *This, DWORD dwNewPosition);
    HRESULT (STDMETHODCALLTYPE *SetFormat)(IDirectSoundBuffer *This, LPCWAVEFORMATEX pcfxFormat);
    HRESULT (STDMETHODCALLTYPE *SetVolume)(IDirectSoundBuffer *This, LONG lVolume);
    HRESULT (STDMETHODCALLTYPE *SetPan)(IDirectSoundBuffer *This, LONG lPan);
    HRESULT (STDMETHODCALLTYPE *SetFrequency)(IDirectSoundBuffer *This, DWORD dwFrequency);
    HRESULT (STDMETHODCALLTYPE *Stop)(IDirectSoundBuffer *This);
    HRESULT (STDMETHODCALLTYPE *Unlock)(IDirectSoundBuffer *This, LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2);
    HRESULT (STDMETHODCALLTYPE *Restore)(IDirectSoundBuffer *This);
} IDirectSoundBufferVtbl;

struct IDirectSoundBuffer
{
    IDirectSoundBufferVtbl *lpVtbl;

    HRESULT QueryInterface(REFIID riid, void **ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG   AddRef()  { return lpVtbl->AddRef(this); }
    ULONG   Release() { return lpVtbl->Release(this); }
    HRESULT GetCaps(LPDSBCAPS c) { return lpVtbl->GetCaps(this, c); }
    HRESULT GetCurrentPosition(LPDWORD p, LPDWORD w) { return lpVtbl->GetCurrentPosition(this, p, w); }
    HRESULT GetFormat(LPWAVEFORMATEX f, DWORD a, LPDWORD w) { return lpVtbl->GetFormat(this, f, a, w); }
    HRESULT GetVolume(LPLONG v) { return lpVtbl->GetVolume(this, v); }
    HRESULT GetPan(LPLONG p) { return lpVtbl->GetPan(this, p); }
    HRESULT GetFrequency(LPDWORD f) { return lpVtbl->GetFrequency(this, f); }
    HRESULT GetStatus(LPDWORD s) { return lpVtbl->GetStatus(this, s); }
    HRESULT Initialize(LPDIRECTSOUND ds, LPCDSBUFFERDESC d) { return lpVtbl->Initialize(this, ds, d); }
    HRESULT Lock(DWORD o, DWORD b, LPVOID *p1, LPDWORD b1, LPVOID *p2, LPDWORD b2, DWORD f) { return lpVtbl->Lock(this, o, b, p1, b1, p2, b2, f); }
    HRESULT Play(DWORD r1, DWORD pri, DWORD f) { return lpVtbl->Play(this, r1, pri, f); }
    HRESULT SetCurrentPosition(DWORD p) { return lpVtbl->SetCurrentPosition(this, p); }
    HRESULT SetFormat(LPCWAVEFORMATEX f) { return lpVtbl->SetFormat(this, f); }
    HRESULT SetVolume(LONG v) { return lpVtbl->SetVolume(this, v); }
    HRESULT SetPan(LONG p) { return lpVtbl->SetPan(this, p); }
    HRESULT SetFrequency(DWORD f) { return lpVtbl->SetFrequency(this, f); }
    HRESULT Stop() { return lpVtbl->Stop(this); }
    HRESULT Unlock(LPVOID p1, DWORD b1, LPVOID p2, DWORD b2) { return lpVtbl->Unlock(this, p1, b1, p2, b2); }
    HRESULT Restore() { return lpVtbl->Restore(this); }
};

/* ============================================================
 * IDirectSound3DBuffer
 * ============================================================ */
typedef struct IDirectSound3DBufferVtbl
{
    /* IUnknown */
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IDirectSound3DBuffer *This, REFIID riid, void **ppv);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IDirectSound3DBuffer *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IDirectSound3DBuffer *This);
    /* IDirectSound3DBuffer */
    HRESULT (STDMETHODCALLTYPE *GetAllParameters)(IDirectSound3DBuffer *This, LPDS3DBUFFER pDs3dBuffer);
    HRESULT (STDMETHODCALLTYPE *GetConeAngles)(IDirectSound3DBuffer *This, LPDWORD pdwInsideConeAngle, LPDWORD pdwOutsideConeAngle);
    HRESULT (STDMETHODCALLTYPE *GetConeOrientation)(IDirectSound3DBuffer *This, D3DVECTOR *pvOrientation);
    HRESULT (STDMETHODCALLTYPE *GetConeOutsideVolume)(IDirectSound3DBuffer *This, LPLONG plConeOutsideVolume);
    HRESULT (STDMETHODCALLTYPE *GetMaxDistance)(IDirectSound3DBuffer *This, D3DVALUE *pflMaxDistance);
    HRESULT (STDMETHODCALLTYPE *GetMinDistance)(IDirectSound3DBuffer *This, D3DVALUE *pflMinDistance);
    HRESULT (STDMETHODCALLTYPE *GetMode)(IDirectSound3DBuffer *This, LPDWORD pdwMode);
    HRESULT (STDMETHODCALLTYPE *GetPosition)(IDirectSound3DBuffer *This, D3DVECTOR *pvPosition);
    HRESULT (STDMETHODCALLTYPE *GetVelocity)(IDirectSound3DBuffer *This, D3DVECTOR *pvVelocity);
    HRESULT (STDMETHODCALLTYPE *SetAllParameters)(IDirectSound3DBuffer *This, LPCDS3DBUFFER pcDs3dBuffer, DWORD dwApply);
    HRESULT (STDMETHODCALLTYPE *SetConeAngles)(IDirectSound3DBuffer *This, DWORD dwInsideConeAngle, DWORD dwOutsideConeAngle, DWORD dwApply);
    HRESULT (STDMETHODCALLTYPE *SetConeOrientation)(IDirectSound3DBuffer *This, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
    HRESULT (STDMETHODCALLTYPE *SetConeOutsideVolume)(IDirectSound3DBuffer *This, LONG lConeOutsideVolume, DWORD dwApply);
    HRESULT (STDMETHODCALLTYPE *SetMaxDistance)(IDirectSound3DBuffer *This, D3DVALUE flMaxDistance, DWORD dwApply);
    HRESULT (STDMETHODCALLTYPE *SetMinDistance)(IDirectSound3DBuffer *This, D3DVALUE flMinDistance, DWORD dwApply);
    HRESULT (STDMETHODCALLTYPE *SetMode)(IDirectSound3DBuffer *This, DWORD dwMode, DWORD dwApply);
    HRESULT (STDMETHODCALLTYPE *SetPosition)(IDirectSound3DBuffer *This, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
    HRESULT (STDMETHODCALLTYPE *SetVelocity)(IDirectSound3DBuffer *This, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
} IDirectSound3DBufferVtbl;

struct IDirectSound3DBuffer
{
    IDirectSound3DBufferVtbl *lpVtbl;

    HRESULT QueryInterface(REFIID riid, void **ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG   AddRef()  { return lpVtbl->AddRef(this); }
    ULONG   Release() { return lpVtbl->Release(this); }
    HRESULT GetAllParameters(LPDS3DBUFFER b) { return lpVtbl->GetAllParameters(this, b); }
    HRESULT GetConeAngles(LPDWORD in, LPDWORD out) { return lpVtbl->GetConeAngles(this, in, out); }
    HRESULT GetConeOrientation(D3DVECTOR *o) { return lpVtbl->GetConeOrientation(this, o); }
    HRESULT GetConeOutsideVolume(LPLONG v) { return lpVtbl->GetConeOutsideVolume(this, v); }
    HRESULT GetMaxDistance(D3DVALUE *d) { return lpVtbl->GetMaxDistance(this, d); }
    HRESULT GetMinDistance(D3DVALUE *d) { return lpVtbl->GetMinDistance(this, d); }
    HRESULT GetMode(LPDWORD m) { return lpVtbl->GetMode(this, m); }
    HRESULT GetPosition(D3DVECTOR *p) { return lpVtbl->GetPosition(this, p); }
    HRESULT GetVelocity(D3DVECTOR *v) { return lpVtbl->GetVelocity(this, v); }
    HRESULT SetAllParameters(LPCDS3DBUFFER b, DWORD a) { return lpVtbl->SetAllParameters(this, b, a); }
    HRESULT SetConeAngles(DWORD in, DWORD out, DWORD a) { return lpVtbl->SetConeAngles(this, in, out, a); }
    HRESULT SetConeOrientation(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD a) { return lpVtbl->SetConeOrientation(this, x, y, z, a); }
    HRESULT SetConeOutsideVolume(LONG v, DWORD a) { return lpVtbl->SetConeOutsideVolume(this, v, a); }
    HRESULT SetMaxDistance(D3DVALUE d, DWORD a) { return lpVtbl->SetMaxDistance(this, d, a); }
    HRESULT SetMinDistance(D3DVALUE d, DWORD a) { return lpVtbl->SetMinDistance(this, d, a); }
    HRESULT SetMode(DWORD m, DWORD a) { return lpVtbl->SetMode(this, m, a); }
    HRESULT SetPosition(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD a) { return lpVtbl->SetPosition(this, x, y, z, a); }
    HRESULT SetVelocity(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD a) { return lpVtbl->SetVelocity(this, x, y, z, a); }
};

/* ============================================================
 * IDirectSound3DListener
 * ============================================================ */
typedef struct IDirectSound3DListenerVtbl
{
    /* IUnknown */
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IDirectSound3DListener *This, REFIID riid, void **ppv);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IDirectSound3DListener *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IDirectSound3DListener *This);
    /* IDirectSound3DListener */
    HRESULT (STDMETHODCALLTYPE *GetAllParameters)(IDirectSound3DListener *This, LPDS3DLISTENER pListener);
    HRESULT (STDMETHODCALLTYPE *GetDistanceFactor)(IDirectSound3DListener *This, D3DVALUE *pflDistanceFactor);
    HRESULT (STDMETHODCALLTYPE *GetDopplerFactor)(IDirectSound3DListener *This, D3DVALUE *pflDopplerFactor);
    HRESULT (STDMETHODCALLTYPE *GetOrientation)(IDirectSound3DListener *This, D3DVECTOR *pvOrientFront, D3DVECTOR *pvOrientTop);
    HRESULT (STDMETHODCALLTYPE *GetPosition)(IDirectSound3DListener *This, D3DVECTOR *pvPosition);
    HRESULT (STDMETHODCALLTYPE *GetRolloffFactor)(IDirectSound3DListener *This, D3DVALUE *pflRolloffFactor);
    HRESULT (STDMETHODCALLTYPE *GetVelocity)(IDirectSound3DListener *This, D3DVECTOR *pvVelocity);
    HRESULT (STDMETHODCALLTYPE *SetAllParameters)(IDirectSound3DListener *This, LPCDS3DLISTENER pcListener, DWORD dwApply);
    HRESULT (STDMETHODCALLTYPE *SetDistanceFactor)(IDirectSound3DListener *This, D3DVALUE flDistanceFactor, DWORD dwApply);
    HRESULT (STDMETHODCALLTYPE *SetDopplerFactor)(IDirectSound3DListener *This, D3DVALUE flDopplerFactor, DWORD dwApply);
    HRESULT (STDMETHODCALLTYPE *SetOrientation)(IDirectSound3DListener *This, D3DVALUE xFront, D3DVALUE yFront, D3DVALUE zFront, D3DVALUE xTop, D3DVALUE yTop, D3DVALUE zTop, DWORD dwApply);
    HRESULT (STDMETHODCALLTYPE *SetPosition)(IDirectSound3DListener *This, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
    HRESULT (STDMETHODCALLTYPE *SetRolloffFactor)(IDirectSound3DListener *This, D3DVALUE flRolloffFactor, DWORD dwApply);
    HRESULT (STDMETHODCALLTYPE *SetVelocity)(IDirectSound3DListener *This, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply);
    HRESULT (STDMETHODCALLTYPE *CommitDeferredSettings)(IDirectSound3DListener *This);
} IDirectSound3DListenerVtbl;

struct IDirectSound3DListener
{
    IDirectSound3DListenerVtbl *lpVtbl;

    HRESULT QueryInterface(REFIID riid, void **ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG   AddRef()  { return lpVtbl->AddRef(this); }
    ULONG   Release() { return lpVtbl->Release(this); }
    HRESULT GetAllParameters(LPDS3DLISTENER l) { return lpVtbl->GetAllParameters(this, l); }
    HRESULT GetDistanceFactor(D3DVALUE *f) { return lpVtbl->GetDistanceFactor(this, f); }
    HRESULT GetDopplerFactor(D3DVALUE *f) { return lpVtbl->GetDopplerFactor(this, f); }
    HRESULT GetOrientation(D3DVECTOR *f, D3DVECTOR *t) { return lpVtbl->GetOrientation(this, f, t); }
    HRESULT GetPosition(D3DVECTOR *p) { return lpVtbl->GetPosition(this, p); }
    HRESULT GetRolloffFactor(D3DVALUE *f) { return lpVtbl->GetRolloffFactor(this, f); }
    HRESULT GetVelocity(D3DVECTOR *v) { return lpVtbl->GetVelocity(this, v); }
    HRESULT SetAllParameters(LPCDS3DLISTENER l, DWORD a) { return lpVtbl->SetAllParameters(this, l, a); }
    HRESULT SetDistanceFactor(D3DVALUE f, DWORD a) { return lpVtbl->SetDistanceFactor(this, f, a); }
    HRESULT SetDopplerFactor(D3DVALUE f, DWORD a) { return lpVtbl->SetDopplerFactor(this, f, a); }
    HRESULT SetOrientation(D3DVALUE xf, D3DVALUE yf, D3DVALUE zf, D3DVALUE xt, D3DVALUE yt, D3DVALUE zt, DWORD a) { return lpVtbl->SetOrientation(this, xf, yf, zf, xt, yt, zt, a); }
    HRESULT SetPosition(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD a) { return lpVtbl->SetPosition(this, x, y, z, a); }
    HRESULT SetRolloffFactor(D3DVALUE f, DWORD a) { return lpVtbl->SetRolloffFactor(this, f, a); }
    HRESULT SetVelocity(D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD a) { return lpVtbl->SetVelocity(this, x, y, z, a); }
    HRESULT CommitDeferredSettings() { return lpVtbl->CommitDeferredSettings(this); }
};

/* ============================================================
 * IDirectSoundNotify
 * ============================================================ */
typedef struct IDirectSoundNotifyVtbl
{
    /* IUnknown */
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IDirectSoundNotify *This, REFIID riid, void **ppv);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IDirectSoundNotify *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IDirectSoundNotify *This);
    /* IDirectSoundNotify */
    HRESULT (STDMETHODCALLTYPE *SetNotificationPositions)(IDirectSoundNotify *This, DWORD dwPositionNotifies, LPCDSBPOSITIONNOTIFY pcPositionNotifies);
} IDirectSoundNotifyVtbl;

struct IDirectSoundNotify
{
    IDirectSoundNotifyVtbl *lpVtbl;

    HRESULT QueryInterface(REFIID riid, void **ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG   AddRef()  { return lpVtbl->AddRef(this); }
    ULONG   Release() { return lpVtbl->Release(this); }
    HRESULT SetNotificationPositions(DWORD n, LPCDSBPOSITIONNOTIFY p) { return lpVtbl->SetNotificationPositions(this, n, p); }
};

#endif /* __cplusplus */

/* ============================================================
 * Interface GUIDs
 *
 * These match the real DirectSound IIDs (only compared inside our own
 * QueryInterface implementation, so any consistent values work, but the
 * real ones are used for documentation accuracy).
 * ============================================================ */
#ifndef GUID_NULL_DEFINED
#define GUID_NULL_DEFINED
/* GUID_NULL provided by objbase.h */
#endif

/* IID_IDirectSound3DListener {279AFA84-4981-11CE-A521-0020AF0BE560} */
static const GUID IID_IDirectSound3DListener =
{ 0x279AFA84, 0x4981, 0x11CE, { 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60 } };

/* IID_IDirectSound3DBuffer {279AFA86-4981-11CE-A521-0020AF0BE560} */
static const GUID IID_IDirectSound3DBuffer =
{ 0x279AFA86, 0x4981, 0x11CE, { 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60 } };

/* IID_IDirectSoundNotify {B0210783-89CD-11D0-AF08-00A0C925CD16} */
static const GUID IID_IDirectSoundNotify =
{ 0xB0210783, 0x89CD, 0x11D0, { 0xAF, 0x08, 0x00, 0xA0, 0xC9, 0x25, 0xCD, 0x16 } };

/* DS3DALG_DEFAULT == GUID_NULL */
#define DS3DALG_DEFAULT             GUID_NULL

/* ============================================================
 * Entry points
 * ============================================================ */
#ifdef __cplusplus
extern "C" {
#endif

typedef BOOL (CALLBACK *LPDSENUMCALLBACKA)(GUID *pGuid, const char *pszDescription, const char *pszModule, void *pContext);

HRESULT DirectSoundCreate(GUID *pcGuidDevice, LPDIRECTSOUND *ppDS, IUnknown *pUnkOuter);
HRESULT DirectSoundEnumerateA(LPDSENUMCALLBACKA pDSEnumCallback, void *pContext);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FF_LINUX */
#endif /* __DSOUND_INCLUDED__ */

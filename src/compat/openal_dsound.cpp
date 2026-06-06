/*
 * FreeFalcon Linux Port - OpenAL-backed DirectSound implementation
 *
 * Implements the DirectSound COM interfaces declared in dsound.h on top of
 * OpenAL.  The structure follows the D3D7->OpenGL translation layer in
 * d3d_gl.cpp: each interface is a concrete struct deriving from the matching
 * dsound.h interface; the struct installs a static vtable of free functions in
 * its constructor; each vtable function casts `This` back to the concrete type.
 *
 * Volume:    DirectSound centibels (-10000..0) -> AL gain pow(10, cb/2000).
 * Pan:       -10000..10000 -> AL_SOURCE_RELATIVE position x = pan/10000.
 * Coords:    DirectSound is left-handed, OpenAL right-handed -> negate Z.
 *
 * Thread-safety: every AL/object operation is serialised by g_dsMutex.
 */

#ifdef FF_LINUX

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <mutex>

#include <AL/al.h>
#include <AL/alc.h>

#include "compat_types.h"
#include "objbase.h"
#include "d3dtypes.h"   /* D3DVECTOR / D3DVALUE */
#include "dsound.h"
#include "openal_dsound.h"

/* ============================================================
 * Global AL device/context (opened lazily by DirectSoundCreate)
 * ============================================================ */
static ALCdevice*  g_alcDevice  = nullptr;
static ALCcontext* g_alcContext = nullptr;
static std::recursive_mutex g_dsMutex;
static float       g_distanceFactor = 1.0f;
static float       g_rolloffFactor  = 1.0f;

/* Forward declarations of the concrete types & vtables */
struct OpenALDirectSound;
struct OpenALSoundBuffer;
struct OpenAL3DBuffer;
struct OpenAL3DListener;
struct OpenALSoundNotify;

extern const IDirectSoundVtbl           g_DSVtbl;
extern const IDirectSoundBufferVtbl     g_DSBVtbl;
extern const IDirectSound3DBufferVtbl   g_DS3DBVtbl;
extern const IDirectSound3DListenerVtbl g_DS3DLVtbl;
extern const IDirectSoundNotifyVtbl     g_DSNotifyVtbl;

/* ============================================================
 * Helpers
 * ============================================================ */
static float DSVolumeToGain(LONG cb)
{
    if (cb <= DSBVOLUME_MIN) return 0.0f;
    if (cb >= DSBVOLUME_MAX) return 1.0f;
    float g = powf(10.0f, (float)cb / 2000.0f);
    if (g < 0.0f) g = 0.0f;
    if (g > 1.0f) g = 1.0f;
    return g;
}

static ALenum ALFormatFor(WORD channels, WORD bits)
{
    if (channels >= 2)
        return (bits == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
    return (bits == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
}

/* ============================================================
 * OpenAL3DListener
 * ============================================================ */
struct OpenAL3DListener : public IDirectSound3DListener
{
    LONG refCount;
    OpenAL3DListener() : refCount(1)
    {
        lpVtbl = const_cast<IDirectSound3DListenerVtbl*>(&g_DS3DLVtbl);
    }
};

/* ============================================================
 * OpenAL3DBuffer - 3D wrapper bound to a parent buffer's AL source
 * ============================================================ */
struct OpenAL3DBuffer : public IDirectSound3DBuffer
{
    LONG               refCount;
    OpenALSoundBuffer* parent;   /* not owned */
    OpenAL3DBuffer(OpenALSoundBuffer* p) : refCount(1), parent(p)
    {
        lpVtbl = const_cast<IDirectSound3DBufferVtbl*>(&g_DS3DBVtbl);
    }
};

/* ============================================================
 * OpenALSoundNotify - no-op notification interface
 * ============================================================ */
struct OpenALSoundNotify : public IDirectSoundNotify
{
    LONG               refCount;
    OpenALSoundBuffer* parent;   /* not owned */
    OpenALSoundNotify(OpenALSoundBuffer* p) : refCount(1), parent(p)
    {
        lpVtbl = const_cast<IDirectSoundNotifyVtbl*>(&g_DSNotifyVtbl);
    }
};

/* ============================================================
 * OpenALSoundBuffer
 * ============================================================ */
struct OpenALSoundBuffer : public IDirectSoundBuffer
{
    LONG          refCount;

    bool          isPrimary;
    DWORD         dwFlags;        /* DSBCAPS_* from creation */
    WAVEFORMATEX  format;

    /* PCM backing store for secondary buffers */
    unsigned char* pcm;           /* malloc'd, dwBufferBytes long */
    DWORD          bufferBytes;

    /* Lazily-created AL objects (secondary buffers only) */
    ALuint         alSource;
    ALuint         alBuffer;
    bool           alUploaded;    /* alBufferData has been called */
    bool           looping;

    LONG           volume;        /* centibels */
    LONG           pan;           /* -10000..10000 */
    DWORD          frequency;     /* current play frequency (Hz) */

    OpenAL3DListener* listener;   /* primary buffer only; owned */
    OpenAL3DBuffer*   threeD;     /* secondary; owned, lazily created */
    OpenALSoundNotify* notify;    /* secondary; owned, lazily created */
    bool           is3dEnabled;

    OpenALSoundBuffer()
        : refCount(1), isPrimary(false), dwFlags(0),
          pcm(nullptr), bufferBytes(0),
          alSource(0), alBuffer(0), alUploaded(false), looping(false),
          volume(DSBVOLUME_MAX), pan(DSBPAN_CENTER), frequency(0),
          listener(nullptr), threeD(nullptr), notify(nullptr), is3dEnabled(false)
    {
        lpVtbl = const_cast<IDirectSoundBufferVtbl*>(&g_DSBVtbl);
        memset(&format, 0, sizeof(format));
    }

    ~OpenALSoundBuffer()
    {
        if (alSource) { alSourceStop(alSource); alDeleteSources(1, &alSource); }
        if (alBuffer) { alDeleteBuffers(1, &alBuffer); }
        if (pcm)      { free(pcm); }
        if (listener) { delete listener; }
        if (threeD)   { delete threeD; }
        if (notify)   { delete notify; }
    }

    /* Ensure an AL source exists (lazy). */
    void EnsureSource()
    {
        if (!alSource && !isPrimary)
        {
            alGenSources(1, &alSource);
            if (alSource)
            {
                /* FF_LINUX: apply state that was set while no source existed.
                   DuplicateSoundBuffer and SetVolume can run before the source
                   is created; without this, a freshly created source has no
                   buffer attached and default gain, so Play() is silent. */
                if (alBuffer) alSourcei(alSource, AL_BUFFER, (ALint)alBuffer);
                alSourcef(alSource, AL_GAIN, DSVolumeToGain(volume));
                if (frequency && format.nSamplesPerSec)
                    alSourcef(alSource, AL_PITCH, (float)frequency / (float)format.nSamplesPerSec);
            }
        }
        else if (alSource && alBuffer)
        {
            /* Re-attach if the buffer was created after the source */
            ALint cur = 0;
            alGetSourcei(alSource, AL_BUFFER, &cur);
            if (!cur) alSourcei(alSource, AL_BUFFER, (ALint)alBuffer);
        }
    }
};

/* ============================================================
 * OpenALDirectSound
 * ============================================================ */
struct OpenALDirectSound : public IDirectSound
{
    LONG refCount;
    OpenALDirectSound() : refCount(1)
    {
        lpVtbl = const_cast<IDirectSoundVtbl*>(&g_DSVtbl);
    }
};

/* ============================================================
 * IDirectSoundNotify implementation (no-op)
 * ============================================================ */
static HRESULT STDMETHODCALLTYPE DSNotify_QueryInterface(IDirectSoundNotify* This, REFIID riid, void** ppv)
{
    if (!ppv) return E_POINTER;
    if (IsEqualIID(riid, IID_IDirectSoundNotify)) { This->AddRef(); *ppv = This; return DS_OK; }
    *ppv = nullptr;
    return E_NOINTERFACE;
}
static ULONG STDMETHODCALLTYPE DSNotify_AddRef(IDirectSoundNotify* This)
{
    OpenALSoundNotify* n = (OpenALSoundNotify*)This;
    return (ULONG)++n->refCount;
}
static ULONG STDMETHODCALLTYPE DSNotify_Release(IDirectSoundNotify* This)
{
    OpenALSoundNotify* n = (OpenALSoundNotify*)This;
    LONG rc = --n->refCount;
    if (rc <= 0) { if (n->parent) n->parent->notify = nullptr; delete n; return 0; }
    return (ULONG)rc;
}
static HRESULT STDMETHODCALLTYPE DSNotify_SetNotificationPositions(IDirectSoundNotify* This, DWORD n, LPCDSBPOSITIONNOTIFY p)
{
    /* Streaming voice notification not modelled; accept silently. */
    (void)This; (void)n; (void)p;
    return DS_OK;
}

const IDirectSoundNotifyVtbl g_DSNotifyVtbl =
{
    DSNotify_QueryInterface,
    DSNotify_AddRef,
    DSNotify_Release,
    DSNotify_SetNotificationPositions,
};

/* ============================================================
 * IDirectSound3DListener implementation
 * ============================================================ */
static HRESULT STDMETHODCALLTYPE DS3DL_QueryInterface(IDirectSound3DListener* This, REFIID riid, void** ppv)
{
    if (!ppv) return E_POINTER;
    if (IsEqualIID(riid, IID_IDirectSound3DListener)) { This->AddRef(); *ppv = This; return DS_OK; }
    *ppv = nullptr;
    return E_NOINTERFACE;
}
static ULONG STDMETHODCALLTYPE DS3DL_AddRef(IDirectSound3DListener* This)
{
    OpenAL3DListener* l = (OpenAL3DListener*)This;
    return (ULONG)++l->refCount;
}
static ULONG STDMETHODCALLTYPE DS3DL_Release(IDirectSound3DListener* This)
{
    OpenAL3DListener* l = (OpenAL3DListener*)This;
    LONG rc = --l->refCount;
    if (rc <= 0) { delete l; return 0; }
    return (ULONG)rc;
}
static HRESULT STDMETHODCALLTYPE DS3DL_GetAllParameters(IDirectSound3DListener* This, LPDS3DLISTENER l)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    (void)This;
    if (!l) return DSERR_INVALIDPARAM;
    ALfloat pos[3] = {0,0,0}, vel[3] = {0,0,0}, ori[6] = {0,0,0,0,0,0};
    alGetListenerfv(AL_POSITION, pos);
    alGetListenerfv(AL_VELOCITY, vel);
    alGetListenerfv(AL_ORIENTATION, ori);
    l->vPosition    = D3DVECTOR(pos[0], pos[1], -pos[2]);
    l->vVelocity    = D3DVECTOR(vel[0], vel[1], -vel[2]);
    l->vOrientFront = D3DVECTOR(ori[0], ori[1], -ori[2]);
    l->vOrientTop   = D3DVECTOR(ori[3], ori[4], -ori[5]);
    l->flDistanceFactor = g_distanceFactor;
    l->flRolloffFactor  = g_rolloffFactor;
    l->flDopplerFactor  = alGetFloat(AL_DOPPLER_FACTOR);
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DL_GetDistanceFactor(IDirectSound3DListener* This, D3DVALUE* f)
{
    (void)This; if (!f) return DSERR_INVALIDPARAM; *f = g_distanceFactor; return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DL_GetDopplerFactor(IDirectSound3DListener* This, D3DVALUE* f)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    (void)This; if (!f) return DSERR_INVALIDPARAM; *f = alGetFloat(AL_DOPPLER_FACTOR); return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DL_GetOrientation(IDirectSound3DListener* This, D3DVECTOR* front, D3DVECTOR* top)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    (void)This;
    ALfloat ori[6] = {0,0,0,0,0,0};
    alGetListenerfv(AL_ORIENTATION, ori);
    if (front) *front = D3DVECTOR(ori[0], ori[1], -ori[2]);
    if (top)   *top   = D3DVECTOR(ori[3], ori[4], -ori[5]);
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DL_GetPosition(IDirectSound3DListener* This, D3DVECTOR* p)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    (void)This;
    ALfloat v[3] = {0,0,0};
    alGetListenerfv(AL_POSITION, v);
    if (p) *p = D3DVECTOR(v[0], v[1], -v[2]);
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DL_GetRolloffFactor(IDirectSound3DListener* This, D3DVALUE* f)
{
    (void)This; if (!f) return DSERR_INVALIDPARAM; *f = g_rolloffFactor; return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DL_GetVelocity(IDirectSound3DListener* This, D3DVECTOR* v)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    (void)This;
    ALfloat a[3] = {0,0,0};
    alGetListenerfv(AL_VELOCITY, a);
    if (v) *v = D3DVECTOR(a[0], a[1], -a[2]);
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DL_SetAllParameters(IDirectSound3DListener* This, LPCDS3DLISTENER l, DWORD apply)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    (void)This; (void)apply;
    if (!l) return DSERR_INVALIDPARAM;
    alListener3f(AL_POSITION, l->vPosition.x, l->vPosition.y, -l->vPosition.z);
    alListener3f(AL_VELOCITY, l->vVelocity.x, l->vVelocity.y, -l->vVelocity.z);
    ALfloat ori[6] = { l->vOrientFront.x, l->vOrientFront.y, -l->vOrientFront.z,
                       l->vOrientTop.x,   l->vOrientTop.y,   -l->vOrientTop.z };
    alListenerfv(AL_ORIENTATION, ori);
    g_distanceFactor = l->flDistanceFactor;
    g_rolloffFactor  = l->flRolloffFactor;
    alDopplerFactor(l->flDopplerFactor);
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DL_SetDistanceFactor(IDirectSound3DListener* This, D3DVALUE f, DWORD apply)
{
    (void)This; (void)apply; g_distanceFactor = f; return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DL_SetDopplerFactor(IDirectSound3DListener* This, D3DVALUE f, DWORD apply)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    (void)This; (void)apply; alDopplerFactor(f); return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DL_SetOrientation(IDirectSound3DListener* This, D3DVALUE xf, D3DVALUE yf, D3DVALUE zf, D3DVALUE xt, D3DVALUE yt, D3DVALUE zt, DWORD apply)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    (void)This; (void)apply;
    ALfloat ori[6] = { xf, yf, -zf, xt, yt, -zt };
    alListenerfv(AL_ORIENTATION, ori);
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DL_SetPosition(IDirectSound3DListener* This, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD apply)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    (void)This; (void)apply;
    alListener3f(AL_POSITION, x, y, -z);
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DL_SetRolloffFactor(IDirectSound3DListener* This, D3DVALUE f, DWORD apply)
{
    (void)This; (void)apply; g_rolloffFactor = f; return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DL_SetVelocity(IDirectSound3DListener* This, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD apply)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    (void)This; (void)apply;
    alListener3f(AL_VELOCITY, x, y, -z);
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DL_CommitDeferredSettings(IDirectSound3DListener* This)
{
    (void)This; return DS_OK;  /* OpenAL applies immediately */
}

const IDirectSound3DListenerVtbl g_DS3DLVtbl =
{
    DS3DL_QueryInterface,
    DS3DL_AddRef,
    DS3DL_Release,
    DS3DL_GetAllParameters,
    DS3DL_GetDistanceFactor,
    DS3DL_GetDopplerFactor,
    DS3DL_GetOrientation,
    DS3DL_GetPosition,
    DS3DL_GetRolloffFactor,
    DS3DL_GetVelocity,
    DS3DL_SetAllParameters,
    DS3DL_SetDistanceFactor,
    DS3DL_SetDopplerFactor,
    DS3DL_SetOrientation,
    DS3DL_SetPosition,
    DS3DL_SetRolloffFactor,
    DS3DL_SetVelocity,
    DS3DL_CommitDeferredSettings,
};

/* ============================================================
 * IDirectSound3DBuffer implementation (bound to parent->alSource)
 * ============================================================ */
static HRESULT STDMETHODCALLTYPE DS3DB_QueryInterface(IDirectSound3DBuffer* This, REFIID riid, void** ppv)
{
    if (!ppv) return E_POINTER;
    if (IsEqualIID(riid, IID_IDirectSound3DBuffer)) { This->AddRef(); *ppv = This; return DS_OK; }
    *ppv = nullptr;
    return E_NOINTERFACE;
}
static ULONG STDMETHODCALLTYPE DS3DB_AddRef(IDirectSound3DBuffer* This)
{
    OpenAL3DBuffer* b = (OpenAL3DBuffer*)This;
    return (ULONG)++b->refCount;
}
static ULONG STDMETHODCALLTYPE DS3DB_Release(IDirectSound3DBuffer* This)
{
    OpenAL3DBuffer* b = (OpenAL3DBuffer*)This;
    LONG rc = --b->refCount;
    if (rc <= 0) { if (b->parent) b->parent->threeD = nullptr; delete b; return 0; }
    return (ULONG)rc;
}
static HRESULT STDMETHODCALLTYPE DS3DB_GetAllParameters(IDirectSound3DBuffer* This, LPDS3DBUFFER p)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    OpenAL3DBuffer* b = (OpenAL3DBuffer*)This;
    if (!p || !b->parent) return DSERR_INVALIDPARAM;
    ALuint src = b->parent->alSource;
    ALfloat pos[3] = {0,0,0}, vel[3] = {0,0,0};
    ALfloat minD = 1.0f, maxD = 1000000000.0f;
    if (src) {
        alGetSourcefv(src, AL_POSITION, pos);
        alGetSourcefv(src, AL_VELOCITY, vel);
        alGetSourcef(src, AL_REFERENCE_DISTANCE, &minD);
        alGetSourcef(src, AL_MAX_DISTANCE, &maxD);
    }
    p->vPosition = D3DVECTOR(pos[0], pos[1], -pos[2]);
    p->vVelocity = D3DVECTOR(vel[0], vel[1], -vel[2]);
    p->flMinDistance = minD;
    p->flMaxDistance = maxD;
    p->dwMode = b->parent->is3dEnabled ? DS3DMODE_NORMAL : DS3DMODE_DISABLE;
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DB_GetConeAngles(IDirectSound3DBuffer* This, LPDWORD in, LPDWORD out)
{
    (void)This; if (in) *in = 360; if (out) *out = 360; return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DB_GetConeOrientation(IDirectSound3DBuffer* This, D3DVECTOR* o)
{
    (void)This; if (o) *o = D3DVECTOR(0, 0, 1); return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DB_GetConeOutsideVolume(IDirectSound3DBuffer* This, LPLONG v)
{
    (void)This; if (v) *v = DSBVOLUME_MAX; return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DB_GetMaxDistance(IDirectSound3DBuffer* This, D3DVALUE* d)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    OpenAL3DBuffer* b = (OpenAL3DBuffer*)This;
    if (!d) return DSERR_INVALIDPARAM;
    *d = 1000000000.0f;
    if (b->parent && b->parent->alSource) alGetSourcef(b->parent->alSource, AL_MAX_DISTANCE, d);
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DB_GetMinDistance(IDirectSound3DBuffer* This, D3DVALUE* d)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    OpenAL3DBuffer* b = (OpenAL3DBuffer*)This;
    if (!d) return DSERR_INVALIDPARAM;
    *d = 1.0f;
    if (b->parent && b->parent->alSource) alGetSourcef(b->parent->alSource, AL_REFERENCE_DISTANCE, d);
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DB_GetMode(IDirectSound3DBuffer* This, LPDWORD m)
{
    OpenAL3DBuffer* b = (OpenAL3DBuffer*)This;
    if (!m) return DSERR_INVALIDPARAM;
    *m = (b->parent && b->parent->is3dEnabled) ? DS3DMODE_NORMAL : DS3DMODE_DISABLE;
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DB_GetPosition(IDirectSound3DBuffer* This, D3DVECTOR* p)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    OpenAL3DBuffer* b = (OpenAL3DBuffer*)This;
    if (!p) return DSERR_INVALIDPARAM;
    ALfloat v[3] = {0,0,0};
    if (b->parent && b->parent->alSource) alGetSourcefv(b->parent->alSource, AL_POSITION, v);
    *p = D3DVECTOR(v[0], v[1], -v[2]);
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DB_GetVelocity(IDirectSound3DBuffer* This, D3DVECTOR* v)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    OpenAL3DBuffer* b = (OpenAL3DBuffer*)This;
    if (!v) return DSERR_INVALIDPARAM;
    ALfloat a[3] = {0,0,0};
    if (b->parent && b->parent->alSource) alGetSourcefv(b->parent->alSource, AL_VELOCITY, a);
    *v = D3DVECTOR(a[0], a[1], -a[2]);
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DB_SetAllParameters(IDirectSound3DBuffer* This, LPCDS3DBUFFER p, DWORD apply)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    OpenAL3DBuffer* b = (OpenAL3DBuffer*)This;
    (void)apply;
    if (!p || !b->parent) return DSERR_INVALIDPARAM;
    b->parent->EnsureSource();
    ALuint src = b->parent->alSource;
    if (src) {
        alSource3f(src, AL_POSITION, p->vPosition.x, p->vPosition.y, -p->vPosition.z);
        alSource3f(src, AL_VELOCITY, p->vVelocity.x, p->vVelocity.y, -p->vVelocity.z);
        alSourcef(src, AL_REFERENCE_DISTANCE, p->flMinDistance);
        alSourcef(src, AL_MAX_DISTANCE, p->flMaxDistance);
    }
    b->parent->is3dEnabled = (p->dwMode != DS3DMODE_DISABLE);
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DB_SetConeAngles(IDirectSound3DBuffer* This, DWORD in, DWORD out, DWORD apply)
{
    (void)This; (void)in; (void)out; (void)apply; return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DB_SetConeOrientation(IDirectSound3DBuffer* This, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD apply)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    OpenAL3DBuffer* b = (OpenAL3DBuffer*)This;
    (void)apply;
    if (b->parent && b->parent->alSource) alSource3f(b->parent->alSource, AL_DIRECTION, x, y, -z);
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DB_SetConeOutsideVolume(IDirectSound3DBuffer* This, LONG v, DWORD apply)
{
    (void)This; (void)v; (void)apply; return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DB_SetMaxDistance(IDirectSound3DBuffer* This, D3DVALUE d, DWORD apply)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    OpenAL3DBuffer* b = (OpenAL3DBuffer*)This;
    (void)apply;
    b->parent->EnsureSource();
    if (b->parent && b->parent->alSource) alSourcef(b->parent->alSource, AL_MAX_DISTANCE, d);
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DB_SetMinDistance(IDirectSound3DBuffer* This, D3DVALUE d, DWORD apply)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    OpenAL3DBuffer* b = (OpenAL3DBuffer*)This;
    (void)apply;
    b->parent->EnsureSource();
    if (b->parent && b->parent->alSource) alSourcef(b->parent->alSource, AL_REFERENCE_DISTANCE, d);
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DB_SetMode(IDirectSound3DBuffer* This, DWORD mode, DWORD apply)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    OpenAL3DBuffer* b = (OpenAL3DBuffer*)This;
    (void)apply;
    if (!b->parent) return DSERR_INVALIDPARAM;
    b->parent->EnsureSource();
    ALuint src = b->parent->alSource;
    if (mode == DS3DMODE_DISABLE) {
        b->parent->is3dEnabled = false;
        if (src) {
            alSourcei(src, AL_SOURCE_RELATIVE, AL_TRUE);
            alSource3f(src, AL_POSITION, 0.0f, 0.0f, 0.0f);
        }
    } else {
        b->parent->is3dEnabled = true;
        if (src) alSourcei(src, AL_SOURCE_RELATIVE,
                           (mode == DS3DMODE_HEADRELATIVE) ? AL_TRUE : AL_FALSE);
    }
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DB_SetPosition(IDirectSound3DBuffer* This, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD apply)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    OpenAL3DBuffer* b = (OpenAL3DBuffer*)This;
    (void)apply;
    b->parent->EnsureSource();
    if (b->parent && b->parent->alSource) alSource3f(b->parent->alSource, AL_POSITION, x, y, -z);
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS3DB_SetVelocity(IDirectSound3DBuffer* This, D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD apply)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    OpenAL3DBuffer* b = (OpenAL3DBuffer*)This;
    (void)apply;
    b->parent->EnsureSource();
    if (b->parent && b->parent->alSource) alSource3f(b->parent->alSource, AL_VELOCITY, x, y, -z);
    return DS_OK;
}

const IDirectSound3DBufferVtbl g_DS3DBVtbl =
{
    DS3DB_QueryInterface,
    DS3DB_AddRef,
    DS3DB_Release,
    DS3DB_GetAllParameters,
    DS3DB_GetConeAngles,
    DS3DB_GetConeOrientation,
    DS3DB_GetConeOutsideVolume,
    DS3DB_GetMaxDistance,
    DS3DB_GetMinDistance,
    DS3DB_GetMode,
    DS3DB_GetPosition,
    DS3DB_GetVelocity,
    DS3DB_SetAllParameters,
    DS3DB_SetConeAngles,
    DS3DB_SetConeOrientation,
    DS3DB_SetConeOutsideVolume,
    DS3DB_SetMaxDistance,
    DS3DB_SetMinDistance,
    DS3DB_SetMode,
    DS3DB_SetPosition,
    DS3DB_SetVelocity,
};

/* ============================================================
 * IDirectSoundBuffer implementation
 * ============================================================ */
static HRESULT STDMETHODCALLTYPE DSB_QueryInterface(IDirectSoundBuffer* This, REFIID riid, void** ppv)
{
    OpenALSoundBuffer* b = (OpenALSoundBuffer*)This;
    if (!ppv) return E_POINTER;
    *ppv = nullptr;

    if (IsEqualIID(riid, IID_IDirectSound3DListener)) {
        /* The primary buffer exposes the listener. */
        if (!b->isPrimary) return E_NOINTERFACE;
        if (!b->listener) b->listener = new OpenAL3DListener();
        b->listener->AddRef();
        *ppv = (IDirectSound3DListener*)b->listener;
        return DS_OK;
    }
    if (IsEqualIID(riid, IID_IDirectSound3DBuffer)) {
        if (b->isPrimary) return E_NOINTERFACE;
        b->EnsureSource();
        if (!b->threeD) b->threeD = new OpenAL3DBuffer(b);
        b->threeD->AddRef();
        *ppv = (IDirectSound3DBuffer*)b->threeD;
        return DS_OK;
    }
    if (IsEqualIID(riid, IID_IDirectSoundNotify)) {
        if (b->isPrimary) return E_NOINTERFACE;
        if (!b->notify) b->notify = new OpenALSoundNotify(b);
        b->notify->AddRef();
        *ppv = (IDirectSoundNotify*)b->notify;
        return DS_OK;
    }
    return E_NOINTERFACE;
}
static ULONG STDMETHODCALLTYPE DSB_AddRef(IDirectSoundBuffer* This)
{
    OpenALSoundBuffer* b = (OpenALSoundBuffer*)This;
    return (ULONG)++b->refCount;
}
static ULONG STDMETHODCALLTYPE DSB_Release(IDirectSoundBuffer* This)
{
    OpenALSoundBuffer* b = (OpenALSoundBuffer*)This;
    LONG rc = --b->refCount;
    if (rc <= 0) { std::lock_guard<std::recursive_mutex> lk(g_dsMutex); delete b; return 0; }
    return (ULONG)rc;
}
static HRESULT STDMETHODCALLTYPE DSB_GetCaps(IDirectSoundBuffer* This, LPDSBCAPS c)
{
    OpenALSoundBuffer* b = (OpenALSoundBuffer*)This;
    if (!c) return DSERR_INVALIDPARAM;
    DWORD sz = c->dwSize ? c->dwSize : (DWORD)sizeof(DSBCAPS);
    memset(c, 0, sz);
    c->dwSize = sz;
    c->dwFlags = b->dwFlags;
    c->dwBufferBytes = b->bufferBytes;
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DSB_GetCurrentPosition(IDirectSoundBuffer* This, LPDWORD play, LPDWORD write)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    OpenALSoundBuffer* b = (OpenALSoundBuffer*)This;
    DWORD pos = 0;
    if (b->alSource) {
        ALint sampleOff = 0;
        alGetSourcei(b->alSource, AL_SAMPLE_OFFSET, &sampleOff);
        WORD blockAlign = b->format.nBlockAlign ? b->format.nBlockAlign : 1;
        pos = (DWORD)sampleOff * blockAlign;
        if (b->bufferBytes) pos %= b->bufferBytes;
    }
    if (play)  *play  = pos;
    if (write) *write = pos;   /* approximate write cursor */
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DSB_GetFormat(IDirectSoundBuffer* This, LPWAVEFORMATEX f, DWORD sizeAlloc, LPDWORD written)
{
    OpenALSoundBuffer* b = (OpenALSoundBuffer*)This;
    DWORD need = (DWORD)sizeof(WAVEFORMATEX);
    if (f) {
        DWORD copy = (sizeAlloc < need) ? sizeAlloc : need;
        memcpy(f, &b->format, copy);
    }
    if (written) *written = need;
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DSB_GetVolume(IDirectSoundBuffer* This, LPLONG v)
{
    OpenALSoundBuffer* b = (OpenALSoundBuffer*)This;
    if (!v) return DSERR_INVALIDPARAM;
    *v = b->volume;
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DSB_GetPan(IDirectSoundBuffer* This, LPLONG p)
{
    OpenALSoundBuffer* b = (OpenALSoundBuffer*)This;
    if (!p) return DSERR_INVALIDPARAM;
    *p = b->pan;
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DSB_GetFrequency(IDirectSoundBuffer* This, LPDWORD f)
{
    OpenALSoundBuffer* b = (OpenALSoundBuffer*)This;
    if (!f) return DSERR_INVALIDPARAM;
    *f = b->frequency ? b->frequency : b->format.nSamplesPerSec;
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DSB_GetStatus(IDirectSoundBuffer* This, LPDWORD s)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    OpenALSoundBuffer* b = (OpenALSoundBuffer*)This;
    if (!s) return DSERR_INVALIDPARAM;
    DWORD status = 0;
    if (b->alSource) {
        ALint state = AL_STOPPED;
        alGetSourcei(b->alSource, AL_SOURCE_STATE, &state);
        if (state == AL_PLAYING) {
            status |= DSBSTATUS_PLAYING;
            if (b->looping) status |= DSBSTATUS_LOOPING;
        }
    }
    *s = status;
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DSB_Initialize(IDirectSoundBuffer* This, LPDIRECTSOUND ds, LPCDSBUFFERDESC d)
{
    (void)This; (void)ds; (void)d;
    return DSERR_ALREADYINITIALIZED;
}
static HRESULT STDMETHODCALLTYPE DSB_Lock(IDirectSoundBuffer* This, DWORD offset, DWORD bytes,
                                          LPVOID* p1, LPDWORD b1, LPVOID* p2, LPDWORD b2, DWORD flags)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    OpenALSoundBuffer* b = (OpenALSoundBuffer*)This;
    if (!b->pcm || b->bufferBytes == 0) return DSERR_INVALIDPARAM;

    if (flags & DSBLOCK_ENTIREBUFFER) { offset = 0; bytes = b->bufferBytes; }
    if (offset > b->bufferBytes) offset %= b->bufferBytes;
    if (bytes == 0) bytes = b->bufferBytes;

    DWORD region1 = bytes;
    DWORD region2 = 0;
    if (offset + bytes > b->bufferBytes) {
        region1 = b->bufferBytes - offset;
        region2 = bytes - region1;     /* wrap around to start */
    }

    if (p1) *p1 = b->pcm + offset;
    if (b1) *b1 = region1;
    if (p2) *p2 = region2 ? b->pcm : nullptr;
    if (b2) *b2 = region2;
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DSB_Play(IDirectSoundBuffer* This, DWORD r1, DWORD pri, DWORD flags)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    OpenALSoundBuffer* b = (OpenALSoundBuffer*)This;
    (void)r1; (void)pri;
    if (b->isPrimary) return DS_OK;   /* primary "play" is a no-op */
    b->EnsureSource();
    if (!b->alSource) return DSERR_GENERIC;

    b->looping = (flags & DSBPLAY_LOOPING) != 0;
    alSourcei(b->alSource, AL_LOOPING, b->looping ? AL_TRUE : AL_FALSE);
    alSourcePlay(b->alSource);
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DSB_SetCurrentPosition(IDirectSoundBuffer* This, DWORD pos)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    OpenALSoundBuffer* b = (OpenALSoundBuffer*)This;
    if (b->alSource) {
        WORD blockAlign = b->format.nBlockAlign ? b->format.nBlockAlign : 1;
        ALint sampleOff = (ALint)(pos / blockAlign);
        alSourcei(b->alSource, AL_SAMPLE_OFFSET, sampleOff);
    }
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DSB_SetFormat(IDirectSoundBuffer* This, LPCWAVEFORMATEX f)
{
    OpenALSoundBuffer* b = (OpenALSoundBuffer*)This;
    if (!f) return DSERR_INVALIDPARAM;
    memcpy(&b->format, f, sizeof(WAVEFORMATEX));
    if (b->frequency == 0) b->frequency = f->nSamplesPerSec;
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DSB_SetVolume(IDirectSoundBuffer* This, LONG v)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    OpenALSoundBuffer* b = (OpenALSoundBuffer*)This;
    if (v < DSBVOLUME_MIN) v = DSBVOLUME_MIN;
    if (v > DSBVOLUME_MAX) v = DSBVOLUME_MAX;
    b->volume = v;
    if (b->alSource) alSourcef(b->alSource, AL_GAIN, DSVolumeToGain(v));
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DSB_SetPan(IDirectSoundBuffer* This, LONG p)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    OpenALSoundBuffer* b = (OpenALSoundBuffer*)This;
    if (p < DSBPAN_LEFT)  p = DSBPAN_LEFT;
    if (p > DSBPAN_RIGHT) p = DSBPAN_RIGHT;
    b->pan = p;
    if (b->alSource && !b->is3dEnabled) {
        /* 2D pan: place the source left/right relative to the listener. */
        alSourcei(b->alSource, AL_SOURCE_RELATIVE, AL_TRUE);
        alSource3f(b->alSource, AL_POSITION, (float)p / 10000.0f, 0.0f, 0.0f);
    }
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DSB_SetFrequency(IDirectSoundBuffer* This, DWORD f)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    OpenALSoundBuffer* b = (OpenALSoundBuffer*)This;
    DWORD original = b->format.nSamplesPerSec ? b->format.nSamplesPerSec : 44100;
    if (f == DSBFREQUENCY_ORIGINAL) f = original;
    b->frequency = f;
    if (b->alSource) {
        float pitch = (float)f / (float)original;
        if (pitch <= 0.0f) pitch = 1.0f;
        alSourcef(b->alSource, AL_PITCH, pitch);
    }
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DSB_Stop(IDirectSoundBuffer* This)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    OpenALSoundBuffer* b = (OpenALSoundBuffer*)This;
    if (b->alSource) alSourceStop(b->alSource);
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DSB_Unlock(IDirectSoundBuffer* This, LPVOID p1, DWORD b1, LPVOID p2, DWORD b2)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    OpenALSoundBuffer* b = (OpenALSoundBuffer*)This;
    (void)p1; (void)p2;
    if (!b->pcm || b->bufferBytes == 0) return DS_OK;
    (void)b1; (void)b2;

    /* Re-upload the whole PCM store to the AL buffer.
       FF_LINUX: do NOT create a source here - sources are a scarce resource
       (OpenAL Soft defaults to 256) and every loaded sample passes through
       Unlock. Creating one per sample exhausted the pool, so in-flight
       voices could no longer get sources (AL_OUT_OF_MEMORY). The source is
       created lazily in Play via EnsureSource. */
    if (!b->alBuffer) alGenBuffers(1, &b->alBuffer);

    ALenum fmt = ALFormatFor(b->format.nChannels, b->format.wBitsPerSample);
    ALsizei freq = (ALsizei)(b->format.nSamplesPerSec ? b->format.nSamplesPerSec : 44100);

    /* FF_LINUX: preserve play state across the re-upload. DirectSound
       streaming buffers are Play()ed once (looping) and then continuously
       written via Lock/Unlock; stopping without resuming here silenced
       every streaming sound after its first Unlock. */
    ALint wasState = 0, sampleOff = 0;
    if (b->alSource) {
        alGetSourcei(b->alSource, AL_SOURCE_STATE, &wasState);
        alGetSourcei(b->alSource, AL_SAMPLE_OFFSET, &sampleOff);
        alSourceStop(b->alSource);
        alSourcei(b->alSource, AL_BUFFER, 0);   /* detach before re-filling */
    }
    alBufferData(b->alBuffer, fmt, b->pcm, (ALsizei)b->bufferBytes, freq);
    if (b->alSource) {
        alSourcei(b->alSource, AL_BUFFER, (ALint)b->alBuffer);
        if (wasState == AL_PLAYING) {
            ALint maxOff = (ALint)(b->bufferBytes / (b->format.nBlockAlign ? b->format.nBlockAlign : 1));
            if (sampleOff >= 0 && sampleOff < maxOff)
                alSourcei(b->alSource, AL_SAMPLE_OFFSET, sampleOff);
            alSourcePlay(b->alSource);
        }
    }
    b->alUploaded = true;
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DSB_Restore(IDirectSoundBuffer* This)
{
    (void)This; return DS_OK;
}

const IDirectSoundBufferVtbl g_DSBVtbl =
{
    DSB_QueryInterface,
    DSB_AddRef,
    DSB_Release,
    DSB_GetCaps,
    DSB_GetCurrentPosition,
    DSB_GetFormat,
    DSB_GetVolume,
    DSB_GetPan,
    DSB_GetFrequency,
    DSB_GetStatus,
    DSB_Initialize,
    DSB_Lock,
    DSB_Play,
    DSB_SetCurrentPosition,
    DSB_SetFormat,
    DSB_SetVolume,
    DSB_SetPan,
    DSB_SetFrequency,
    DSB_Stop,
    DSB_Unlock,
    DSB_Restore,
};

/* ============================================================
 * IDirectSound implementation
 * ============================================================ */
static HRESULT STDMETHODCALLTYPE DS_QueryInterface(IDirectSound* This, REFIID riid, void** ppv)
{
    (void)riid;
    if (!ppv) return E_POINTER;
    /* Only the IDirectSound identity is exposed. */
    This->AddRef();
    *ppv = This;
    return DS_OK;
}
static ULONG STDMETHODCALLTYPE DS_AddRef(IDirectSound* This)
{
    OpenALDirectSound* d = (OpenALDirectSound*)This;
    return (ULONG)++d->refCount;
}
static ULONG STDMETHODCALLTYPE DS_Release(IDirectSound* This)
{
    OpenALDirectSound* d = (OpenALDirectSound*)This;
    LONG rc = --d->refCount;
    if (rc <= 0) { delete d; return 0; }
    return (ULONG)rc;
}
static HRESULT STDMETHODCALLTYPE DS_CreateSoundBuffer(IDirectSound* This, LPCDSBUFFERDESC desc,
                                                      LPDIRECTSOUNDBUFFER* out, IUnknown* outer)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    (void)This; (void)outer;
    if (!desc || !out) return DSERR_INVALIDPARAM;
    *out = nullptr;

    OpenALSoundBuffer* b = new OpenALSoundBuffer();
    b->dwFlags = desc->dwFlags;

    if (desc->dwFlags & DSBCAPS_PRIMARYBUFFER) {
        b->isPrimary = true;
        if (desc->lpwfxFormat) memcpy(&b->format, desc->lpwfxFormat, sizeof(WAVEFORMATEX));
    } else {
        if (!desc->lpwfxFormat || desc->dwBufferBytes == 0) {
            delete b;
            return DSERR_INVALIDPARAM;
        }
        memcpy(&b->format, desc->lpwfxFormat, sizeof(WAVEFORMATEX));
        b->frequency   = desc->lpwfxFormat->nSamplesPerSec;
        b->bufferBytes = desc->dwBufferBytes;
        b->pcm = (unsigned char*)malloc(b->bufferBytes);
        if (!b->pcm) { delete b; return DSERR_OUTOFMEMORY; }
        memset(b->pcm, 0, b->bufferBytes);
        /* FF_LINUX: no EnsureSource here - every loaded sample creates a
           buffer, and AL sources are limited (~256). Created lazily at Play. */
        if (b->dwFlags & DSBCAPS_CTRL3D) b->is3dEnabled = true;
    }

    *out = (IDirectSoundBuffer*)b;
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS_GetCaps(IDirectSound* This, LPDSCAPS c)
{
    (void)This;
    if (!c) return DSERR_INVALIDPARAM;
    DWORD sz = c->dwSize ? c->dwSize : (DWORD)sizeof(DSCAPS);
    memset(c, 0, sz);
    c->dwSize = sz;
    c->dwFlags = DSCAPS_PRIMARYSTEREO | DSCAPS_PRIMARY16BIT |
                 DSCAPS_SECONDARYSTEREO | DSCAPS_SECONDARY16BIT |
                 DSCAPS_SECONDARYMONO | DSCAPS_CONTINUOUSRATE | DSCAPS_CERTIFIED;
    c->dwMinSecondarySampleRate = DSBFREQUENCY_MIN;
    c->dwMaxSecondarySampleRate = DSBFREQUENCY_MAX;
    c->dwPrimaryBuffers = 1;
    c->dwMaxHwMixingAllBuffers = 64;
    c->dwMaxHw3DAllBuffers = 64;
    c->dwFreeHwMixingAllBuffers = 64;
    c->dwFreeHw3DAllBuffers = 64;
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS_DuplicateSoundBuffer(IDirectSound* This, LPDIRECTSOUNDBUFFER orig,
                                                         LPDIRECTSOUNDBUFFER* dup)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    (void)This;
    if (!orig || !dup) return DSERR_INVALIDPARAM;
    *dup = nullptr;

    OpenALSoundBuffer* src = (OpenALSoundBuffer*)orig;
    if (src->isPrimary) return DSERR_INVALIDCALL;

    OpenALSoundBuffer* nb = new OpenALSoundBuffer();
    nb->dwFlags     = src->dwFlags;
    nb->format      = src->format;
    nb->frequency   = src->frequency;
    nb->bufferBytes = src->bufferBytes;
    nb->volume      = src->volume;
    nb->pan         = src->pan;
    nb->is3dEnabled = src->is3dEnabled;

    if (src->bufferBytes && src->pcm) {
        nb->pcm = (unsigned char*)malloc(src->bufferBytes);
        if (!nb->pcm) { delete nb; return DSERR_OUTOFMEMORY; }
        memcpy(nb->pcm, src->pcm, src->bufferBytes);
    }
    /* Upload its own copy of the PCM data so it can play independently.
       The source is created lazily at Play; EnsureSource attaches alBuffer. */
    if (nb->pcm && nb->bufferBytes) {
        alGenBuffers(1, &nb->alBuffer);
        ALenum fmt = ALFormatFor(nb->format.nChannels, nb->format.wBitsPerSample);
        ALsizei freq = (ALsizei)(nb->format.nSamplesPerSec ? nb->format.nSamplesPerSec : 44100);
        alBufferData(nb->alBuffer, fmt, nb->pcm, (ALsizei)nb->bufferBytes, freq);
        nb->alUploaded = true;
    }

    *dup = (IDirectSoundBuffer*)nb;
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS_SetCooperativeLevel(IDirectSound* This, HWND h, DWORD level)
{
    (void)This; (void)h; (void)level; return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS_Compact(IDirectSound* This)
{
    (void)This; return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS_GetSpeakerConfig(IDirectSound* This, LPDWORD cfg)
{
    (void)This;
    if (!cfg) return DSERR_INVALIDPARAM;
    *cfg = DSSPEAKER_COMBINED(DSSPEAKER_STEREO, DSSPEAKER_GEOMETRY_WIDE);
    return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS_SetSpeakerConfig(IDirectSound* This, DWORD cfg)
{
    (void)This; (void)cfg; return DS_OK;
}
static HRESULT STDMETHODCALLTYPE DS_Initialize(IDirectSound* This, const GUID* dev)
{
    (void)This; (void)dev; return DS_OK;
}

const IDirectSoundVtbl g_DSVtbl =
{
    DS_QueryInterface,
    DS_AddRef,
    DS_Release,
    DS_CreateSoundBuffer,
    DS_GetCaps,
    DS_DuplicateSoundBuffer,
    DS_SetCooperativeLevel,
    DS_Compact,
    DS_GetSpeakerConfig,
    DS_SetSpeakerConfig,
    DS_Initialize,
};

/* ============================================================
 * Entry points
 * ============================================================ */
extern "C" HRESULT DirectSoundCreate(GUID* pcGuidDevice, LPDIRECTSOUND* ppDS, IUnknown* pUnkOuter)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    (void)pcGuidDevice; (void)pUnkOuter;
    if (!ppDS) return DSERR_INVALIDPARAM;
    *ppDS = nullptr;

    if (!g_alcDevice) {
        g_alcDevice = alcOpenDevice(nullptr);   /* default device */
        if (!g_alcDevice) {
            fprintf(stderr, "[OpenAL] alcOpenDevice failed\n");
            return DSERR_NODRIVER;
        }
        g_alcContext = alcCreateContext(g_alcDevice, nullptr);
        if (!g_alcContext) {
            fprintf(stderr, "[OpenAL] alcCreateContext failed\n");
            alcCloseDevice(g_alcDevice);
            g_alcDevice = nullptr;
            return DSERR_NODRIVER;
        }
        alcMakeContextCurrent(g_alcContext);
        alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
    }

    OpenALDirectSound* ds = new OpenALDirectSound();
    *ppDS = (IDirectSound*)ds;
    return DS_OK;
}

extern "C" HRESULT DirectSoundEnumerateA(LPDSENUMCALLBACKA cb, void* ctx)
{
    if (cb) {
        /* Report a single default device. */
        cb(nullptr, "Primary Sound Driver", "", ctx);
    }
    return DS_OK;
}

extern "C" void FF_OpenALShutdown(void)
{
    std::lock_guard<std::recursive_mutex> lk(g_dsMutex);
    if (g_alcContext) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(g_alcContext);
        g_alcContext = nullptr;
    }
    if (g_alcDevice) {
        alcCloseDevice(g_alcDevice);
        g_alcDevice = nullptr;
    }
}

#endif /* FF_LINUX */

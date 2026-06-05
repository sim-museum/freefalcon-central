/*
 * FreeFalcon Linux Port - OpenAL-backed DirectSound implementation (declarations)
 *
 * Declares the concrete classes that implement the DirectSound COM interfaces
 * (from dsound.h) on top of OpenAL.  The full implementation lives in
 * openal_dsound.cpp.  This header is intentionally minimal: only what the .cpp
 * (and any future consumer) needs.
 */

#ifndef FF_COMPAT_OPENAL_DSOUND_H
#define FF_COMPAT_OPENAL_DSOUND_H

#ifdef FF_LINUX

#include "compat_types.h"
#include "objbase.h"
#include "dsound.h"

/* Concrete implementation classes (defined in openal_dsound.cpp).
 * Each derives from the matching DirectSound interface and installs a
 * static vtable in its constructor (mirrors the D3D7 pattern in d3d_gl.cpp). */
struct OpenALDirectSound;
struct OpenALSoundBuffer;
struct OpenAL3DBuffer;
struct OpenAL3DListener;
struct OpenALSoundNotify;

/* Process-wide teardown helper (closes the ALC context/device). */
extern "C" void FF_OpenALShutdown(void);

#endif /* FF_LINUX */
#endif /* FF_COMPAT_OPENAL_DSOUND_H */

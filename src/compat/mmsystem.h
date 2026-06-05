/* FreeFalcon Linux Port - mmsystem.h compatibility */
#ifndef FF_COMPAT_MMSYSTEM_H
#define FF_COMPAT_MMSYSTEM_H
#ifdef FF_LINUX

#include "compat_types.h"
#include "compat_winbase.h"  /* timeGetTime / timeBeginPeriod / timeEndPeriod */

typedef UINT MMRESULT;
#define MMSYSERR_NOERROR  0
#define MMSYSERR_ERROR    1
#define TIMERR_NOERROR    0

#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_
#pragma pack(push, 1)
typedef struct tWAVEFORMATEX {
    WORD  wFormatTag;
    WORD  nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD  nBlockAlign;
    WORD  wBitsPerSample;
    WORD  cbSize;
} WAVEFORMATEX, *PWAVEFORMATEX, *LPWAVEFORMATEX;
typedef const WAVEFORMATEX *LPCWAVEFORMATEX;
#pragma pack(pop)
#endif

#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM 1
#endif

#ifndef _PCMWAVEFORMAT_
#define _PCMWAVEFORMAT_
#pragma pack(push, 1)
typedef struct waveformat_tag {
    WORD  wFormatTag;
    WORD  nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD  nBlockAlign;
} WAVEFORMAT, *PWAVEFORMAT, *LPWAVEFORMAT;

typedef struct pcmwaveformat_tag {
    WAVEFORMAT wf;
    WORD       wBitsPerSample;
} PCMWAVEFORMAT, *PPCMWAVEFORMAT, *LPPCMWAVEFORMAT;
#pragma pack(pop)
#endif /* _PCMWAVEFORMAT_ */

typedef struct timecaps_tag {
    UINT wPeriodMin;
    UINT wPeriodMax;
} TIMECAPS, *PTIMECAPS, *LPTIMECAPS;

static inline MMRESULT timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc) {
    (void)cbtc;
    if (ptc) { ptc->wPeriodMin = 1; ptc->wPeriodMax = 1000000; }
    return MMSYSERR_NOERROR;
}

/* PlaySound stubs */
#define SND_SYNC      0x0000
#define SND_ASYNC     0x0001
#define SND_NODEFAULT 0x0002
#define SND_LOOP      0x0008
#define SND_PURGE     0x0040
#define SND_FILENAME  0x00020000
static inline BOOL PlaySoundA(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound) {
    (void)pszSound; (void)hmod; (void)fdwSound;
    return TRUE;
}
#define PlaySound PlaySoundA
static inline BOOL sndPlaySoundA(LPCSTR pszSound, UINT fuSound) { (void)pszSound; (void)fuSound; return TRUE; }
#define sndPlaySound sndPlaySoundA

#endif /* FF_LINUX */
#endif

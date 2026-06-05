/* FreeFalcon Linux Port - mmreg.h compatibility */
#ifndef FF_COMPAT_MMREG_H
#define FF_COMPAT_MMREG_H
#ifdef FF_LINUX
#include "mmsystem.h"
#define WAVE_FORMAT_ADPCM      2
#define WAVE_FORMAT_IEEE_FLOAT 3
#define WAVE_FORMAT_IMA_ADPCM  0x0011
#endif
#endif

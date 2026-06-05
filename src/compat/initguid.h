/* FreeFalcon Linux Port - initguid.h: make DEFINE_GUID instantiate GUIDs */
#ifndef FF_COMPAT_INITGUID_H
#define FF_COMPAT_INITGUID_H
#include "objbase.h"
#undef DEFINE_GUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    const GUID name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }
#endif

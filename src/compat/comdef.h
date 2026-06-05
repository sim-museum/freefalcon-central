/* FreeFalcon Linux Port - comdef.h stub */
#ifndef FF_COMPAT_COMDEF_H
#define FF_COMPAT_COMDEF_H
#ifdef FF_LINUX
#include "objbase.h"

class _com_error {
public:
    HRESULT m_hr;
    _com_error(HRESULT hr, void *pEI = NULL) : m_hr(hr) { (void)pEI; }
    HRESULT Error() const { return m_hr; }
    const char *ErrorMessage() const { return "COM error"; }
};

/* Minimal _bstr_t: char-string wrapper (ANSI build) */
class _bstr_t {
public:
    char buf[512];
    _bstr_t() { buf[0] = '\0'; }
    _bstr_t(const char *s) {
        if (s) { strncpy(buf, s, sizeof(buf) - 1); buf[sizeof(buf) - 1] = '\0'; }
        else buf[0] = '\0';
    }
    operator LPSTR() { return buf; }
    operator LPCSTR() const { return buf; }
    int length() const { return (int)strlen(buf); }
};

static inline HRESULT GetErrorInfo(DWORD r, void **ppEI) {
    (void)r;
    if (ppEI) *ppEI = NULL;
    return S_FALSE;
}
#endif
#endif

/* FreeFalcon Linux Port - atlbase.h stub */
#ifndef FF_COMPAT_ATLBASE_H
#define FF_COMPAT_ATLBASE_H
#ifdef FF_LINUX
#include "objbase.h"

struct _ATL_OBJMAP_ENTRY {
    const GUID *pclsid;
};

#define BEGIN_OBJECT_MAP(x) static _ATL_OBJMAP_ENTRY x[] = {
#define END_OBJECT_MAP()    { NULL } };
#define OBJECT_ENTRY(clsid, cls) { &clsid },

class CComModule {
public:
    HRESULT Init(_ATL_OBJMAP_ENTRY *p, HINSTANCE h) { (void)p; (void)h; return S_OK; }
    HRESULT Init(void *p, HINSTANCE h) { (void)p; (void)h; return S_OK; }
    void Term() {}
};

#define ATLASSERT(x) ((void)0)

template <class T>
class CComPtr {
public:
    T *p;
    CComPtr() : p(NULL) {}
    CComPtr(T *lp) : p(lp) { if (p) p->AddRef(); }
    ~CComPtr() { if (p) p->Release(); }
    operator T *() const { return p; }
    T *operator->() const { return p; }
    T **operator&() { return &p; }
    CComPtr &operator=(T *lp) {
        if (lp) lp->AddRef();
        if (p) p->Release();
        p = lp;
        return *this;
    }
    void Release() { if (p) { p->Release(); p = NULL; } }
};
#endif
#endif

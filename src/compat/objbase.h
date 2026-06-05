/*
 * FreeFalcon Linux Port - objbase.h compatibility (COM basics)
 */

#ifndef FF_COMPAT_OBJBASE_H
#define FF_COMPAT_OBJBASE_H

#ifdef FF_LINUX

#include "compat_types.h"

/* ============================================================
 * Calling conventions / method macros
 * ============================================================ */
#ifndef STDMETHODCALLTYPE
#define STDMETHODCALLTYPE
#endif
#ifndef STDAPICALLTYPE
#define STDAPICALLTYPE
#endif
#ifndef STDMETHODIMP
#define STDMETHODIMP HRESULT STDMETHODCALLTYPE
#endif
#ifndef STDMETHODIMP_
#define STDMETHODIMP_(type) type STDMETHODCALLTYPE
#endif
#ifndef STDMETHOD
#define STDMETHOD(method)        virtual HRESULT STDMETHODCALLTYPE method
#define STDMETHOD_(type, method) virtual type STDMETHODCALLTYPE method
#endif
#ifndef PURE
#define PURE = 0
#endif
#ifndef THIS_
#define THIS_
#endif
#ifndef THIS
#define THIS void
#endif

/* ============================================================
 * HRESULT helpers
 * ============================================================ */
#ifndef S_OK
#define S_OK            ((HRESULT)0)
#endif
#ifndef S_FALSE
#define S_FALSE         ((HRESULT)1)
#endif
#ifndef E_FAIL
#define E_FAIL          ((HRESULT)0x80004005)
#endif
#ifndef E_NOTIMPL
#define E_NOTIMPL       ((HRESULT)0x80004001)
#endif
#ifndef E_NOINTERFACE
#define E_NOINTERFACE   ((HRESULT)0x80004002)
#endif
#ifndef E_POINTER
#define E_POINTER       ((HRESULT)0x80004003)
#endif
#ifndef E_ABORT
#define E_ABORT         ((HRESULT)0x80004004)
#endif
#ifndef E_OUTOFMEMORY
#define E_OUTOFMEMORY   ((HRESULT)0x8007000E)
#endif
#ifndef E_INVALIDARG
#define E_INVALIDARG    ((HRESULT)0x80070057)
#endif
#ifndef E_UNEXPECTED
#define E_UNEXPECTED    ((HRESULT)0x8000FFFF)
#endif
#ifndef E_HANDLE
#define E_HANDLE        ((HRESULT)0x80070006)
#endif
#ifndef E_ACCESSDENIED
#define E_ACCESSDENIED  ((HRESULT)0x80070005)
#endif
#ifndef E_PENDING
#define E_PENDING       ((HRESULT)0x8000000A)
#endif

#ifndef SUCCEEDED
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#endif
#ifndef FAILED
#define FAILED(hr)    (((HRESULT)(hr)) < 0)
#endif

#define MAKE_HRESULT(sev, fac, code) \
    ((HRESULT)(((unsigned long)(sev) << 31) | ((unsigned long)(fac) << 16) | ((unsigned long)(code))))

/* ============================================================
 * GUID helpers
 * ============================================================ */
#ifdef __cplusplus
static inline int InlineIsEqualGUID(REFGUID g1, REFGUID g2) {
    return memcmp(&g1, &g2, sizeof(GUID)) == 0;
}
static inline int IsEqualGUID(REFGUID g1, REFGUID g2) {
    return InlineIsEqualGUID(g1, g2);
}
#define IsEqualIID(a, b)   IsEqualGUID(a, b)
#define IsEqualCLSID(a, b) IsEqualGUID(a, b)
static inline bool operator==(REFGUID a, REFGUID b) { return InlineIsEqualGUID(a, b) != 0; }
static inline bool operator!=(REFGUID a, REFGUID b) { return !InlineIsEqualGUID(a, b); }
#else
#define IsEqualGUID(a, b)  (memcmp((a), (b), sizeof(GUID)) == 0)
#define IsEqualIID(a, b)   IsEqualGUID(a, b)
#endif

/* DEFINE_GUID: with INITGUID defined, instantiates; otherwise declares */
#ifdef INITGUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    const GUID name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }
#else
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    extern const GUID name
#endif

#define GUID_NULL_INIT { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } }
#ifdef __cplusplus
static const GUID GUID_NULL = GUID_NULL_INIT;
#endif

/* ============================================================
 * IUnknown
 * ============================================================ */
#ifdef __cplusplus

struct IUnknown;
typedef struct IUnknownVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IUnknown *This, REFIID riid, void **ppvObject);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IUnknown *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IUnknown *This);
} IUnknownVtbl;

struct IUnknown {
    IUnknownVtbl *lpVtbl;
    HRESULT QueryInterface(REFIID riid, void **ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG   AddRef()  { return lpVtbl->AddRef(this); }
    ULONG   Release() { return lpVtbl->Release(this); }
};
typedef IUnknown *LPUNKNOWN;

#else
typedef struct IUnknown IUnknown;
typedef IUnknown *LPUNKNOWN;
#endif

typedef struct _COSERVERINFO {
    DWORD dwReserved1;
    LPWSTR pwszName;
    void *pAuthInfo;
    DWORD dwReserved2;
} COSERVERINFO;

typedef struct _MULTI_QI {
    const IID *pIID;
    IUnknown *pItf;
    HRESULT hr;
} MULTI_QI;

/* ============================================================
 * COM lifetime stubs
 * ============================================================ */
#define COINIT_APARTMENTTHREADED 0x2
#define COINIT_MULTITHREADED     0x0

static inline HRESULT CoInitialize(LPVOID pvReserved) { (void)pvReserved; return S_OK; }
static inline HRESULT CoInitializeEx(LPVOID pvReserved, DWORD dwCoInit) { (void)pvReserved; (void)dwCoInit; return S_OK; }
static inline void CoUninitialize(void) {}
static inline HRESULT CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv) {
    (void)rclsid; (void)pUnkOuter; (void)dwClsContext; (void)riid;
    if (ppv) *ppv = NULL;
    return E_NOINTERFACE;
}
static inline LPVOID CoTaskMemAlloc(SIZE_T cb) { return malloc(cb); }
static inline void CoTaskMemFree(LPVOID pv) { free(pv); }

#define CLSCTX_INPROC_SERVER 0x1
#define CLSCTX_ALL           0x17

#endif /* FF_LINUX */
#endif /* FF_COMPAT_OBJBASE_H */

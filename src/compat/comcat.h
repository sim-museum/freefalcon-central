/* FreeFalcon Linux Port - comcat.h stub (COM component categories) */
#ifndef FF_COMPAT_COMCAT_H
#define FF_COMPAT_COMCAT_H
#ifdef FF_LINUX
#include "objbase.h"

typedef GUID CATID;
typedef const CATID &REFCATID;

typedef struct tagCATEGORYINFO {
    CATID catid;
    DWORD lcid;
    WCHAR szDescription[128];
} CATEGORYINFO;

struct ICatRegister;
typedef struct ICatRegisterVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(ICatRegister *This, REFIID riid, void **ppv);
    ULONG   (STDMETHODCALLTYPE *AddRef)(ICatRegister *This);
    ULONG   (STDMETHODCALLTYPE *Release)(ICatRegister *This);
    HRESULT (STDMETHODCALLTYPE *RegisterCategories)(ICatRegister *This, ULONG cCategories, CATEGORYINFO *rgCategoryInfo);
    HRESULT (STDMETHODCALLTYPE *UnRegisterCategories)(ICatRegister *This, ULONG cCategories, CATID *rgcatid);
    HRESULT (STDMETHODCALLTYPE *RegisterClassImplCategories)(ICatRegister *This, REFCLSID rclsid, ULONG cCategories, CATID *rgcatid);
    HRESULT (STDMETHODCALLTYPE *UnRegisterClassImplCategories)(ICatRegister *This, REFCLSID rclsid, ULONG cCategories, CATID *rgcatid);
    HRESULT (STDMETHODCALLTYPE *RegisterClassReqCategories)(ICatRegister *This, REFCLSID rclsid, ULONG cCategories, CATID *rgcatid);
    HRESULT (STDMETHODCALLTYPE *UnRegisterClassReqCategories)(ICatRegister *This, REFCLSID rclsid, ULONG cCategories, CATID *rgcatid);
} ICatRegisterVtbl;

struct ICatRegister {
    ICatRegisterVtbl *lpVtbl;
#ifdef __cplusplus
    HRESULT QueryInterface(REFIID riid, void **ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG AddRef() { return lpVtbl->AddRef(this); }
    ULONG Release() { return lpVtbl->Release(this); }
    HRESULT RegisterCategories(ULONG a, CATEGORYINFO *b) { return lpVtbl->RegisterCategories(this, a, b); }
    HRESULT UnRegisterCategories(ULONG a, CATID *b) { return lpVtbl->UnRegisterCategories(this, a, b); }
    HRESULT RegisterClassImplCategories(REFCLSID a, ULONG b, CATID *c) { return lpVtbl->RegisterClassImplCategories(this, a, b, c); }
    HRESULT UnRegisterClassImplCategories(REFCLSID a, ULONG b, CATID *c) { return lpVtbl->UnRegisterClassImplCategories(this, a, b, c); }
    HRESULT RegisterClassReqCategories(REFCLSID a, ULONG b, CATID *c) { return lpVtbl->RegisterClassReqCategories(this, a, b, c); }
    HRESULT UnRegisterClassReqCategories(REFCLSID a, ULONG b, CATID *c) { return lpVtbl->UnRegisterClassReqCategories(this, a, b, c); }
#endif
};

#ifdef __cplusplus
static const GUID CLSID_StdComponentCategoriesMgr =
    { 0x0002E005, 0x0000, 0x0000, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
static const GUID IID_ICatRegister =
    { 0x0002E012, 0x0000, 0x0000, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
#endif

#endif /* FF_LINUX */
#endif

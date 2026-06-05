/*
 * FreeFalcon Linux Port - d3d.h compatibility
 *
 * Direct3D 7 interfaces (COM lpVtbl layout + C++ inline wrappers).
 * Implemented over OpenGL in src/compat/d3d_gl.cpp.
 */

#ifndef FF_COMPAT_D3D_H
#define FF_COMPAT_D3D_H

#ifdef FF_LINUX

#include "compat_types.h"
#include "objbase.h"
#include "ddraw.h"
#include "d3dtypes.h"

/* ============================================================
 * Forward decls / typedefs
 * ============================================================ */
struct IDirect3D7;
struct IDirect3DDevice7;
struct IDirect3DVertexBuffer7;

typedef struct IDirect3D7             *LPDIRECT3D7;
typedef struct IDirect3DDevice7       *LPDIRECT3DDEVICE7;
typedef struct IDirect3DVertexBuffer7 *LPDIRECT3DVERTEXBUFFER7;

/* ============================================================
 * Result codes
 * ============================================================ */
#define D3D_OK S_OK
#define MAKE_D3DHRESULT(code) MAKE_HRESULT(1, 0x876, code)

/* Generated: every D3DERR_* referenced in the tree, unique values */
#define D3DERR_BADMAJORVERSION                       MAKE_D3DHRESULT(4000)
#define D3DERR_BADMINORVERSION                       MAKE_D3DHRESULT(4001)
#define D3DERR_CONFLICTINGTEXTUREFILTER              MAKE_D3DHRESULT(4002)
#define D3DERR_CONFLICTINGTEXTUREPALETTE             MAKE_D3DHRESULT(4003)
#define D3DERR_EXECUTE_CLIPPED_FAILED                MAKE_D3DHRESULT(4004)
#define D3DERR_EXECUTE_CREATE_FAILED                 MAKE_D3DHRESULT(4005)
#define D3DERR_EXECUTE_DESTROY_FAILED                MAKE_D3DHRESULT(4006)
#define D3DERR_EXECUTE_FAILED                        MAKE_D3DHRESULT(4007)
#define D3DERR_EXECUTE_LOCKED                        MAKE_D3DHRESULT(4008)
#define D3DERR_EXECUTE_LOCK_FAILED                   MAKE_D3DHRESULT(4009)
#define D3DERR_EXECUTE_NOT_LOCKED                    MAKE_D3DHRESULT(4010)
#define D3DERR_EXECUTE_UNLOCK_FAILED                 MAKE_D3DHRESULT(4011)
#define D3DERR_INITFAILED                            MAKE_D3DHRESULT(4012)
#define D3DERR_INVALIDCURRENTVIEWPORT                MAKE_D3DHRESULT(4013)
#define D3DERR_INVALIDPRIMITIVETYPE                  MAKE_D3DHRESULT(4014)
#define D3DERR_INVALIDVERTEXFORMAT                   MAKE_D3DHRESULT(4015)
#define D3DERR_INVALIDVERTEXTYPE                     MAKE_D3DHRESULT(4016)
#define D3DERR_INVALID_DEVICE                        MAKE_D3DHRESULT(4017)
#define D3DERR_LIGHT_SET_FAILED                      MAKE_D3DHRESULT(4018)
#define D3DERR_MATERIAL_CREATE_FAILED                MAKE_D3DHRESULT(4019)
#define D3DERR_MATERIAL_DESTROY_FAILED               MAKE_D3DHRESULT(4020)
#define D3DERR_MATERIAL_GETDATA_FAILED               MAKE_D3DHRESULT(4021)
#define D3DERR_MATERIAL_SETDATA_FAILED               MAKE_D3DHRESULT(4022)
#define D3DERR_MATRIX_CREATE_FAILED                  MAKE_D3DHRESULT(4023)
#define D3DERR_MATRIX_DESTROY_FAILED                 MAKE_D3DHRESULT(4024)
#define D3DERR_MATRIX_GETDATA_FAILED                 MAKE_D3DHRESULT(4025)
#define D3DERR_MATRIX_SETDATA_FAILED                 MAKE_D3DHRESULT(4026)
#define D3DERR_NOTAVAILABLE                          MAKE_D3DHRESULT(4027)
#define D3DERR_SCENE_BEGIN_FAILED                    MAKE_D3DHRESULT(4028)
#define D3DERR_SCENE_END_FAILED                      MAKE_D3DHRESULT(4029)
#define D3DERR_SCENE_IN_SCENE                        MAKE_D3DHRESULT(4030)
#define D3DERR_SCENE_NOT_IN_SCENE                    MAKE_D3DHRESULT(4031)
#define D3DERR_SETVIEWPORTDATA_FAILED                MAKE_D3DHRESULT(4032)
#define D3DERR_STENCILBUFFER_NOTPRESENT              MAKE_D3DHRESULT(4033)
#define D3DERR_TEXTURE_CREATE_FAILED                 MAKE_D3DHRESULT(4034)
#define D3DERR_TEXTURE_DESTROY_FAILED                MAKE_D3DHRESULT(4035)
#define D3DERR_TEXTURE_GETSURF_FAILED                MAKE_D3DHRESULT(4036)
#define D3DERR_TEXTURE_LOAD_FAILED                   MAKE_D3DHRESULT(4037)
#define D3DERR_TEXTURE_LOCKED                        MAKE_D3DHRESULT(4038)
#define D3DERR_TEXTURE_LOCK_FAILED                   MAKE_D3DHRESULT(4039)
#define D3DERR_TEXTURE_NOT_LOCKED                    MAKE_D3DHRESULT(4040)
#define D3DERR_TEXTURE_NO_SUPPORT                    MAKE_D3DHRESULT(4041)
#define D3DERR_TEXTURE_SWAP_FAILED                   MAKE_D3DHRESULT(4042)
#define D3DERR_TEXTURE_UNLOCK_FAILED                 MAKE_D3DHRESULT(4043)
#define D3DERR_TOOMANYOPERATIONS                     MAKE_D3DHRESULT(4044)
#define D3DERR_UNSUPPORTEDALPHAARG                   MAKE_D3DHRESULT(4045)
#define D3DERR_UNSUPPORTEDALPHAOPERATION             MAKE_D3DHRESULT(4046)
#define D3DERR_UNSUPPORTEDCOLORARG                   MAKE_D3DHRESULT(4047)
#define D3DERR_UNSUPPORTEDCOLOROPERATION             MAKE_D3DHRESULT(4048)
#define D3DERR_UNSUPPORTEDFACTORVALUE                MAKE_D3DHRESULT(4049)
#define D3DERR_UNSUPPORTEDTEXTUREFILTER              MAKE_D3DHRESULT(4050)
#define D3DERR_VBUF_CREATE_FAILED                    MAKE_D3DHRESULT(4051)
#define D3DERR_VERTEXBUFFERLOCKED                    MAKE_D3DHRESULT(4052)
#define D3DERR_WRONGTEXTUREFORMAT                    MAKE_D3DHRESULT(4053)
#define D3DERR_ZBUFFER_NOTPRESENT                    MAKE_D3DHRESULT(4054)

/* ============================================================
 * Device caps structures
 * ============================================================ */
typedef struct _D3DPRIMCAPS {
    DWORD dwSize;
    DWORD dwMiscCaps;
    DWORD dwRasterCaps;
    DWORD dwZCmpCaps;
    DWORD dwSrcBlendCaps;
    DWORD dwDestBlendCaps;
    DWORD dwAlphaCmpCaps;
    DWORD dwShadeCaps;
    DWORD dwTextureCaps;
    DWORD dwTextureFilterCaps;
    DWORD dwTextureBlendCaps;
    DWORD dwTextureAddressCaps;
    DWORD dwStippleWidth;
    DWORD dwStippleHeight;
} D3DPRIMCAPS, *LPD3DPRIMCAPS;

typedef struct _D3DDeviceDesc7 {
    DWORD       dwDevCaps;
    D3DPRIMCAPS dpcLineCaps;
    D3DPRIMCAPS dpcTriCaps;
    DWORD       dwDeviceRenderBitDepth;
    DWORD       dwDeviceZBufferBitDepth;
    DWORD       dwMinTextureWidth, dwMinTextureHeight;
    DWORD       dwMaxTextureWidth, dwMaxTextureHeight;
    DWORD       dwMaxTextureRepeat;
    DWORD       dwMaxTextureAspectRatio;
    DWORD       dwMaxAnisotropy;
    D3DVALUE    dvGuardBandLeft;
    D3DVALUE    dvGuardBandTop;
    D3DVALUE    dvGuardBandRight;
    D3DVALUE    dvGuardBandBottom;
    D3DVALUE    dvExtentsAdjust;
    DWORD       dwStencilCaps;
    DWORD       dwFVFCaps;
    DWORD       dwTextureOpCaps;
    WORD        wMaxTextureBlendStages;
    WORD        wMaxSimultaneousTextures;
    DWORD       dwMaxActiveLights;
    D3DVALUE    dvMaxVertexW;
    GUID        deviceGUID;
    WORD        wMaxUserClipPlanes;
    WORD        wMaxVertexBlendMatrices;
    DWORD       dwVertexProcessingCaps;
    DWORD       dwReserved1;
    DWORD       dwReserved2;
    DWORD       dwReserved3;
    DWORD       dwReserved4;
} D3DDEVICEDESC7, *LPD3DDEVICEDESC7;

/* Caps bits */
#define D3DPMISCCAPS_MASKPLANES   0x00000001
#define D3DPMISCCAPS_MASKZ        0x00000002
#define D3DPMISCCAPS_LINEPATTERNREP 0x00000004
#define D3DPMISCCAPS_CONFORMANT   0x00000008
#define D3DPMISCCAPS_CULLNONE     0x00000010
#define D3DPMISCCAPS_CULLCW       0x00000020
#define D3DPMISCCAPS_CULLCCW      0x00000040

#define D3DPRASTERCAPS_DITHER          0x00000001
#define D3DPRASTERCAPS_ROP2            0x00000002
#define D3DPRASTERCAPS_XOR             0x00000004
#define D3DPRASTERCAPS_PAT             0x00000008
#define D3DPRASTERCAPS_ZTEST           0x00000010
#define D3DPRASTERCAPS_SUBPIXEL        0x00000020
#define D3DPRASTERCAPS_SUBPIXELX       0x00000040
#define D3DPRASTERCAPS_FOGVERTEX       0x00000080
#define D3DPRASTERCAPS_FOGTABLE        0x00000100
#define D3DPRASTERCAPS_STIPPLE         0x00000200
#define D3DPRASTERCAPS_ANTIALIASSORTDEPENDENT   0x00000400
#define D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT 0x00000800
#define D3DPRASTERCAPS_ANTIALIASEDGES  0x00001000
#define D3DPRASTERCAPS_MIPMAPLODBIAS   0x00002000
#define D3DPRASTERCAPS_ZBIAS           0x00004000
#define D3DPRASTERCAPS_ZBUFFERLESSHSR  0x00008000
#define D3DPRASTERCAPS_FOGRANGE        0x00010000
#define D3DPRASTERCAPS_ANISOTROPY      0x00020000
#define D3DPRASTERCAPS_WBUFFER         0x00040000
#define D3DPRASTERCAPS_WFOG            0x00100000
#define D3DPRASTERCAPS_ZFOG            0x00200000

#define D3DPCMPCAPS_NEVER        0x00000001
#define D3DPCMPCAPS_LESS         0x00000002
#define D3DPCMPCAPS_EQUAL        0x00000004
#define D3DPCMPCAPS_LESSEQUAL    0x00000008
#define D3DPCMPCAPS_GREATER      0x00000010
#define D3DPCMPCAPS_NOTEQUAL     0x00000020
#define D3DPCMPCAPS_GREATEREQUAL 0x00000040
#define D3DPCMPCAPS_ALWAYS       0x00000080

#define D3DPBLENDCAPS_ZERO         0x00000001
#define D3DPBLENDCAPS_ONE          0x00000002
#define D3DPBLENDCAPS_SRCCOLOR     0x00000004
#define D3DPBLENDCAPS_INVSRCCOLOR  0x00000008
#define D3DPBLENDCAPS_SRCALPHA     0x00000010
#define D3DPBLENDCAPS_INVSRCALPHA  0x00000020
#define D3DPBLENDCAPS_DESTALPHA    0x00000040
#define D3DPBLENDCAPS_INVDESTALPHA 0x00000080
#define D3DPBLENDCAPS_DESTCOLOR    0x00000100
#define D3DPBLENDCAPS_INVDESTCOLOR 0x00000200
#define D3DPBLENDCAPS_SRCALPHASAT  0x00000400
#define D3DPBLENDCAPS_BOTHSRCALPHA 0x00000800
#define D3DPBLENDCAPS_BOTHINVSRCALPHA 0x00001000

#define D3DPSHADECAPS_COLORFLATRGB       0x00000002
#define D3DPSHADECAPS_COLORGOURAUDRGB    0x00000008
#define D3DPSHADECAPS_SPECULARFLATRGB    0x00000080
#define D3DPSHADECAPS_SPECULARGOURAUDRGB 0x00000200
#define D3DPSHADECAPS_ALPHAFLATBLEND     0x00001000
#define D3DPSHADECAPS_ALPHAGOURAUDBLEND  0x00004000
#define D3DPSHADECAPS_FOGFLAT            0x00010000
#define D3DPSHADECAPS_FOGGOURAUD         0x00080000

#define D3DPTEXTURECAPS_PERSPECTIVE  0x00000001
#define D3DPTEXTURECAPS_POW2         0x00000002
#define D3DPTEXTURECAPS_ALPHA        0x00000004
#define D3DPTEXTURECAPS_TRANSPARENCY 0x00000008
#define D3DPTEXTURECAPS_BORDER       0x00000010
#define D3DPTEXTURECAPS_SQUAREONLY   0x00000020
#define D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE 0x00000040
#define D3DPTEXTURECAPS_ALPHAPALETTE 0x00000080
#define D3DPTEXTURECAPS_COLORKEYBLEND 0x00004000

#define D3DPTFILTERCAPS_NEAREST          0x00000001
#define D3DPTFILTERCAPS_LINEAR           0x00000002
#define D3DPTFILTERCAPS_MIPNEAREST       0x00000004
#define D3DPTFILTERCAPS_MIPLINEAR        0x00000008
#define D3DPTFILTERCAPS_LINEARMIPNEAREST 0x00000010
#define D3DPTFILTERCAPS_LINEARMIPLINEAR  0x00000020
#define D3DPTFILTERCAPS_MINFPOINT        0x00000100
#define D3DPTFILTERCAPS_MINFLINEAR       0x00000200
#define D3DPTFILTERCAPS_MINFANISOTROPIC  0x00000400
#define D3DPTFILTERCAPS_MIPFPOINT        0x00010000
#define D3DPTFILTERCAPS_MIPFLINEAR       0x00020000
#define D3DPTFILTERCAPS_MAGFPOINT        0x01000000
#define D3DPTFILTERCAPS_MAGFLINEAR       0x02000000
#define D3DPTFILTERCAPS_MAGFANISOTROPIC  0x04000000

#define D3DPTADDRESSCAPS_WRAP        0x00000001
#define D3DPTADDRESSCAPS_MIRROR      0x00000002
#define D3DPTADDRESSCAPS_CLAMP       0x00000004
#define D3DPTADDRESSCAPS_BORDER      0x00000008
#define D3DPTADDRESSCAPS_INDEPENDENTUV 0x00000010

#define D3DDEVCAPS_FLOATTLVERTEX        0x00000001
#define D3DDEVCAPS_EXECUTESYSTEMMEMORY  0x00000002
#define D3DDEVCAPS_EXECUTEVIDEOMEMORY   0x00000004
#define D3DDEVCAPS_TLVERTEXSYSTEMMEMORY 0x00000008
#define D3DDEVCAPS_TLVERTEXVIDEOMEMORY  0x00000010
#define D3DDEVCAPS_TEXTURESYSTEMMEMORY  0x00000020
#define D3DDEVCAPS_TEXTUREVIDEOMEMORY   0x00000040
#define D3DDEVCAPS_DRAWPRIMTLVERTEX     0x00000080
#define D3DDEVCAPS_CANRENDERAFTERFLIP   0x00000100
#define D3DDEVCAPS_TEXTURENONLOCALVIDMEM 0x00000200
#define D3DDEVCAPS_DRAWPRIMITIVES2      0x00002000
#define D3DDEVCAPS_SEPARATETEXTUREMEMORIES 0x00004000
#define D3DDEVCAPS_DRAWPRIMITIVES2EX    0x00008000
#define D3DDEVCAPS_HWTRANSFORMANDLIGHT  0x00010000
#define D3DDEVCAPS_CANBLTSYSTONONLOCAL  0x00020000
#define D3DDEVCAPS_HWRASTERIZATION      0x00080000

#define D3DSTENCILCAPS_KEEP    0x00000001
#define D3DSTENCILCAPS_ZERO    0x00000002
#define D3DSTENCILCAPS_REPLACE 0x00000004
#define D3DSTENCILCAPS_INCRSAT 0x00000008
#define D3DSTENCILCAPS_DECRSAT 0x00000010
#define D3DSTENCILCAPS_INVERT  0x00000020
#define D3DSTENCILCAPS_INCR    0x00000040
#define D3DSTENCILCAPS_DECR    0x00000080

#define D3DTEXOPCAPS_DISABLE     0x00000001
#define D3DTEXOPCAPS_SELECTARG1  0x00000002
#define D3DTEXOPCAPS_SELECTARG2  0x00000004
#define D3DTEXOPCAPS_MODULATE    0x00000008
#define D3DTEXOPCAPS_MODULATE2X  0x00000010
#define D3DTEXOPCAPS_MODULATE4X  0x00000020
#define D3DTEXOPCAPS_ADD         0x00000040
#define D3DTEXOPCAPS_ADDSIGNED   0x00000080
#define D3DTEXOPCAPS_SUBTRACT    0x00000200
#define D3DTEXOPCAPS_ADDSMOOTH   0x00000400
#define D3DTEXOPCAPS_BLENDDIFFUSEALPHA 0x00000800
#define D3DTEXOPCAPS_BLENDTEXTUREALPHA 0x00001000
#define D3DTEXOPCAPS_DOTPRODUCT3 0x00800000

#define D3DVTXPCAPS_TEXGEN          0x00000001
#define D3DVTXPCAPS_MATERIALSOURCE7 0x00000002
#define D3DVTXPCAPS_VERTEXFOG       0x00000004
#define D3DVTXPCAPS_DIRECTIONALLIGHTS 0x00000008
#define D3DVTXPCAPS_POSITIONALLIGHTS 0x00000010
#define D3DVTXPCAPS_LOCALVIEWER     0x00000020

#define D3DFVFCAPS_TEXCOORDCOUNTMASK 0x0000ffff
#define D3DFVFCAPS_DONOTSTRIPELEMENTS 0x00080000

/* ============================================================
 * Vertex buffer
 * ============================================================ */
typedef struct _D3DVERTEXBUFFERDESC {
    DWORD dwSize;
    DWORD dwCaps;
    DWORD dwFVF;
    DWORD dwNumVertices;
} D3DVERTEXBUFFERDESC, *LPD3DVERTEXBUFFERDESC;

#define D3DVBCAPS_SYSTEMMEMORY 0x00000800
#define D3DVBCAPS_WRITEONLY    0x00010000
#define D3DVBCAPS_OPTIMIZED    0x80000000
#define D3DVBCAPS_DONOTCLIP    0x00000001

/* Vertex ops for ProcessVertices */
#define D3DVOP_LIGHT     (1 << 10)
#define D3DVOP_TRANSFORM (1 << 0)
#define D3DVOP_CLIP      (1 << 2)
#define D3DVOP_EXTENTS   (1 << 3)

/* ============================================================
 * Misc types
 * ============================================================ */
typedef struct _D3DCLIPSTATUS {
    DWORD dwFlags;
    DWORD dwStatus;
    float minx, maxx;
    float miny, maxy;
    float minz, maxz;
} D3DCLIPSTATUS, *LPD3DCLIPSTATUS;

typedef struct _D3DDP_PTRSTRIDE {
    LPVOID lpvData;
    DWORD  dwStride;
} D3DDP_PTRSTRIDE;

typedef struct _D3DDRAWPRIMITIVESTRIDEDDATA {
    D3DDP_PTRSTRIDE position;
    D3DDP_PTRSTRIDE normal;
    D3DDP_PTRSTRIDE diffuse;
    D3DDP_PTRSTRIDE specular;
    D3DDP_PTRSTRIDE textureCoords[8];
} D3DDRAWPRIMITIVESTRIDEDDATA, *LPD3DDRAWPRIMITIVESTRIDEDDATA;

#define D3DENUMRET_CANCEL 0
#define D3DENUMRET_OK     1

#define D3DDEVINFOID_TEXTUREMANAGER 1
#define D3DDEVINFOID_D3DTEXTUREMANAGER 2
#define D3DDEVINFOID_TEXTURING 3

typedef struct _D3DDEVINFO_TEXTURING {
    DWORD dwNumLoads;
    DWORD dwApproxBytesLoaded;
    DWORD dwNumPreLoads;
    DWORD dwNumSet;
    DWORD dwNumCreates;
    DWORD dwNumDestroys;
    DWORD dwNumSetPriorities;
    DWORD dwNumSetLODs;
    DWORD dwNumLocks;
    DWORD dwNumGetDCs;
} D3DDEVINFO_TEXTURING, *LPD3DDEVINFO_TEXTURING;

typedef struct _D3DDEVINFO_TEXTUREMANAGER {
    BOOL  bThrashing;
    DWORD dwApproxBytesDownloaded;
    DWORD dwNumEvicts;
    DWORD dwNumVidCreates;
    DWORD dwNumTexturesUsed;
    DWORD dwNumUsedTexInVid;
    DWORD dwWorkingSet;
    DWORD dwWorkingSetBytes;
    DWORD dwTotalManaged;
    DWORD dwTotalBytes;
} D3DDEVINFO_TEXTUREMANAGER, *LPD3DDEVINFO_TEXTUREMANAGER;

/* ============================================================
 * Callback typedefs
 * ============================================================ */
typedef HRESULT (CALLBACK *LPD3DENUMPIXELFORMATSCALLBACK)(LPDDPIXELFORMAT lpDDPixFmt, LPVOID lpContext);
typedef HRESULT (CALLBACK *LPD3DENUMDEVICESCALLBACK7)(LPSTR lpDeviceDescription, LPSTR lpDeviceName, LPD3DDEVICEDESC7 lpD3DDeviceDesc, LPVOID lpContext);
typedef LPD3DENUMPIXELFORMATSCALLBACK LPDDENUMTEXTUREFORMATSCALLBACK;

/* ============================================================
 * Device GUIDs
 * ============================================================ */
#ifdef __cplusplus
static const GUID IID_IDirect3D7 =
    { 0xf5049e77, 0x4861, 0x11d2, { 0xa4, 0x07, 0x00, 0xa0, 0xc9, 0x06, 0x29, 0xa8 } };
static const GUID IID_IDirect3DDevice7 =
    { 0xf5049e79, 0x4861, 0x11d2, { 0xa4, 0x07, 0x00, 0xa0, 0xc9, 0x06, 0x29, 0xa8 } };
static const GUID IID_IDirect3DVertexBuffer7 =
    { 0xf5049e7d, 0x4861, 0x11d2, { 0xa4, 0x07, 0x00, 0xa0, 0xc9, 0x06, 0x29, 0xa8 } };
static const GUID IID_IDirect3DHALDevice =
    { 0x84e63de0, 0x46aa, 0x11cf, { 0x81, 0x6f, 0x00, 0x00, 0xc0, 0x20, 0x15, 0x6e } };
static const GUID IID_IDirect3DTnLHalDevice =
    { 0xf5049e78, 0x4861, 0x11d2, { 0xa4, 0x07, 0x00, 0xa0, 0xc9, 0x06, 0x29, 0xa8 } };
static const GUID IID_IDirect3DRGBDevice =
    { 0xa4665c60, 0x2673, 0x11cf, { 0xa3, 0x1a, 0x00, 0xaa, 0x00, 0xb9, 0x33, 0x56 } };
static const GUID IID_IDirect3DRampDevice =
    { 0xf2086b20, 0x259f, 0x11cf, { 0xa3, 0x1a, 0x00, 0xaa, 0x00, 0xb9, 0x33, 0x56 } };
static const GUID IID_IDirect3DMMXDevice =
    { 0x881949a1, 0xd3f6, 0x11d0, { 0x89, 0xab, 0x00, 0xa0, 0xc9, 0x05, 0x41, 0x29 } };
static const GUID IID_IDirect3DRefDevice =
    { 0x50936643, 0x13e9, 0x11d1, { 0x89, 0xaa, 0x00, 0xa0, 0xc9, 0x05, 0x41, 0x29 } };
#endif

/* ============================================================
 * IDirect3DVertexBuffer7
 * ============================================================ */
typedef struct IDirect3DVertexBuffer7Vtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IDirect3DVertexBuffer7 *This, REFIID riid, void **ppv);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IDirect3DVertexBuffer7 *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IDirect3DVertexBuffer7 *This);
    HRESULT (STDMETHODCALLTYPE *Lock)(IDirect3DVertexBuffer7 *This, DWORD dwFlags, LPVOID *lplpData, LPDWORD lpdwSize);
    HRESULT (STDMETHODCALLTYPE *Unlock)(IDirect3DVertexBuffer7 *This);
    HRESULT (STDMETHODCALLTYPE *ProcessVertices)(IDirect3DVertexBuffer7 *This, DWORD dwVertexOp, DWORD dwDestIndex, DWORD dwCount, IDirect3DVertexBuffer7 *lpSrcBuffer, DWORD dwSrcIndex, IDirect3DDevice7 *lpD3DDevice, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetVertexBufferDesc)(IDirect3DVertexBuffer7 *This, LPD3DVERTEXBUFFERDESC lpVBDesc);
    HRESULT (STDMETHODCALLTYPE *Optimize)(IDirect3DVertexBuffer7 *This, IDirect3DDevice7 *lpD3DDevice, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *ProcessVerticesStrided)(IDirect3DVertexBuffer7 *This, DWORD dwVertexOp, DWORD dwDestIndex, DWORD dwCount, void *lpVertexArray, DWORD dwSrcIndex, IDirect3DDevice7 *lpD3DDevice, DWORD dwFlags);
} IDirect3DVertexBuffer7Vtbl;

struct IDirect3DVertexBuffer7 {
    IDirect3DVertexBuffer7Vtbl *lpVtbl;
#ifdef __cplusplus
    HRESULT QueryInterface(REFIID riid, void **ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG   AddRef()  { return lpVtbl->AddRef(this); }
    ULONG   Release() { return lpVtbl->Release(this); }
    HRESULT Lock(DWORD a, LPVOID *b, LPDWORD c) { return lpVtbl->Lock(this, a, b, c); }
    HRESULT Unlock() { return lpVtbl->Unlock(this); }
    HRESULT ProcessVertices(DWORD a, DWORD b, DWORD c, IDirect3DVertexBuffer7 *d, DWORD e, IDirect3DDevice7 *f, DWORD g) { return lpVtbl->ProcessVertices(this, a, b, c, d, e, f, g); }
    HRESULT GetVertexBufferDesc(LPD3DVERTEXBUFFERDESC a) { return lpVtbl->GetVertexBufferDesc(this, a); }
    HRESULT Optimize(IDirect3DDevice7 *a, DWORD b) { return lpVtbl->Optimize(this, a, b); }
    HRESULT ProcessVerticesStrided(DWORD a, DWORD b, DWORD c, void *d, DWORD e, IDirect3DDevice7 *f, DWORD g) { return lpVtbl->ProcessVerticesStrided(this, a, b, c, d, e, f, g); }
#endif
};

/* ============================================================
 * IDirect3DDevice7
 * ============================================================ */
typedef struct IDirect3DDevice7Vtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IDirect3DDevice7 *This, REFIID riid, void **ppv);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IDirect3DDevice7 *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IDirect3DDevice7 *This);
    HRESULT (STDMETHODCALLTYPE *GetCaps)(IDirect3DDevice7 *This, LPD3DDEVICEDESC7 lpDesc);
    HRESULT (STDMETHODCALLTYPE *EnumTextureFormats)(IDirect3DDevice7 *This, void *cb, LPVOID arg);
    HRESULT (STDMETHODCALLTYPE *BeginScene)(IDirect3DDevice7 *This);
    HRESULT (STDMETHODCALLTYPE *EndScene)(IDirect3DDevice7 *This);
    HRESULT (STDMETHODCALLTYPE *GetDirect3D)(IDirect3DDevice7 *This, LPDIRECT3D7 *lplpD3D);
    HRESULT (STDMETHODCALLTYPE *SetRenderTarget)(IDirect3DDevice7 *This, LPDIRECTDRAWSURFACE7 lpNewRT, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetRenderTarget)(IDirect3DDevice7 *This, LPDIRECTDRAWSURFACE7 *lplpRT);
    HRESULT (STDMETHODCALLTYPE *Clear)(IDirect3DDevice7 *This, DWORD dwCount, LPD3DRECT lpRects, DWORD dwFlags, D3DCOLOR dwColor, D3DVALUE dvZ, DWORD dwStencil);
    HRESULT (STDMETHODCALLTYPE *SetTransform)(IDirect3DDevice7 *This, D3DTRANSFORMSTATETYPE state, LPD3DMATRIX lpMatrix);
    HRESULT (STDMETHODCALLTYPE *GetTransform)(IDirect3DDevice7 *This, D3DTRANSFORMSTATETYPE state, LPD3DMATRIX lpMatrix);
    HRESULT (STDMETHODCALLTYPE *SetViewport)(IDirect3DDevice7 *This, LPD3DVIEWPORT7 lpViewport);
    HRESULT (STDMETHODCALLTYPE *MultiplyTransform)(IDirect3DDevice7 *This, D3DTRANSFORMSTATETYPE state, LPD3DMATRIX lpMatrix);
    HRESULT (STDMETHODCALLTYPE *GetViewport)(IDirect3DDevice7 *This, LPD3DVIEWPORT7 lpViewport);
    HRESULT (STDMETHODCALLTYPE *SetMaterial)(IDirect3DDevice7 *This, LPD3DMATERIAL7 lpMaterial);
    HRESULT (STDMETHODCALLTYPE *GetMaterial)(IDirect3DDevice7 *This, LPD3DMATERIAL7 lpMaterial);
    HRESULT (STDMETHODCALLTYPE *SetLight)(IDirect3DDevice7 *This, DWORD dwLightIndex, LPD3DLIGHT7 lpLight);
    HRESULT (STDMETHODCALLTYPE *GetLight)(IDirect3DDevice7 *This, DWORD dwLightIndex, LPD3DLIGHT7 lpLight);
    HRESULT (STDMETHODCALLTYPE *SetRenderState)(IDirect3DDevice7 *This, D3DRENDERSTATETYPE dwState, DWORD dwValue);
    HRESULT (STDMETHODCALLTYPE *GetRenderState)(IDirect3DDevice7 *This, D3DRENDERSTATETYPE dwState, LPDWORD lpdwValue);
    HRESULT (STDMETHODCALLTYPE *BeginStateBlock)(IDirect3DDevice7 *This);
    HRESULT (STDMETHODCALLTYPE *EndStateBlock)(IDirect3DDevice7 *This, LPDWORD lpdwBlockHandle);
    HRESULT (STDMETHODCALLTYPE *PreLoad)(IDirect3DDevice7 *This, LPDIRECTDRAWSURFACE7 lpddsTexture);
    HRESULT (STDMETHODCALLTYPE *DrawPrimitive)(IDirect3DDevice7 *This, D3DPRIMITIVETYPE dptPrimitiveType, DWORD dwVertexTypeDesc, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *DrawIndexedPrimitive)(IDirect3DDevice7 *This, D3DPRIMITIVETYPE d3dptPrimitiveType, DWORD dwVertexTypeDesc, LPVOID lpvVertices, DWORD dwVertexCount, LPWORD lpwIndices, DWORD dwIndexCount, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *SetClipStatus)(IDirect3DDevice7 *This, void *lpD3DClipStatus);
    HRESULT (STDMETHODCALLTYPE *GetClipStatus)(IDirect3DDevice7 *This, void *lpD3DClipStatus);
    HRESULT (STDMETHODCALLTYPE *DrawPrimitiveStrided)(IDirect3DDevice7 *This, D3DPRIMITIVETYPE dptPrimitiveType, DWORD dwVertexTypeDesc, void *lpVertexArray, DWORD dwVertexCount, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *DrawIndexedPrimitiveStrided)(IDirect3DDevice7 *This, D3DPRIMITIVETYPE d3dptPrimitiveType, DWORD dwVertexTypeDesc, void *lpVertexArray, DWORD dwVertexCount, LPWORD lpwIndices, DWORD dwIndexCount, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *DrawPrimitiveVB)(IDirect3DDevice7 *This, D3DPRIMITIVETYPE d3dptPrimitiveType, LPDIRECT3DVERTEXBUFFER7 lpd3dVertexBuffer, DWORD dwStartVertex, DWORD dwNumVertices, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *DrawIndexedPrimitiveVB)(IDirect3DDevice7 *This, D3DPRIMITIVETYPE d3dptPrimitiveType, LPDIRECT3DVERTEXBUFFER7 lpd3dVertexBuffer, DWORD dwStartVertex, DWORD dwNumVertices, LPWORD lpwIndices, DWORD dwIndexCount, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *ComputeSphereVisibility)(IDirect3DDevice7 *This, LPD3DVECTOR lpCenters, LPD3DVALUE lpRadii, DWORD dwNumSpheres, DWORD dwFlags, LPDWORD lpdwReturnValues);
    HRESULT (STDMETHODCALLTYPE *GetTexture)(IDirect3DDevice7 *This, DWORD dwStage, LPDIRECTDRAWSURFACE7 *lplpTexture);
    HRESULT (STDMETHODCALLTYPE *SetTexture)(IDirect3DDevice7 *This, DWORD dwStage, LPDIRECTDRAWSURFACE7 lpTexture);
    HRESULT (STDMETHODCALLTYPE *GetTextureStageState)(IDirect3DDevice7 *This, DWORD dwStage, D3DTEXTURESTAGESTATETYPE dwState, LPDWORD lpdwValue);
    HRESULT (STDMETHODCALLTYPE *SetTextureStageState)(IDirect3DDevice7 *This, DWORD dwStage, D3DTEXTURESTAGESTATETYPE dwState, DWORD dwValue);
    HRESULT (STDMETHODCALLTYPE *ValidateDevice)(IDirect3DDevice7 *This, LPDWORD lpdwPasses);
    HRESULT (STDMETHODCALLTYPE *ApplyStateBlock)(IDirect3DDevice7 *This, DWORD dwBlockHandle);
    HRESULT (STDMETHODCALLTYPE *CaptureStateBlock)(IDirect3DDevice7 *This, DWORD dwBlockHandle);
    HRESULT (STDMETHODCALLTYPE *DeleteStateBlock)(IDirect3DDevice7 *This, DWORD dwBlockHandle);
    HRESULT (STDMETHODCALLTYPE *CreateStateBlock)(IDirect3DDevice7 *This, DWORD d3dsbType, LPDWORD lpdwBlockHandle);
    HRESULT (STDMETHODCALLTYPE *Load)(IDirect3DDevice7 *This, LPDIRECTDRAWSURFACE7 lpDestTex, LPPOINT lpDestPoint, LPDIRECTDRAWSURFACE7 lpSrcTex, LPRECT lprcSrcRect, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *LightEnable)(IDirect3DDevice7 *This, DWORD dwLightIndex, BOOL bEnable);
    HRESULT (STDMETHODCALLTYPE *GetLightEnable)(IDirect3DDevice7 *This, DWORD dwLightIndex, BOOL *pbEnable);
    HRESULT (STDMETHODCALLTYPE *SetClipPlane)(IDirect3DDevice7 *This, DWORD dwIndex, D3DVALUE *pPlaneEquation);
    HRESULT (STDMETHODCALLTYPE *GetClipPlane)(IDirect3DDevice7 *This, DWORD dwIndex, D3DVALUE *pPlaneEquation);
    HRESULT (STDMETHODCALLTYPE *GetInfo)(IDirect3DDevice7 *This, DWORD dwDevInfoID, LPVOID pDevInfoStruct, DWORD dwSize);
} IDirect3DDevice7Vtbl;

struct IDirect3DDevice7 {
    IDirect3DDevice7Vtbl *lpVtbl;
#ifdef __cplusplus
    HRESULT QueryInterface(REFIID riid, void **ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG   AddRef()  { return lpVtbl->AddRef(this); }
    ULONG   Release() { return lpVtbl->Release(this); }
    HRESULT GetCaps(LPD3DDEVICEDESC7 a) { return lpVtbl->GetCaps(this, a); }
    HRESULT EnumTextureFormats(LPD3DENUMPIXELFORMATSCALLBACK a, LPVOID b) { return lpVtbl->EnumTextureFormats(this, (void *)a, b); }
    HRESULT BeginScene() { return lpVtbl->BeginScene(this); }
    HRESULT EndScene() { return lpVtbl->EndScene(this); }
    HRESULT GetDirect3D(LPDIRECT3D7 *a) { return lpVtbl->GetDirect3D(this, a); }
    HRESULT SetRenderTarget(LPDIRECTDRAWSURFACE7 a, DWORD b) { return lpVtbl->SetRenderTarget(this, a, b); }
    HRESULT GetRenderTarget(LPDIRECTDRAWSURFACE7 *a) { return lpVtbl->GetRenderTarget(this, a); }
    HRESULT Clear(DWORD a, LPD3DRECT b, DWORD c, D3DCOLOR d, D3DVALUE e, DWORD f) { return lpVtbl->Clear(this, a, b, c, d, e, f); }
    HRESULT SetTransform(D3DTRANSFORMSTATETYPE a, LPD3DMATRIX b) { return lpVtbl->SetTransform(this, a, b); }
    HRESULT GetTransform(D3DTRANSFORMSTATETYPE a, LPD3DMATRIX b) { return lpVtbl->GetTransform(this, a, b); }
    HRESULT SetViewport(LPD3DVIEWPORT7 a) { return lpVtbl->SetViewport(this, a); }
    HRESULT MultiplyTransform(D3DTRANSFORMSTATETYPE a, LPD3DMATRIX b) { return lpVtbl->MultiplyTransform(this, a, b); }
    HRESULT GetViewport(LPD3DVIEWPORT7 a) { return lpVtbl->GetViewport(this, a); }
    HRESULT SetMaterial(LPD3DMATERIAL7 a) { return lpVtbl->SetMaterial(this, a); }
    HRESULT GetMaterial(LPD3DMATERIAL7 a) { return lpVtbl->GetMaterial(this, a); }
    HRESULT SetLight(DWORD a, LPD3DLIGHT7 b) { return lpVtbl->SetLight(this, a, b); }
    HRESULT GetLight(DWORD a, LPD3DLIGHT7 b) { return lpVtbl->GetLight(this, a, b); }
    HRESULT SetRenderState(D3DRENDERSTATETYPE a, DWORD b) { return lpVtbl->SetRenderState(this, a, b); }
    HRESULT GetRenderState(D3DRENDERSTATETYPE a, LPDWORD b) { return lpVtbl->GetRenderState(this, a, b); }
    HRESULT BeginStateBlock() { return lpVtbl->BeginStateBlock(this); }
    HRESULT EndStateBlock(LPDWORD a) { return lpVtbl->EndStateBlock(this, a); }
    HRESULT PreLoad(LPDIRECTDRAWSURFACE7 a) { return lpVtbl->PreLoad(this, a); }
    HRESULT DrawPrimitive(D3DPRIMITIVETYPE a, DWORD b, LPVOID c, DWORD d, DWORD e) { return lpVtbl->DrawPrimitive(this, a, b, c, d, e); }
    HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE a, DWORD b, LPVOID c, DWORD d, LPWORD e, DWORD f, DWORD g) { return lpVtbl->DrawIndexedPrimitive(this, a, b, c, d, e, f, g); }
    HRESULT SetClipStatus(void *a) { return lpVtbl->SetClipStatus(this, a); }
    HRESULT GetClipStatus(void *a) { return lpVtbl->GetClipStatus(this, a); }
    HRESULT DrawPrimitiveStrided(D3DPRIMITIVETYPE a, DWORD b, void *c, DWORD d, DWORD e) { return lpVtbl->DrawPrimitiveStrided(this, a, b, c, d, e); }
    HRESULT DrawIndexedPrimitiveStrided(D3DPRIMITIVETYPE a, DWORD b, void *c, DWORD d, LPWORD e, DWORD f, DWORD g) { return lpVtbl->DrawIndexedPrimitiveStrided(this, a, b, c, d, e, f, g); }
    HRESULT DrawPrimitiveVB(D3DPRIMITIVETYPE a, LPDIRECT3DVERTEXBUFFER7 b, DWORD c, DWORD d, DWORD e) { return lpVtbl->DrawPrimitiveVB(this, a, b, c, d, e); }
    HRESULT DrawIndexedPrimitiveVB(D3DPRIMITIVETYPE a, LPDIRECT3DVERTEXBUFFER7 b, DWORD c, DWORD d, LPWORD e, DWORD f, DWORD g) { return lpVtbl->DrawIndexedPrimitiveVB(this, a, b, c, d, e, f, g); }
    HRESULT ComputeSphereVisibility(LPD3DVECTOR a, LPD3DVALUE b, DWORD c, DWORD d, LPDWORD e) { return lpVtbl->ComputeSphereVisibility(this, a, b, c, d, e); }
    HRESULT GetTexture(DWORD a, LPDIRECTDRAWSURFACE7 *b) { return lpVtbl->GetTexture(this, a, b); }
    HRESULT SetTexture(DWORD a, LPDIRECTDRAWSURFACE7 b) { return lpVtbl->SetTexture(this, a, b); }
    HRESULT GetTextureStageState(DWORD a, D3DTEXTURESTAGESTATETYPE b, LPDWORD c) { return lpVtbl->GetTextureStageState(this, a, b, c); }
    HRESULT SetTextureStageState(DWORD a, D3DTEXTURESTAGESTATETYPE b, DWORD c) { return lpVtbl->SetTextureStageState(this, a, b, c); }
    HRESULT ValidateDevice(LPDWORD a) { return lpVtbl->ValidateDevice(this, a); }
    HRESULT ApplyStateBlock(DWORD a) { return lpVtbl->ApplyStateBlock(this, a); }
    HRESULT CaptureStateBlock(DWORD a) { return lpVtbl->CaptureStateBlock(this, a); }
    HRESULT DeleteStateBlock(DWORD a) { return lpVtbl->DeleteStateBlock(this, a); }
    HRESULT CreateStateBlock(DWORD a, LPDWORD b) { return lpVtbl->CreateStateBlock(this, a, b); }
    HRESULT Load(LPDIRECTDRAWSURFACE7 a, LPPOINT b, LPDIRECTDRAWSURFACE7 c, LPRECT d, DWORD e) { return lpVtbl->Load(this, a, b, c, d, e); }
    HRESULT LightEnable(DWORD a, BOOL b) { return lpVtbl->LightEnable(this, a, b); }
    HRESULT GetLightEnable(DWORD a, BOOL *b) { return lpVtbl->GetLightEnable(this, a, b); }
    HRESULT SetClipPlane(DWORD a, D3DVALUE *b) { return lpVtbl->SetClipPlane(this, a, b); }
    HRESULT GetClipPlane(DWORD a, D3DVALUE *b) { return lpVtbl->GetClipPlane(this, a, b); }
    HRESULT GetInfo(DWORD a, LPVOID b, DWORD c) { return lpVtbl->GetInfo(this, a, b, c); }
#endif
};

/* ============================================================
 * IDirect3D7
 * ============================================================ */
typedef struct IDirect3D7Vtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IDirect3D7 *This, REFIID riid, void **ppv);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IDirect3D7 *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IDirect3D7 *This);
    HRESULT (STDMETHODCALLTYPE *EnumDevices)(IDirect3D7 *This, void *cb, LPVOID arg);
    HRESULT (STDMETHODCALLTYPE *CreateDevice)(IDirect3D7 *This, REFCLSID rclsid, LPDIRECTDRAWSURFACE7 lpDDS, LPDIRECT3DDEVICE7 *lplpD3DDevice);
    HRESULT (STDMETHODCALLTYPE *CreateVertexBuffer)(IDirect3D7 *This, LPD3DVERTEXBUFFERDESC lpVBDesc, LPDIRECT3DVERTEXBUFFER7 *lplpD3DVertexBuffer, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *EnumZBufferFormats)(IDirect3D7 *This, REFCLSID riidDevice, void *cb, LPVOID ctx);
    HRESULT (STDMETHODCALLTYPE *EvictManagedTextures)(IDirect3D7 *This);
} IDirect3D7Vtbl;

struct IDirect3D7 {
    IDirect3D7Vtbl *lpVtbl;
#ifdef __cplusplus
    HRESULT QueryInterface(REFIID riid, void **ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG   AddRef()  { return lpVtbl->AddRef(this); }
    ULONG   Release() { return lpVtbl->Release(this); }
    HRESULT EnumDevices(LPD3DENUMDEVICESCALLBACK7 a, LPVOID b) { return lpVtbl->EnumDevices(this, (void *)a, b); }
    HRESULT CreateDevice(REFCLSID a, LPDIRECTDRAWSURFACE7 b, LPDIRECT3DDEVICE7 *c) { return lpVtbl->CreateDevice(this, a, b, c); }
    HRESULT CreateVertexBuffer(LPD3DVERTEXBUFFERDESC a, LPDIRECT3DVERTEXBUFFER7 *b, DWORD c) { return lpVtbl->CreateVertexBuffer(this, a, b, c); }
    HRESULT EnumZBufferFormats(REFCLSID a, LPD3DENUMPIXELFORMATSCALLBACK b, LPVOID c) { return lpVtbl->EnumZBufferFormats(this, a, (void *)b, c); }
    HRESULT EvictManagedTextures() { return lpVtbl->EvictManagedTextures(this); }
#endif
};

/* ============================================================
 * Factory functions (implemented in d3d_gl.cpp)
 * ============================================================ */
#ifdef __cplusplus
extern "C" {
#endif
IDirect3D7 *FF_CreateDirect3D7(void);
IDirect3DDevice7 *FF_CreateDirect3DDevice7(IDirect3D7 *d3d, IDirectDrawSurface7 *renderTarget);
struct DXContext;
struct DXContext *FF_CreateDXContext(int width, int height, IDirect3D7 *d3d, IDirect3DDevice7 *d3dDevice, IDirectDraw7 *dd);
#ifdef __cplusplus
}
#endif

/* Pull in the D3DX surface-format enum: legacy headers use the elaborated
 * `enum _D3DX_SURFACEFORMAT` member form which requires a complete type. */
#include "d3dxcore.h"

#endif /* FF_LINUX */
#endif /* FF_COMPAT_D3D_H */

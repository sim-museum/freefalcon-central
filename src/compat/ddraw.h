/*
 * FreeFalcon Linux Port - ddraw.h compatibility
 *
 * DirectDraw 7 types, constants and COM-style interfaces.
 * The implementations live in src/compat/d3d_gl.cpp (OpenGL backend).
 *
 * Interfaces follow the COM C layout: the only data member is lpVtbl,
 * plus inline C++ wrapper methods that dispatch through the vtable.
 * d3d_gl.cpp populates static ...Vtbl tables in the declared order.
 */

#ifndef FF_COMPAT_DDRAW_H
#define FF_COMPAT_DDRAW_H

#ifdef FF_LINUX

#include "compat_types.h"
#include "objbase.h"

/* ============================================================
 * Forward declarations / pointer typedefs
 * ============================================================ */
struct IDirectDraw;
struct IDirectDraw7;
struct IDirectDrawSurface7;
struct IDirectDrawClipper;
struct IDirectDrawPalette;

typedef struct IDirectDraw          *LPDIRECTDRAW;
typedef struct IDirectDraw7         *LPDIRECTDRAW7;
typedef struct IDirectDrawSurface7  *LPDIRECTDRAWSURFACE7;
typedef struct IDirectDrawClipper   *LPDIRECTDRAWCLIPPER;
typedef struct IDirectDrawPalette   *LPDIRECTDRAWPALETTE;

/* PALETTEENTRY lives in wingdi compat */
#include "compat_wingdi.h"

/* ============================================================
 * FOURCC
 * ============================================================ */
#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
    ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) | \
     ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24))
#endif

/* ============================================================
 * Result codes
 * ============================================================ */
#define DD_OK                            S_OK
#define DD_FALSE                         S_FALSE

#define _FACDD 0x876
#define MAKE_DDHRESULT(code) MAKE_HRESULT(1, _FACDD, code)




/* Generated: every DDERR_* referenced in the tree, unique values */
#define DDERR_ALREADYINITIALIZED                 MAKE_DDHRESULT(2000)
#define DDERR_BLTFASTCANTCLIP                    MAKE_DDHRESULT(2001)
#define DDERR_CANNOTATTACHSURFACE                MAKE_DDHRESULT(2002)
#define DDERR_CANNOTATTACHSURFACE_X              MAKE_DDHRESULT(2003)
#define DDERR_CANNOTDETACHSURFACE                MAKE_DDHRESULT(2004)
#define DDERR_CANTCREATEDC                       MAKE_DDHRESULT(2005)
#define DDERR_CANTDUPLICATE                      MAKE_DDHRESULT(2006)
#define DDERR_CANTLOCKSURFACE                    MAKE_DDHRESULT(2007)
#define DDERR_CANTPAGELOCK                       MAKE_DDHRESULT(2008)
#define DDERR_CANTPAGEUNLOCK                     MAKE_DDHRESULT(2009)
#define DDERR_CLIPPERISUSINGHWND                 MAKE_DDHRESULT(2010)
#define DDERR_COLORKEYNOTSET                     MAKE_DDHRESULT(2011)
#define DDERR_CURRENTLYNOTAVAIL                  MAKE_DDHRESULT(2012)
#define DDERR_DCALREADYCREATED                   MAKE_DDHRESULT(2013)
#define DDERR_DDSCAPSCOMPLEXREQUIRED             MAKE_DDHRESULT(2014)
#define DDERR_DEVICEDOESNTOWNSURFACE             MAKE_DDHRESULT(2015)
#define DDERR_DIRECTDRAWALREADYCREATED           MAKE_DDHRESULT(2016)
#define DDERR_EXCEPTION                          MAKE_DDHRESULT(2017)
#define DDERR_EXCLUSIVEMODEALREADYSET            MAKE_DDHRESULT(2018)
#define DDERR_EXPIRED                            MAKE_DDHRESULT(2019)
#define DDERR_GENERIC                            E_FAIL
#define DDERR_HEIGHTALIGN                        MAKE_DDHRESULT(2020)
#define DDERR_HWNDALREADYSET                     MAKE_DDHRESULT(2021)
#define DDERR_HWNDSUBCLASSED                     MAKE_DDHRESULT(2022)
#define DDERR_IMPLICITLYCREATED                  MAKE_DDHRESULT(2023)
#define DDERR_INCOMPATIBLEPRIMARY                MAKE_DDHRESULT(2024)
#define DDERR_INVALIDCAPS                        MAKE_DDHRESULT(2025)
#define DDERR_INVALIDCLIPLIST                    MAKE_DDHRESULT(2026)
#define DDERR_INVALIDDIRECTDRAWGUID              MAKE_DDHRESULT(2027)
#define DDERR_INVALIDMODE                        MAKE_DDHRESULT(2028)
#define DDERR_INVALIDOBJECT                      MAKE_DDHRESULT(2029)
#define DDERR_INVALIDPARAMS                      E_INVALIDARG
#define DDERR_INVALIDPIXELFORMAT                 MAKE_DDHRESULT(2030)
#define DDERR_INVALIDPOSITION                    MAKE_DDHRESULT(2031)
#define DDERR_INVALIDRECT                        MAKE_DDHRESULT(2032)
#define DDERR_INVALIDSTREAM                      MAKE_DDHRESULT(2033)
#define DDERR_INVALIDSURFACETYPE                 MAKE_DDHRESULT(2034)
#define DDERR_LOCKEDSURFACES                     MAKE_DDHRESULT(2035)
#define DDERR_MOREDATA                           MAKE_DDHRESULT(2036)
#define DDERR_MOREDATA_X                         MAKE_DDHRESULT(2037)
#define DDERR_NEWMODE                            MAKE_DDHRESULT(2038)
#define DDERR_NO3D                               MAKE_DDHRESULT(2039)
#define DDERR_NO3D_X                             MAKE_DDHRESULT(2040)
#define DDERR_NOALPHAHW                          MAKE_DDHRESULT(2041)
#define DDERR_NOALPHAHW_X                        MAKE_DDHRESULT(2042)
#define DDERR_NOBLTHW                            MAKE_DDHRESULT(2043)
#define DDERR_NOCLIPLIST                         MAKE_DDHRESULT(2044)
#define DDERR_NOCLIPLIST_X                       MAKE_DDHRESULT(2045)
#define DDERR_NOCLIPPERATTACHED                  MAKE_DDHRESULT(2046)
#define DDERR_NOCOLORCONVHW                      MAKE_DDHRESULT(2047)
#define DDERR_NOCOLORKEY                         MAKE_DDHRESULT(2048)
#define DDERR_NOCOLORKEYHW                       MAKE_DDHRESULT(2049)
#define DDERR_NOCOOPERATIVELEVELSET              MAKE_DDHRESULT(2050)
#define DDERR_NODC                               MAKE_DDHRESULT(2051)
#define DDERR_NODDROPSHW                         MAKE_DDHRESULT(2052)
#define DDERR_NODIRECTDRAWHW                     MAKE_DDHRESULT(2053)
#define DDERR_NODIRECTDRAWSUPPORT                MAKE_DDHRESULT(2054)
#define DDERR_NOEMULATION                        MAKE_DDHRESULT(2055)
#define DDERR_NOEXCLUSIVEMODE                    MAKE_DDHRESULT(2056)
#define DDERR_NOFLIPHW                           MAKE_DDHRESULT(2057)
#define DDERR_NOFOCUSWINDOW                      MAKE_DDHRESULT(2058)
#define DDERR_NOGDI                              MAKE_DDHRESULT(2059)
#define DDERR_NOHWND                             MAKE_DDHRESULT(2060)
#define DDERR_NOMIPMAPHW                         MAKE_DDHRESULT(2061)
#define DDERR_NOMIRRORHW                         MAKE_DDHRESULT(2062)
#define DDERR_NOMONITORINFORMATION               MAKE_DDHRESULT(2063)
#define DDERR_NONONLOCALVIDMEM                   MAKE_DDHRESULT(2064)
#define DDERR_NOOPTIMIZEHW                       MAKE_DDHRESULT(2065)
#define DDERR_NOOVERLAYDEST                      MAKE_DDHRESULT(2066)
#define DDERR_NOOVERLAYHW                        MAKE_DDHRESULT(2067)
#define DDERR_NOPALETTEATTACHED                  MAKE_DDHRESULT(2068)
#define DDERR_NOPALETTEHW                        MAKE_DDHRESULT(2069)
#define DDERR_NORASTEROPHW                       MAKE_DDHRESULT(2070)
#define DDERR_NOROTATIONHW                       MAKE_DDHRESULT(2071)
#define DDERR_NOSTEREOHARDWARE                   MAKE_DDHRESULT(2072)
#define DDERR_NOSTRETCHHW                        MAKE_DDHRESULT(2073)
#define DDERR_NOSURFACELEFT                      MAKE_DDHRESULT(2074)
#define DDERR_NOT4BITCOLOR                       MAKE_DDHRESULT(2075)
#define DDERR_NOT4BITCOLORINDEX                  MAKE_DDHRESULT(2076)
#define DDERR_NOT8BITCOLOR                       MAKE_DDHRESULT(2077)
#define DDERR_NOTAOVERLAYSURFACE                 MAKE_DDHRESULT(2078)
#define DDERR_NOTEXTUREHW                        MAKE_DDHRESULT(2079)
#define DDERR_NOTFLIPPABLE                       MAKE_DDHRESULT(2080)
#define DDERR_NOTFLIPPABLE_X                     MAKE_DDHRESULT(2081)
#define DDERR_NOTFOUND                           MAKE_DDHRESULT(2082)
#define DDERR_NOTINITIALIZED                     ((HRESULT)0x800401F0)
#define DDERR_NOTLOCKED                          MAKE_DDHRESULT(2083)
#define DDERR_NOTONMIPMAPSUBLEVEL                MAKE_DDHRESULT(2084)
#define DDERR_NOTPAGELOCKED                      MAKE_DDHRESULT(2085)
#define DDERR_NOTPALETTIZED                      MAKE_DDHRESULT(2086)
#define DDERR_NOVSYNCHW                          MAKE_DDHRESULT(2087)
#define DDERR_NOZBUFFERHW                        MAKE_DDHRESULT(2088)
#define DDERR_NOZBUFFERHW_X                      MAKE_DDHRESULT(2089)
#define DDERR_NOZOVERLAYHW                       MAKE_DDHRESULT(2090)
#define DDERR_OUTOFCAPS                          MAKE_DDHRESULT(2091)
#define DDERR_OUTOFMEMORY                        E_OUTOFMEMORY
#define DDERR_OUTOFVIDEOMEMORY                   MAKE_DDHRESULT(2092)
#define DDERR_OUTOFVIDEOMEMORY_X                 MAKE_DDHRESULT(2093)
#define DDERR_OVERLAPPINGRECTS                   MAKE_DDHRESULT(2094)
#define DDERR_OVERLAYCANTCLIP                    MAKE_DDHRESULT(2095)
#define DDERR_OVERLAYCOLORKEYONLYONEACTIVE       MAKE_DDHRESULT(2096)
#define DDERR_OVERLAYNOTVISIBLE                  MAKE_DDHRESULT(2097)
#define DDERR_PALETTEBUSY                        MAKE_DDHRESULT(2098)
#define DDERR_PRIMARYSURFACEALREADYEXISTS        MAKE_DDHRESULT(2099)
#define DDERR_REGIONTOOSMALL                     MAKE_DDHRESULT(2100)
#define DDERR_SURFACEALREADYATTACHED             MAKE_DDHRESULT(2101)
#define DDERR_SURFACEALREADYDEPENDENT            MAKE_DDHRESULT(2102)
#define DDERR_SURFACEBUSY                        MAKE_DDHRESULT(2103)
#define DDERR_SURFACEISOBSCURED                  MAKE_DDHRESULT(2104)
#define DDERR_SURFACELOST                        MAKE_DDHRESULT(2105)
#define DDERR_SURFACENOTATTACHED                 MAKE_DDHRESULT(2106)
#define DDERR_TESTFINISHED                       MAKE_DDHRESULT(2107)
#define DDERR_TOOBIGHEIGHT                       MAKE_DDHRESULT(2108)
#define DDERR_TOOBIGHEIGHT_X                     MAKE_DDHRESULT(2109)
#define DDERR_TOOBIGSIZE                         MAKE_DDHRESULT(2110)
#define DDERR_TOOBIGWIDTH                        MAKE_DDHRESULT(2111)
#define DDERR_UNSUPPORTED                        E_NOTIMPL
#define DDERR_UNSUPPORTEDFORMAT                  MAKE_DDHRESULT(2112)
#define DDERR_UNSUPPORTEDMASK                    MAKE_DDHRESULT(2113)
#define DDERR_UNSUPPORTEDMODE                    MAKE_DDHRESULT(2114)
#define DDERR_VERTICALBLANKINPROGRESS            MAKE_DDHRESULT(2115)
#define DDERR_VIDEONOTACTIVE                     MAKE_DDHRESULT(2116)
#define DDERR_WASSTILLDRAWING                    MAKE_DDHRESULT(2117)
#define DDERR_WRONGMODE                          MAKE_DDHRESULT(2118)
#define DDERR_XALIGN                             MAKE_DDHRESULT(2119)

/* Enum callback returns */
#define DDENUMRET_CANCEL  0
#define DDENUMRET_OK      1

/* ============================================================
 * Structures
 * ============================================================ */
typedef struct _DDCOLORKEY {
    DWORD dwColorSpaceLowValue;
    DWORD dwColorSpaceHighValue;
} DDCOLORKEY, *LPDDCOLORKEY;

typedef struct _DDPIXELFORMAT {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwFourCC;
    union {
        DWORD dwRGBBitCount;
        DWORD dwYUVBitCount;
        DWORD dwZBufferBitDepth;
        DWORD dwAlphaBitDepth;
        DWORD dwLuminanceBitCount;
        DWORD dwBumpBitCount;
    };
    union {
        DWORD dwRBitMask;
        DWORD dwYBitMask;
        DWORD dwStencilBitDepth;
        DWORD dwLuminanceBitMask;
        DWORD dwBumpDuBitMask;
    };
    union {
        DWORD dwGBitMask;
        DWORD dwUBitMask;
        DWORD dwZBitMask;
        DWORD dwBumpDvBitMask;
    };
    union {
        DWORD dwBBitMask;
        DWORD dwVBitMask;
        DWORD dwStencilBitMask;
        DWORD dwBumpLuminanceBitMask;
    };
    union {
        DWORD dwRGBAlphaBitMask;
        DWORD dwYUVAlphaBitMask;
        DWORD dwLuminanceAlphaBitMask;
        DWORD dwRGBZBitMask;
        DWORD dwYUVZBitMask;
    };
} DDPIXELFORMAT, *LPDDPIXELFORMAT;

typedef struct _DDSCAPS {
    DWORD dwCaps;
} DDSCAPS, *LPDDSCAPS;

typedef struct _DDSCAPS2 {
    DWORD dwCaps;
    DWORD dwCaps2;
    DWORD dwCaps3;
    DWORD dwCaps4;
} DDSCAPS2, *LPDDSCAPS2;

typedef struct _DDSURFACEDESC2 {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwHeight;
    DWORD dwWidth;
    union {
        LONG  lPitch;
        DWORD dwLinearSize;
    };
    union {
        DWORD dwBackBufferCount;
        DWORD dwDepth;
    };
    union {
        DWORD dwMipMapCount;
        DWORD dwRefreshRate;
        DWORD dwSrcVBHandle;
    };
    DWORD dwAlphaBitDepth;
    DWORD dwReserved;
    LPVOID lpSurface;
    union {
        DDCOLORKEY ddckCKDestOverlay;
        DWORD      dwEmptyFaceColor;
    };
    DDCOLORKEY ddckCKDestBlt;
    DDCOLORKEY ddckCKSrcOverlay;
    DDCOLORKEY ddckCKSrcBlt;
    union {
        DDPIXELFORMAT ddpfPixelFormat;
        DWORD         dwFVF;
    };
    DDSCAPS2 ddsCaps;
    DWORD    dwTextureStage;
} DDSURFACEDESC2, *LPDDSURFACEDESC2;

/* Legacy descriptor (rarely used; provide for completeness) */
typedef struct _DDSURFACEDESC {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwHeight;
    DWORD dwWidth;
    union {
        LONG  lPitch;
        DWORD dwLinearSize;
    };
    DWORD dwBackBufferCount;
    union {
        DWORD dwMipMapCount;
        DWORD dwZBufferBitDepth;
        DWORD dwRefreshRate;
    };
    DWORD dwAlphaBitDepth;
    DWORD dwReserved;
    LPVOID lpSurface;
    DDCOLORKEY ddckCKDestOverlay;
    DDCOLORKEY ddckCKDestBlt;
    DDCOLORKEY ddckCKSrcOverlay;
    DDCOLORKEY ddckCKSrcBlt;
    DDPIXELFORMAT ddpfPixelFormat;
    DDSCAPS ddsCaps;
} DDSURFACEDESC, *LPDDSURFACEDESC;

typedef struct _DDBLTFX {
    DWORD dwSize;
    DWORD dwDDFX;
    DWORD dwROP;
    DWORD dwDDROP;
    DWORD dwRotationAngle;
    DWORD dwZBufferOpCode;
    DWORD dwZBufferLow;
    DWORD dwZBufferHigh;
    DWORD dwZBufferBaseDest;
    DWORD dwZDestConstBitDepth;
    union {
        DWORD dwZDestConst;
        LPDIRECTDRAWSURFACE7 lpDDSZBufferDest;
    };
    DWORD dwZSrcConstBitDepth;
    union {
        DWORD dwZSrcConst;
        LPDIRECTDRAWSURFACE7 lpDDSZBufferSrc;
    };
    DWORD dwAlphaEdgeBlendBitDepth;
    DWORD dwAlphaEdgeBlend;
    DWORD dwReserved;
    DWORD dwAlphaDestConstBitDepth;
    union {
        DWORD dwAlphaDestConst;
        LPDIRECTDRAWSURFACE7 lpDDSAlphaDest;
    };
    DWORD dwAlphaSrcConstBitDepth;
    union {
        DWORD dwAlphaSrcConst;
        LPDIRECTDRAWSURFACE7 lpDDSAlphaSrc;
    };
    union {
        DWORD dwFillColor;
        DWORD dwFillDepth;
        DWORD dwFillPixel;
        LPDIRECTDRAWSURFACE7 lpDDSPattern;
    };
    DDCOLORKEY ddckDestColorkey;
    DDCOLORKEY ddckSrcColorkey;
} DDBLTFX, *LPDDBLTFX;

typedef struct _DDOVERLAYFX {
    DWORD dwSize;
    DWORD dwAlphaEdgeBlendBitDepth;
    DWORD dwAlphaEdgeBlend;
    DWORD dwReserved;
    DWORD dwAlphaDestConstBitDepth;
    DWORD dwAlphaDestConst;
    DWORD dwAlphaSrcConstBitDepth;
    DWORD dwAlphaSrcConst;
    DDCOLORKEY dckDestColorkey;
    DDCOLORKEY dckSrcColorkey;
    DWORD dwDDFX;
    DWORD dwFlags;
} DDOVERLAYFX, *LPDDOVERLAYFX;

typedef struct _DDCAPS_DX7 {
    DWORD dwSize;
    DWORD dwCaps;
    DWORD dwCaps2;
    DWORD dwCKeyCaps;
    DWORD dwFXCaps;
    DWORD dwFXAlphaCaps;
    DWORD dwPalCaps;
    DWORD dwSVCaps;
    DWORD dwAlphaBltConstBitDepths;
    DWORD dwAlphaBltPixelBitDepths;
    DWORD dwAlphaBltSurfaceBitDepths;
    DWORD dwAlphaOverlayConstBitDepths;
    DWORD dwAlphaOverlayPixelBitDepths;
    DWORD dwAlphaOverlaySurfaceBitDepths;
    DWORD dwZBufferBitDepths;
    DWORD dwVidMemTotal;
    DWORD dwVidMemFree;
    DWORD dwMaxVisibleOverlays;
    DWORD dwCurrVisibleOverlays;
    DWORD dwNumFourCCCodes;
    DWORD dwAlignBoundarySrc;
    DWORD dwAlignSizeSrc;
    DWORD dwAlignBoundaryDest;
    DWORD dwAlignSizeDest;
    DWORD dwAlignStrideAlign;
    DWORD dwRops[8];
    DDSCAPS ddsOldCaps;
    DWORD dwMinOverlayStretch;
    DWORD dwMaxOverlayStretch;
    DWORD dwMinLiveVideoStretch;
    DWORD dwMaxLiveVideoStretch;
    DWORD dwMinHwCodecStretch;
    DWORD dwMaxHwCodecStretch;
    DWORD dwReserved1;
    DWORD dwReserved2;
    DWORD dwReserved3;
    DWORD dwSVBCaps;
    DWORD dwSVBCKeyCaps;
    DWORD dwSVBFXCaps;
    DWORD dwSVBRops[8];
    DWORD dwVSBCaps;
    DWORD dwVSBCKeyCaps;
    DWORD dwVSBFXCaps;
    DWORD dwVSBRops[8];
    DWORD dwSSBCaps;
    DWORD dwSSBCKeyCaps;
    DWORD dwSSBFXCaps;
    DWORD dwSSBRops[8];
    DWORD dwMaxVideoPorts;
    DWORD dwCurrVideoPorts;
    DWORD dwSVBCaps2;
    DWORD dwNLVBCaps;
    DWORD dwNLVBCaps2;
    DWORD dwNLVBCKeyCaps;
    DWORD dwNLVBFXCaps;
    DWORD dwNLVBRops[8];
    DDSCAPS2 ddsCaps;
} DDCAPS, DDCAPS_DX7, *LPDDCAPS;

typedef struct tagDDDEVICEIDENTIFIER2 {
    char szDriver[512];
    char szDescription[512];
    LARGE_INTEGER liDriverVersion;
    DWORD dwVendorId;
    DWORD dwDeviceId;
    DWORD dwSubSysId;
    DWORD dwRevision;
    GUID  guidDeviceIdentifier;
    DWORD dwWHQLLevel;
} DDDEVICEIDENTIFIER2, *LPDDDEVICEIDENTIFIER2;

typedef struct _DDBLTBATCH {
    LPRECT lprDest;
    LPDIRECTDRAWSURFACE7 lpDDSSrc;
    LPRECT lprSrc;
    DWORD dwFlags;
    LPDDBLTFX lpDDBltFx;
} DDBLTBATCH;
typedef void *LPDDBLTBATCH;  /* matches the legacy void* usage in d3d_gl.cpp */

typedef struct _RGNDATAHEADER {
    DWORD dwSize;
    DWORD iType;
    DWORD nCount;
    DWORD nRgnSize;
    RECT  rcBound;
} RGNDATAHEADER;

typedef struct _RGNDATA {
    RGNDATAHEADER rdh;
    char Buffer[1];
} RGNDATA, *PRGNDATA, *LPRGNDATA;

/* On-disk DDS file header: the 124-byte 32-bit DDSURFACEDESC2 layout.
 * Used instead of DDSURFACEDESC2 for fread() on 64-bit Linux where
 * LPVOID lpSurface would change the struct size. */
typedef struct _DDS_FILE_HEADER {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwHeight;
    DWORD dwWidth;
    union {
        LONG  lPitch;
        DWORD dwLinearSize;
    };
    union {
        DWORD dwBackBufferCount;
        DWORD dwDepth;
    };
    union {
        DWORD dwMipMapCount;
        DWORD dwRefreshRate;
    };
    DWORD dwAlphaBitDepth;
    DWORD dwReserved;
    DWORD lpSurface32;          /* 32-bit pointer slot in the file */
    DDCOLORKEY ddckCKDestOverlay;
    DDCOLORKEY ddckCKDestBlt;
    DDCOLORKEY ddckCKSrcOverlay;
    DDCOLORKEY ddckCKSrcBlt;
    DDPIXELFORMAT ddpfPixelFormat;
    DDSCAPS2 ddsCaps;
    DWORD dwTextureStage;
} DDS_FILE_HEADER;

/* ============================================================
 * Flags / caps constants (real DX7 values)
 * ============================================================ */

/* DDSURFACEDESC2.dwFlags */
#define DDSD_CAPS               0x00000001
#define DDSD_HEIGHT             0x00000002
#define DDSD_WIDTH              0x00000004
#define DDSD_PITCH              0x00000008
#define DDSD_BACKBUFFERCOUNT    0x00000020
#define DDSD_ZBUFFERBITDEPTH    0x00000040
#define DDSD_ALPHABITDEPTH      0x00000080
#define DDSD_LPSURFACE          0x00000800
#define DDSD_PIXELFORMAT        0x00001000
#define DDSD_CKDESTOVERLAY      0x00002000
#define DDSD_CKDESTBLT          0x00004000
#define DDSD_CKSRCOVERLAY       0x00008000
#define DDSD_CKSRCBLT           0x00010000
#define DDSD_MIPMAPCOUNT        0x00020000
#define DDSD_REFRESHRATE        0x00040000
#define DDSD_LINEARSIZE         0x00080000
#define DDSD_TEXTURESTAGE       0x00100000
#define DDSD_FVF                0x00200000
#define DDSD_SRCVBHANDLE        0x00400000
#define DDSD_DEPTH              0x00800000
#define DDSD_ALL                0x00FFF9EE

/* DDSCAPS.dwCaps */
#define DDSCAPS_RESERVED1       0x00000001
#define DDSCAPS_ALPHA           0x00000002
#define DDSCAPS_BACKBUFFER      0x00000004
#define DDSCAPS_COMPLEX         0x00000008
#define DDSCAPS_FLIP            0x00000010
#define DDSCAPS_FRONTBUFFER     0x00000020
#define DDSCAPS_OFFSCREENPLAIN  0x00000040
#define DDSCAPS_OVERLAY         0x00000080
#define DDSCAPS_PALETTE         0x00000100
#define DDSCAPS_PRIMARYSURFACE  0x00000200
#define DDSCAPS_SYSTEMMEMORY    0x00000800
#define DDSCAPS_TEXTURE         0x00001000
#define DDSCAPS_3DDEVICE        0x00002000
#define DDSCAPS_VIDEOMEMORY     0x00004000
#define DDSCAPS_VISIBLE         0x00008000
#define DDSCAPS_WRITEONLY       0x00010000
#define DDSCAPS_ZBUFFER         0x00020000
#define DDSCAPS_OWNDC           0x00040000
#define DDSCAPS_LIVEVIDEO       0x00080000
#define DDSCAPS_HWCODEC         0x00100000
#define DDSCAPS_MODEX           0x00200000
#define DDSCAPS_MIPMAP          0x00400000
#define DDSCAPS_ALLOCONLOAD     0x04000000
#define DDSCAPS_VIDEOPORT       0x08000000
#define DDSCAPS_LOCALVIDMEM     0x10000000
#define DDSCAPS_NONLOCALVIDMEM  0x20000000
#define DDSCAPS_STANDARDVGAMODE 0x40000000
#define DDSCAPS_OPTIMIZED       0x80000000

/* DDSCAPS2.dwCaps2 */
#define DDSCAPS2_HARDWAREDEINTERLACE 0x00000002
#define DDSCAPS2_HINTDYNAMIC         0x00000004
#define DDSCAPS2_HINTSTATIC          0x00000008
#define DDSCAPS2_TEXTUREMANAGE       0x00000010
#define DDSCAPS2_OPAQUE              0x00000080
#define DDSCAPS2_HINTANTIALIASING    0x00000100
#define DDSCAPS2_CUBEMAP             0x00000200
#define DDSCAPS2_MIPMAPSUBLEVEL      0x00010000
#define DDSCAPS2_D3DTEXTUREMANAGE    0x00020000
#define DDSCAPS2_DONOTPERSIST        0x00040000
#define DDSCAPS2_STEREOSURFACELEFT   0x00080000

/* DDPIXELFORMAT.dwFlags */
#define DDPF_ALPHAPIXELS        0x00000001
#define DDPF_ALPHA              0x00000002
#define DDPF_FOURCC             0x00000004
#define DDPF_PALETTEINDEXED4    0x00000008
#define DDPF_PALETTEINDEXEDTO8  0x00000010
#define DDPF_PALETTEINDEXED8    0x00000020
#define DDPF_RGB                0x00000040
#define DDPF_COMPRESSED         0x00000080
#define DDPF_RGBTOYUV           0x00000100
#define DDPF_YUV                0x00000200
#define DDPF_ZBUFFER            0x00000400
#define DDPF_PALETTEINDEXED1    0x00000800
#define DDPF_PALETTEINDEXED2    0x00001000
#define DDPF_ZPIXELS            0x00002000
#define DDPF_STENCILBUFFER      0x00004000
#define DDPF_LUMINANCE          0x00020000
#define DDPF_BUMPLUMINANCE      0x00040000
#define DDPF_BUMPDUDV           0x00080000

/* DDCAPS.dwCaps */
#define DDCAPS_3D               0x00000001
#define DDCAPS_ALIGNBOUNDARYDEST 0x00000002
#define DDCAPS_BLT              0x00000040
#define DDCAPS_BLTQUEUE         0x00000080
#define DDCAPS_BLTFOURCC        0x00000100
#define DDCAPS_BLTSTRETCH       0x00000200
#define DDCAPS_GDI              0x00000400
#define DDCAPS_OVERLAY          0x00000800
#define DDCAPS_PALETTE          0x00100000
#define DDCAPS_BLTCOLORFILL     0x04000000
#define DDCAPS_BANKSWITCHED     0x08000000
#define DDCAPS_BLTDEPTHFILL     0x10000000
#define DDCAPS_CANCLIP          0x20000000
#define DDCAPS_COLORKEY         0x00400000
#define DDCAPS_NOHARDWARE       0x00080000
#define DDCKEYCAPS_DESTBLT          0x00000001
#define DDCKEYCAPS_SRCBLT           0x00000040
#define DDCKEYCAPS_DESTBLTCLRSPACE  0x00000002
#define DDCKEYCAPS_SRCBLTCLRSPACE   0x00000080
/* Bit depth flags */
#define DDBD_1  0x00004000
#define DDBD_2  0x00002000
#define DDBD_4  0x00001000
#define DDBD_8  0x00000800
#define DDBD_16 0x00000400
#define DDBD_24 0x00000200
#define DDBD_32 0x00000100

/* DDCAPS.dwCaps2 */
#define DDCAPS2_CERTIFIED            0x00000001
#define DDCAPS2_NO2DDURING3DSCENE    0x00000002
#define DDCAPS2_VIDEOPORT            0x00000004
#define DDCAPS2_WIDESURFACES         0x00001000
#define DDCAPS2_CANRENDERWINDOWED    0x00080000
#define DDCAPS2_CANMANAGETEXTURE     0x00800000

/* IDirectDrawSurface7::Blt flags */
#define DDBLT_ALPHADEST                 0x00000001
#define DDBLT_ASYNC                     0x00000200
#define DDBLT_COLORFILL                 0x00000400
#define DDBLT_DDFX                      0x00000800
#define DDBLT_DDROPS                    0x00001000
#define DDBLT_KEYDEST                   0x00002000
#define DDBLT_KEYSRC                    0x00008000
#define DDBLT_KEYDESTOVERRIDE           0x00004000
#define DDBLT_KEYSRCOVERRIDE            0x00010000
#define DDBLT_ROP                       0x00020000
#define DDBLT_ROTATIONANGLE             0x00040000
#define DDBLT_ZBUFFER                   0x00080000
#define DDBLT_WAIT                      0x01000000
#define DDBLT_DEPTHFILL                 0x02000000
#define DDBLT_DONOTWAIT                 0x08000000

/* IDirectDrawSurface7::BltFast flags */
#define DDBLTFAST_NOCOLORKEY            0x00000000
#define DDBLTFAST_SRCCOLORKEY           0x00000001
#define DDBLTFAST_DESTCOLORKEY          0x00000002
#define DDBLTFAST_WAIT                  0x00000010
#define DDBLTFAST_DONOTWAIT             0x00000020

/* SetColorKey / GetColorKey flags */
#define DDCKEY_COLORSPACE               0x00000001
#define DDCKEY_DESTBLT                  0x00000002
#define DDCKEY_DESTOVERLAY              0x00000004
#define DDCKEY_SRCBLT                   0x00000008
#define DDCKEY_SRCOVERLAY               0x00000010

/* Lock flags */
#define DDLOCK_SURFACEMEMORYPTR         0x00000000
#define DDLOCK_WAIT                     0x00000001
#define DDLOCK_EVENT                    0x00000002
#define DDLOCK_READONLY                 0x00000010
#define DDLOCK_WRITEONLY                0x00000020
#define DDLOCK_NOSYSLOCK                0x00000800
#define DDLOCK_NOOVERWRITE              0x00001000
#define DDLOCK_DISCARDCONTENTS          0x00002000
#define DDLOCK_DONOTWAIT                0x00004000

/* SetCooperativeLevel flags */
#define DDSCL_FULLSCREEN                0x00000001
#define DDSCL_ALLOWREBOOT               0x00000002
#define DDSCL_NOWINDOWCHANGES           0x00000004
#define DDSCL_NORMAL                    0x00000008
#define DDSCL_EXCLUSIVE                 0x00000010
#define DDSCL_ALLOWMODEX                0x00000040
#define DDSCL_SETFOCUSWINDOW            0x00000080
#define DDSCL_SETDEVICEWINDOW           0x00000100
#define DDSCL_CREATEDEVICEWINDOW        0x00000200
#define DDSCL_MULTITHREADED             0x00000400
#define DDSCL_FPUSETUP                  0x00000800
#define DDSCL_FPUPRESERVE               0x00001000

/* Flip flags */
#define DDFLIP_WAIT                     0x00000001
#define DDFLIP_EVEN                     0x00000002
#define DDFLIP_ODD                      0x00000004
#define DDFLIP_NOVSYNC                  0x00000008
#define DDFLIP_INTERVAL2                0x02000000
#define DDFLIP_INTERVAL3                0x03000000
#define DDFLIP_INTERVAL4                0x04000000
#define DDFLIP_STEREO                   0x00000010
#define DDFLIP_DONOTWAIT                0x00000020

/* Palette caps */
#define DDPCAPS_4BIT                    0x00000001
#define DDPCAPS_8BITENTRIES             0x00000002
#define DDPCAPS_8BIT                    0x00000004
#define DDPCAPS_INITIALIZE              0x00000008
#define DDPCAPS_PRIMARYSURFACE          0x00000010
#define DDPCAPS_PRIMARYSURFACELEFT      0x00000020
#define DDPCAPS_ALLOW256                0x00000040
#define DDPCAPS_VSYNC                   0x00000080
#define DDPCAPS_1BIT                    0x00000100
#define DDPCAPS_2BIT                    0x00000200
#define DDPCAPS_ALPHA                   0x00000400

/* EnumDisplayModes flags */
#define DDEDM_REFRESHRATES              0x00000001
#define DDEDM_STANDARDVGAMODES          0x00000002

/* EnumSurfaces flags */
#define DDENUMSURFACES_ALL              0x00000001
#define DDENUMSURFACES_MATCH            0x00000002
#define DDENUMSURFACES_NOMATCH          0x00000004
#define DDENUMSURFACES_CANBECREATED     0x00000008
#define DDENUMSURFACES_DOESEXIST        0x00000010

/* GetBltStatus / GetFlipStatus flags */
#define DDGBS_CANBLT                    0x00000001
#define DDGBS_ISBLTDONE                 0x00000002
#define DDGFS_CANFLIP                   0x00000001
#define DDGFS_ISFLIPDONE                0x00000002

/* WaitForVerticalBlank flags */
#define DDWAITVB_BLOCKBEGIN             0x00000001
#define DDWAITVB_BLOCKBEGINEVENT        0x00000002
#define DDWAITVB_BLOCKEND               0x00000004

/* ============================================================
 * Callback typedefs
 * ============================================================ */
typedef HRESULT (CALLBACK *LPDDENUMMODESCALLBACK2)(LPDDSURFACEDESC2, LPVOID);
typedef HRESULT (CALLBACK *LPDDENUMSURFACESCALLBACK7)(LPDIRECTDRAWSURFACE7, LPDDSURFACEDESC2, LPVOID);
typedef BOOL    (CALLBACK *LPDDENUMCALLBACKA)(GUID *, LPSTR, LPSTR, LPVOID);
typedef BOOL    (CALLBACK *LPDDENUMCALLBACKEXA)(GUID *, LPSTR, LPSTR, LPVOID, HMONITOR);
typedef LPDDENUMCALLBACKA LPDDENUMCALLBACK;

/* ============================================================
 * Interface GUIDs (header-local copies)
 * ============================================================ */
#ifdef __cplusplus
static const GUID IID_IDirectDraw7 =
    { 0x15e65ec0, 0x3b9c, 0x11d2, { 0xb9, 0x2f, 0x00, 0x60, 0x97, 0x97, 0xea, 0x5b } };
static const GUID IID_IDirectDrawSurface7 =
    { 0x06675a80, 0x3b9b, 0x11d2, { 0xb9, 0x2f, 0x00, 0x60, 0x97, 0x97, 0xea, 0x5b } };
#endif

/* ============================================================
 * IDirectDrawClipper
 * ============================================================ */
struct IDirectDrawClipper;
typedef struct IDirectDrawClipperVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IDirectDrawClipper *This, REFIID riid, LPVOID *ppvObj);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IDirectDrawClipper *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IDirectDrawClipper *This);
    HRESULT (STDMETHODCALLTYPE *GetClipList)(IDirectDrawClipper *This, LPRECT lpRect, LPVOID lpClipList, LPDWORD lpdwSize);
    HRESULT (STDMETHODCALLTYPE *GetHWnd)(IDirectDrawClipper *This, HWND *lphWnd);
    HRESULT (STDMETHODCALLTYPE *Initialize)(IDirectDrawClipper *This, LPDIRECTDRAW lpDD, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *IsClipListChanged)(IDirectDrawClipper *This, LPBOOL lpbChanged);
    HRESULT (STDMETHODCALLTYPE *SetClipList)(IDirectDrawClipper *This, LPVOID lpClipList, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *SetHWnd)(IDirectDrawClipper *This, DWORD dwFlags, HWND hWnd);
} IDirectDrawClipperVtbl;

struct IDirectDrawClipper {
    IDirectDrawClipperVtbl *lpVtbl;
#ifdef __cplusplus
    HRESULT QueryInterface(REFIID riid, LPVOID *ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG   AddRef()  { return lpVtbl->AddRef(this); }
    ULONG   Release() { return lpVtbl->Release(this); }
    HRESULT GetClipList(LPRECT a, LPVOID b, LPDWORD c) { return lpVtbl->GetClipList(this, a, b, c); }
    HRESULT GetHWnd(HWND *a) { return lpVtbl->GetHWnd(this, a); }
    HRESULT Initialize(LPDIRECTDRAW a, DWORD b) { return lpVtbl->Initialize(this, a, b); }
    HRESULT IsClipListChanged(LPBOOL a) { return lpVtbl->IsClipListChanged(this, a); }
    HRESULT SetClipList(LPVOID a, DWORD b) { return lpVtbl->SetClipList(this, a, b); }
    HRESULT SetHWnd(DWORD a, HWND b) { return lpVtbl->SetHWnd(this, a, b); }
#endif
};

/* ============================================================
 * IDirectDrawPalette
 * ============================================================ */
struct IDirectDrawPalette;
typedef struct IDirectDrawPaletteVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IDirectDrawPalette *This, REFIID riid, LPVOID *ppvObj);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IDirectDrawPalette *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IDirectDrawPalette *This);
    HRESULT (STDMETHODCALLTYPE *GetCaps)(IDirectDrawPalette *This, LPDWORD lpdwCaps);
    HRESULT (STDMETHODCALLTYPE *GetEntries)(IDirectDrawPalette *This, DWORD dwFlags, DWORD dwBase, DWORD dwNumEntries, LPPALETTEENTRY lpEntries);
    HRESULT (STDMETHODCALLTYPE *Initialize)(IDirectDrawPalette *This, LPDIRECTDRAW lpDD, DWORD dwFlags, LPPALETTEENTRY lpDDColorTable);
    HRESULT (STDMETHODCALLTYPE *SetEntries)(IDirectDrawPalette *This, DWORD dwFlags, DWORD dwStartingEntry, DWORD dwCount, LPPALETTEENTRY lpEntries);
} IDirectDrawPaletteVtbl;

struct IDirectDrawPalette {
    IDirectDrawPaletteVtbl *lpVtbl;
#ifdef __cplusplus
    HRESULT QueryInterface(REFIID riid, LPVOID *ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG   AddRef()  { return lpVtbl->AddRef(this); }
    ULONG   Release() { return lpVtbl->Release(this); }
    HRESULT GetCaps(LPDWORD a) { return lpVtbl->GetCaps(this, a); }
    HRESULT GetEntries(DWORD a, DWORD b, DWORD c, LPPALETTEENTRY d) { return lpVtbl->GetEntries(this, a, b, c, d); }
    HRESULT Initialize(LPDIRECTDRAW a, DWORD b, LPPALETTEENTRY c) { return lpVtbl->Initialize(this, a, b, c); }
    HRESULT SetEntries(DWORD a, DWORD b, DWORD c, LPPALETTEENTRY d) { return lpVtbl->SetEntries(this, a, b, c, d); }
#endif
};

/* ============================================================
 * IDirectDrawSurface7
 * ============================================================ */
struct IDirectDrawSurface7;
typedef struct IDirectDrawSurface7Vtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IDirectDrawSurface7 *This, REFIID riid, void **ppvObject);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IDirectDrawSurface7 *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IDirectDrawSurface7 *This);
    HRESULT (STDMETHODCALLTYPE *AddAttachedSurface)(IDirectDrawSurface7 *This, IDirectDrawSurface7 *pDDS);
    HRESULT (STDMETHODCALLTYPE *AddOverlayDirtyRect)(IDirectDrawSurface7 *This, LPRECT pRect);
    HRESULT (STDMETHODCALLTYPE *Blt)(IDirectDrawSurface7 *This, LPRECT lpDestRect, IDirectDrawSurface7 *lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx);
    HRESULT (STDMETHODCALLTYPE *BltBatch)(IDirectDrawSurface7 *This, LPDDBLTBATCH lpDDBltBatch, DWORD dwCount, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *BltFast)(IDirectDrawSurface7 *This, DWORD dwX, DWORD dwY, IDirectDrawSurface7 *lpDDSrcSurface, LPRECT lpSrcRect, DWORD dwTrans);
    HRESULT (STDMETHODCALLTYPE *DeleteAttachedSurface)(IDirectDrawSurface7 *This, DWORD dwFlags, IDirectDrawSurface7 *lpDDSAttachedSurface);
    HRESULT (STDMETHODCALLTYPE *EnumAttachedSurfaces)(IDirectDrawSurface7 *This, LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpEnumSurfacesCallback);
    HRESULT (STDMETHODCALLTYPE *EnumOverlayZOrders)(IDirectDrawSurface7 *This, DWORD dwFlags, LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpfnCallback);
    HRESULT (STDMETHODCALLTYPE *Flip)(IDirectDrawSurface7 *This, IDirectDrawSurface7 *lpDDSurfaceTargetOverride, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetAttachedSurface)(IDirectDrawSurface7 *This, LPDDSCAPS2 lpDDSCaps, IDirectDrawSurface7 **lplpDDAttachedSurface);
    HRESULT (STDMETHODCALLTYPE *GetBltStatus)(IDirectDrawSurface7 *This, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetCaps)(IDirectDrawSurface7 *This, LPDDSCAPS2 lpDDSCaps);
    HRESULT (STDMETHODCALLTYPE *GetClipper)(IDirectDrawSurface7 *This, LPDIRECTDRAWCLIPPER *lplpDDClipper);
    HRESULT (STDMETHODCALLTYPE *GetColorKey)(IDirectDrawSurface7 *This, DWORD dwFlags, LPDDCOLORKEY lpDDColorKey);
    HRESULT (STDMETHODCALLTYPE *GetDC)(IDirectDrawSurface7 *This, HDC *lphDC);
    HRESULT (STDMETHODCALLTYPE *GetFlipStatus)(IDirectDrawSurface7 *This, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetOverlayPosition)(IDirectDrawSurface7 *This, LPLONG lplX, LPLONG lplY);
    HRESULT (STDMETHODCALLTYPE *GetPalette)(IDirectDrawSurface7 *This, LPDIRECTDRAWPALETTE *lplpDDPalette);
    HRESULT (STDMETHODCALLTYPE *GetPixelFormat)(IDirectDrawSurface7 *This, LPDDPIXELFORMAT lpDDPixelFormat);
    HRESULT (STDMETHODCALLTYPE *GetSurfaceDesc)(IDirectDrawSurface7 *This, LPDDSURFACEDESC2 lpDDSurfaceDesc);
    HRESULT (STDMETHODCALLTYPE *Initialize)(IDirectDrawSurface7 *This, LPDIRECTDRAW lpDD, LPDDSURFACEDESC2 lpDDSurfaceDesc);
    HRESULT (STDMETHODCALLTYPE *IsLost)(IDirectDrawSurface7 *This);
    HRESULT (STDMETHODCALLTYPE *Lock)(IDirectDrawSurface7 *This, LPRECT lpDestRect, LPDDSURFACEDESC2 lpDDSurfaceDesc, DWORD dwFlags, HANDLE hEvent);
    HRESULT (STDMETHODCALLTYPE *ReleaseDC)(IDirectDrawSurface7 *This, HDC hDC);
    HRESULT (STDMETHODCALLTYPE *Restore)(IDirectDrawSurface7 *This);
    HRESULT (STDMETHODCALLTYPE *SetClipper)(IDirectDrawSurface7 *This, LPDIRECTDRAWCLIPPER lpDDClipper);
    HRESULT (STDMETHODCALLTYPE *SetColorKey)(IDirectDrawSurface7 *This, DWORD dwFlags, LPDDCOLORKEY lpDDColorKey);
    HRESULT (STDMETHODCALLTYPE *SetOverlayPosition)(IDirectDrawSurface7 *This, LONG lX, LONG lY);
    HRESULT (STDMETHODCALLTYPE *SetPalette)(IDirectDrawSurface7 *This, LPDIRECTDRAWPALETTE lpDDPalette);
    HRESULT (STDMETHODCALLTYPE *Unlock)(IDirectDrawSurface7 *This, LPRECT lpRect);
    HRESULT (STDMETHODCALLTYPE *UpdateOverlay)(IDirectDrawSurface7 *This, LPRECT lpSrcRect, IDirectDrawSurface7 *lpDDDestSurface, LPRECT lpDestRect, DWORD dwFlags, LPDDOVERLAYFX lpDDOverlayFx);
    HRESULT (STDMETHODCALLTYPE *UpdateOverlayDisplay)(IDirectDrawSurface7 *This, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *UpdateOverlayZOrder)(IDirectDrawSurface7 *This, DWORD dwFlags, IDirectDrawSurface7 *lpDDSReference);
    HRESULT (STDMETHODCALLTYPE *GetDDInterface)(IDirectDrawSurface7 *This, LPVOID *lplpDD);
    HRESULT (STDMETHODCALLTYPE *PageLock)(IDirectDrawSurface7 *This, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *PageUnlock)(IDirectDrawSurface7 *This, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *SetSurfaceDesc)(IDirectDrawSurface7 *This, LPDDSURFACEDESC2 lpDDsd2, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *SetPrivateData)(IDirectDrawSurface7 *This, REFGUID guidTag, LPVOID lpData, DWORD cbSize, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetPrivateData)(IDirectDrawSurface7 *This, REFGUID guidTag, LPVOID lpBuffer, LPDWORD lpcbBufferSize);
    HRESULT (STDMETHODCALLTYPE *FreePrivateData)(IDirectDrawSurface7 *This, REFGUID guidTag);
    HRESULT (STDMETHODCALLTYPE *GetUniquenessValue)(IDirectDrawSurface7 *This, LPDWORD lpValue);
    HRESULT (STDMETHODCALLTYPE *ChangeUniquenessValue)(IDirectDrawSurface7 *This);
    HRESULT (STDMETHODCALLTYPE *SetPriority)(IDirectDrawSurface7 *This, DWORD dwPriority);
    HRESULT (STDMETHODCALLTYPE *GetPriority)(IDirectDrawSurface7 *This, LPDWORD lpdwPriority);
    HRESULT (STDMETHODCALLTYPE *SetLOD)(IDirectDrawSurface7 *This, DWORD dwMaxLOD);
    HRESULT (STDMETHODCALLTYPE *GetLOD)(IDirectDrawSurface7 *This, LPDWORD lpdwMaxLOD);
} IDirectDrawSurface7Vtbl;

struct IDirectDrawSurface7 {
    IDirectDrawSurface7Vtbl *lpVtbl;
#ifdef __cplusplus
    HRESULT QueryInterface(REFIID riid, void **ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG   AddRef()  { return lpVtbl->AddRef(this); }
    ULONG   Release() { return lpVtbl->Release(this); }
    HRESULT AddAttachedSurface(IDirectDrawSurface7 *a) { return lpVtbl->AddAttachedSurface(this, a); }
    HRESULT AddOverlayDirtyRect(LPRECT a) { return lpVtbl->AddOverlayDirtyRect(this, a); }
    HRESULT Blt(LPRECT a, IDirectDrawSurface7 *b, LPRECT c, DWORD d, LPDDBLTFX e) { return lpVtbl->Blt(this, a, b, c, d, e); }
    HRESULT BltBatch(LPDDBLTBATCH a, DWORD b, DWORD c) { return lpVtbl->BltBatch(this, a, b, c); }
    HRESULT BltFast(DWORD a, DWORD b, IDirectDrawSurface7 *c, LPRECT d, DWORD e) { return lpVtbl->BltFast(this, a, b, c, d, e); }
    HRESULT DeleteAttachedSurface(DWORD a, IDirectDrawSurface7 *b) { return lpVtbl->DeleteAttachedSurface(this, a, b); }
    HRESULT EnumAttachedSurfaces(LPVOID a, LPDDENUMSURFACESCALLBACK7 b) { return lpVtbl->EnumAttachedSurfaces(this, a, b); }
    HRESULT EnumOverlayZOrders(DWORD a, LPVOID b, LPDDENUMSURFACESCALLBACK7 c) { return lpVtbl->EnumOverlayZOrders(this, a, b, c); }
    HRESULT Flip(IDirectDrawSurface7 *a, DWORD b) { return lpVtbl->Flip(this, a, b); }
    HRESULT GetAttachedSurface(LPDDSCAPS2 a, IDirectDrawSurface7 **b) { return lpVtbl->GetAttachedSurface(this, a, b); }
    HRESULT GetBltStatus(DWORD a) { return lpVtbl->GetBltStatus(this, a); }
    HRESULT GetCaps(LPDDSCAPS2 a) { return lpVtbl->GetCaps(this, a); }
    HRESULT GetClipper(LPDIRECTDRAWCLIPPER *a) { return lpVtbl->GetClipper(this, a); }
    HRESULT GetColorKey(DWORD a, LPDDCOLORKEY b) { return lpVtbl->GetColorKey(this, a, b); }
    HRESULT GetDC(HDC *a) { return lpVtbl->GetDC(this, a); }
    HRESULT GetFlipStatus(DWORD a) { return lpVtbl->GetFlipStatus(this, a); }
    HRESULT GetOverlayPosition(LPLONG a, LPLONG b) { return lpVtbl->GetOverlayPosition(this, a, b); }
    HRESULT GetPalette(LPDIRECTDRAWPALETTE *a) { return lpVtbl->GetPalette(this, a); }
    HRESULT GetPixelFormat(LPDDPIXELFORMAT a) { return lpVtbl->GetPixelFormat(this, a); }
    HRESULT GetSurfaceDesc(LPDDSURFACEDESC2 a) { return lpVtbl->GetSurfaceDesc(this, a); }
    HRESULT Initialize(LPDIRECTDRAW a, LPDDSURFACEDESC2 b) { return lpVtbl->Initialize(this, a, b); }
    HRESULT IsLost() { return lpVtbl->IsLost(this); }
    HRESULT Lock(LPRECT a, LPDDSURFACEDESC2 b, DWORD c, HANDLE d) { return lpVtbl->Lock(this, a, b, c, d); }
    HRESULT ReleaseDC(HDC a) { return lpVtbl->ReleaseDC(this, a); }
    HRESULT Restore() { return lpVtbl->Restore(this); }
    HRESULT SetClipper(LPDIRECTDRAWCLIPPER a) { return lpVtbl->SetClipper(this, a); }
    HRESULT SetColorKey(DWORD a, LPDDCOLORKEY b) { return lpVtbl->SetColorKey(this, a, b); }
    HRESULT SetOverlayPosition(LONG a, LONG b) { return lpVtbl->SetOverlayPosition(this, a, b); }
    HRESULT SetPalette(LPDIRECTDRAWPALETTE a) { return lpVtbl->SetPalette(this, a); }
    HRESULT Unlock(LPRECT a) { return lpVtbl->Unlock(this, a); }
    HRESULT UpdateOverlay(LPRECT a, IDirectDrawSurface7 *b, LPRECT c, DWORD d, LPDDOVERLAYFX e) { return lpVtbl->UpdateOverlay(this, a, b, c, d, e); }
    HRESULT UpdateOverlayDisplay(DWORD a) { return lpVtbl->UpdateOverlayDisplay(this, a); }
    HRESULT UpdateOverlayZOrder(DWORD a, IDirectDrawSurface7 *b) { return lpVtbl->UpdateOverlayZOrder(this, a, b); }
    HRESULT GetDDInterface(LPVOID *a) { return lpVtbl->GetDDInterface(this, a); }
    HRESULT PageLock(DWORD a) { return lpVtbl->PageLock(this, a); }
    HRESULT PageUnlock(DWORD a) { return lpVtbl->PageUnlock(this, a); }
    HRESULT SetSurfaceDesc(LPDDSURFACEDESC2 a, DWORD b) { return lpVtbl->SetSurfaceDesc(this, a, b); }
    HRESULT SetPrivateData(REFGUID a, LPVOID b, DWORD c, DWORD d) { return lpVtbl->SetPrivateData(this, a, b, c, d); }
    HRESULT GetPrivateData(REFGUID a, LPVOID b, LPDWORD c) { return lpVtbl->GetPrivateData(this, a, b, c); }
    HRESULT FreePrivateData(REFGUID a) { return lpVtbl->FreePrivateData(this, a); }
    HRESULT GetUniquenessValue(LPDWORD a) { return lpVtbl->GetUniquenessValue(this, a); }
    HRESULT ChangeUniquenessValue() { return lpVtbl->ChangeUniquenessValue(this); }
    HRESULT SetPriority(DWORD a) { return lpVtbl->SetPriority(this, a); }
    HRESULT GetPriority(LPDWORD a) { return lpVtbl->GetPriority(this, a); }
    HRESULT SetLOD(DWORD a) { return lpVtbl->SetLOD(this, a); }
    HRESULT GetLOD(LPDWORD a) { return lpVtbl->GetLOD(this, a); }
#endif
};

/* ============================================================
 * IDirectDraw7
 * ============================================================ */
struct IDirectDraw7;
typedef struct IDirectDraw7Vtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IDirectDraw7 *This, REFIID riid, void **ppvObject);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IDirectDraw7 *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IDirectDraw7 *This);
    HRESULT (STDMETHODCALLTYPE *Compact)(IDirectDraw7 *This);
    HRESULT (STDMETHODCALLTYPE *CreateClipper)(IDirectDraw7 *This, DWORD dwFlags, LPDIRECTDRAWCLIPPER *lplpDDClipper, IUnknown *pUnkOuter);
    HRESULT (STDMETHODCALLTYPE *CreatePalette)(IDirectDraw7 *This, DWORD dwFlags, LPPALETTEENTRY lpDDColorArray, LPDIRECTDRAWPALETTE *lplpDDPalette, IUnknown *pUnkOuter);
    HRESULT (STDMETHODCALLTYPE *CreateSurface)(IDirectDraw7 *This, LPDDSURFACEDESC2 lpDDSurfaceDesc, IDirectDrawSurface7 **lplpDDSurface, IUnknown *pUnkOuter);
    HRESULT (STDMETHODCALLTYPE *DuplicateSurface)(IDirectDraw7 *This, IDirectDrawSurface7 *lpDDSurface, IDirectDrawSurface7 **lplpDupDDSurface);
    HRESULT (STDMETHODCALLTYPE *EnumDisplayModes)(IDirectDraw7 *This, DWORD dwFlags, LPDDSURFACEDESC2 lpDDSurfaceDesc, LPVOID lpContext, LPDDENUMMODESCALLBACK2 lpEnumModesCallback);
    HRESULT (STDMETHODCALLTYPE *EnumSurfaces)(IDirectDraw7 *This, DWORD dwFlags, LPDDSURFACEDESC2 lpDDSD, LPVOID lpContext, LPDDENUMSURFACESCALLBACK7 lpEnumSurfacesCallback);
    HRESULT (STDMETHODCALLTYPE *FlipToGDISurface)(IDirectDraw7 *This);
    HRESULT (STDMETHODCALLTYPE *GetCaps)(IDirectDraw7 *This, LPDDCAPS lpDDDriverCaps, LPDDCAPS lpDDHELCaps);
    HRESULT (STDMETHODCALLTYPE *GetDisplayMode)(IDirectDraw7 *This, LPDDSURFACEDESC2 lpDDSurfaceDesc);
    HRESULT (STDMETHODCALLTYPE *GetFourCCCodes)(IDirectDraw7 *This, LPDWORD lpNumCodes, LPDWORD lpCodes);
    HRESULT (STDMETHODCALLTYPE *GetGDISurface)(IDirectDraw7 *This, IDirectDrawSurface7 **lplpGDIDDSSurface);
    HRESULT (STDMETHODCALLTYPE *GetMonitorFrequency)(IDirectDraw7 *This, LPDWORD lpdwFrequency);
    HRESULT (STDMETHODCALLTYPE *GetScanLine)(IDirectDraw7 *This, LPDWORD lpdwScanLine);
    HRESULT (STDMETHODCALLTYPE *GetVerticalBlankStatus)(IDirectDraw7 *This, LPBOOL lpbIsInVB);
    HRESULT (STDMETHODCALLTYPE *Initialize)(IDirectDraw7 *This, GUID *lpGUID);
    HRESULT (STDMETHODCALLTYPE *RestoreDisplayMode)(IDirectDraw7 *This);
    HRESULT (STDMETHODCALLTYPE *SetCooperativeLevel)(IDirectDraw7 *This, HWND hWnd, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *SetDisplayMode)(IDirectDraw7 *This, DWORD dwWidth, DWORD dwHeight, DWORD dwBPP, DWORD dwRefreshRate, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *WaitForVerticalBlank)(IDirectDraw7 *This, DWORD dwFlags, HANDLE hEvent);
    HRESULT (STDMETHODCALLTYPE *GetAvailableVidMem)(IDirectDraw7 *This, LPDDSCAPS2 lpDDSCaps2, LPDWORD lpdwTotal, LPDWORD lpdwFree);
    HRESULT (STDMETHODCALLTYPE *GetSurfaceFromDC)(IDirectDraw7 *This, HDC hdc, IDirectDrawSurface7 **lpDDS);
    HRESULT (STDMETHODCALLTYPE *RestoreAllSurfaces)(IDirectDraw7 *This);
    HRESULT (STDMETHODCALLTYPE *TestCooperativeLevel)(IDirectDraw7 *This);
    HRESULT (STDMETHODCALLTYPE *GetDeviceIdentifier)(IDirectDraw7 *This, LPDDDEVICEIDENTIFIER2 lpdddi, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *StartModeTest)(IDirectDraw7 *This, LPSIZE lpModesToTest, DWORD dwNumEntries, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *EvaluateMode)(IDirectDraw7 *This, DWORD dwFlags, DWORD *pSecondsUntilTimeout);
} IDirectDraw7Vtbl;

struct IDirectDraw7 {
    IDirectDraw7Vtbl *lpVtbl;
#ifdef __cplusplus
    HRESULT QueryInterface(REFIID riid, void **ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG   AddRef()  { return lpVtbl->AddRef(this); }
    ULONG   Release() { return lpVtbl->Release(this); }
    HRESULT Compact() { return lpVtbl->Compact(this); }
    HRESULT CreateClipper(DWORD a, LPDIRECTDRAWCLIPPER *b, IUnknown *c) { return lpVtbl->CreateClipper(this, a, b, c); }
    HRESULT CreatePalette(DWORD a, LPPALETTEENTRY b, LPDIRECTDRAWPALETTE *c, IUnknown *d) { return lpVtbl->CreatePalette(this, a, b, c, d); }
    HRESULT CreateSurface(LPDDSURFACEDESC2 a, IDirectDrawSurface7 **b, IUnknown *c) { return lpVtbl->CreateSurface(this, a, b, c); }
    HRESULT DuplicateSurface(IDirectDrawSurface7 *a, IDirectDrawSurface7 **b) { return lpVtbl->DuplicateSurface(this, a, b); }
    HRESULT EnumDisplayModes(DWORD a, LPDDSURFACEDESC2 b, LPVOID c, LPDDENUMMODESCALLBACK2 d) { return lpVtbl->EnumDisplayModes(this, a, b, c, d); }
    HRESULT EnumSurfaces(DWORD a, LPDDSURFACEDESC2 b, LPVOID c, LPDDENUMSURFACESCALLBACK7 d) { return lpVtbl->EnumSurfaces(this, a, b, c, d); }
    HRESULT FlipToGDISurface() { return lpVtbl->FlipToGDISurface(this); }
    HRESULT GetCaps(LPDDCAPS a, LPDDCAPS b) { return lpVtbl->GetCaps(this, a, b); }
    HRESULT GetDisplayMode(LPDDSURFACEDESC2 a) { return lpVtbl->GetDisplayMode(this, a); }
    HRESULT GetFourCCCodes(LPDWORD a, LPDWORD b) { return lpVtbl->GetFourCCCodes(this, a, b); }
    HRESULT GetGDISurface(IDirectDrawSurface7 **a) { return lpVtbl->GetGDISurface(this, a); }
    HRESULT GetMonitorFrequency(LPDWORD a) { return lpVtbl->GetMonitorFrequency(this, a); }
    HRESULT GetScanLine(LPDWORD a) { return lpVtbl->GetScanLine(this, a); }
    HRESULT GetVerticalBlankStatus(LPBOOL a) { return lpVtbl->GetVerticalBlankStatus(this, a); }
    HRESULT Initialize(GUID *a) { return lpVtbl->Initialize(this, a); }
    HRESULT RestoreDisplayMode() { return lpVtbl->RestoreDisplayMode(this); }
    HRESULT SetCooperativeLevel(HWND a, DWORD b) { return lpVtbl->SetCooperativeLevel(this, a, b); }
    HRESULT SetDisplayMode(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e) { return lpVtbl->SetDisplayMode(this, a, b, c, d, e); }
    HRESULT WaitForVerticalBlank(DWORD a, HANDLE b) { return lpVtbl->WaitForVerticalBlank(this, a, b); }
    HRESULT GetAvailableVidMem(LPDDSCAPS2 a, LPDWORD b, LPDWORD c) { return lpVtbl->GetAvailableVidMem(this, a, b, c); }
    HRESULT GetSurfaceFromDC(HDC a, IDirectDrawSurface7 **b) { return lpVtbl->GetSurfaceFromDC(this, a, b); }
    HRESULT RestoreAllSurfaces() { return lpVtbl->RestoreAllSurfaces(this); }
    HRESULT TestCooperativeLevel() { return lpVtbl->TestCooperativeLevel(this); }
    HRESULT GetDeviceIdentifier(LPDDDEVICEIDENTIFIER2 a, DWORD b) { return lpVtbl->GetDeviceIdentifier(this, a, b); }
    HRESULT StartModeTest(LPSIZE a, DWORD b, DWORD c) { return lpVtbl->StartModeTest(this, a, b, c); }
    HRESULT EvaluateMode(DWORD a, DWORD *b) { return lpVtbl->EvaluateMode(this, a, b); }
#endif
};

/* Legacy DX1-6 interfaces - opaque (only used as pointer types in
 * declarations like ddutil.h; never instantiated on Linux) */
struct IDirectDraw {
    void *lpVtbl;
};
struct IDirectDrawSurface;
typedef struct IDirectDrawSurface *LPDIRECTDRAWSURFACE;
struct IDirectDrawSurface4;
typedef struct IDirectDrawSurface4 *LPDIRECTDRAWSURFACE4;
struct IDirectDraw4;
typedef struct IDirectDraw4 *LPDIRECTDRAW4;
struct IDirectDraw2;
typedef struct IDirectDraw2 *LPDIRECTDRAW2;
struct IDirect3D;
typedef struct IDirect3D *LPDIRECT3D;
struct IDirectDrawGammaControl;
typedef struct IDirectDrawGammaControl *LPDIRECTDRAWGAMMACONTROL;

/* ============================================================
 * Creation entry points (implemented in compat layer)
 * ============================================================ */
#ifdef __cplusplus
extern "C" {
#endif

IDirectDraw7 *FF_CreateDirectDraw7(void);
IDirectDrawSurface7 *FF_CreateRenderTargetSurface(int width, int height);

HRESULT DirectDrawCreateEx(GUID *lpGuid, LPVOID *lplpDD, REFIID iid, IUnknown *pUnkOuter);
HRESULT DirectDrawCreate(GUID *lpGUID, LPDIRECTDRAW *lplpDD, IUnknown *pUnkOuter);
HRESULT DirectDrawEnumerateA(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext);
HRESULT DirectDrawEnumerateExA(LPDDENUMCALLBACKEXA lpCallback, LPVOID lpContext, DWORD dwFlags);

#ifdef __cplusplus
}
#endif

#define DirectDrawEnumerate DirectDrawEnumerateA
#define DirectDrawEnumerateEx DirectDrawEnumerateExA

/* Function-pointer typedefs used with GetProcAddress-style loading */
typedef HRESULT (WINAPI *LPDIRECTDRAWCREATE)(GUID *, LPDIRECTDRAW *, IUnknown *);
typedef HRESULT (WINAPI *LPDIRECTDRAWCREATEEX)(GUID *, LPVOID *, REFIID, IUnknown *);
typedef HRESULT (WINAPI *LPDIRECTDRAWENUMERATEA)(LPDDENUMCALLBACKA, LPVOID);
typedef HRESULT (WINAPI *LPDIRECTDRAWENUMERATEEX)(LPDDENUMCALLBACKEXA, LPVOID, DWORD);
#define LPDIRECTDRAWENUMERATEEXA LPDIRECTDRAWENUMERATEEX
#define DDENUM_ATTACHEDSECONDARYDEVICES 0x00000001

#endif /* FF_LINUX */
#endif /* FF_COMPAT_DDRAW_H */

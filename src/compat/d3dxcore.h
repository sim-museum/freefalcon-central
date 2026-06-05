/* FreeFalcon Linux Port - d3dxcore.h compatibility (D3DX surface formats) */
#ifndef FF_COMPAT_D3DXCORE_H
#define FF_COMPAT_D3DXCORE_H
#ifdef FF_LINUX

#include "ddraw.h"
#include "d3d.h"

typedef enum _D3DX_SURFACEFORMAT {
    D3DX_SF_UNKNOWN  = 0,
    D3DX_SF_R8G8B8   = 1,
    D3DX_SF_A8R8G8B8 = 2,
    D3DX_SF_X8R8G8B8 = 3,
    D3DX_SF_R5G6B5   = 4,
    D3DX_SF_R5G5B5   = 5,
    D3DX_SF_PALETTE4 = 6,
    D3DX_SF_PALETTE8 = 7,
    D3DX_SF_A1R5G5B5 = 8,
    D3DX_SF_X4R4G4B4 = 9,
    D3DX_SF_A4R4G4B4 = 10,
    D3DX_SF_L8       = 11,
    D3DX_SF_A8L8     = 12,
    D3DX_SF_U8V8     = 13,
    D3DX_SF_L6V5U5   = 14,
    D3DX_SF_X8L8V8U8 = 15,
    D3DX_SF_DXT1     = 16,
    D3DX_SF_DXT3     = 17,
    D3DX_SF_DXT5     = 18,
    D3DX_SF_A8       = 19,
    D3DX_SF_FORCEMASK = 0x7fffffff
} D3DX_SURFACEFORMAT;

#define D3DX_DEFAULT ((DWORD)-1)
#define D3DX_TEXTURE_NOMIPMAP ((DWORD)1)
#define D3DX_FT_POINT  0x1
#define D3DX_FT_LINEAR 0x2
#define D3DX_FT_DEFAULT D3DX_DEFAULT

/* Map a DDPIXELFORMAT to a D3DX surface format (implemented in linux_stubs.cpp) */
#ifdef __cplusplus
extern "C" {
#endif
D3DX_SURFACEFORMAT D3DXMakeSurfaceFormat(LPDDPIXELFORMAT pddpf);
HRESULT D3DXMakePixelFormat(D3DX_SURFACEFORMAT fmt, LPDDPIXELFORMAT pddpf);
HRESULT D3DXInitialize(void);
HRESULT D3DXUninitialize(void);

/* Texture creation helpers (implemented in linux_stubs.cpp) */
HRESULT D3DXCreateTexture(struct IDirect3DDevice7 *pd3dDevice, DWORD *pdwWidth, DWORD *pdwHeight,
                          DWORD *pdwMipMapCount, D3DX_SURFACEFORMAT *pPixelFormat,
                          struct IDirectDrawPalette *pDDPal, struct IDirectDrawSurface7 **ppDDSurf,
                          DWORD *pdwNumMips);
HRESULT D3DXLoadTextureFromMemory(struct IDirect3DDevice7 *pd3dDevice, struct IDirectDrawSurface7 *pTexture,
                                  DWORD dwMipMapLevel, LPVOID pMemory, struct IDirectDrawPalette *pDDPal,
                                  D3DX_SURFACEFORMAT srcFormat, DWORD dwPitch, RECT *pSrcRect, DWORD dwFilterFlags);
#ifdef __cplusplus
}
#endif

#endif /* FF_LINUX */
#endif

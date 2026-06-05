/*
 * FreeFalcon Linux Port - d3dtypes.h compatibility
 *
 * Direct3D 7 value types, enums and constants (real DX7 values).
 */

#ifndef FF_COMPAT_D3DTYPES_H
#define FF_COMPAT_D3DTYPES_H

#ifdef FF_LINUX

#include "compat_types.h"
#include "ddraw.h"

/* ============================================================
 * Scalar types
 * ============================================================ */
typedef float D3DVALUE, *LPD3DVALUE;
typedef DWORD D3DCOLOR, *LPD3DCOLOR;
typedef DWORD D3DTEXTUREHANDLE;
typedef DWORD D3DMATERIALHANDLE;
typedef DWORD D3DMATRIXHANDLE;
typedef LONG  D3DFIXED;

#define D3DVAL(val)      ((D3DVALUE)(val))
#define D3DDivide(a, b)  ((float)(a) / (float)(b))
#define D3DMultiply(a, b) ((a) * (b))

/* ============================================================
 * Color helpers
 * ============================================================ */
#define RGBA_MAKE(r, g, b, a) \
    ((D3DCOLOR)(((DWORD)(a) << 24) | ((DWORD)(r) << 16) | ((DWORD)(g) << 8) | (DWORD)(b)))
#define RGB_MAKE(r, g, b)  RGBA_MAKE(r, g, b, 0xff)
#define D3DRGBA(r, g, b, a) \
    RGBA_MAKE((DWORD)((r) * 255.f), (DWORD)((g) * 255.f), (DWORD)((b) * 255.f), (DWORD)((a) * 255.f))
#define D3DRGB(r, g, b) \
    RGBA_MAKE((DWORD)((r) * 255.f), (DWORD)((g) * 255.f), (DWORD)((b) * 255.f), 0xff)
#define RGBA_GETALPHA(rgb)  ((rgb) >> 24)
#define RGBA_GETRED(rgb)    (((rgb) >> 16) & 0xff)
#define RGBA_GETGREEN(rgb)  (((rgb) >> 8) & 0xff)
#define RGBA_GETBLUE(rgb)   ((rgb) & 0xff)
#define RGB_GETRED(rgb)     RGBA_GETRED(rgb)
#define RGB_GETGREEN(rgb)   RGBA_GETGREEN(rgb)
#define RGB_GETBLUE(rgb)    RGBA_GETBLUE(rgb)
#define RGBA_SETALPHA(rgba, x) (((x) << 24) | ((rgba) & 0x00ffffff))
#define RGB_TORGBA(rgb)     ((D3DCOLOR)((rgb) | 0xff000000))

/* ============================================================
 * Vectors / geometry
 * ============================================================ */
typedef struct _D3DVECTOR {
    union { D3DVALUE x; D3DVALUE dvX; };
    union { D3DVALUE y; D3DVALUE dvY; };
    union { D3DVALUE z; D3DVALUE dvZ; };
#if defined(__cplusplus)
    _D3DVECTOR() { x = y = z = 0.0f; }
    _D3DVECTOR(D3DVALUE _x, D3DVALUE _y, D3DVALUE _z) { x = _x; y = _y; z = _z; }
#endif
} D3DVECTOR, *LPD3DVECTOR;

typedef struct _D3DRECT {
    union { LONG x1; LONG lX1; };
    union { LONG y1; LONG lY1; };
    union { LONG x2; LONG lX2; };
    union { LONG y2; LONG lY2; };
} D3DRECT, *LPD3DRECT;

typedef struct _D3DCOLORVALUE {
    union { D3DVALUE r; D3DVALUE dvR; };
    union { D3DVALUE g; D3DVALUE dvG; };
    union { D3DVALUE b; D3DVALUE dvB; };
    union { D3DVALUE a; D3DVALUE dvA; };
} D3DCOLORVALUE, *LPD3DCOLORVALUE;

typedef struct _D3DMATRIX {
    union { D3DVALUE _11; D3DVALUE dv_11; };
    union { D3DVALUE _12; D3DVALUE dv_12; };
    union { D3DVALUE _13; D3DVALUE dv_13; };
    union { D3DVALUE _14; D3DVALUE dv_14; };
    union { D3DVALUE _21; D3DVALUE dv_21; };
    union { D3DVALUE _22; D3DVALUE dv_22; };
    union { D3DVALUE _23; D3DVALUE dv_23; };
    union { D3DVALUE _24; D3DVALUE dv_24; };
    union { D3DVALUE _31; D3DVALUE dv_31; };
    union { D3DVALUE _32; D3DVALUE dv_32; };
    union { D3DVALUE _33; D3DVALUE dv_33; };
    union { D3DVALUE _34; D3DVALUE dv_34; };
    union { D3DVALUE _41; D3DVALUE dv_41; };
    union { D3DVALUE _42; D3DVALUE dv_42; };
    union { D3DVALUE _43; D3DVALUE dv_43; };
    union { D3DVALUE _44; D3DVALUE dv_44; };
} D3DMATRIX, *LPD3DMATRIX;

/* ============================================================
 * Vertex types
 * ============================================================ */
typedef struct _D3DTLVERTEX {
    union { D3DVALUE sx; D3DVALUE dvSX; };
    union { D3DVALUE sy; D3DVALUE dvSY; };
    union { D3DVALUE sz; D3DVALUE dvSZ; };
    union { D3DVALUE rhw; D3DVALUE dvRHW; };
    union { D3DCOLOR color; D3DCOLOR dcColor; };
    union { D3DCOLOR specular; D3DCOLOR dcSpecular; };
    union { D3DVALUE tu; D3DVALUE dvTU; };
    union { D3DVALUE tv; D3DVALUE dvTV; };
} D3DTLVERTEX, *LPD3DTLVERTEX;

typedef struct _D3DLVERTEX {
    union { D3DVALUE x; D3DVALUE dvX; };
    union { D3DVALUE y; D3DVALUE dvY; };
    union { D3DVALUE z; D3DVALUE dvZ; };
    DWORD dwReserved;
    union { D3DCOLOR color; D3DCOLOR dcColor; };
    union { D3DCOLOR specular; D3DCOLOR dcSpecular; };
    union { D3DVALUE tu; D3DVALUE dvTU; };
    union { D3DVALUE tv; D3DVALUE dvTV; };
} D3DLVERTEX, *LPD3DLVERTEX;

typedef struct _D3DVERTEX {
    union { D3DVALUE x; D3DVALUE dvX; };
    union { D3DVALUE y; D3DVALUE dvY; };
    union { D3DVALUE z; D3DVALUE dvZ; };
    union { D3DVALUE nx; D3DVALUE dvNX; };
    union { D3DVALUE ny; D3DVALUE dvNY; };
    union { D3DVALUE nz; D3DVALUE dvNZ; };
    union { D3DVALUE tu; D3DVALUE dvTU; };
    union { D3DVALUE tv; D3DVALUE dvTV; };
} D3DVERTEX, *LPD3DVERTEX;

/* ============================================================
 * Viewport / material / light
 * ============================================================ */
typedef struct _D3DVIEWPORT7 {
    DWORD dwX;
    DWORD dwY;
    DWORD dwWidth;
    DWORD dwHeight;
    D3DVALUE dvMinZ;
    D3DVALUE dvMaxZ;
} D3DVIEWPORT7, *LPD3DVIEWPORT7;

typedef struct _D3DMATERIAL7 {
    union { D3DCOLORVALUE diffuse;  D3DCOLORVALUE dcvDiffuse; };
    union { D3DCOLORVALUE ambient;  D3DCOLORVALUE dcvAmbient; };
    union { D3DCOLORVALUE specular; D3DCOLORVALUE dcvSpecular; };
    union { D3DCOLORVALUE emissive; D3DCOLORVALUE dcvEmissive; };
    union { D3DVALUE      power;    D3DVALUE      dvPower; };
} D3DMATERIAL7, *LPD3DMATERIAL7;

typedef enum _D3DLIGHTTYPE {
    D3DLIGHT_POINT       = 1,
    D3DLIGHT_SPOT        = 2,
    D3DLIGHT_DIRECTIONAL = 3,
    D3DLIGHT_PARALLELPOINT = 4,
    D3DLIGHT_FORCE_DWORD = 0x7fffffff
} D3DLIGHTTYPE;

typedef struct _D3DLIGHT7 {
    D3DLIGHTTYPE  dltType;
    D3DCOLORVALUE dcvDiffuse;
    D3DCOLORVALUE dcvSpecular;
    D3DCOLORVALUE dcvAmbient;
    D3DVECTOR     dvPosition;
    D3DVECTOR     dvDirection;
    D3DVALUE      dvRange;
    D3DVALUE      dvFalloff;
    D3DVALUE      dvAttenuation0;
    D3DVALUE      dvAttenuation1;
    D3DVALUE      dvAttenuation2;
    D3DVALUE      dvTheta;
    D3DVALUE      dvPhi;
} D3DLIGHT7, *LPD3DLIGHT7;

#define D3DLIGHT_RANGE_MAX ((float)sqrt(3.402823466e+38f))

/* Legacy (DX6 and earlier) variants */
typedef struct _D3DMATERIAL {
    DWORD dwSize;
    union { D3DCOLORVALUE diffuse;  D3DCOLORVALUE dcvDiffuse; };
    union { D3DCOLORVALUE ambient;  D3DCOLORVALUE dcvAmbient; };
    union { D3DCOLORVALUE specular; D3DCOLORVALUE dcvSpecular; };
    union { D3DCOLORVALUE emissive; D3DCOLORVALUE dcvEmissive; };
    union { D3DVALUE      power;    D3DVALUE      dvPower; };
    D3DTEXTUREHANDLE hTexture;
    DWORD dwRampSize;
} D3DMATERIAL, *LPD3DMATERIAL;

typedef struct _D3DLIGHT {
    DWORD dwSize;
    D3DLIGHTTYPE dltType;
    D3DCOLORVALUE dcvColor;
    D3DVECTOR dvPosition;
    D3DVECTOR dvDirection;
    D3DVALUE dvRange;
    D3DVALUE dvFalloff;
    D3DVALUE dvAttenuation0;
    D3DVALUE dvAttenuation1;
    D3DVALUE dvAttenuation2;
    D3DVALUE dvTheta;
    D3DVALUE dvPhi;
} D3DLIGHT, *LPD3DLIGHT;

/* ============================================================
 * Enums
 * ============================================================ */
typedef enum _D3DPRIMITIVETYPE {
    D3DPT_POINTLIST     = 1,
    D3DPT_LINELIST      = 2,
    D3DPT_LINESTRIP     = 3,
    D3DPT_TRIANGLELIST  = 4,
    D3DPT_TRIANGLESTRIP = 5,
    D3DPT_TRIANGLEFAN   = 6,
    D3DPT_FORCE_DWORD   = 0x7fffffff
} D3DPRIMITIVETYPE;

typedef enum _D3DTRANSFORMSTATETYPE {
    D3DTRANSFORMSTATE_WORLD      = 1,
    D3DTRANSFORMSTATE_VIEW       = 2,
    D3DTRANSFORMSTATE_PROJECTION = 3,
    D3DTRANSFORMSTATE_WORLD1     = 4,
    D3DTRANSFORMSTATE_WORLD2     = 5,
    D3DTRANSFORMSTATE_WORLD3     = 6,
    D3DTRANSFORMSTATE_TEXTURE0   = 16,
    D3DTRANSFORMSTATE_TEXTURE1   = 17,
    D3DTRANSFORMSTATE_FORCE_DWORD = 0x7fffffff
} D3DTRANSFORMSTATETYPE;

typedef enum _D3DFILLMODE {
    D3DFILL_POINT     = 1,
    D3DFILL_WIREFRAME = 2,
    D3DFILL_SOLID     = 3,
    D3DFILL_FORCE_DWORD = 0x7fffffff
} D3DFILLMODE;

typedef enum _D3DSHADEMODE {
    D3DSHADE_FLAT    = 1,
    D3DSHADE_GOURAUD = 2,
    D3DSHADE_PHONG   = 3,
    D3DSHADE_FORCE_DWORD = 0x7fffffff
} D3DSHADEMODE;

typedef enum _D3DCULL {
    D3DCULL_NONE = 1,
    D3DCULL_CW   = 2,
    D3DCULL_CCW  = 3,
    D3DCULL_FORCE_DWORD = 0x7fffffff
} D3DCULL;

typedef enum _D3DCMPFUNC {
    D3DCMP_NEVER        = 1,
    D3DCMP_LESS         = 2,
    D3DCMP_EQUAL        = 3,
    D3DCMP_LESSEQUAL    = 4,
    D3DCMP_GREATER      = 5,
    D3DCMP_NOTEQUAL     = 6,
    D3DCMP_GREATEREQUAL = 7,
    D3DCMP_ALWAYS       = 8,
    D3DCMP_FORCE_DWORD  = 0x7fffffff
} D3DCMPFUNC;

typedef enum _D3DSTENCILOP {
    D3DSTENCILOP_KEEP    = 1,
    D3DSTENCILOP_ZERO    = 2,
    D3DSTENCILOP_REPLACE = 3,
    D3DSTENCILOP_INCRSAT = 4,
    D3DSTENCILOP_DECRSAT = 5,
    D3DSTENCILOP_INVERT  = 6,
    D3DSTENCILOP_INCR    = 7,
    D3DSTENCILOP_DECR    = 8,
    D3DSTENCILOP_FORCE_DWORD = 0x7fffffff
} D3DSTENCILOP;

typedef enum _D3DBLEND {
    D3DBLEND_ZERO         = 1,
    D3DBLEND_ONE          = 2,
    D3DBLEND_SRCCOLOR     = 3,
    D3DBLEND_INVSRCCOLOR  = 4,
    D3DBLEND_SRCALPHA     = 5,
    D3DBLEND_INVSRCALPHA  = 6,
    D3DBLEND_DESTALPHA    = 7,
    D3DBLEND_INVDESTALPHA = 8,
    D3DBLEND_DESTCOLOR    = 9,
    D3DBLEND_INVDESTCOLOR = 10,
    D3DBLEND_SRCALPHASAT  = 11,
    D3DBLEND_BOTHSRCALPHA = 12,
    D3DBLEND_BOTHINVSRCALPHA = 13,
    D3DBLEND_FORCE_DWORD  = 0x7fffffff
} D3DBLEND;

typedef enum _D3DFOGMODE {
    D3DFOG_NONE   = 0,
    D3DFOG_EXP    = 1,
    D3DFOG_EXP2   = 2,
    D3DFOG_LINEAR = 3,
    D3DFOG_FORCE_DWORD = 0x7fffffff
} D3DFOGMODE;

typedef enum _D3DZBUFFERTYPE {
    D3DZB_FALSE = 0,
    D3DZB_TRUE  = 1,
    D3DZB_USEW  = 2,
    D3DZB_FORCE_DWORD = 0x7fffffff
} D3DZBUFFERTYPE;

typedef enum _D3DMATERIALCOLORSOURCE {
    D3DMCS_MATERIAL = 0,
    D3DMCS_COLOR1   = 1,
    D3DMCS_COLOR2   = 2,
    D3DMCS_FORCE_DWORD = 0x7fffffff
} D3DMATERIALCOLORSOURCE;

typedef enum _D3DANTIALIASMODE {
    D3DANTIALIAS_NONE           = 0,
    D3DANTIALIAS_SORTDEPENDENT  = 1,
    D3DANTIALIAS_SORTINDEPENDENT = 2,
    D3DANTIALIAS_FORCE_DWORD    = 0x7fffffff
} D3DANTIALIASMODE;

typedef enum _D3DTEXTUREADDRESS {
    D3DTADDRESS_WRAP   = 1,
    D3DTADDRESS_MIRROR = 2,
    D3DTADDRESS_CLAMP  = 3,
    D3DTADDRESS_BORDER = 4,
    D3DTADDRESS_FORCE_DWORD = 0x7fffffff
} D3DTEXTUREADDRESS;

typedef enum _D3DRENDERSTATETYPE {
    D3DRENDERSTATE_TEXTUREHANDLE      = 1,
    D3DRENDERSTATE_ANTIALIAS          = 2,
    D3DRENDERSTATE_TEXTUREADDRESS     = 3,
    D3DRENDERSTATE_TEXTUREPERSPECTIVE = 4,
    D3DRENDERSTATE_WRAPU              = 5,
    D3DRENDERSTATE_WRAPV              = 6,
    D3DRENDERSTATE_ZENABLE            = 7,
    D3DRENDERSTATE_FILLMODE           = 8,
    D3DRENDERSTATE_SHADEMODE          = 9,
    D3DRENDERSTATE_LINEPATTERN        = 10,
    D3DRENDERSTATE_MONOENABLE         = 11,
    D3DRENDERSTATE_ROP2               = 12,
    D3DRENDERSTATE_PLANEMASK          = 13,
    D3DRENDERSTATE_ZWRITEENABLE       = 14,
    D3DRENDERSTATE_ALPHATESTENABLE    = 15,
    D3DRENDERSTATE_LASTPIXEL          = 16,
    D3DRENDERSTATE_TEXTUREMAG         = 17,
    D3DRENDERSTATE_TEXTUREMIN         = 18,
    D3DRENDERSTATE_SRCBLEND           = 19,
    D3DRENDERSTATE_DESTBLEND          = 20,
    D3DRENDERSTATE_TEXTUREMAPBLEND    = 21,
    D3DRENDERSTATE_CULLMODE           = 22,
    D3DRENDERSTATE_ZFUNC              = 23,
    D3DRENDERSTATE_ALPHAREF           = 24,
    D3DRENDERSTATE_ALPHAFUNC          = 25,
    D3DRENDERSTATE_DITHERENABLE       = 26,
    D3DRENDERSTATE_ALPHABLENDENABLE   = 27,
    D3DRENDERSTATE_FOGENABLE          = 28,
    D3DRENDERSTATE_SPECULARENABLE     = 29,
    D3DRENDERSTATE_ZVISIBLE           = 30,
    D3DRENDERSTATE_SUBPIXEL           = 31,
    D3DRENDERSTATE_SUBPIXELX          = 32,
    D3DRENDERSTATE_STIPPLEDALPHA      = 33,
    D3DRENDERSTATE_FOGCOLOR           = 34,
    D3DRENDERSTATE_FOGTABLEMODE       = 35,
    D3DRENDERSTATE_FOGSTART           = 36,
    D3DRENDERSTATE_FOGEND             = 37,
    D3DRENDERSTATE_FOGDENSITY         = 38,
    D3DRENDERSTATE_STIPPLEENABLE      = 39,
    D3DRENDERSTATE_EDGEANTIALIAS      = 40,
    D3DRENDERSTATE_COLORKEYENABLE     = 41,
    D3DRENDERSTATE_BORDERCOLOR        = 43,
    D3DRENDERSTATE_TEXTUREADDRESSU    = 44,
    D3DRENDERSTATE_TEXTUREADDRESSV    = 45,
    D3DRENDERSTATE_MIPMAPLODBIAS      = 46,
    D3DRENDERSTATE_ZBIAS              = 47,
    D3DRENDERSTATE_RANGEFOGENABLE     = 48,
    D3DRENDERSTATE_ANISOTROPY         = 49,
    D3DRENDERSTATE_FLUSHBATCH         = 50,
    D3DRENDERSTATE_TRANSLUCENTSORTINDEPENDENT = 51,
    D3DRENDERSTATE_STENCILENABLE      = 52,
    D3DRENDERSTATE_STENCILFAIL        = 53,
    D3DRENDERSTATE_STENCILZFAIL       = 54,
    D3DRENDERSTATE_STENCILPASS        = 55,
    D3DRENDERSTATE_STENCILFUNC        = 56,
    D3DRENDERSTATE_STENCILREF         = 57,
    D3DRENDERSTATE_STENCILMASK        = 58,
    D3DRENDERSTATE_STENCILWRITEMASK   = 59,
    D3DRENDERSTATE_TEXTUREFACTOR      = 60,
    D3DRENDERSTATE_STIPPLEPATTERN00   = 64,
    D3DRENDERSTATE_WRAP0              = 128,
    D3DRENDERSTATE_WRAP1              = 129,
    D3DRENDERSTATE_CLIPPING           = 136,
    D3DRENDERSTATE_LIGHTING           = 137,
    D3DRENDERSTATE_EXTENTS            = 138,
    D3DRENDERSTATE_AMBIENT            = 139,
    D3DRENDERSTATE_FOGVERTEXMODE      = 140,
    D3DRENDERSTATE_COLORVERTEX        = 141,
    D3DRENDERSTATE_LOCALVIEWER        = 142,
    D3DRENDERSTATE_NORMALIZENORMALS   = 143,
    D3DRENDERSTATE_COLORKEYBLENDENABLE = 144,
    D3DRENDERSTATE_DIFFUSEMATERIALSOURCE  = 145,
    D3DRENDERSTATE_SPECULARMATERIALSOURCE = 146,
    D3DRENDERSTATE_AMBIENTMATERIALSOURCE  = 147,
    D3DRENDERSTATE_EMISSIVEMATERIALSOURCE = 148,
    D3DRENDERSTATE_VERTEXBLEND        = 151,
    D3DRENDERSTATE_CLIPPLANEENABLE    = 152,
    D3DRENDERSTATE_FORCE_DWORD        = 0x7fffffff
} D3DRENDERSTATETYPE;

typedef enum _D3DTEXTURESTAGESTATETYPE {
    D3DTSS_COLOROP       = 1,
    D3DTSS_COLORARG1     = 2,
    D3DTSS_COLORARG2     = 3,
    D3DTSS_ALPHAOP       = 4,
    D3DTSS_ALPHAARG1     = 5,
    D3DTSS_ALPHAARG2     = 6,
    D3DTSS_BUMPENVMAT00  = 7,
    D3DTSS_BUMPENVMAT01  = 8,
    D3DTSS_BUMPENVMAT10  = 9,
    D3DTSS_BUMPENVMAT11  = 10,
    D3DTSS_TEXCOORDINDEX = 11,
    D3DTSS_ADDRESS       = 12,
    D3DTSS_ADDRESSU      = 13,
    D3DTSS_ADDRESSV      = 14,
    D3DTSS_BORDERCOLOR   = 15,
    D3DTSS_MAGFILTER     = 16,
    D3DTSS_MINFILTER     = 17,
    D3DTSS_MIPFILTER     = 18,
    D3DTSS_MIPMAPLODBIAS = 19,
    D3DTSS_MAXMIPLEVEL   = 20,
    D3DTSS_MAXANISOTROPY = 21,
    D3DTSS_BUMPENVLSCALE = 22,
    D3DTSS_BUMPENVLOFFSET = 23,
    D3DTSS_TEXTURETRANSFORMFLAGS = 24,
    D3DTSS_FORCE_DWORD   = 0x7fffffff
} D3DTEXTURESTAGESTATETYPE;

typedef enum _D3DTEXTUREOP {
    D3DTOP_DISABLE    = 1,
    D3DTOP_SELECTARG1 = 2,
    D3DTOP_SELECTARG2 = 3,
    D3DTOP_MODULATE   = 4,
    D3DTOP_MODULATE2X = 5,
    D3DTOP_MODULATE4X = 6,
    D3DTOP_ADD        = 7,
    D3DTOP_ADDSIGNED  = 8,
    D3DTOP_ADDSIGNED2X = 9,
    D3DTOP_SUBTRACT   = 10,
    D3DTOP_ADDSMOOTH  = 11,
    D3DTOP_BLENDDIFFUSEALPHA = 12,
    D3DTOP_BLENDTEXTUREALPHA = 13,
    D3DTOP_BLENDFACTORALPHA  = 14,
    D3DTOP_BLENDTEXTUREALPHAPM = 15,
    D3DTOP_BLENDCURRENTALPHA = 16,
    D3DTOP_PREMODULATE = 17,
    D3DTOP_MODULATEALPHA_ADDCOLOR = 18,
    D3DTOP_MODULATECOLOR_ADDALPHA = 19,
    D3DTOP_MODULATEINVALPHA_ADDCOLOR = 20,
    D3DTOP_MODULATEINVCOLOR_ADDALPHA = 21,
    D3DTOP_BUMPENVMAP = 22,
    D3DTOP_BUMPENVMAPLUMINANCE = 23,
    D3DTOP_DOTPRODUCT3 = 24,
    D3DTOP_FORCE_DWORD = 0x7fffffff
} D3DTEXTUREOP;

typedef enum _D3DTEXTUREMAGFILTER {
    D3DTFG_POINT         = 1,
    D3DTFG_LINEAR        = 2,
    D3DTFG_FLATCUBIC     = 3,
    D3DTFG_GAUSSIANCUBIC = 4,
    D3DTFG_ANISOTROPIC   = 5,
    D3DTFG_FORCE_DWORD   = 0x7fffffff
} D3DTEXTUREMAGFILTER;

typedef enum _D3DTEXTUREMINFILTER {
    D3DTFN_POINT       = 1,
    D3DTFN_LINEAR      = 2,
    D3DTFN_ANISOTROPIC = 3,
    D3DTFN_FORCE_DWORD = 0x7fffffff
} D3DTEXTUREMINFILTER;

typedef enum _D3DTEXTUREMIPFILTER {
    D3DTFP_NONE        = 1,
    D3DTFP_POINT       = 2,
    D3DTFP_LINEAR      = 3,
    D3DTFP_FORCE_DWORD = 0x7fffffff
} D3DTEXTUREMIPFILTER;

typedef enum _D3DSTATEBLOCKTYPE {
    D3DSBT_ALL         = 1,
    D3DSBT_PIXELSTATE  = 2,
    D3DSBT_VERTEXSTATE = 3,
    D3DSBT_FORCE_DWORD = 0x7fffffff
} D3DSTATEBLOCKTYPE;

typedef enum _D3DVERTEXBLENDFLAGS {
    D3DVBLEND_DISABLE  = 0,
    D3DVBLEND_1WEIGHT  = 1,
    D3DVBLEND_2WEIGHTS = 2,
    D3DVBLEND_3WEIGHTS = 3
} D3DVERTEXBLENDFLAGS;

/* Texture argument flags */
#define D3DTA_SELECTMASK     0x0000000f
#define D3DTA_DIFFUSE        0x00000000
#define D3DTA_CURRENT        0x00000001
#define D3DTA_TEXTURE        0x00000002
#define D3DTA_TFACTOR        0x00000003
#define D3DTA_SPECULAR       0x00000004
#define D3DTA_COMPLEMENT     0x00000010
#define D3DTA_ALPHAREPLICATE 0x00000020

/* ============================================================
 * FVF flags
 * ============================================================ */
#define D3DFVF_RESERVED0     0x001
#define D3DFVF_POSITION_MASK 0x00E
#define D3DFVF_XYZ           0x002
#define D3DFVF_XYZRHW        0x004
#define D3DFVF_XYZB1         0x006
#define D3DFVF_NORMAL        0x010
#define D3DFVF_RESERVED1     0x020
#define D3DFVF_DIFFUSE       0x040
#define D3DFVF_SPECULAR      0x080
#define D3DFVF_TEXCOUNT_MASK 0xf00
#define D3DFVF_TEXCOUNT_SHIFT 8
#define D3DFVF_TEX0          0x000
#define D3DFVF_TEX1          0x100
#define D3DFVF_TEX2          0x200
#define D3DFVF_TEX3          0x300
#define D3DFVF_TEX4          0x400

#define D3DFVF_VERTEX    (D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1)
#define D3DFVF_LVERTEX   (D3DFVF_XYZ | D3DFVF_RESERVED1 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1)
#define D3DFVF_TLVERTEX  (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1)

/* ============================================================
 * Clear / DrawPrimitive flags
 * ============================================================ */
#define D3DCLEAR_TARGET  0x00000001
#define D3DCLEAR_ZBUFFER 0x00000002
#define D3DCLEAR_STENCIL 0x00000004

#define D3DDP_WAIT                0x00000001
#define D3DDP_OUTOFORDER          0x00000002
#define D3DDP_DONOTCLIP           0x00000004
#define D3DDP_DONOTUPDATEEXTENTS  0x00000008
#define D3DDP_DONOTLIGHT          0x00000010

/* Clip status flags */
#define D3DSTATUS_CLIPUNIONLEFT   0x00000001
#define D3DSTATUS_CLIPUNIONRIGHT  0x00000002
#define D3DSTATUS_CLIPUNIONTOP    0x00000004
#define D3DSTATUS_CLIPUNIONBOTTOM 0x00000008
#define D3DSTATUS_CLIPUNIONFRONT  0x00000010
#define D3DSTATUS_CLIPUNIONBACK   0x00000020
#define D3DSTATUS_CLIPINTERSECTIONLEFT   0x00040000
#define D3DSTATUS_CLIPINTERSECTIONRIGHT  0x00080000
#define D3DSTATUS_CLIPINTERSECTIONTOP    0x00100000
#define D3DSTATUS_CLIPINTERSECTIONBOTTOM 0x00200000
#define D3DSTATUS_CLIPINTERSECTIONFRONT  0x00400000
#define D3DSTATUS_CLIPINTERSECTIONBACK   0x00800000
#define D3DSTATUS_ZNOTVISIBLE            0x01000000
#define D3DSTATUS_CLIPUNIONALL        0x0000003F
#define D3DSTATUS_CLIPINTERSECTIONALL 0x00FC0000
#define D3DSTATUS_DEFAULT (D3DSTATUS_CLIPINTERSECTIONALL | D3DSTATUS_ZNOTVISIBLE)

/* ============================================================
 * Wrap flags
 * ============================================================ */
#define D3DWRAP_U  0x00000001
#define D3DWRAP_V  0x00000002
#define D3DWRAPCOORD_0 0x00000001
#define D3DWRAPCOORD_1 0x00000002

/* Legacy headers use `enum _D3DX_SURFACEFORMAT member;` which needs the
 * complete enum - pull in d3dxcore (which itself includes d3d.h). */
#include "d3dxcore.h"

#endif /* FF_LINUX */
#endif /* FF_COMPAT_D3DTYPES_H */

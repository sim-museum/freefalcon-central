/* FreeFalcon Linux Port - d3dxmath.h compatibility */
#ifndef FF_COMPAT_D3DXMATH_H
#define FF_COMPAT_D3DXMATH_H
#ifdef FF_LINUX

#include <math.h>
#include "d3dtypes.h"

#ifdef __cplusplus

struct D3DXMATRIX : public D3DMATRIX {
    D3DXMATRIX() {}
    D3DXMATRIX(const D3DMATRIX &m) { *(D3DMATRIX *)this = m; }
    D3DXMATRIX &operator=(const D3DMATRIX &m) { *(D3DMATRIX *)this = m; return *this; }
    operator float *() { return &_11; }
    operator const float *() const { return &_11; }
};
typedef D3DXMATRIX *LPD3DXMATRIX;

struct D3DXVECTOR3 {
    float x, y, z;
    D3DXVECTOR3() : x(0), y(0), z(0) {}
    D3DXVECTOR3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    D3DXVECTOR3(const D3DVECTOR &v) : x(v.x), y(v.y), z(v.z) {}
    operator float *() { return &x; }
    operator const float *() const { return &x; }
    D3DXVECTOR3 operator+(const D3DXVECTOR3 &o) const { return D3DXVECTOR3(x + o.x, y + o.y, z + o.z); }
    D3DXVECTOR3 operator-(const D3DXVECTOR3 &o) const { return D3DXVECTOR3(x - o.x, y - o.y, z - o.z); }
    D3DXVECTOR3 operator*(float s) const { return D3DXVECTOR3(x * s, y * s, z * s); }
    D3DXVECTOR3 &operator+=(const D3DXVECTOR3 &o) { x += o.x; y += o.y; z += o.z; return *this; }
    D3DXVECTOR3 &operator-=(const D3DXVECTOR3 &o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
    D3DXVECTOR3 &operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
    D3DXVECTOR3 operator-() const { return D3DXVECTOR3(-x, -y, -z); }
};
typedef D3DXVECTOR3 *LPD3DXVECTOR3;
static inline D3DXVECTOR3 operator*(float s, const D3DXVECTOR3 &v) { return D3DXVECTOR3(v.x * s, v.y * s, v.z * s); }

struct D3DXVECTOR4 {
    float x, y, z, w;
    D3DXVECTOR4() : x(0), y(0), z(0), w(0) {}
    D3DXVECTOR4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
};
typedef D3DXVECTOR4 *LPD3DXVECTOR4;

static inline D3DXMATRIX *D3DXMatrixIdentity(D3DXMATRIX *pOut) {
    memset(pOut, 0, sizeof(D3DMATRIX));
    pOut->_11 = pOut->_22 = pOut->_33 = pOut->_44 = 1.0f;
    return pOut;
}

static inline D3DXMATRIX *D3DXMatrixMultiply(D3DXMATRIX *pOut, const D3DXMATRIX *pM1, const D3DXMATRIX *pM2) {
    D3DXMATRIX tmp;
    const float *a = &pM1->_11;
    const float *b = &pM2->_11;
    float *o = &tmp._11;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            float sum = 0.0f;
            for (int k = 0; k < 4; k++)
                sum += a[i * 4 + k] * b[k * 4 + j];
            o[i * 4 + j] = sum;
        }
    *pOut = tmp;
    return pOut;
}

static inline D3DXMATRIX *D3DXMatrixTranslation(D3DXMATRIX *pOut, float x, float y, float z) {
    D3DXMatrixIdentity(pOut);
    pOut->_41 = x; pOut->_42 = y; pOut->_43 = z;
    return pOut;
}

static inline D3DXMATRIX *D3DXMatrixScaling(D3DXMATRIX *pOut, float sx, float sy, float sz) {
    D3DXMatrixIdentity(pOut);
    pOut->_11 = sx; pOut->_22 = sy; pOut->_33 = sz;
    return pOut;
}

static inline D3DXMATRIX *D3DXMatrixRotationX(D3DXMATRIX *pOut, float angle) {
    D3DXMatrixIdentity(pOut);
    float c = cosf(angle), s = sinf(angle);
    pOut->_22 = c; pOut->_23 = s;
    pOut->_32 = -s; pOut->_33 = c;
    return pOut;
}

static inline D3DXMATRIX *D3DXMatrixRotationY(D3DXMATRIX *pOut, float angle) {
    D3DXMatrixIdentity(pOut);
    float c = cosf(angle), s = sinf(angle);
    pOut->_11 = c; pOut->_13 = -s;
    pOut->_31 = s; pOut->_33 = c;
    return pOut;
}

static inline D3DXMATRIX *D3DXMatrixRotationZ(D3DXMATRIX *pOut, float angle) {
    D3DXMatrixIdentity(pOut);
    float c = cosf(angle), s = sinf(angle);
    pOut->_11 = c; pOut->_12 = s;
    pOut->_21 = -s; pOut->_22 = c;
    return pOut;
}

static inline D3DXMATRIX *D3DXMatrixPerspectiveFov(D3DXMATRIX *pOut, float fovY, float aspect, float zn, float zf) {
    float h = 1.0f / tanf(fovY * 0.5f);
    float w = h / aspect;
    memset(pOut, 0, sizeof(D3DMATRIX));
    pOut->_11 = w;
    pOut->_22 = h;
    pOut->_33 = zf / (zf - zn);
    pOut->_34 = 1.0f;
    pOut->_43 = -zn * zf / (zf - zn);
    return pOut;
}
#define D3DXMatrixPerspectiveFovLH D3DXMatrixPerspectiveFov

static inline D3DXVECTOR3 *D3DXVec3TransformCoord(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV, const D3DXMATRIX *pM) {
    float x = pV->x, y = pV->y, z = pV->z;
    float w = x * pM->_14 + y * pM->_24 + z * pM->_34 + pM->_44;
    if (w == 0.0f) w = 1.0f;
    float ox = (x * pM->_11 + y * pM->_21 + z * pM->_31 + pM->_41) / w;
    float oy = (x * pM->_12 + y * pM->_22 + z * pM->_32 + pM->_42) / w;
    float oz = (x * pM->_13 + y * pM->_23 + z * pM->_33 + pM->_43) / w;
    pOut->x = ox; pOut->y = oy; pOut->z = oz;
    return pOut;
}

static inline D3DXVECTOR3 *D3DXVec3TransformNormal(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV, const D3DXMATRIX *pM) {
    float x = pV->x, y = pV->y, z = pV->z;
    float ox = x * pM->_11 + y * pM->_21 + z * pM->_31;
    float oy = x * pM->_12 + y * pM->_22 + z * pM->_32;
    float oz = x * pM->_13 + y * pM->_23 + z * pM->_33;
    pOut->x = ox; pOut->y = oy; pOut->z = oz;
    return pOut;
}

static inline float D3DXVec3Length(const D3DXVECTOR3 *pV) {
    return sqrtf(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z);
}

static inline D3DXVECTOR3 *D3DXVec3Normalize(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV) {
    float len = D3DXVec3Length(pV);
    if (len > 0.0f) {
        pOut->x = pV->x / len;
        pOut->y = pV->y / len;
        pOut->z = pV->z / len;
    } else {
        pOut->x = pOut->y = pOut->z = 0.0f;
    }
    return pOut;
}

static inline D3DXVECTOR3 *D3DXVec3Cross(D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV1, const D3DXVECTOR3 *pV2) {
    D3DXVECTOR3 r(pV1->y * pV2->z - pV1->z * pV2->y,
                  pV1->z * pV2->x - pV1->x * pV2->z,
                  pV1->x * pV2->y - pV1->y * pV2->x);
    *pOut = r;
    return pOut;
}

static inline float D3DXVec3Dot(const D3DXVECTOR3 *pV1, const D3DXVECTOR3 *pV2) {
    return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z;
}

#endif /* __cplusplus */
#endif /* FF_LINUX */
#endif

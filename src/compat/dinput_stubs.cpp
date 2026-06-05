/*
 * FreeFalcon Linux Port - dinput_stubs.cpp
 *
 * Backing definitions for the DirectInput 8 compatibility header (dinput.h).
 *
 * The port does NOT implement a working DirectInput backend. Input is handled
 * by SDL2 (see ffviper/main_linux.cpp). These stubs exist so the sim input
 * sources link, and so the creation entry points fail cleanly: when
 * DirectInput8Create() / DirectInputCreateEx() return E_FAIL, sim input setup
 * sets gDIEnabled = FALSE and routes everything through SDL instead.
 *
 * - c_dfDI* data formats are zero-filled (never consumed by a real driver).
 * - GUID_* device/axis/effect GUIDs are distinct, non-zero, otherwise unused.
 */

#ifdef FF_LINUX

#include "dinput.h"

/* ------------------------------------------------------------------
 * Predefined data formats.
 * Real DirectInput exposes populated DIDATAFORMAT objects here. Nothing in
 * the Linux port dereferences rgodf, so zero-filled instances are sufficient
 * (SetDataFormat is a no-op stub and the create path fails before use).
 * ------------------------------------------------------------------ */
extern const DIDATAFORMAT c_dfDIKeyboard  = { sizeof(DIDATAFORMAT), sizeof(DIOBJECTDATAFORMAT), DIDF_RELAXIS, 256, 0, nullptr };
extern const DIDATAFORMAT c_dfDIMouse     = { sizeof(DIDATAFORMAT), sizeof(DIOBJECTDATAFORMAT), DIDF_RELAXIS, sizeof(DIMOUSESTATE),  0, nullptr };
extern const DIDATAFORMAT c_dfDIMouse2    = { sizeof(DIDATAFORMAT), sizeof(DIOBJECTDATAFORMAT), DIDF_RELAXIS, sizeof(DIMOUSESTATE2), 0, nullptr };
extern const DIDATAFORMAT c_dfDIJoystick  = { sizeof(DIDATAFORMAT), sizeof(DIOBJECTDATAFORMAT), DIDF_ABSAXIS, sizeof(DIJOYSTATE),    0, nullptr };
extern const DIDATAFORMAT c_dfDIJoystick2 = { sizeof(DIDATAFORMAT), sizeof(DIOBJECTDATAFORMAT), DIDF_ABSAXIS, sizeof(DIJOYSTATE2),   0, nullptr };

/* ------------------------------------------------------------------
 * System device / axis / force-feedback effect GUIDs.
 * Distinct {data1} values keep equality comparisons (==) well-defined.
 * ------------------------------------------------------------------ */
extern "C" {

const GUID GUID_SysKeyboard   = { 0x6F1D2B61, 0xD5A0, 0x11CF, { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID GUID_SysMouse      = { 0x6F1D2B60, 0xD5A0, 0x11CF, { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID GUID_Joystick      = { 0x6F1D2B70, 0xD5A0, 0x11CF, { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };

const GUID GUID_XAxis         = { 0xA36D02E0, 0xC9F3, 0x11CF, { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID GUID_YAxis         = { 0xA36D02E1, 0xC9F3, 0x11CF, { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID GUID_ZAxis         = { 0xA36D02E2, 0xC9F3, 0x11CF, { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID GUID_RxAxis        = { 0xA36D02F4, 0xC9F3, 0x11CF, { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID GUID_RyAxis        = { 0xA36D02F5, 0xC9F3, 0x11CF, { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID GUID_RzAxis        = { 0xA36D02E3, 0xC9F3, 0x11CF, { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID GUID_Slider        = { 0xA36D02E4, 0xC9F3, 0x11CF, { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID GUID_Button        = { 0xA36D02F0, 0xC9F3, 0x11CF, { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID GUID_Key           = { 0x55728220, 0xD33C, 0x11CF, { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
const GUID GUID_POV           = { 0xA36D02F2, 0xC9F3, 0x11CF, { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };

const GUID GUID_ConstantForce = { 0x13541C20, 0x8E33, 0x11D0, { 0x9A, 0xD0, 0x00, 0xA0, 0xC9, 0xA0, 0x6E, 0x35 } };
const GUID GUID_RampForce     = { 0x13541C21, 0x8E33, 0x11D0, { 0x9A, 0xD0, 0x00, 0xA0, 0xC9, 0xA0, 0x6E, 0x35 } };
const GUID GUID_Square        = { 0x13541C22, 0x8E33, 0x11D0, { 0x9A, 0xD0, 0x00, 0xA0, 0xC9, 0xA0, 0x6E, 0x35 } };
const GUID GUID_Sine          = { 0x13541C23, 0x8E33, 0x11D0, { 0x9A, 0xD0, 0x00, 0xA0, 0xC9, 0xA0, 0x6E, 0x35 } };
const GUID GUID_Triangle      = { 0x13541C24, 0x8E33, 0x11D0, { 0x9A, 0xD0, 0x00, 0xA0, 0xC9, 0xA0, 0x6E, 0x35 } };
const GUID GUID_SawtoothUp    = { 0x13541C25, 0x8E33, 0x11D0, { 0x9A, 0xD0, 0x00, 0xA0, 0xC9, 0xA0, 0x6E, 0x35 } };
const GUID GUID_SawtoothDown  = { 0x13541C26, 0x8E33, 0x11D0, { 0x9A, 0xD0, 0x00, 0xA0, 0xC9, 0xA0, 0x6E, 0x35 } };
const GUID GUID_Spring        = { 0x13541C27, 0x8E33, 0x11D0, { 0x9A, 0xD0, 0x00, 0xA0, 0xC9, 0xA0, 0x6E, 0x35 } };
const GUID GUID_Damper        = { 0x13541C28, 0x8E33, 0x11D0, { 0x9A, 0xD0, 0x00, 0xA0, 0xC9, 0xA0, 0x6E, 0x35 } };
const GUID GUID_Inertia       = { 0x13541C29, 0x8E33, 0x11D0, { 0x9A, 0xD0, 0x00, 0xA0, 0xC9, 0xA0, 0x6E, 0x35 } };
const GUID GUID_Friction      = { 0x13541C2A, 0x8E33, 0x11D0, { 0x9A, 0xD0, 0x00, 0xA0, 0xC9, 0xA0, 0x6E, 0x35 } };
const GUID GUID_CustomForce   = { 0x13541C2B, 0x8E33, 0x11D0, { 0x9A, 0xD0, 0x00, 0xA0, 0xC9, 0xA0, 0x6E, 0x35 } };

/* ------------------------------------------------------------------
 * Creation entry points - intentionally fail so SDL handles input.
 * ------------------------------------------------------------------ */
HRESULT DirectInput8Create(HINSTANCE /*hinst*/, DWORD /*dwVersion*/,
                           REFIID /*riidltf*/, LPVOID *ppvOut, IUnknown * /*punkOuter*/)
{
    if (ppvOut) *ppvOut = nullptr;
    return E_FAIL;
}

HRESULT DirectInputCreateA(HINSTANCE /*hinst*/, DWORD /*dwVersion*/,
                           LPDIRECTINPUT *ppDI, IUnknown * /*punkOuter*/)
{
    if (ppDI) *ppDI = nullptr;
    return E_FAIL;
}

HRESULT DirectInputCreateEx(HINSTANCE /*hinst*/, DWORD /*dwVersion*/,
                            REFIID /*riidltf*/, LPVOID *ppvOut, IUnknown * /*punkOuter*/)
{
    if (ppvOut) *ppvOut = nullptr;
    return E_FAIL;
}

} /* extern "C" */

#endif /* FF_LINUX */

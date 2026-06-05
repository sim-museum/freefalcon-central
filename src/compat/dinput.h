/*
 * FreeFalcon Linux Port - dinput.h compatibility
 *
 * DirectInput 8 (DIRECTINPUT_VERSION 0x0800) types, constants, scancodes
 * and COM-style interfaces.
 *
 * The port intentionally does NOT provide a working DirectInput backend:
 * DirectInput8Create()/DirectInputCreateA()/DirectInputCreateEx() all fail,
 * which causes sim input setup to set gDIEnabled = FALSE and fall through to
 * SDL2 input handling. This header exists so the sim input sources (sijoy.cpp,
 * sikeybd.cpp, simouse.cpp, sidevice.cpp, siloop.cpp, sierror.cpp, ...) and
 * main_linux.cpp's SDL->DIK translation table compile cleanly.
 *
 * Interfaces follow the same COM C layout used by compat/ddraw.h: the only
 * data member is lpVtbl, plus inline C++ wrapper methods that dispatch through
 * the vtable. The stub implementations live in src/compat/dinput_stubs.cpp.
 */

#ifndef FF_COMPAT_DINPUT_H
#define FF_COMPAT_DINPUT_H

#ifdef FF_LINUX

#include "compat_types.h"
#include "objbase.h"
#include <cstdint>
#include <cstddef>   /* offsetof */

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif

/* ============================================================
 * DInput key (scancode) constants - full DX8 standard table
 * ============================================================ */
#define DIK_ESCAPE          0x01
#define DIK_1               0x02
#define DIK_2               0x03
#define DIK_3               0x04
#define DIK_4               0x05
#define DIK_5               0x06
#define DIK_6               0x07
#define DIK_7               0x08
#define DIK_8               0x09
#define DIK_9               0x0A
#define DIK_0               0x0B
#define DIK_MINUS           0x0C    /* - on main keyboard */
#define DIK_EQUALS          0x0D
#define DIK_BACK            0x0E    /* backspace */
#define DIK_TAB             0x0F
#define DIK_Q               0x10
#define DIK_W               0x11
#define DIK_E               0x12
#define DIK_R               0x13
#define DIK_T               0x14
#define DIK_Y               0x15
#define DIK_U               0x16
#define DIK_I               0x17
#define DIK_O               0x18
#define DIK_P               0x19
#define DIK_LBRACKET        0x1A
#define DIK_RBRACKET        0x1B
#define DIK_RETURN          0x1C    /* Enter on main keyboard */
#define DIK_LCONTROL        0x1D
#define DIK_A               0x1E
#define DIK_S               0x1F
#define DIK_D               0x20
#define DIK_F               0x21
#define DIK_G               0x22
#define DIK_H               0x23
#define DIK_J               0x24
#define DIK_K               0x25
#define DIK_L               0x26
#define DIK_SEMICOLON       0x27
#define DIK_APOSTROPHE      0x28
#define DIK_GRAVE           0x29    /* accent grave */
#define DIK_LSHIFT          0x2A
#define DIK_BACKSLASH       0x2B
#define DIK_Z               0x2C
#define DIK_X               0x2D
#define DIK_C               0x2E
#define DIK_V               0x2F
#define DIK_B               0x30
#define DIK_N               0x31
#define DIK_M               0x32
#define DIK_COMMA           0x33
#define DIK_PERIOD          0x34    /* . on main keyboard */
#define DIK_SLASH           0x35    /* / on main keyboard */
#define DIK_RSHIFT          0x36
#define DIK_MULTIPLY        0x37    /* * on numeric keypad */
#define DIK_LMENU           0x38    /* left Alt */
#define DIK_SPACE           0x39
#define DIK_CAPITAL         0x3A    /* CapsLock */
#define DIK_F1              0x3B
#define DIK_F2              0x3C
#define DIK_F3              0x3D
#define DIK_F4              0x3E
#define DIK_F5              0x3F
#define DIK_F6              0x40
#define DIK_F7              0x41
#define DIK_F8              0x42
#define DIK_F9              0x43
#define DIK_F10             0x44
#define DIK_NUMLOCK         0x45
#define DIK_SCROLL          0x46    /* Scroll Lock */
#define DIK_NUMPAD7         0x47
#define DIK_NUMPAD8         0x48
#define DIK_NUMPAD9         0x49
#define DIK_SUBTRACT        0x4A    /* - on numeric keypad */
#define DIK_NUMPAD4         0x4B
#define DIK_NUMPAD5         0x4C
#define DIK_NUMPAD6         0x4D
#define DIK_ADD             0x4E    /* + on numeric keypad */
#define DIK_NUMPAD1         0x4F
#define DIK_NUMPAD2         0x50
#define DIK_NUMPAD3         0x51
#define DIK_NUMPAD0         0x52
#define DIK_DECIMAL         0x53    /* . on numeric keypad */
#define DIK_OEM_102         0x56    /* < > | on UK/Germany keyboards */
#define DIK_F11             0x57
#define DIK_F12             0x58
#define DIK_F13             0x64
#define DIK_F14             0x65
#define DIK_F15             0x66
#define DIK_KANA            0x70    /* (Japanese keyboard) */
#define DIK_ABNT_C1         0x73    /* / ? on Portuguese (Brazilian) keyboards */
#define DIK_CONVERT         0x79    /* (Japanese keyboard) */
#define DIK_NOCONVERT       0x7B    /* (Japanese keyboard) */
#define DIK_YEN             0x7D    /* (Japanese keyboard) */
#define DIK_ABNT_C2         0x7E    /* Numpad . on Portuguese (Brazilian) keyboards */
#define DIK_NUMPADEQUALS    0x8D    /* = on numeric keypad (NEC PC98) */
#define DIK_PREVTRACK       0x90    /* Previous Track (= DIK_CIRCUMFLEX on Japanese keyboard) */
#define DIK_AT              0x91    /* (NEC PC98) */
#define DIK_COLON           0x92    /* (NEC PC98) */
#define DIK_UNDERLINE       0x93    /* (NEC PC98) */
#define DIK_KANJI           0x94    /* (Japanese keyboard) */
#define DIK_STOP            0x95    /* (NEC PC98) */
#define DIK_AX              0x96    /* (Japan AX) */
#define DIK_UNLABELED       0x97    /* (J3100) */
#define DIK_NEXTTRACK       0x99    /* Next Track */
#define DIK_NUMPADENTER     0x9C    /* Enter on numeric keypad */
#define DIK_RCONTROL        0x9D
#define DIK_MUTE            0xA0    /* Mute */
#define DIK_CALCULATOR      0xA1    /* Calculator */
#define DIK_PLAYPAUSE       0xA2    /* Play / Pause */
#define DIK_MEDIASTOP       0xA4    /* Media Stop */
#define DIK_VOLUMEDOWN      0xAE    /* Volume - */
#define DIK_VOLUMEUP        0xB0    /* Volume + */
#define DIK_WEBHOME         0xB2    /* Web home */
#define DIK_NUMPADCOMMA     0xB3    /* , on numeric keypad (NEC PC98) */
#define DIK_DIVIDE          0xB5    /* / on numeric keypad */
#define DIK_SYSRQ           0xB7
#define DIK_RMENU           0xB8    /* right Alt */
#define DIK_PAUSE           0xC5    /* Pause */
#define DIK_HOME            0xC7    /* Home on arrow keypad */
#define DIK_UP              0xC8    /* UpArrow on arrow keypad */
#define DIK_PRIOR           0xC9    /* PgUp on arrow keypad */
#define DIK_LEFT            0xCB    /* LeftArrow on arrow keypad */
#define DIK_RIGHT           0xCD    /* RightArrow on arrow keypad */
#define DIK_END             0xCF    /* End on arrow keypad */
#define DIK_DOWN            0xD0    /* DownArrow on arrow keypad */
#define DIK_NEXT            0xD1    /* PgDn on arrow keypad */
#define DIK_INSERT          0xD2    /* Insert on arrow keypad */
#define DIK_DELETE          0xD3    /* Delete on arrow keypad */
#define DIK_LWIN            0xDB    /* Left Windows key */
#define DIK_RWIN            0xDC    /* Right Windows key */
#define DIK_APPS            0xDD    /* AppMenu key */
#define DIK_POWER           0xDE    /* System Power */
#define DIK_SLEEP           0xDF    /* System Sleep */
#define DIK_WAKE            0xE3    /* System Wake */
#define DIK_WEBSEARCH       0xE5    /* Web Search */
#define DIK_WEBFAVORITES    0xE6    /* Web Favorites */
#define DIK_WEBREFRESH      0xE7    /* Web Refresh */
#define DIK_WEBSTOP         0xE8    /* Web Stop */
#define DIK_WEBFORWARD      0xE9    /* Web Forward */
#define DIK_WEBBACK         0xEA    /* Web Back */
#define DIK_MYCOMPUTER      0xEB    /* My Computer */
#define DIK_MAIL            0xEC    /* Mail */
#define DIK_MEDIASELECT     0xED    /* Media Select */

/* Alternate names */
#define DIK_CIRCUMFLEX      DIK_PREVTRACK
#define DIK_BACKSPACE       DIK_BACK
#define DIK_NUMPADSTAR      DIK_MULTIPLY
#define DIK_LALT            DIK_LMENU
#define DIK_CAPSLOCK        DIK_CAPITAL
#define DIK_NUMPADMINUS     DIK_SUBTRACT
#define DIK_NUMPADPLUS      DIK_ADD
#define DIK_NUMPADPERIOD    DIK_DECIMAL
#define DIK_NUMPADSLASH     DIK_DIVIDE
#define DIK_RALT            DIK_RMENU
#define DIK_UPARROW         DIK_UP
#define DIK_PGUP            DIK_PRIOR
#define DIK_LEFTARROW       DIK_LEFT
#define DIK_RIGHTARROW      DIK_RIGHT
#define DIK_DOWNARROW       DIK_DOWN
#define DIK_PGDN            DIK_NEXT

/* ============================================================
 * Result codes
 * ============================================================ */
#define DI_OK                           S_OK
#define DI_NOTATTACHED                  S_FALSE
#define DI_BUFFEROVERFLOW               S_FALSE
#define DI_PROPNOEFFECT                 S_FALSE
#define DI_NOEFFECT                     S_FALSE
#define DI_POLLEDDEVICE                 ((HRESULT)0x00000002L)
#define DI_DOWNLOADSKIPPED              ((HRESULT)0x00000003L)
#define DI_EFFECTRESTARTED              ((HRESULT)0x00000004L)
#define DI_TRUNCATED                    ((HRESULT)0x00000008L)
#define DI_SETTINGSNOTSAVED             ((HRESULT)0x0000000BL)
#define DI_TRUNCATEDANDRESTARTED        ((HRESULT)0x0000000CL)
#define DI_WRITEPROTECT                 ((HRESULT)0x00000013L)

#define _FACDI 0x9FF
#ifndef MAKE_DIHRESULT
#define MAKE_DIHRESULT(code)            MAKE_HRESULT(1, _FACDI, code)
#endif

#define DIERR_OLDDIRECTINPUTVERSION     MAKE_DIHRESULT(0xBC)
#define DIERR_BETADIRECTINPUTVERSION    MAKE_DIHRESULT(0xBB)
#define DIERR_BADDRIVERVER              MAKE_DIHRESULT(0xBA)
#define DIERR_DEVICENOTREG              ((HRESULT)0x80040154L)  /* REGDB_E_CLASSNOTREG */
#define DIERR_NOTFOUND                  MAKE_DIHRESULT(0x8D)
#define DIERR_OBJECTNOTFOUND            MAKE_DIHRESULT(0x8D)
#define DIERR_INVALIDPARAM              E_INVALIDARG
#define DIERR_NOINTERFACE               ((HRESULT)0x80004002L)  /* E_NOINTERFACE */
#define DIERR_GENERIC                   E_FAIL
#define DIERR_OUTOFMEMORY               E_OUTOFMEMORY
#define DIERR_UNSUPPORTED               E_NOTIMPL
#define DIERR_NOTINITIALIZED            MAKE_DIHRESULT(0x21)
#define DIERR_ALREADYINITIALIZED        MAKE_DIHRESULT(0x22)
#define DIERR_NOAGGREGATION             ((HRESULT)0x80040110L)  /* CLASS_E_NOAGGREGATION */
#define DIERR_OTHERAPPHASPRIO           ((HRESULT)0x80070005L)  /* E_ACCESSDENIED */
#define DIERR_INPUTLOST                 ((HRESULT)0x8007001EL)  /* HRESULT_FROM_WIN32(ERROR_READ_FAULT) */
#define DIERR_ACQUIRED                  ((HRESULT)0x80070005L)  /* E_ACCESSDENIED */
#define DIERR_NOTACQUIRED               ((HRESULT)0x8007000CL)  /* HRESULT_FROM_WIN32(ERROR_INVALID_ACCESS) */
#define DIERR_READONLY                  ((HRESULT)0x80070005L)  /* E_ACCESSDENIED */
#define DIERR_HANDLEEXISTS              ((HRESULT)0x80070005L)  /* E_ACCESSDENIED */
#define DIERR_INSUFFICIENTPRIVS         ((HRESULT)0x80040200L)
#define DIERR_DEVICEFULL                ((HRESULT)0x80040201L)
#define DIERR_MOREDATA                  ((HRESULT)0x80040202L)
#define DIERR_NOTDOWNLOADED             ((HRESULT)0x80040203L)
#define DIERR_HASEFFECTS                ((HRESULT)0x80040204L)
#define DIERR_NOTEXCLUSIVEACQUIRED      ((HRESULT)0x80040205L)
#define DIERR_INCOMPLETEEFFECT          ((HRESULT)0x80040206L)
#define DIERR_NOTBUFFERED               ((HRESULT)0x80040207L)
#define DIERR_EFFECTPLAYING             ((HRESULT)0x80040208L)
#define DIERR_UNPLUGGED                 ((HRESULT)0x80040209L)
#define DIERR_REPORTFULL                ((HRESULT)0x8004020AL)
#define DIERR_MAPFILEFAIL               ((HRESULT)0x8004020BL)

/* ============================================================
 * Property header "How" values
 * ============================================================ */
#define DIPH_DEVICE                     0
#define DIPH_BYOFFSET                   1
#define DIPH_BYID                       2
#define DIPH_BYUSAGE                    3

/* ============================================================
 * Cooperative level flags
 * ============================================================ */
#define DISCL_EXCLUSIVE                 0x00000001
#define DISCL_NONEXCLUSIVE              0x00000002
#define DISCL_FOREGROUND                0x00000004
#define DISCL_BACKGROUND                0x00000008
#define DISCL_NOWINKEY                  0x00000010

/* ============================================================
 * Enumeration callback return values
 * ============================================================ */
#define DIENUM_STOP                     0
#define DIENUM_CONTINUE                 1

/* EnumDevices dwFlags */
#define DIEDFL_ALLDEVICES               0x00000000
#define DIEDFL_ATTACHEDONLY             0x00000001
#define DIEDFL_FORCEFEEDBACK            0x00000100
#define DIEDFL_INCLUDEALIASES           0x00010000
#define DIEDFL_INCLUDEPHANTOMS          0x00020000
#define DIEDFL_INCLUDEHIDDEN            0x00040000

/* GetDeviceData dwFlags */
#define DIGDD_PEEK                      0x00000001

/* ============================================================
 * Device classes / types (DX8 EnumDevices)
 * ============================================================ */
#define DI8DEVCLASS_ALL                 0
#define DI8DEVCLASS_DEVICE              1
#define DI8DEVCLASS_POINTER             2
#define DI8DEVCLASS_KEYBOARD            3
#define DI8DEVCLASS_GAMECTRL            4

/* Legacy (pre-DX8) device type categories used by EnumDevices */
#define DIDEVTYPE_DEVICE                1
#define DIDEVTYPE_MOUSE                 2
#define DIDEVTYPE_KEYBOARD             3
#define DIDEVTYPE_JOYSTICK             4
#define DIDEVTYPE_HID                  0x00010000

/* DIDEVCAPS.dwFlags */
#define DIDC_ATTACHED                  0x00000001
#define DIDC_POLLEDDEVICE              0x00000002
#define DIDC_EMULATED                  0x00000004
#define DIDC_POLLEDDATAFORMAT          0x00000008
#define DIDC_FORCEFEEDBACK             0x00000100
#define DIDC_FFATTACK                  0x00000200
#define DIDC_FFFADE                    0x00000400
#define DIDC_SATURATION                0x00000800
#define DIDC_POSNEGCOEFFICIENTS        0x00001000
#define DIDC_POSNEGSATURATION          0x00002000
#define DIDC_DEADBAND                  0x00004000
#define DIDC_STARTDELAY                0x00008000
#define DIDC_ALIAS                     0x00010000
#define DIDC_PHANTOM                   0x00020000

/* ============================================================
 * Data format flags / object flags
 * ============================================================ */
#define DIDF_ABSAXIS                   0x00000001
#define DIDF_RELAXIS                   0x00000002

/* DIDATAFORMAT.dwFlags / DIOBJECTDATAFORMAT.dwType field components */
#define DIDFT_ALL                      0x00000000
#define DIDFT_RELAXIS                  0x00000001
#define DIDFT_ABSAXIS                  0x00000002
#define DIDFT_AXIS                     0x00000003
#define DIDFT_PSHBUTTON                0x00000004
#define DIDFT_TGLBUTTON                0x00000008
#define DIDFT_BUTTON                   0x0000000C
#define DIDFT_POV                      0x00000010
#define DIDFT_COLLECTION               0x00000040
#define DIDFT_NODATA                   0x00000080
#define DIDFT_ANYINSTANCE              0x00FFFF00
#define DIDFT_INSTANCEMASK             DIDFT_ANYINSTANCE
#define DIDFT_FFACTUATOR               0x01000000
#define DIDFT_FFEFFECTTRIGGER          0x02000000
#define DIDFT_OUTPUT                   0x10000000
#define DIDFT_VENDORDEFINED            0x04000000
#define DIDFT_ALIAS                    0x08000000
#define DIDFT_OPTIONAL                 0x80000000
#define DIDFT_GETTYPE(n)               LOBYTE(n)
#define DIDFT_MAKEINSTANCE(n)          ((WORD)(n) << 8)
#define DIDFT_GETINSTANCE(n)           LOWORD((n) >> 8)
#define DIDFT_ENUMCOLLECTION(n)        ((WORD)(n) << 8)
#define DIDFT_NOCOLLECTION             0x00FFFF00

/* ============================================================
 * Force-feedback effect types / flags
 * ============================================================ */
#define DIEFT_ALL                      0x00000000
#define DIEFT_CONSTANTFORCE            0x00000001
#define DIEFT_RAMPFORCE               0x00000002
#define DIEFT_PERIODIC                0x00000003
#define DIEFT_CONDITION               0x00000004
#define DIEFT_CUSTOMFORCE             0x00000005
#define DIEFT_HARDWARE                0x000000FF
#define DIEFT_FFATTACK                0x00000200
#define DIEFT_FFFADE                  0x00000400
#define DIEFT_SATURATION              0x00000800
#define DIEFT_POSNEGCOEFFICIENTS      0x00001000
#define DIEFT_POSNEGSATURATION        0x00002000
#define DIEFT_DEADBAND                0x00004000
#define DIEFT_STARTDELAY              0x00008000
#define DIEFT_GETTYPE(n)              LOBYTE(n)

/* DIEFFECT.dwFlags */
#define DIEFF_OBJECTIDS               0x00000001
#define DIEFF_OBJECTOFFSETS           0x00000002
#define DIEFF_CARTESIAN               0x00000010
#define DIEFF_POLAR                   0x00000020
#define DIEFF_SPHERICAL               0x00000040

/* SetParameters dwFlags (DIEP_*) */
#define DIEP_DURATION                 0x00000001
#define DIEP_SAMPLEPERIOD             0x00000002
#define DIEP_GAIN                     0x00000004
#define DIEP_TRIGGERBUTTON            0x00000008
#define DIEP_TRIGGERREPEATINTERVAL    0x00000010
#define DIEP_AXES                     0x00000020
#define DIEP_DIRECTION                0x00000040
#define DIEP_ENVELOPE                 0x00000080
#define DIEP_TYPESPECIFICPARAMS       0x00000100
#define DIEP_STARTDELAY               0x00000200
#define DIEP_ALLPARAMS_DX5            0x000001FF
#define DIEP_ALLPARAMS                0x000003FF
#define DIEP_START                    0x20000000
#define DIEP_NORESTART                0x40000000
#define DIEP_NODOWNLOAD               0x80000000

/* Effect Start() flags */
#define DIES_SOLO                     0x00000001
#define DIES_NODOWNLOAD               0x80000000

/* Effect Stop()/play count */
#define INFINITE                      0xFFFFFFFF

/* ============================================================
 * AUTOCENTER property values
 * ============================================================ */
#define DIPROPAUTOCENTER_OFF          0
#define DIPROPAUTOCENTER_ON           1

#define DIPROPAXISMODE_ABS            0
#define DIPROPAXISMODE_REL            1

/* ============================================================
 * Helpers macros referenced by DIDFT_* above
 * ============================================================ */
#ifndef LOBYTE
#define LOBYTE(w) ((BYTE)((w) & 0xff))
#endif
#ifndef LOWORD
#define LOWORD(l) ((WORD)((l) & 0xffff))
#endif

/* ============================================================
 * Forward declarations / interface typedefs
 * ============================================================ */
struct IDirectInputA;
struct IDirectInputDeviceA;
struct IDirectInputEffect;

typedef struct IDirectInputA        *LPDIRECTINPUT;
typedef struct IDirectInputA        *LPDIRECTINPUT7;
typedef struct IDirectInputA        *LPDIRECTINPUT8;
typedef struct IDirectInputA        *LPDIRECTINPUT7A;
typedef struct IDirectInputA        *LPDIRECTINPUT8A;

typedef struct IDirectInputDeviceA  *LPDIRECTINPUTDEVICE;
typedef struct IDirectInputDeviceA  *LPDIRECTINPUTDEVICE2;
typedef struct IDirectInputDeviceA  *LPDIRECTINPUTDEVICE7;
typedef struct IDirectInputDeviceA  *LPDIRECTINPUTDEVICE8;
typedef struct IDirectInputDeviceA  *LPDIRECTINPUTDEVICE2A;
typedef struct IDirectInputDeviceA  *LPDIRECTINPUTDEVICE7A;
typedef struct IDirectInputDeviceA  *LPDIRECTINPUTDEVICE8A;

typedef struct IDirectInputEffect   *LPDIRECTINPUTEFFECT;

/* Source code uses the plain (un-suffixed) interface struct names too */
typedef struct IDirectInputA        IDirectInput7;
typedef struct IDirectInputA        IDirectInput8;
typedef struct IDirectInputDeviceA  IDirectInputDevice2;
typedef struct IDirectInputDeviceA  IDirectInputDevice7;
typedef struct IDirectInputDeviceA  IDirectInputDevice8;

/* ============================================================
 * Structures
 * ============================================================ */
typedef struct DIDEVICEOBJECTDATA {
    DWORD    dwOfs;
    DWORD    dwData;
    DWORD    dwTimeStamp;
    DWORD    dwSequence;
    UINT_PTR uAppData;
} DIDEVICEOBJECTDATA, *LPDIDEVICEOBJECTDATA;
typedef const DIDEVICEOBJECTDATA *LPCDIDEVICEOBJECTDATA;

typedef struct DIJOYSTATE {
    LONG  lX;
    LONG  lY;
    LONG  lZ;
    LONG  lRx;
    LONG  lRy;
    LONG  lRz;
    LONG  rglSlider[2];
    DWORD rgdwPOV[4];
    BYTE  rgbButtons[32];
} DIJOYSTATE, *LPDIJOYSTATE;

typedef struct DIJOYSTATE2 {
    LONG  lX;
    LONG  lY;
    LONG  lZ;
    LONG  lRx;
    LONG  lRy;
    LONG  lRz;
    LONG  rglSlider[2];
    DWORD rgdwPOV[4];
    BYTE  rgbButtons[128];
    LONG  lVX;            /* 'v' as in velocity */
    LONG  lVY;
    LONG  lVZ;
    LONG  lVRx;
    LONG  lVRy;
    LONG  lVRz;
    LONG  rglVSlider[2];
    LONG  lAX;            /* 'a' as in acceleration */
    LONG  lAY;
    LONG  lAZ;
    LONG  lARx;
    LONG  lARy;
    LONG  lARz;
    LONG  rglASlider[2];
    LONG  lFX;            /* 'f' as in force */
    LONG  lFY;
    LONG  lFZ;
    LONG  lFRx;          /* 'fr' as in rotational force == torque */
    LONG  lFRy;
    LONG  lFRz;
    LONG  rglFSlider[2];
} DIJOYSTATE2, *LPDIJOYSTATE2;

typedef struct DIMOUSESTATE {
    LONG lX;
    LONG lY;
    LONG lZ;
    BYTE rgbButtons[4];
} DIMOUSESTATE, *LPDIMOUSESTATE;

typedef struct DIMOUSESTATE2 {
    LONG lX;
    LONG lY;
    LONG lZ;
    BYTE rgbButtons[8];
} DIMOUSESTATE2, *LPDIMOUSESTATE2;

typedef struct DIPROPHEADER {
    DWORD dwSize;
    DWORD dwHeaderSize;
    DWORD dwObj;
    DWORD dwHow;
} DIPROPHEADER, *LPDIPROPHEADER;
typedef const DIPROPHEADER *LPCDIPROPHEADER;

typedef struct DIPROPDWORD {
    DIPROPHEADER diph;
    DWORD        dwData;
} DIPROPDWORD, *LPDIPROPDWORD;
typedef const DIPROPDWORD *LPCDIPROPDWORD;

#define MAXCPOINTSNUM 8
typedef struct _CPOINT {
    LONG  lP;
    DWORD dwLog;
} CPOINT, *PCPOINT;

typedef struct DIPROPCPOINTS {
    DIPROPHEADER diph;
    DWORD dwCPointsNum;
    CPOINT cp[MAXCPOINTSNUM];
} DIPROPCPOINTS, *LPDIPROPCPOINTS;

typedef struct DIPROPRANGE {
    DIPROPHEADER diph;
    LONG         lMin;
    LONG         lMax;
} DIPROPRANGE, *LPDIPROPRANGE;
typedef const DIPROPRANGE *LPCDIPROPRANGE;

typedef struct DIPROPCAL {
    DIPROPHEADER diph;
    LONG         lMin;
    LONG         lCenter;
    LONG         lMax;
} DIPROPCAL, *LPDIPROPCAL;

typedef struct DIDEVCAPS {
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwDevType;
    DWORD dwAxes;
    DWORD dwButtons;
    DWORD dwPOVs;
    /* DX5+ fields */
    DWORD dwFFSamplePeriod;
    DWORD dwFFMinTimeResolution;
    DWORD dwFirmwareRevision;
    DWORD dwHardwareRevision;
    DWORD dwFFDriverVersion;
} DIDEVCAPS, *LPDIDEVCAPS;

typedef struct DIDEVICEINSTANCEA {
    DWORD dwSize;
    GUID  guidInstance;
    GUID  guidProduct;
    DWORD dwDevType;
    char  tszInstanceName[260];
    char  tszProductName[260];
    GUID  guidFFDriver;
    WORD  wUsagePage;
    WORD  wUsage;
} DIDEVICEINSTANCEA, *LPDIDEVICEINSTANCEA;
typedef const DIDEVICEINSTANCEA *LPCDIDEVICEINSTANCEA;

typedef struct DIDEVICEOBJECTINSTANCEA {
    DWORD dwSize;
    GUID  guidType;
    DWORD dwOfs;
    DWORD dwType;
    DWORD dwFlags;
    char  tszName[260];
    DWORD dwFFMaxForce;
    DWORD dwFFForceResolution;
    WORD  wCollectionNumber;
    WORD  wDesignatorIndex;
    WORD  wUsagePage;
    WORD  wUsage;
    DWORD dwDimension;
    WORD  wExponent;
    WORD  wReportId;
} DIDEVICEOBJECTINSTANCEA, *LPDIDEVICEOBJECTINSTANCEA;
typedef const DIDEVICEOBJECTINSTANCEA *LPCDIDEVICEOBJECTINSTANCEA;

typedef struct _DIOBJECTDATAFORMAT {
    const GUID *pguid;
    DWORD       dwOfs;
    DWORD       dwType;
    DWORD       dwFlags;
} DIOBJECTDATAFORMAT, *LPDIOBJECTDATAFORMAT;
typedef const DIOBJECTDATAFORMAT *LPCDIOBJECTDATAFORMAT;

typedef struct _DIDATAFORMAT {
    DWORD                dwSize;
    DWORD                dwObjSize;
    DWORD                dwFlags;
    DWORD                dwDataSize;
    DWORD                dwNumObjs;
    LPDIOBJECTDATAFORMAT rgodf;
} DIDATAFORMAT, *LPDIDATAFORMAT;
typedef const DIDATAFORMAT *LPCDIDATAFORMAT;

/* ---- Force-feedback structures ---- */
typedef struct DIENVELOPE {
    DWORD dwSize;
    DWORD dwAttackLevel;
    DWORD dwAttackTime;
    DWORD dwFadeLevel;
    DWORD dwFadeTime;
} DIENVELOPE, *LPDIENVELOPE;
typedef const DIENVELOPE *LPCDIENVELOPE;

typedef struct DIEFFECT {
    DWORD  dwSize;
    DWORD  dwFlags;
    DWORD  dwDuration;
    DWORD  dwSamplePeriod;
    DWORD  dwGain;
    DWORD  dwTriggerButton;
    DWORD  dwTriggerRepeatInterval;
    DWORD  cAxes;
    LPDWORD rgdwAxes;
    LPLONG rglDirection;
    LPDIENVELOPE lpEnvelope;
    DWORD  cbTypeSpecificParams;
    LPVOID lpvTypeSpecificParams;
    DWORD  dwStartDelay;
} DIEFFECT, *LPDIEFFECT;
typedef const DIEFFECT *LPCDIEFFECT;

typedef struct DICONSTANTFORCE {
    LONG lMagnitude;
} DICONSTANTFORCE, *LPDICONSTANTFORCE;
typedef const DICONSTANTFORCE *LPCDICONSTANTFORCE;

typedef struct DIRAMPFORCE {
    LONG lStart;
    LONG lEnd;
} DIRAMPFORCE, *LPDIRAMPFORCE;
typedef const DIRAMPFORCE *LPCDIRAMPFORCE;

typedef struct DIPERIODIC {
    DWORD dwMagnitude;
    LONG  lOffset;
    DWORD dwPhase;
    DWORD dwPeriod;
} DIPERIODIC, *LPDIPERIODIC;
typedef const DIPERIODIC *LPCDIPERIODIC;

typedef struct DICONDITION {
    LONG  lOffset;
    LONG  lPositiveCoefficient;
    LONG  lNegativeCoefficient;
    DWORD dwPositiveSaturation;
    DWORD dwNegativeSaturation;
    LONG  lDeadBand;
} DICONDITION, *LPDICONDITION;
typedef const DICONDITION *LPCDICONDITION;

typedef struct DICUSTOMFORCE {
    DWORD  cChannels;
    DWORD  dwSamplePeriod;
    DWORD  cSamples;
    LPLONG rglForceData;
} DICUSTOMFORCE, *LPDICUSTOMFORCE;
typedef const DICUSTOMFORCE *LPCDICUSTOMFORCE;

typedef struct DIEFFECTINFOA {
    DWORD dwSize;
    GUID  guid;
    DWORD dwEffType;
    DWORD dwStaticParams;
    DWORD dwDynamicParams;
    char  tszName[260];
} DIEFFECTINFOA, *LPDIEFFECTINFOA;
typedef const DIEFFECTINFOA *LPCDIEFFECTINFOA;

/* ANSI build: un-suffixed aliases */
typedef DIDEVICEINSTANCEA        DIDEVICEINSTANCE,       *LPDIDEVICEINSTANCE;
typedef const DIDEVICEINSTANCEA *LPCDIDEVICEINSTANCE;
typedef DIDEVICEOBJECTINSTANCEA  DIDEVICEOBJECTINSTANCE, *LPDIDEVICEOBJECTINSTANCE;
typedef const DIDEVICEOBJECTINSTANCEA *LPCDIDEVICEOBJECTINSTANCE;
typedef DIEFFECTINFOA            DIEFFECTINFO,           *LPDIEFFECTINFO;
typedef const DIEFFECTINFOA     *LPCDIEFFECTINFO;

/* ============================================================
 * Predefined data format byte offsets (DIJOFS_* / DIMOFS_*)
 * ============================================================ */
#define DIJOFS_X            (DWORD)offsetof(DIJOYSTATE, lX)
#define DIJOFS_Y            (DWORD)offsetof(DIJOYSTATE, lY)
#define DIJOFS_Z            (DWORD)offsetof(DIJOYSTATE, lZ)
#define DIJOFS_RX           (DWORD)offsetof(DIJOYSTATE, lRx)
#define DIJOFS_RY           (DWORD)offsetof(DIJOYSTATE, lRy)
#define DIJOFS_RZ           (DWORD)offsetof(DIJOYSTATE, lRz)
#define DIJOFS_SLIDER(n)    ((DWORD)(offsetof(DIJOYSTATE, rglSlider) + (n) * sizeof(LONG)))
#define DIJOFS_POV(n)       ((DWORD)(offsetof(DIJOYSTATE, rgdwPOV) + (n) * sizeof(DWORD)))
#define DIJOFS_BUTTON(n)    ((DWORD)(offsetof(DIJOYSTATE, rgbButtons) + (n)))
#define DIJOFS_BUTTON0      DIJOFS_BUTTON(0)
#define DIJOFS_BUTTON1      DIJOFS_BUTTON(1)
#define DIJOFS_BUTTON2      DIJOFS_BUTTON(2)
#define DIJOFS_BUTTON3      DIJOFS_BUTTON(3)

#define DIMOFS_X            (DWORD)offsetof(DIMOUSESTATE2, lX)
#define DIMOFS_Y            (DWORD)offsetof(DIMOUSESTATE2, lY)
#define DIMOFS_Z            (DWORD)offsetof(DIMOUSESTATE2, lZ)
#define DIMOFS_BUTTON(n)    ((DWORD)(offsetof(DIMOUSESTATE2, rgbButtons) + (n)))
#define DIMOFS_BUTTON0      DIMOFS_BUTTON(0)
#define DIMOFS_BUTTON1      DIMOFS_BUTTON(1)
#define DIMOFS_BUTTON2      DIMOFS_BUTTON(2)
#define DIMOFS_BUTTON3      DIMOFS_BUTTON(3)
#define DIMOFS_BUTTON4      DIMOFS_BUTTON(4)
#define DIMOFS_BUTTON5      DIMOFS_BUTTON(5)
#define DIMOFS_BUTTON6      DIMOFS_BUTTON(6)
#define DIMOFS_BUTTON7      DIMOFS_BUTTON(7)

/* ============================================================
 * Callback typedefs
 * ============================================================ */
typedef BOOL (CALLBACK *LPDIENUMDEVICESCALLBACKA)(const DIDEVICEINSTANCEA *, LPVOID);
typedef LPDIENUMDEVICESCALLBACKA LPDIENUMDEVICESCALLBACK;
typedef BOOL (CALLBACK *LPDIENUMDEVICEOBJECTSCALLBACKA)(const DIDEVICEOBJECTINSTANCEA *, LPVOID);
typedef LPDIENUMDEVICEOBJECTSCALLBACKA LPDIENUMDEVICEOBJECTSCALLBACK;
typedef BOOL (CALLBACK *LPDIENUMEFFECTSCALLBACKA)(const DIEFFECTINFOA *, LPVOID);
typedef LPDIENUMEFFECTSCALLBACKA LPDIENUMEFFECTSCALLBACK;
typedef BOOL (CALLBACK *LPDIENUMCREATEDEFFECTOBJECTSCALLBACK)(LPDIRECTINPUTEFFECT, LPVOID);

/* ============================================================
 * Property GUIDs and interface IID / device GUIDs.
 *
 * In real DirectInput the DIPROP_* macros are MAKEDIPROP(n), i.e. a
 * (REFGUID)(uintptr_t)n cast. Our stub SetProperty()/GetProperty() never
 * dereference the GUID, but to keep "pass a const GUID&" valid (REFGUID is
 * "const GUID &" in C++), each property is a real static const GUID object.
 * ============================================================ */
#ifdef __cplusplus
static const GUID DIPROP_BUFFERSIZE     = { 1, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
static const GUID DIPROP_AXISMODE       = { 2, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
static const GUID DIPROP_GRANULARITY    = { 3, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
static const GUID DIPROP_RANGE          = { 4, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
static const GUID DIPROP_DEADZONE       = { 5, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
static const GUID DIPROP_SATURATION     = { 6, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
static const GUID DIPROP_FFGAIN         = { 7, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
static const GUID DIPROP_FFLOAD         = { 8, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
static const GUID DIPROP_AUTOCENTER     = { 9, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
static const GUID DIPROP_CALIBRATIONMODE= { 10, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
static const GUID DIPROP_CALIBRATION    = { 11, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
static const GUID DIPROP_GUIDANDPATH    = { 12, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
static const GUID DIPROP_INSTANCENAME   = { 13, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
static const GUID DIPROP_PRODUCTNAME    = { 14, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };

/* Interface IIDs (values not meaningful on Linux; kept distinct) */
static const GUID IID_IDirectInput7 =
    { 0x9a4cb685, 0x236d, 0x11d3, { 0x8e, 0x9d, 0x00, 0xc0, 0x4f, 0x68, 0x44, 0xae } };
static const GUID IID_IDirectInput8 =
    { 0xbf798031, 0x483a, 0x4da2, { 0xaa, 0x99, 0x5d, 0x64, 0xed, 0x36, 0x97, 0x00 } };
static const GUID IID_IDirectInputDevice2 =
    { 0x5944e682, 0xc92e, 0x11cf, { 0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
static const GUID IID_IDirectInputDevice7 =
    { 0x57d7c6bc, 0x2356, 0x11d3, { 0x8e, 0x9d, 0x00, 0xc0, 0x4f, 0x68, 0x44, 0xae } };
static const GUID IID_IDirectInputDevice8 =
    { 0x54d41080, 0xdc15, 0x4833, { 0xa4, 0x1b, 0x74, 0x8f, 0x73, 0xa3, 0x81, 0x79 } };
#endif /* __cplusplus */

/* System device / axis / effect GUIDs - defined in dinput_stubs.cpp */
#ifdef __cplusplus
extern "C" {
#endif
extern const GUID GUID_SysKeyboard;
extern const GUID GUID_SysMouse;
extern const GUID GUID_Joystick;
extern const GUID GUID_XAxis;
extern const GUID GUID_YAxis;
extern const GUID GUID_ZAxis;
extern const GUID GUID_RxAxis;
extern const GUID GUID_RyAxis;
extern const GUID GUID_RzAxis;
extern const GUID GUID_Slider;
extern const GUID GUID_Button;
extern const GUID GUID_Key;
extern const GUID GUID_POV;
extern const GUID GUID_ConstantForce;
extern const GUID GUID_RampForce;
extern const GUID GUID_Square;
extern const GUID GUID_Sine;
extern const GUID GUID_Triangle;
extern const GUID GUID_SawtoothUp;
extern const GUID GUID_SawtoothDown;
extern const GUID GUID_Spring;
extern const GUID GUID_Damper;
extern const GUID GUID_Inertia;
extern const GUID GUID_Friction;
extern const GUID GUID_CustomForce;
#ifdef __cplusplus
}
#endif

/* ============================================================
 * Predefined data formats (defined in dinput_stubs.cpp)
 * ============================================================ */
extern const DIDATAFORMAT c_dfDIKeyboard;
extern const DIDATAFORMAT c_dfDIMouse;
extern const DIDATAFORMAT c_dfDIMouse2;
extern const DIDATAFORMAT c_dfDIJoystick;
extern const DIDATAFORMAT c_dfDIJoystick2;

/* ============================================================
 * IDirectInputEffect
 * ============================================================ */
typedef struct IDirectInputEffectVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IDirectInputEffect *This, REFIID riid, void **ppvObj);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IDirectInputEffect *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IDirectInputEffect *This);
    HRESULT (STDMETHODCALLTYPE *Initialize)(IDirectInputEffect *This, void *hinst, DWORD dwVersion, REFGUID rguid);
    HRESULT (STDMETHODCALLTYPE *GetEffectGuid)(IDirectInputEffect *This, GUID *pguid);
    HRESULT (STDMETHODCALLTYPE *GetParameters)(IDirectInputEffect *This, LPDIEFFECT peff, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *SetParameters)(IDirectInputEffect *This, LPCDIEFFECT peff, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *Start)(IDirectInputEffect *This, DWORD dwIterations, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *Stop)(IDirectInputEffect *This);
    HRESULT (STDMETHODCALLTYPE *GetEffectStatus)(IDirectInputEffect *This, LPDWORD pdwFlags);
    HRESULT (STDMETHODCALLTYPE *Download)(IDirectInputEffect *This);
    HRESULT (STDMETHODCALLTYPE *Unload)(IDirectInputEffect *This);
    HRESULT (STDMETHODCALLTYPE *Escape)(IDirectInputEffect *This, void *pesc);
} IDirectInputEffectVtbl;

struct IDirectInputEffect {
    IDirectInputEffectVtbl *lpVtbl;
#ifdef __cplusplus
    HRESULT QueryInterface(REFIID riid, void **ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG   AddRef()  { return lpVtbl->AddRef(this); }
    ULONG   Release() { return lpVtbl->Release(this); }
    HRESULT Initialize(void *a, DWORD b, REFGUID c) { return lpVtbl->Initialize(this, a, b, c); }
    HRESULT GetEffectGuid(GUID *a) { return lpVtbl->GetEffectGuid(this, a); }
    HRESULT GetParameters(LPDIEFFECT a, DWORD b) { return lpVtbl->GetParameters(this, a, b); }
    HRESULT SetParameters(LPCDIEFFECT a, DWORD b) { return lpVtbl->SetParameters(this, a, b); }
    HRESULT Start(DWORD a, DWORD b) { return lpVtbl->Start(this, a, b); }
    HRESULT Stop() { return lpVtbl->Stop(this); }
    HRESULT GetEffectStatus(LPDWORD a) { return lpVtbl->GetEffectStatus(this, a); }
    HRESULT Download() { return lpVtbl->Download(this); }
    HRESULT Unload() { return lpVtbl->Unload(this); }
    HRESULT Escape(void *a) { return lpVtbl->Escape(this, a); }
#endif
};

/* ============================================================
 * IDirectInputDevice (combined IDirectInputDevice2/7/8 shape)
 * ============================================================ */
typedef struct IDirectInputDeviceVtbl {
    /* IUnknown */
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IDirectInputDeviceA *This, REFIID riid, void **ppvObj);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IDirectInputDeviceA *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IDirectInputDeviceA *This);
    /* IDirectInputDevice */
    HRESULT (STDMETHODCALLTYPE *GetCapabilities)(IDirectInputDeviceA *This, LPDIDEVCAPS lpDIDevCaps);
    HRESULT (STDMETHODCALLTYPE *EnumObjects)(IDirectInputDeviceA *This, LPDIENUMDEVICEOBJECTSCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetProperty)(IDirectInputDeviceA *This, REFGUID rguidProp, LPDIPROPHEADER pdiph);
    HRESULT (STDMETHODCALLTYPE *SetProperty)(IDirectInputDeviceA *This, REFGUID rguidProp, LPCDIPROPHEADER pdiph);
    HRESULT (STDMETHODCALLTYPE *Acquire)(IDirectInputDeviceA *This);
    HRESULT (STDMETHODCALLTYPE *Unacquire)(IDirectInputDeviceA *This);
    HRESULT (STDMETHODCALLTYPE *GetDeviceState)(IDirectInputDeviceA *This, DWORD cbData, LPVOID lpvData);
    HRESULT (STDMETHODCALLTYPE *GetDeviceData)(IDirectInputDeviceA *This, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *SetDataFormat)(IDirectInputDeviceA *This, LPCDIDATAFORMAT lpdf);
    HRESULT (STDMETHODCALLTYPE *SetEventNotification)(IDirectInputDeviceA *This, HANDLE hEvent);
    HRESULT (STDMETHODCALLTYPE *SetCooperativeLevel)(IDirectInputDeviceA *This, HWND hwnd, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetObjectInfo)(IDirectInputDeviceA *This, LPDIDEVICEOBJECTINSTANCEA pdidoi, DWORD dwObj, DWORD dwHow);
    HRESULT (STDMETHODCALLTYPE *GetDeviceInfo)(IDirectInputDeviceA *This, LPDIDEVICEINSTANCEA pdidi);
    HRESULT (STDMETHODCALLTYPE *RunControlPanel)(IDirectInputDeviceA *This, HWND hwndOwner, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *Initialize)(IDirectInputDeviceA *This, void *hinst, DWORD dwVersion, REFGUID rguid);
    /* IDirectInputDevice2 */
    HRESULT (STDMETHODCALLTYPE *CreateEffect)(IDirectInputDeviceA *This, REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT *ppdeff, IUnknown *punkOuter);
    HRESULT (STDMETHODCALLTYPE *EnumEffects)(IDirectInputDeviceA *This, LPDIENUMEFFECTSCALLBACKA lpCallback, LPVOID pvRef, DWORD dwEffType);
    HRESULT (STDMETHODCALLTYPE *GetEffectInfo)(IDirectInputDeviceA *This, LPDIEFFECTINFOA pdei, REFGUID rguid);
    HRESULT (STDMETHODCALLTYPE *GetForceFeedbackState)(IDirectInputDeviceA *This, LPDWORD pdwOut);
    HRESULT (STDMETHODCALLTYPE *SendForceFeedbackCommand)(IDirectInputDeviceA *This, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *EnumCreatedEffectObjects)(IDirectInputDeviceA *This, LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD fl);
    HRESULT (STDMETHODCALLTYPE *Escape)(IDirectInputDeviceA *This, void *pesc);
    HRESULT (STDMETHODCALLTYPE *Poll)(IDirectInputDeviceA *This);
    HRESULT (STDMETHODCALLTYPE *SendDeviceData)(IDirectInputDeviceA *This, DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl);
} IDirectInputDeviceVtbl;

struct IDirectInputDeviceA {
    IDirectInputDeviceVtbl *lpVtbl;
#ifdef __cplusplus
    HRESULT QueryInterface(REFIID riid, void **ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG   AddRef()  { return lpVtbl->AddRef(this); }
    ULONG   Release() { return lpVtbl->Release(this); }
    HRESULT GetCapabilities(LPDIDEVCAPS a) { return lpVtbl->GetCapabilities(this, a); }
    HRESULT EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACKA a, LPVOID b, DWORD c) { return lpVtbl->EnumObjects(this, a, b, c); }
    HRESULT GetProperty(REFGUID a, LPDIPROPHEADER b) { return lpVtbl->GetProperty(this, a, b); }
    HRESULT SetProperty(REFGUID a, LPCDIPROPHEADER b) { return lpVtbl->SetProperty(this, a, b); }
    HRESULT Acquire() { return lpVtbl->Acquire(this); }
    HRESULT Unacquire() { return lpVtbl->Unacquire(this); }
    HRESULT GetDeviceState(DWORD a, LPVOID b) { return lpVtbl->GetDeviceState(this, a, b); }
    HRESULT GetDeviceData(DWORD a, LPDIDEVICEOBJECTDATA b, LPDWORD c, DWORD d) { return lpVtbl->GetDeviceData(this, a, b, c, d); }
    HRESULT SetDataFormat(LPCDIDATAFORMAT a) { return lpVtbl->SetDataFormat(this, a); }
    HRESULT SetEventNotification(HANDLE a) { return lpVtbl->SetEventNotification(this, a); }
    HRESULT SetCooperativeLevel(HWND a, DWORD b) { return lpVtbl->SetCooperativeLevel(this, a, b); }
    HRESULT GetObjectInfo(LPDIDEVICEOBJECTINSTANCEA a, DWORD b, DWORD c) { return lpVtbl->GetObjectInfo(this, a, b, c); }
    HRESULT GetDeviceInfo(LPDIDEVICEINSTANCEA a) { return lpVtbl->GetDeviceInfo(this, a); }
    HRESULT RunControlPanel(HWND a, DWORD b) { return lpVtbl->RunControlPanel(this, a, b); }
    HRESULT Initialize(void *a, DWORD b, REFGUID c) { return lpVtbl->Initialize(this, a, b, c); }
    HRESULT CreateEffect(REFGUID a, LPCDIEFFECT b, LPDIRECTINPUTEFFECT *c, IUnknown *d) { return lpVtbl->CreateEffect(this, a, b, c, d); }
    HRESULT EnumEffects(LPDIENUMEFFECTSCALLBACKA a, LPVOID b, DWORD c) { return lpVtbl->EnumEffects(this, a, b, c); }
    HRESULT GetEffectInfo(LPDIEFFECTINFOA a, REFGUID b) { return lpVtbl->GetEffectInfo(this, a, b); }
    HRESULT GetForceFeedbackState(LPDWORD a) { return lpVtbl->GetForceFeedbackState(this, a); }
    HRESULT SendForceFeedbackCommand(DWORD a) { return lpVtbl->SendForceFeedbackCommand(this, a); }
    HRESULT EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK a, LPVOID b, DWORD c) { return lpVtbl->EnumCreatedEffectObjects(this, a, b, c); }
    HRESULT Escape(void *a) { return lpVtbl->Escape(this, a); }
    HRESULT Poll() { return lpVtbl->Poll(this); }
    HRESULT SendDeviceData(DWORD a, LPCDIDEVICEOBJECTDATA b, LPDWORD c, DWORD d) { return lpVtbl->SendDeviceData(this, a, b, c, d); }
#endif
};

/* ============================================================
 * IDirectInput (combined IDirectInput7/8 shape)
 * ============================================================ */
typedef struct IDirectInputVtbl {
    /* IUnknown */
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(IDirectInputA *This, REFIID riid, void **ppvObj);
    ULONG   (STDMETHODCALLTYPE *AddRef)(IDirectInputA *This);
    ULONG   (STDMETHODCALLTYPE *Release)(IDirectInputA *This);
    /* IDirectInput */
    HRESULT (STDMETHODCALLTYPE *CreateDevice)(IDirectInputA *This, REFGUID rguid, LPDIRECTINPUTDEVICE *lplpDirectInputDevice, IUnknown *pUnkOuter);
    HRESULT (STDMETHODCALLTYPE *EnumDevices)(IDirectInputA *This, DWORD dwDevType, LPDIENUMDEVICESCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *GetDeviceStatus)(IDirectInputA *This, REFGUID rguidInstance);
    HRESULT (STDMETHODCALLTYPE *RunControlPanel)(IDirectInputA *This, HWND hwndOwner, DWORD dwFlags);
    HRESULT (STDMETHODCALLTYPE *Initialize)(IDirectInputA *This, void *hinst, DWORD dwVersion);
    /* IDirectInput2 / IDirectInput7 */
    HRESULT (STDMETHODCALLTYPE *FindDevice)(IDirectInputA *This, REFGUID rguid, const char *pszName, GUID *pguidInstance);
    HRESULT (STDMETHODCALLTYPE *CreateDeviceEx)(IDirectInputA *This, REFGUID rguid, REFIID riid, LPVOID *ppvObj, IUnknown *pUnkOuter);
} IDirectInputVtbl;

struct IDirectInputA {
    IDirectInputVtbl *lpVtbl;
#ifdef __cplusplus
    HRESULT QueryInterface(REFIID riid, void **ppv) { return lpVtbl->QueryInterface(this, riid, ppv); }
    ULONG   AddRef()  { return lpVtbl->AddRef(this); }
    ULONG   Release() { return lpVtbl->Release(this); }
    HRESULT CreateDevice(REFGUID a, LPDIRECTINPUTDEVICE *b, IUnknown *c) { return lpVtbl->CreateDevice(this, a, b, c); }
    HRESULT EnumDevices(DWORD a, LPDIENUMDEVICESCALLBACKA b, LPVOID c, DWORD d) { return lpVtbl->EnumDevices(this, a, b, c, d); }
    HRESULT GetDeviceStatus(REFGUID a) { return lpVtbl->GetDeviceStatus(this, a); }
    HRESULT RunControlPanel(HWND a, DWORD b) { return lpVtbl->RunControlPanel(this, a, b); }
    HRESULT Initialize(void *a, DWORD b) { return lpVtbl->Initialize(this, a, b); }
    HRESULT FindDevice(REFGUID a, const char *b, GUID *c) { return lpVtbl->FindDevice(this, a, b, c); }
    HRESULT CreateDeviceEx(REFGUID a, REFIID b, LPVOID *c, IUnknown *d) { return lpVtbl->CreateDeviceEx(this, a, b, c, d); }
#endif
};

/* ============================================================
 * Creation entry points (implemented in dinput_stubs.cpp).
 * All deliberately fail on Linux so the engine falls back to SDL input.
 * ============================================================ */
#ifdef __cplusplus
extern "C" {
#endif

HRESULT DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, IUnknown *punkOuter);
HRESULT DirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, LPDIRECTINPUT *ppDI, IUnknown *punkOuter);
HRESULT DirectInputCreateEx(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, IUnknown *punkOuter);

#ifdef __cplusplus
}
#endif

static const GUID DIPROP_CPOINTS_GUID = { 11, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 } };
#define DIPROP_CPOINTS DIPROP_CPOINTS_GUID

#define DirectInputCreate DirectInputCreateA

#endif /* FF_LINUX */
#endif /* FF_COMPAT_DINPUT_H */

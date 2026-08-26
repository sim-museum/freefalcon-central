// sfr: math constants
#define _USE_MATH_DEFINES
#include <math.h>

#include "stdhdr.h"
#include "airframe.h"
#include "simbase.h"
#include "aircrft.h"
#include "otwdrive.h"
#include "fakerand.h"
#include "graphics/include/tmap.h"
#include "graphics/include/rviewpnt.h"  // to get ground type
#include "vutypes.h"
#include "pilotinputs.h"
#include "limiters.h"
#include "fack.h"
#include "falcsess.h"
#include "MsgInc/DamageMsg.h"
#include "MsgInc/LandingMessage.h"
#include "campbase.h"
#include "fsound.h"
#include "soundfx.h"
#include "playerop.h"
#include "simio.h"
#include "weather.h"
#include "objectiv.h"
#include "find.h"
#include "atcbrain.h"
#include "graphics/include/terrtex.h"
#include "ffeedbk.h"

// OW FIXME: this had to be added after installing the DX8 Beta 1 SDK
//Retro_dead 15Jan2004 #define DIRECTINPUT_VERSION 0x0700
//Retro_dead 15Jan2004 #include "dinput.h"

#include "flight.h"
#include "simdrive.h"
#include "digi.h"
#include "ptdata.h"
#include "dofsnswitches.h"
#include "graphics/include/drawbsp.h"
#include "classtbl.h"
//#include <crtdbg.h> // JPO debug

extern VU_TIME vuxGameTime;

// All headers cut bitand paste from eom.cpp
// this code provides the animations for gear strut compression and rolling wheels.
void AirframeClass::RunLandingGear(void)
{
#ifdef FF_LINUX
    // FF_LINUX (GEAR-2): the landing gear extend/retract animation -- the only
    // continuous write to gearPos in the codebase -- lives in
    // AirframeClass::RemoteUpdate() (airframe.cpp:876). RemoteUpdate is called
    // per frame ONLY from aircraft.cpp:2213, under "if (not IsLocal())", so the
    // player's own aircraft never has its gear animated: gearPos advances while
    // the jet is still on the remote path and then freezes wherever it happens to
    // be. Measured on a PO flight: gearPos rose 0.000 -> 0.869 at the correct
    // 0.3/sec and stopped there permanently.
    //
    // Everything downstream follows from that frozen value. The gear DOF is
    // (gearPos - 0.5) * 2, so it never reaches full extension and the gear is drawn
    // part-way stowed; and CheckHeight()'s gear contact term never wins, leaving
    // deltzGear = 0.00 with the fuselage term at 2.44 -- the aircraft rests on its
    // belly (measured aboveGround 2.33 instead of the gear's 5.99) and appears sunk
    // into a runway that is itself drawn 3ft above the terrain. That is the
    // "aircraft half-buried in the airstrip" report.
    //
    // Fix: run the same animation on the local path. RunLandingGear() is called from
    // both Exec() (local) and RemoteUpdate() (remote), so the IsLocal() guard gives
    // the player the animation without double-stepping remote aircraft, which keep
    // using the existing RemoteUpdate step. NOTE: measured rate is 0.62/sec, ~2x the
    // coded 0.3 -- RunLandingGear has 3 call sites and is not once-per-frame, so this
    // needs a single-call-site home before it can be default-on. gearHandle is NOT written
    // here -- it is the pilot's command (AFGearToggle) and the status bit is already
    // derived from it in RunGearSurfaces. FF_NO_GEAR_ANIM_FIX=1 reverts.
    if (platform and platform->IsLocal())
    {
        static int ffNoFix = -1;

        if (ffNoFix < 0) ffNoFix = getenv("FF_GEAR_ANIM_FIX") ? 0 : 1;  // OPT-IN until validated

        if ( not ffNoFix)
        {
            if (platform->IsAcStatusBitsSet(AircraftClass::ACSTATUS_GEAR_DOWN))
                gearPos += 0.3F * SimLibMajorFrameTime;
            else
                gearPos -= 0.3F * SimLibMajorFrameTime;

            gearPos = min(max(gearPos, 0.0F), 1.0F);
        }
    }
#endif

    if (auxaeroData->animWheelRadius[0] and platform->drawPointer)
    {
        // MLR 2003-10-04 code to support spinning landing wheels
        // the idea here is to set the Radians/Sec rotation of the wheel
        // while in contact with the ground.
        // code in the surface.cpp file actually rotates the wheel, and
        // gradually bleeds off the RPS one the gear is no longer grounded
        // (which means this code is no longer running).
        int i;
        float cgloc = GetAeroData(AeroDataSet::CGLoc);

        // sfr: distance moved in frame
        SM_SCALAR speed = platform->GetVt();
        SM_SCALAR dist = (speed * SimLibMajorFrameTime);

        for (i = 0; i < NumGear(); i++)
        {
            Tpoint PtWorldPos;
            Tpoint PtRelPos;

            PtRelPos.x = cgloc - GetAeroData(AeroDataSet::NosGearX + i * 4);
            PtRelPos.y = GetAeroData(AeroDataSet::NosGearY + i * 4);
            PtRelPos.z = GetAeroData(AeroDataSet::NosGearZ + i * 4);

            MatrixMult(&((DrawableBSP*)platform->drawPointer)->orientation, &PtRelPos, &PtWorldPos);

            PtWorldPos.x += x;
            PtWorldPos.y += y;
            PtWorldPos.z += z;


            // MLR 2003-10-14 animate the landing gear strut
            {
                //gear[i].StrutExtension = OTWDriver.GetGroundLevel(PtWorldPos.x, PtWorldPos.y) - ( PtWorldPos.z + z)
                gear[i].StrutExtension = groundZ - (PtWorldPos.z);

                // limit to values from auxaerodata
                // these will get applied to a DOF or Translator to make it look like
                // the gear is working.

                if (gear[i].StrutExtension < -auxaeroData->animGearMaxComp[i])
                {
                    gear[i].StrutExtension = -auxaeroData->animGearMaxComp[i];
                }

                gear[i].WheelRPS *= (1 - .4f * SimLibMajorFrameTime); // slows wheel down

                if (gear[i].StrutExtension > auxaeroData->animGearMaxExt[i])
                {
                    // wheel off ground
                    gear[i].StrutExtension = auxaeroData->animGearMaxExt[i];
                    // sfr: was in run gear function
                    // deaccel gear, since its not in touch with ground
                    // compute new angle
                    gear[i].WheelAngle += gear[i].WheelRPS * SimLibMajorFrameTime;
                }
                else if (SimLibMajorFrameTime and auxaeroData->animWheelRadius[i])
                {
                    // sfr: using plane speed now
                    // we need this for the case above,
                    // when gear is not in touch with ground anymore
                    gear[i].WheelRPS = speed;
                    // wheel rotation increment
                    SM_SCALAR rInc = dist / auxaeroData->animWheelRadius[i];
                    gear[i].WheelAngle += rInc;
                }

                // can be more than 2pi
                gear[i].WheelAngle = fmod(gear[i].WheelAngle, ((float)(M_PI)) * 2.0f);
            }
        }
    }

    //error - need to have sound played when the radii are 0
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// Includes.
#pragma optimize( "", off )

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "graphics/include/grtypes.h"
#include "graphics/include/tod.h"   /* ACMI-OBJ-1: TheTimeOfDay in the camera trace */
struct IDirectDrawSurface7;
extern "C" int FF_ReadbackPrimaryRect(IDirectDrawSurface7 *, int, int, int, int);   /* ACMI-OBJ-1, d3d_gl.cpp (as c3dview.cpp) */
extern "C" void FF_ReconSetExclusions(int n, const int *rects);   /* d3d_gl.cpp, shared with the recon view */
#include "graphics/include/renderwire.h"
#include "graphics/include/terrtex.h"
#include "graphics/include/drawpole.h"
#include "graphics/include/timemgr.h"
#include "graphics/include/rviewpnt.h"
#include "graphics/include/renderow.h"
#include "codelib/tools/lists/lists.h"
#include "ui95/chandler.h"
#include "ui95/cthook.h"
#include "vu2.h"
#include "sim/include/simbase.h"
#include "sim/include/phyconst.h"
#include "falcmesg.h"
//#include "dispcfg.h"
#include "acmiUI.h"
#include "ui/include/textids.h"
#include "acmitape.h"
#include "AcmiView.h"
#include "falclib/include/dispopts.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// Defines and constants.

#define NSECTIONS 12
#define S_FRAME_NUM 0

#define ST_FRAME_NUM_DEC 0
#define ST_FRAME_NUM_INC 1

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// References.

void CalcTransformMatrix(SimBaseClass* theObject);
extern float CalcKIAS(float, float);

/*
enum
{
 ISO_CAM =200165,
};
*/

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int GLOBAL_WIRE_COCKPIT = 1; // default ON.


extern char FalconDataDirectory[_MAX_PATH];
extern char FalconPictureDirectory[_MAX_PATH]; // JB 010623
extern bool g_bNewAcmiHud;

extern C_Handler
*gMainHandler;

extern BOOL
acmiDraw;

extern RECT
acmiSrcRect,
acmiDestRect;

extern ACMIView
*acmiDrive;

extern int TESTBUTTONPUSH;

static DWORD
frameStart,
lastFrame,
frameTime,
fcount;

BOOL
camTrans = FALSE,
frameAdvance = FALSE;



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void ACMIView::VectorTranslate(Tpoint*)
{
    /*
    acmiCamPos.x +=
    (
     tVector->x * Camera()->Rotation().M11 +
     tVector->y * Camera()->Rotation().M12 +
     tVector->z * Camera()->Rotation().M13
    );

    acmiCamPos.y +=
    (
     tVector->x * Camera()->Rotation().M21 +
     tVector->y * Camera()->Rotation().M22 -
     tVector->z * Camera()->Rotation().M23
    );

    acmiCamPos.z -=
    (
     -1 * tVector->x * Camera()->Rotation().M31
     + tVector->y * Camera()->Rotation().M32 +
     tVector->z * Camera()->Rotation().M33
    );
    */

}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void ACMIView::VectorToVectorTranslation(Tpoint*, Tpoint*)
{
    /*
    offSetV->x +=
    (
     tVector->x * Camera()->Rotation().M11 +
     tVector->y * Camera()->Rotation().M12 +
     tVector->z * Camera()->Rotation().M13
    );

    offSetV->y +=
    (
     tVector->x * Camera()->Rotation().M21 +
     tVector->y * Camera()->Rotation().M22 -
     tVector->z * Camera()->Rotation().M23
    );

    offSetV->z -=
    (
     -1 * tVector->x * Camera()->Rotation().M31 +
     tVector->y * Camera()->Rotation().M32 +
     tVector->z * Camera()->Rotation().M33
    );
    */
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void ACMIView::SelectCamera(long camSel)
{
    if (camSel not_eq FREE_CAM)
    {
        _camPos.x = -100.0f;
        _camPos.y = 0.0f;
        _camPos.z = 0.0f;
    }

    _camRange = -500.0f;
    _camRoll = 0.0f;

    switch (camSel)
    {
        case INTERNAL_CAM: //internal
            GLOBAL_WIRE_COCKPIT = 1;
            _cameraState = INTERNAL_CAM;
            break;

        case EXTERNAL_CAM: // orbit
            GLOBAL_WIRE_COCKPIT = 0;
            _cameraState = EXTERNAL_CAM;
            break;

        case TRACKING_CAM: // orbit
            GLOBAL_WIRE_COCKPIT = 0;
            _cameraState = TRACKING_CAM;
            break;

        case CHASE_CAM: //Chase
            GLOBAL_WIRE_COCKPIT = 0;
            _cameraState = CHASE_CAM;
            break;

        case SAT_CAM: // Satellite
            GLOBAL_WIRE_COCKPIT = 0;
            _cameraState = SAT_CAM;

            // default sat view to looking straight down at 5000ft
            _camPitch = -90.0f * DTR;
            _camRange = -15000.0f;
            break;

        case ISO_CAM: // ISOMETRIC
            GLOBAL_WIRE_COCKPIT = 0;
            _cameraState = ISO_CAM;
            _camRange = -15000.0f;
            _camPitch = -25.0F * DTR;
            break;

        case FREE_CAM: // Free
            GLOBAL_WIRE_COCKPIT = 0;
            _cameraState = FREE_CAM;

            // default camera to last known position
            _camWorldPos.x += _camPos.x;
            _camWorldPos.y += _camPos.y;
            _camWorldPos.z += _camPos.z;

            break;

        default:
            GLOBAL_WIRE_COCKPIT = 0;
            _cameraState = EXTERNAL_CAM;
            break;
    };

    ResetPanner();
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void ACMIView::SetUIVector(Tpoint*)
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void ACMIView::InitUIVector()
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void ACMIView::SwitchCameraObject(long cameraObject)
{
    int
    i,
    numEntities;

    F4Assert(Tape() not_eq NULL);
    F4Assert(_entityUIMappings not_eq NULL);

    numEntities = Tape()->NumEntities();

    for (i = 0; i < numEntities; i++)
    {
        if (cameraObject == _entityUIMappings[i].listboxId)
        {
            SetCameraObject(i);
            return;
        }
    }


}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void ACMIView::SwitchTrackingObject(long cameraObject)
{
    int
    i,
    numEntities;

    F4Assert(Tape() not_eq NULL)
    F4Assert(_entityUIMappings not_eq NULL);

    numEntities = Tape()->NumEntities();

    for (i = 0; i < numEntities; i++)
    {
        if (cameraObject == _entityUIMappings[i].listboxId)
        {
            SetTrackingObject(i);
            return;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void ACMIView::Exec()
{
    Tpoint pos;

    if ( not acmiDraw or not _tapeHasLoaded or Tape() == NULL or not Tape()->IsLoaded())
        return;

    if (fcount++ % 10 == 0)
    {
        frameStart = timeGetTime();
        frameTime = frameStart - lastFrame;
        lastFrame = frameStart;
    }


    //update the tape
    Tape()->Update(-1.0);

    // set the camera's position and rotation matrix
    UpdateViewPosRot();


    pos.x = _camWorldPos.x + _camPos.x;
    pos.y = _camWorldPos.y + _camPos.y;
    pos.z = _camWorldPos.z + _camPos.z;

    // check view isn't below ground
    float groundZ = _renderer->viewpoint->GetGroundLevel(pos.x, pos.y);

    if (pos.z > groundZ - 10.0f)
        pos.z = groundZ - 10.0f;


    Viewpoint()->Update(&pos);

    // draw the ACMI View
    // Draw();

    // ruurrr?
    TheTimeManager.SetTime(DWORD(Tape()->SimTime() * 1000));

}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void ACMIView::Draw()
{
    static int lastButton = -1;
    Tpoint
    pos;

    Trotation
    rot;

    int i, numEntities;
    SimTapeEntity *ep;
    SimTapeEntity *targep;
    int targindex;

    TCHAR speedstring[20];
    TCHAR altitudestring[20];
    TCHAR headingstring[20];

    float ACMI_heading = 0;
    float ACMI_altitude = 0;
    float mph = 0.0f;

    Tpoint posb;
    ThreeDVertex spos;

    if (TapeHasLoaded() and Tape() not_eq NULL and Tape()->IsLoaded())
    {
        gMainHandler->Unlock(); // Make surface available...

        // pos = _camPos;
        pos.x = 0.0f;
        pos.y = 0.0f;
        pos.z = 0.0f;
        rot = _camRot;

        // make sure the internal hud view object isn't displayed
        if ((_cameraState == INTERNAL_CAM))
            Tape()->RemoveEntityFromFrame(CameraObject());

        // edg: ?????
        /*
        ** Remove Bing-ism
        ** causing a crash ....
        if(TESTBUTTONPUSH == 0 and TESTBUTTONPUSH not_eq lastButton)
        {
         lastButton = TESTBUTTONPUSH;
         TheTerrTextures.SetOverrideTexture( wireTexture.TexHandle() );
         Viewpoint()->Update(&pos);
         _renderer->StartFrame();
         _renderer->DrawScene(&pos, &rot);
         _renderer->PostSceneCloudOcclusion();
         _renderer->FinishFrame();
         Viewpoint()->Update(&_camWorldPos);
        }
        */

        //JAM 16Dec03
        if (DisplayOptions.bZBuffering)
            _renderer->context.SetZBuffering(TRUE);

        // start the render frame + Draw
        _renderer->context.StartFrame();
        _renderer->StartDraw();

        // render the 3d view
        _renderer->DrawScene(&pos, &rot);
#ifdef FF_LINUX
        /* ACMI-OBJ-1 (PO 2026-09-20): the entity census proved ~550 drawables ARE in the display
           list every frame while the view shows no objects and no terrain, so the question moved to
           WHERE THE CAMERA IS. `pos` here is the OFFSET handed to DrawScene (gdb names that
           parameter `offset`) and is deliberately zero; the camera's real world position lives in
           the viewpoint, set by ACMIView::Exec -> Viewpoint()->Update(_camWorldPos + _camPos).
           Print all three plus one real entity position once a second: a viewpoint at the origin
           while the entities sit at Korean map coordinates would be the whole defect.
           FF_DEBUG_ACMI=1. */
        {
            static int s_c = -1; static time_t s_l = 0;
            if (s_c < 0) s_c = getenv("FF_DEBUG_ACMI") ? 1 : 0;
            time_t nw = 0; if (s_c) ::time(&nw);
            if (s_c and nw != s_l)
            {
                s_l = nw;
                Tpoint vp; vp.x = vp.y = vp.z = -1.0f;
                if (Viewpoint()) Viewpoint()->GetPos(&vp);
                Tpoint e0; e0.x = e0.y = e0.z = -1.0f; int shown = -1;
                const int ne2 = Tape()->NumEntities();
                for (int q = 0; q < ne2; q++)
                {
                    SimTapeEntity *e2 = Tape()->GetSimTapeEntity(q);
                    if (e2 and (e2->flags bitand ENTITY_FLAG_AIRCRAFT) and e2->objBase and e2->objBase->drawPointer)
                    { e2->objBase->drawPointer->GetPosition(&e0); shown = q; break; }
                }
                /* ACMI-OBJ-1: the camera is now PROVEN correct (orbit gives a 500 ft standoff from
                   the focused jet) and the entities are PROVEN in a display list, yet nothing draws.
                   The remaining suspect is identity: ACMITape is handed a viewpoint at construction
                   (acmiview.cpp:274) while ACMIView creates its RViewPoint separately
                   (acmiview.cpp:519). If those are two objects, entities are inserted into one
                   display list and the renderer draws from the other. Print all three pointers --
                   the tape's is already in the [acmi] census line, so the three can be compared in
                   one log. */
                fprintf(stderr, "[acmicam] light=%.2f amb=%.2f  viewpointPtr=%p rendererViewpointPtr=%p  viewpoint=(%.0f,%.0f,%.0f) camWorld=(%.0f,%.0f,%.0f) camOff=(%.0f,%.0f,%.0f) sceneOffset=(%.0f,%.0f,%.0f) camState=%d  firstAircraft[%d]=(%.0f,%.0f,%.0f)\n",
                        TheTimeOfDay.GetLightLevel(), TheTimeOfDay.GetAmbientValue(), (void*)Viewpoint(), (void*)(_renderer ? _renderer->viewpoint : NULL),
                        vp.x, vp.y, vp.z, _camWorldPos.x, _camWorldPos.y, _camWorldPos.z,
                        _camPos.x, _camPos.y, _camPos.z, pos.x, pos.y, pos.z,
                        (int)_cameraState, shown, e0.x, e0.y, e0.z);
                fflush(stderr);
            }
        }
#endif

        //JAM 12Dec03 - ZBUFFERING OFF
        if (DisplayOptions.bZBuffering)
            _renderer->context.FlushPolyLists();
#ifdef FF_LINUX
        /* ACMI-OBJ-1 (PO 2026-09-22 gold video: "the saved ACMI plays correctly in default and orbit
           view"; ours showed sky, haze and no terrain or objects, with the camera PROVEN on the jet and
           ~550 drawables PROVEN in the display list). The renderer was built on the UI ImageBuffer,
           gMainHandler->GetFront(), and RECON-1 S2 established what that means in this port: that
           buffer's BACK surface is bound once as the render target and never presented, so every
           frame lands in an off-screen FBO. The recon view fixed itself by copying its rectangle back
           into the UI surface after drawing (c3dview.cpp, FF_ReadbackPrimaryRect); the ACMI viewer
           never got the same call. Flush the poly lists unconditionally first (the sim does; here it
           was gated on bZBuffering), then copy the render pane back. FF_ACMI_NOREADBACK=1 reverts. */
        {
            static int s_rb = -1; if (s_rb < 0) s_rb = getenv("FF_ACMI_NOREADBACK") ? 0 : 1;
            if (s_rb)
            {
                _renderer->context.FlushPolyLists();
                /* GOLD-260922: the readback is re-applied at present time as the last write (RECON-1 S4), so it
                   also paints over any window the UI stacks ABOVE the render pane -- the Camera/Focus drop
                   lists open into the pane and were wiped to their top strip (acmi_orb10.png). Same cure as
                   RECON-2: name the visible windows above the pane's owner and the shim leaves them alone. */
                {
                    int rects[8 * 4]; int n = 0; C_Window *owner = NULL;
                    for (C_Window *w = gMainHandler->_GetFirstWindow(); w; w = gMainHandler->_GetNextWindow(w))
                    {
                        if ( not gMainHandler->FFIsWindowVisible(w)) continue;
                        const int wl = w->GetX(), wt = w->GetY(), wr = wl + w->GetW(), wb = wt + w->GetH();
                        if (wl <= _ffWinL and wt <= _ffWinT and wr >= _ffWinR and wb >= _ffWinB) { owner = w; n = 0; continue; }
                        if ( not owner) continue;
                        if (wr <= _ffWinL or wl >= _ffWinR or wb <= _ffWinT or wt >= _ffWinB) continue;
                        if (n < 8) { rects[n * 4] = wl; rects[n * 4 + 1] = wt; rects[n * 4 + 2] = wr; rects[n * 4 + 3] = wb; n++; }
                    }
                    FF_ReconSetExclusions(n, rects);
                    static int lastN = -1;
                    if (n != lastN and getenv("FF_DEBUG_ACMI")) { lastN = n; fprintf(stderr, "[acmi] %d window(s) above the render pane excluded from the readback\n", n); fflush(stderr); }
                }
                const int ok = FF_ReadbackPrimaryRect(gMainHandler->GetFront()->targetSurface(), _ffWinL, _ffWinT, _ffWinR, _ffWinB);
                static int s_said = 0;
                if ( not s_said and getenv("FF_DEBUG_ACMI")) { s_said = 1;
                    fprintf(stderr, "[acmi] readback of the render pane (%d,%d)-(%d,%d) into the UI surface: %s\n", _ffWinL, _ffWinT, _ffWinR, _ffWinB, ok ? "done" : "SKIPPED"); fflush(stderr); }
            }
        }
#endif

        // _renderer->PostSceneCloudOcclusion();

        // now we do post-3d rendering
        // this includes putting in things like alt poles( shouldn't really
        // be done here), crapola hud, and some label stuff.  We need
        // to traverse the entity list on the tape in order to do this

        numEntities = Tape()->NumEntities();

        // 1st pass, turn off target boxes (bleck)
        for (i = 0; i < numEntities; i++)
        {
            // get the entity
            ep = Tape()->GetSimTapeEntity(i);

            // do these only for aircraft
            if (ep->flags bitand ENTITY_FLAG_AIRCRAFT)
            {
                // default target box to off
                ((DrawablePoled *)ep->objBase->drawPointer)->SetTarget(FALSE);
            }
        }

        for (i = 0; i < numEntities; i++)
        {
            // get the entity
            ep = Tape()->GetSimTapeEntity(i);

            // is it in existance at the moment?
            // also no poles or anything on chaff and flares
            if ((ep->flags bitand (ENTITY_FLAG_CHAFF bitor ENTITY_FLAG_FLARE)) or
 not Tape()->IsEntityInFrame(i))
            {
                continue;
            }

            // get world pos and screen pos of object
            ep->objBase->drawPointer->GetPosition(&pos);
            _renderer->TransformPoint(&pos, &spos);


            // radar target line
            if (_doLockLine == 1)
            {
                targindex = Tape()->GetEntityCurrentTarget(i);

                if (targindex not_eq -1)
                {
                    // get the target entity
                    targep = Tape()->GetSimTapeEntity(targindex);
                    ep->objBase->drawPointer->GetPosition(&pos);
                    targep->objBase->drawPointer->GetPosition(&posb);

                    rot = ((DrawableBSP *)ep->objBase->drawPointer)->orientation;

                    // start line out in front of aircraft
                    pos.x += rot.M11 * ep->objBase->drawPointer->Radius();
                    pos.y += rot.M21 * ep->objBase->drawPointer->Radius();
                    pos.z += rot.M31 * ep->objBase->drawPointer->Radius();


                    // do target boxes and lines
                    if (_cameraState not_eq FREE_CAM)  // and targindex == CameraObject() )//me123 we wanna see all lock lines
                    {
                        // current attached camera object is target
                        if (targep->flags bitand ENTITY_FLAG_AIRCRAFT)
                        {
                            ((DrawablePoled *)targep->objBase->drawPointer)->SetTarget(TRUE);
                            ((DrawablePoled *)targep->objBase->drawPointer)->SetTargetBoxColor(0xff00ffff);
                        }

                        _renderer->SetColor(0xff00ffff);
                        _renderer->Render3DLine(&pos, &posb);

                    }
                    else if (_cameraState not_eq FREE_CAM and i == CameraObject())
                    {
                        // current attached camera object's target
                        if (targep->flags bitand ENTITY_FLAG_AIRCRAFT)
                        {
                            ((DrawablePoled *)targep->objBase->drawPointer)->SetTarget(TRUE);
                            ((DrawablePoled *)targep->objBase->drawPointer)->SetTargetBoxColor(0xffffffff);
                        }

                        _renderer->SetColor(0xffffffff);
                        _renderer->Render3DLine(&pos, &posb);
                    }
                    else if (_cameraState == FREE_CAM)
                    {
                        _renderer->SetColor(0xff00ffff);
                        _renderer->Render3DLine(&pos, &posb);
                    }

                } // if target
            } // if radar lines on


            // if we're in internal cam and we're the target object
            // display the "hud"
            if ((_cameraState == INTERNAL_CAM) and i == CameraObject())
            {

                //HEADING
                ACMI_heading = ep->yaw * RTD;

                if (ACMI_heading < 0.0f)
                    ACMI_heading += 360.0f;

                // ALTITUDE
                ACMI_altitude = -(ep->z);

                // SPEED
                // mph=ep->aveSpeed * FTPSEC_TO_KNOTS;
                mph = CalcKIAS(ep->aveSpeed, -ep->z);


                // WIRE COCKPIT.
                if (GLOBAL_WIRE_COCKPIT == 1)
                {
                    _renderer->SetColor(0xff00ff00);
                    _renderer->Line(-.8f, -1.0f, -0.5f, -0.5f);
                    _renderer->Line(-0.5f, -0.5f, 0.5f, -0.5f);
                    //_renderer->Line(  0.5f,-0.5f,1.0f,-1.3f );
                    _renderer->Line(0.5f, -0.5f, 1.0f, -1.4f);

                    _renderer->Line(-0.5f, -0.5f, -.5f, .5f);
                    _renderer->Line(-.5f, .5f, .5f, .5f);
                    _renderer->Line(.5f, .5f, .5f, -.5f);

                    if (g_bNewAcmiHud)
                    {
                        sprintf(speedstring, "%0.0f", mph);
                        sprintf(altitudestring, "%0.0f", ACMI_altitude);
                        sprintf(headingstring, "%0.0f", ACMI_heading);
                        int ofont = _renderer->CurFont();
                        _renderer->SetFont(2);
                        _renderer->TextLeft(-0.48f, 0, speedstring);
                        _renderer->TextRight(0.48f, 0, altitudestring);
                        _renderer->TextCenter(0, 0.48f, headingstring);
                        _renderer->TextCenter(0, -0.45f, ((DrawableBSP*)ep->objBase->drawPointer)->Label());
                        _renderer->SetFont(ofont);
                    }
                    else
                    {
                        sprintf(speedstring, "%s: %0.0f", gStringMgr->GetString(TXT_AIRSPEED), mph);
                        sprintf(altitudestring, "%s: %0.0f", gStringMgr->GetString(TXT_ALTITUDE), ACMI_altitude);
                        sprintf(headingstring, "%s: %0.0f", gStringMgr->GetString(TXT_HEADING), ACMI_heading);
                        _renderer->ScreenText(250.0f, 160.0f, ((DrawableBSP*)ep->objBase->drawPointer)->Label(), 0);
                        _renderer->ScreenText(250.0f, 170.0f, speedstring, 0);
                        _renderer->ScreenText(250.0f, 180.0f, altitudestring, 0);
                        _renderer->ScreenText(250.0f, 190.0f, headingstring, 0);
                    }

                    _renderer->SetColor(0xffff0000);
                }
            }
        }

        // tell renderer we're done
        _renderer->EndDraw();
        _renderer->context.FinishFrame(NULL);

        if (_takeScreenShot)
        {
            TakeScreenShot();
        }

        gMainHandler->Lock();

        // update the entities
        // MUST be done after render
        Tape()->UpdateSimTapeEntities();

    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void ACMIView::TakeScreenShot()
{
    char
    fileName[_MAX_PATH];

    char
    tmpStr[_MAX_PATH];

    time_t
    ltime;

    struct tm*
            today;

    time(&ltime);
    // _takeScreenShot = FALSE;
    today = localtime(&ltime);

    strftime
    (
        tmpStr,
        _MAX_PATH - 1,
        "%m.%d.%Y-%H.%M.%S",
        today
    );

#if 0
    sprintf(fileName, "%s\\%s", FalconDataDirectory, tmpStr);
#else
    sprintf(fileName, "%s\\%s", FalconPictureDirectory, tmpStr);
#endif

    gMainHandler->GetFront()->BackBufferToRAW(fileName);
}


/*
** Name: UpdateViewPosRot
** Description:
** Updates the view world position and rotation depending on
** the camera setting and what we're tracking and panner positions
*/
void ACMIView::UpdateViewPosRot(void)
{
    int camObj;
    int trackObj;
    SimTapeEntity *camEnt;
    SimTapeEntity *trackEnt;
    Tpoint dPos;
    float dT;
    float dRoll;
    float dist;
    float objScale;

    // get any camera objects we might need
    camObj = CameraObject();
    trackObj = TrackingObject();
    camEnt = Tape()->GetSimTapeEntity(camObj);
    trackEnt = Tape()->GetSimTapeEntity(trackObj);

    // get current scaling
    objScale = Tape()->GetObjScale();


    // if we're not in free camera mode, our world position is
    // based on the camera object
    if (_cameraState not_eq FREE_CAM)
    {
        _camWorldPos.x = camEnt->x;
        _camWorldPos.y = camEnt->y;
        _camWorldPos.z = camEnt->z;
    }

    // first pass:
    //  get yaw pitch and roll for camera
    switch (_cameraState)
    {
        case INTERNAL_CAM: //internal
            _camYaw = camEnt->yaw;
            _camPitch = camEnt->pitch;
            _camRoll = camEnt->roll;
            break;

        case EXTERNAL_CAM: // orbit
            _camYaw += _pannerAz;
            _camPitch += _pannerEl;
            _camRoll = 0.0f;
            _camRange += _pannerX;

            if (_camRange > -50.0f)
                _camRange = -50.0f;

            break;

        case TRACKING_CAM: // tracking
            _camYaw += _pannerAz;
            _camPitch += _pannerEl;
            _camRoll = 0.0f;
            _camRange += _pannerX;

            if (_camRange > -50.0f)
                _camRange = -50.0f;

            break;

        case CHASE_CAM: //Chase
            _camRange += _pannerX;

            if (_camRange > -50.0f)
                _camRange = -50.0f;

            // where we want camera to be
            _chaseX = camEnt->objBase->dmx[0][0] * _camRange * objScale;
            _chaseY = camEnt->objBase->dmx[0][1] * _camRange * objScale;
            _chaseZ = camEnt->objBase->dmx[0][2] * _camRange * objScale;

            dT = Tape()->GetDeltaSimTime();

            if (dT <= 0.0f)
                dT = 0.1f;

            // "spring" constants for camera roll and move
#define KMOVE 0.29f
#define KROLL 0.30f

            // convert frame loop time to secs from ms
            // dT = (float)frameTime * 0.001;

            // get the diff between desired and current camera pos
            dPos.x = _chaseX - _camPos.x;
            dPos.y = _chaseY - _camPos.y;
            dPos.z = _chaseZ - _camPos.z;

            // send the camera thataway
            _camPos.x += dPos.x * dT * KMOVE;
            _camPos.y += dPos.y * dT * KMOVE;
            _camPos.z += dPos.z * dT * KMOVE;

            // "look at" vector
            dPos.x = -_camPos.x;
            dPos.y = -_camPos.y;
            dPos.z = -_camPos.z;

            // get new camera roll
            dRoll = camEnt->roll - _camRoll;

            // roll in shortest direction
            if (fabs(dRoll) > 180.0f * DTR)
            {
                if (dRoll < 0.0f)
                    dRoll = 360.0f * DTR + camEnt->roll - _camRoll;
                else
                    dRoll = -360.0f * DTR + camEnt->roll - _camRoll;
            }

            // apply roll
            _camRoll += dRoll * dT * KROLL;

            // keep chase cam roll with +/- 180
            if (_camRoll > 1.0f * PI)
                _camRoll -= 2.0f * PI;
            else if (_camRoll < -1.0f * PI)
                _camRoll += 2.0f * PI;

            // now get yaw and pitch based on look at vector
            dist = (float)sqrt(dPos.x * dPos.x + dPos.y * dPos.y + dPos.z * dPos.z);
            _camPitch = (float) - asin(dPos.z / dist);
            _camYaw = (float)atan2(dPos.y, dPos.x);

            break;

        case SAT_CAM: // Satellite
            _camYaw += _pannerAz;
            _camPitch += _pannerEl;

            if (_camPitch > -45.0f * DTR)
                _camPitch = -45.0f * DTR;
            else if (_camPitch < -90.0f * DTR)
                _camPitch = -90.0f * DTR;

            _camRoll = 0.0f;
            _camRange += _pannerX * 100.0f;

            if (_camRange > -1000.0f)
                _camRange = -1000.0f;

            break;

        case ISO_CAM: // ISOMETRIC
            _camYaw += _pannerAz;
            _camPitch += _pannerEl;

            if (_camPitch > -15.0f * DTR)
                _camPitch = -15.0f * DTR;
            else if (_camPitch < -60.0f * DTR)
                _camPitch = -60.0f * DTR;

            _camRoll = 0.0f;
            _camRange += _pannerX * 100.0f;

            if (_camRange > -50.0f)
                _camRange = -50.0f;

            break;

        case FREE_CAM: // Free
            _camYaw += _pannerAz;
            _camPitch += _pannerEl;
            _camRoll = 0.0f;
            _pannerX *= 20.0f;
            _pannerY *= 20.0f;
            _pannerZ *= 20.0f;
            // head in the direction we're currently facing (ie based on
            // current cam rotation
            _camWorldPos.x += _camRot.M11 * _pannerX + _camRot.M12 * _pannerY + _camRot.M13 * _pannerZ;
            _camWorldPos.y += _camRot.M21 * _pannerX + _camRot.M22 * _pannerY + _camRot.M23 * _pannerZ;
            _camWorldPos.z += _camRot.M31 * _pannerX + _camRot.M32 * _pannerY + _camRot.M33 * _pannerZ;
            break;

        default:
            _camYaw = camEnt->yaw;
            _camPitch = camEnt->pitch;
            _camRoll = camEnt->roll;
            break;
    };

    // second pass:
    // create the rotation matrix
    float costha, sintha, cosphi, sinphi, cospsi, sinpsi;

    costha = (float)cos(_camPitch);

    sintha = (float)sin(_camPitch);

    cosphi = (float)cos(_camRoll);

    sinphi = (float)sin(_camRoll);

    cospsi = (float)cos(_camYaw);

    sinpsi = (float)sin(_camYaw);

    _camRot.M11 = cospsi * costha;

    _camRot.M21 = sinpsi * costha;

    _camRot.M31 = -sintha;

    _camRot.M12 = -sinpsi * cosphi + cospsi * sintha * sinphi;

    _camRot.M22 = cospsi * cosphi + sinpsi * sintha * sinphi;

    _camRot.M32 = costha * sinphi;

    _camRot.M13 = sinpsi * sinphi + cospsi * sintha * cosphi;

    _camRot.M23 = -cospsi * sinphi + sinpsi * sintha * cosphi;

    _camRot.M33 = costha * cosphi;

    // third pass:
    // Set the relative camera positiion
    switch (_cameraState)
    {
        case INTERNAL_CAM: //internal
            _camPos.x = 0.0f;
            _camPos.y = 0.0f;
            _camPos.z = 0.0f;
            break;

        case EXTERNAL_CAM: // orbit
            _camPos.x = _camRot.M11 * _camRange * objScale;
            _camPos.y = _camRot.M21 * _camRange * objScale;
            _camPos.z = _camRot.M31 * _camRange * objScale;
            break;

        case TRACKING_CAM: // orbit
            if (camObj == trackObj)
            {
                _camPos.x = _camRot.M11 * _camRange * objScale;
                _camPos.y = _camRot.M21 * _camRange * objScale;
                _camPos.z = _camRot.M31 * _camRange * objScale;
            }
            else
            {
                // line up a vector between cam and track obj and place
                // camera on the vector, slightly up
                dPos.x = trackEnt->x - camEnt->x;
                dPos.y = trackEnt->y - camEnt->y;
                dPos.z = trackEnt->z - camEnt->z;
                dist = 1.0F / (float)sqrt(dPos.x * dPos.x + dPos.y * dPos.y + dPos.z * dPos.z);
                _camPos.x = dPos.x * dist * _camRange * objScale;
                _camPos.y = dPos.y * dist * _camRange * objScale;
                _camPos.z = dPos.z * dist * _camRange * objScale + _camRange * 0.2f;
            }

            break;

        case CHASE_CAM: //Chase
            break;

        case SAT_CAM: // Satellite
            _camPos.x = _camRot.M11 * _camRange;
            _camPos.y = _camRot.M21 * _camRange;
            _camPos.z = _camRot.M31 * _camRange;
            break;

        case ISO_CAM: // ISOMETRIC
            _camPos.x = _camRot.M11 * _camRange;
            _camPos.y = _camRot.M21 * _camRange;
            _camPos.z = _camRot.M31 * _camRange;
            break;

        case FREE_CAM: // Free
            _camPos.x = 0.0f;
            _camPos.y = 0.0f;
            _camPos.z = 0.0f;
            break;

        default:
            _camPos.x = 0.0f;
            _camPos.y = 0.0f;
            _camPos.z = 0.0f;
            break;
    };

    // fourth pass
    // if we're tracking an object we need to set a new rotation
    // matrix
    // determine if we're tracking an object or not
    if (_cameraState == TRACKING_CAM and camObj not_eq trackObj)
    {
        // get the diff between desired and current camera pos
        // for look at vector
        dPos.x = trackEnt->x - (_camPos.x + _camWorldPos.x);
        dPos.y = trackEnt->y - (_camPos.y + _camWorldPos.y);
        dPos.z = trackEnt->z - (_camPos.z + _camWorldPos.z);

        // now get yaw and pitch based on look at vector
        dist = (float)sqrt(dPos.x * dPos.x + dPos.y * dPos.y + dPos.z * dPos.z);
        float p = (float) - asin(dPos.z / dist);
        float y = (float)atan2(dPos.y, dPos.x);

        costha = (float)cos(p);
        sintha = (float)sin(p);
        cosphi = (float)(1.0f);
        sinphi = (float)(0.0f);
        cospsi = (float)cos(y);
        sinpsi = (float)sin(y);

        _camRot.M11 = cospsi * costha;
        _camRot.M21 = sinpsi * costha;
        _camRot.M31 = -sintha;

        _camRot.M12 = -sinpsi * cosphi + cospsi * sintha * sinphi;
        _camRot.M22 = cospsi * cosphi + sinpsi * sintha * sinphi;
        _camRot.M32 = costha * sinphi;

        _camRot.M13 = sinpsi * sinphi + cospsi * sintha * cosphi;
        _camRot.M23 = -cospsi * sinphi + sinpsi * sintha * cosphi;
        _camRot.M33 = costha * cosphi;
    }


    // reset panning
    ResetPanner();

}

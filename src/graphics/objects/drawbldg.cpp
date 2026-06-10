/***************************************************************************\
    DrawBldg.cpp    Scott Randolph
    July 10, 1996

    Derived class to do special position processing for buildings on the
 ground.  (More precisly, any object which is to be placed on the
 ground but not reoriented.)
\***************************************************************************/
#include "matrix.h"
#include "rviewpnt.h"
#include "renderow.h"
#include "drawbldg.h"

// edg just testing smoke stacks
/*
#include "stdhdr.h"
#include "otwdrive.h"
#include "sfx.h"
*/

#ifdef USE_SH_POOLS
MEM_POOL DrawableBuilding::pool;
#endif

/***************************************************************************\
    Initialize a container for a BSP object to be drawn
\***************************************************************************/
DrawableBuilding::DrawableBuilding(int ID, Tpoint *pos, float heading, float s)
    : DrawableBSP(s, ID)
{
    float cosYaw;
    float sinYaw;

    // Compute the sine and cosine of the objects desired heading
    cosYaw = (float)cos(heading);
    sinYaw = (float)sin(heading);

    // Store this objects properties
    scale = s;
    previousLOD = -1;
    drawClassID = Building;

    // Construct the rotation matrix to orient the object correctly
    orientation.M11 = cosYaw, orientation.M12 = -sinYaw, orientation.M13 = 0.0f;
    orientation.M21 = sinYaw, orientation.M22 = cosYaw, orientation.M23 = 0.0f;
    orientation.M31 = 0.0f, orientation.M32 = 0.0f, orientation.M33 = 1.0f;

    // Record our position (Z will be updated later)
    position = *pos;
}

/***************************************************************************\
    Make sure the object is placed on the ground then draw it.
\***************************************************************************/
void DrawableBuilding::Draw(class RenderOTW *renderer, int LOD)
{
    // See if we need to update our ground position
    if (LOD not_eq previousLOD)
    {
        // Update our position to reflect the terrain beneath us
        //position.z = renderer->viewpoint->GetGroundLevel(position.x,position.y);
        position.z = renderer->viewpoint->GetGroundLevelApproximation(position.x, position.y);

        previousLOD = LOD;
    }

#ifdef FF_LINUX
    // FF_LINUX test (FF_RUNWAY_ZLIFT=<feet>): GetGroundLevelApproximation returns
    // 0 at the airbase (post out of the loaded range), so the FLAT platform
    // surfaces (runways) sit at sea level, buried under the slightly elevated
    // rendered terrain (buildings survive because they stick up through it).
    // While drawing flat surfaces (g_ffRunwayDbg set by DrawablePlatform::Draw)
    // lift z toward the camera/up (NED: -Z is up) to confirm burial is the cause.
    {
        extern int g_ffRunwayDbg;
        static float s_zl = -9999.f;
        if (s_zl < -9000.f) { const char *e = getenv("FF_RUNWAY_ZLIFT"); s_zl = e ? (float)atof(e) : 0.f; }
        if (g_ffRunwayDbg && s_zl != 0.f) position.z -= s_zl;
    }
#endif

    if (renderer)
        DrawableBSP::Draw(renderer, LOD);
}

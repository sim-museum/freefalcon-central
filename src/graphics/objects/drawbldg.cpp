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
#ifdef FF_LINUX
    {
        extern int g_ffRunwayDbg;
        if (g_ffRunwayDbg && previousLOD == -1 && getenv("FF_DEBUG_RUNWAY"))
        {
            static int n = 0;
            if (n++ < 20)
                fprintf(stderr, "[RUNWAY] flat ORIGINAL z (feature data, pre-overwrite) = %.2f pos=(%.0f,%.0f)\n",
                        position.z, position.x, position.y);
        }
    }
#endif
    // See if we need to update our ground position
#ifdef FF_LINUX
    // FF_LINUX: flat runway/tarmac surfaces (g_ffRunwayDbg, set by DrawablePlatform::Draw)
    // were placed ONLY on LOD change via the COARSE GetGroundLevelApproximation. On a
    // landing approach the surface's render LOD usually doesn't change, so its z stays
    // frozen at the far-time coarse value (~0) while the terrain mesh and the landing
    // collision (GetGroundLevel, fresh each frame) use the fine elevation (~10ft) -- the
    // runway ends up in a ~10ft trench you land beside. Fix: for flat surfaces re-fetch
    // the ACCURATE ground level EVERY frame so the runway tracks the terrain it sits on
    // and matches where the jet touches down. FF_RUNWAY_OLD=1 reverts to old behavior.
    {
        extern int g_ffRunwayDbg;
        if (g_ffRunwayDbg && !getenv("FF_RUNWAY_OLD"))
        {
            float gl = renderer->viewpoint->GetGroundLevel(position.x, position.y);
            if (getenv("FF_DEBUG_RUNWAY"))
            {
                static long c = 0;
                if ((c++ % 120) == 0)
                    fprintf(stderr, "[RUNWAY] flat re-fetch GetGroundLevel=%.1f (was z=%.1f) pos=(%.0f,%.0f)\n",
                            gl, position.z, position.x, position.y);
            }
            position.z = gl;
            previousLOD = LOD;
        }
        else if (LOD not_eq previousLOD)
        {
            position.z = renderer->viewpoint->GetGroundLevelApproximation(position.x, position.y);
            previousLOD = LOD;
        }
    }
#else
    if (LOD not_eq previousLOD)
    {
        // Update our position to reflect the terrain beneath us
        //position.z = renderer->viewpoint->GetGroundLevel(position.x,position.y);
        position.z = renderer->viewpoint->GetGroundLevelApproximation(position.x, position.y);

        previousLOD = LOD;
    }
#endif

#ifdef FF_LINUX
    // FF_LINUX: the FLAT platform surfaces (runways/tarmac) get z=0 from
    // GetGroundLevelApproximation (returns 0 - post out of loaded range), so they
    // sit at sea level and are buried under the rendered terrain. Re-fetch the
    // ground level with the accurate GetGroundLevel every frame for flat surfaces
    // (g_ffRunwayDbg set by DrawablePlatform::Draw) and place them there, minus a
    // small decal offset to keep them just above the terrain. NON-accumulating.
    {
        extern int g_ffRunwayDbg;
        static float s_off = -9999.f;
        if (s_off < -9000.f) { const char *e = getenv("FF_RUNWAY_ZLIFT"); s_off = e ? (float)atof(e) : 0.f; }
        if (g_ffRunwayDbg && s_off != 0.f)
        {
            float gl = renderer->viewpoint->GetGroundLevel(position.x, position.y);
            position.z = gl - s_off;
            if (getenv("FF_DEBUG_RUNWAY"))
            {
                static int n = 0;
                if (n++ < 20)
                    fprintf(stderr, "[RUNWAY] flat z: GetGroundLevel=%.1f off=%.1f -> z=%.1f pos=(%.0f,%.0f)\n",
                            gl, s_off, position.z, position.x, position.y);
            }
        }
    }
#endif

    if (renderer)
        DrawableBSP::Draw(renderer, LOD);
}

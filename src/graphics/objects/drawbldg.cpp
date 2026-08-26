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

#ifdef FF_LINUX
// FF_LINUX (TE2-5): the runway decal, shared with OTWDriverClass::ObjectSetData.
// Flat runway/tarmac surfaces are DRAWN this far above the terrain so they win the
// depth test; ground aircraft are PLACED with their wheels at the terrain height, so
// without the same offset on the drawable a parked jet sits inside the tarmac.
float FF_RunwayDecal(void)
{
    static float decal = -9999.f;

    if (decal < -9000.f)
    {
        const char* e = getenv("FF_RUNWAY_ZLIFT");
        decal = e ? (float)atof(e) : 3.0f;
    }

    return decal;
}
#endif

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
            // FF_LINUX: lift the flat runway/tarmac surface slightly ABOVE the terrain
            // (a decal) so it wins the depth test against the terrain mesh.
            //
            // MEASURED 2026-08-15 (TE 2, player's airbase): the slope-scaled
            // glPolygonOffset from RWY-2 (9ed8f3b2, FF_SetRunwayDepthBias) is NOT
            // sufficient on its own. At decal 0 the tarmac disappears completely -- the
            // cockpit view is plain grass; at 1ft it is still gone; at 3ft the runway and
            // apron render and match the PO's Wine gold. So the geometric lift is doing
            // real work and 3ft stands. (An earlier change here defaulted it to 0 on the
            // reasoning that the depth bias made it redundant; that could not be observed
            // at the time because the player's airbase was not inserting any flat
            // surfaces at all -- see the container re-pick in addobj.cpp.)
            //
            // Known cost: aircraft are placed with their wheels at GetGroundLevel, so a
            // runway drawn 3ft above that leaves a parked jet sunk by 3ft (gear hidden).
            // That visual/collision split is TE2-5; fixing it means making ground contact
            // use the drawn surface, not shrinking this lift.
            const float decal = FF_RunwayDecal();
            if (getenv("FF_DEBUG_RUNWAY"))
            {
                static long c = 0;
                if ((c++ % 120) == 0)
                {
                    // FF_LINUX: also report the COARSE approximation the original
                    // (non-FF_LINUX) path uses. The whole Linux workaround -- per-frame
                    // accurate refetch + 3ft decal + heavy polygon offset -- exists
                    // because that approximation was returning 0 at airfields. If it now
                    // agrees with the accurate value, the workaround is obsolete and the
                    // original path would put runway, terrain and wheels on one plane.
                    float ap = renderer->viewpoint->GetGroundLevelApproximation(position.x, position.y);
                    fprintf(stderr, "[RUNWAY] flat GetGroundLevel=%.1f approx=%.1f delta=%.1f decal=%.1f -> z=%.1f pos=(%.0f,%.0f)\n",
                            gl, ap, gl - ap, decal, gl - decal, position.x, position.y);
                }
            }
            // FF_LINUX (TERRAIN-Z): FF_DEBUG_MESHZ=1 reports the terrain post height at
            // every LOD next to the accurate and coarse queries, so which surface the
            // DRAWN mesh actually corresponds to can be identified instead of assumed.
            if (getenv("FF_DEBUG_MESHZ"))
            {
                static long m = 0;

                if ((m++ % 240) == 0)
                {
                    extern float FF_GroundLevelAtLOD(class TViewPoint *vp, float x, float y, int lod);
                    char buf[192];
                    int o = 0;

                    for (int L = 0; L <= 5 and o < (int)sizeof(buf) - 24; ++L)
                        o += snprintf(buf + o, sizeof(buf) - o, "L%d=%.1f ", L,
                                      FF_GroundLevelAtLOD((class TViewPoint *)renderer->viewpoint,
                                                          position.x, position.y, L));

                    fprintf(stderr, "[MESHZ] pos=(%.0f,%.0f) accurate=%.1f approx=%.1f | %s\n",
                            position.x, position.y, gl,
                            renderer->viewpoint->GetGroundLevelApproximation(position.x, position.y),
                            buf);
                    fflush(stderr);
                }
            }

            position.z = gl - decal;
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

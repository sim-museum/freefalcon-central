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
    { extern int g_ffBldgDraws; g_ffBldgDraws++; }
#endif
#ifdef FF_LINUX
    {
        extern int g_ffRunwayDbg;
        static int s_rwyDbg = -1;

        if (s_rwyDbg < 0) s_rwyDbg = getenv("FF_DEBUG_RUNWAY") ? 1 : 0;

        if (g_ffRunwayDbg && previousLOD == -1 && s_rwyDbg)
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
        // FF_LINUX: cached -- this gates the per-frame accurate-refetch block for every
        // flat surface, so an uncached getenv here costs on every runway draw of every
        // frame. (Missed on the first pass through this file, which cached the three
        // FF_DEBUG_RUNWAY sites but not the one guarding the whole block.)
        static int s_rwyOld = -1;

        if (s_rwyOld < 0) s_rwyOld = getenv("FF_RUNWAY_OLD") ? 1 : 0;

        if (g_ffRunwayDbg && not s_rwyOld)
        {
            int glLod = 99;
            float gl = renderer->viewpoint->GetGroundLevel(position.x, position.y, NULL, &glLod);

            // FF_LINUX (TERRAIN-Z): the per-frame ACCURATE refetch is only accurate
            // where fine posts exist. On a landing approach L0/L1 are not streamed at
            // the airfield, and GetGroundLevel then interpolates between widely-spaced
            // COARSE posts -- returning values matching no available post (measured:
            // L2=L3=-26.0 while it answers -18.3). That interpolation does not preserve
            // the flattened airbase plateau, so the strip is drawn warped by up to 9.5ft
            // until the fine LOD arrives.
            //
            // Gate on the answer's PROVENANCE, not on whether we re-queried: when a
            // coarse LOD answered, prefer the nearest-post approximation, which does
            // return the plateau. DEFAULT ON since 2026-08-26: PO confirmed it cures the
            // aircraft sinking into the drawn runway and emerging again during rollout
            // and takeoff. FF_NO_RUNWAY_LODGATE=1 reverts.
            {
                static int gate = -1;

                if (gate < 0) gate = getenv("FF_NO_RUNWAY_LODGATE") ? 0 : 1;  // DEFAULT ON

                // FF_LINUX (TERRAIN-Z / LODGATE-2): the glLod > 1 test is the wrong
                // question. It asks "did a coarse LOD answer?", but glLod reports which
                // level the walk STOPPED at, not what the returned number was actually
                // interpolated from. MEASURED 2026-08-27, PO's TE-9 landing at the Korea
                // airbase (log ff-acmi-dbg3, 3914 MESHZ samples): at three positions on
                // the field glLod came back 0 -- finest, gate stays shut -- while gl was
                // -22.5 / -25.2 / -23.4 against L0 = -26.0 and approx = -26.0. Physics
                // answered -26.00 for the whole rollout. Across all 1799 glLod == 0
                // samples gl differs from L0 by up to 3.5ft, over 1ft in 98 of them.
                //
                // So the runway got drawn on a surface up to 3.5ft off the one the wheels
                // rest on, varying with position -- and that variation IS the aircraft
                // sinking into the strip and re-emerging as it rolls. The gate was
                // supposed to have cured that (PO confirmed 2026-08-26); it recurred
                // because the provenance flag it trusts does not mean what it says.
                //
                // Ask the answerable question instead: does the accurate query disagree
                // with the nearest-post approximation? This block only ever runs for FLAT
                // surfaces -- runway, taxiway, apron -- which sit on a flattened plateau,
                // so on a correct plateau the two agree and this changes nothing. Any
                // disagreement means the interpolation has wandered off the plateau, and
                // the approximation is the one that matches both the plateau and physics.
                // FF_LODGATE_LODONLY=1 restores the old glLod > 1 test for A/B.
                static int lodOnly = -1;

                if (lodOnly < 0) lodOnly = getenv("FF_LODGATE_LODONLY") ? 1 : 0;

                if (gate)
                {
                    float ap = renderer->viewpoint->GetGroundLevelApproximation(position.x, position.y);

                    if (ap > -99000.0f)
                    {
                        const float FF_PLATEAU_TOL = 0.5f;   // ft

                        if (glLod > 1 or ( not lodOnly and fabsf(gl - ap) > FF_PLATEAU_TOL))
                            gl = ap;
                    }
                }
            }
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
            static int s_rwyDbg2 = -1;

            if (s_rwyDbg2 < 0) s_rwyDbg2 = getenv("FF_DEBUG_RUNWAY") ? 1 : 0;

            if (s_rwyDbg2)
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
            static int meshDbg = -1;

            if (meshDbg < 0) meshDbg = getenv("FF_DEBUG_MESHZ") ? 1 : 0;

            if (meshDbg)
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

                    fprintf(stderr, "[MESHZ] pos=(%.0f,%.0f) glLod=%d used=%.1f approx=%.1f | %s\n",
                            position.x, position.y, glLod, gl,
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
#ifdef FF_LINUX
            // FF_LINUX (BURIED-2): measured with the [BLDGZ] probe -- the nearest-
            // post approximation returns QUANTIZED COARSE-POST heights (-131, -52,
            // -209...) that miss the drawn surface by 30-100 ft in both directions
            // when fine terrain has not streamed at the building's location. Nearest
            // post at LOD 3-4 spacing can be a hilltop miles away. Buildings below
            // the refined surface are invisible (the PO's "all buildings under the
            // terrain"); ones above float. The runway workaround avoids exactly this
            // by refetching accurate INTERPOLATED ground -- extend the same cure to
            // ordinary buildings. FF_NO_BLDG_INTERP=1 restores the approximation.
            {
                static int s_noInterp = -1;

                if (s_noInterp < 0) s_noInterp = getenv("FF_NO_BLDG_INTERP") ? 1 : 0;

                position.z = s_noInterp
                             ? renderer->viewpoint->GetGroundLevelApproximation(position.x, position.y)
                             : renderer->viewpoint->GetGroundLevel(position.x, position.y);
            }
#else
            position.z = renderer->viewpoint->GetGroundLevelApproximation(position.x, position.y);
#endif
            previousLOD = LOD;

            // FF_LINUX (BURIED-2): FF_DEBUG_MESHZ=1 -- measure the burial at the
            // moment it is drawn, for ORDINARY buildings (the runway/flat branch
            // above has its own probe; this branch had none, and it is the one the
            // PO's buried buildings take).
            //
            // position.z here is the NEAREST POST at the finest available LOD
            // (GetGroundLevelApproximation does no interpolation), while the drawn
            // terrain surface is the INTERPOLATED triangle mesh. On any slope those
            // differ, and if the drawn surface sits above this z the building is
            // buried by exactly the printed delta. This is the same nearest-vs-
            // interpolated gap that sank ground explosions (BOOM-2), measured at
            // the building-placement site instead of argued about.
            {
                static int s_dbgMZ = -1;

                if (s_dbgMZ < 0) s_dbgMZ = getenv("FF_DEBUG_MESHZ") ? 1 : 0;

                if (s_dbgMZ)
                {
                    static long s_n = 0;

                    if ((s_n++ % 60) == 0)
                    {
                        extern float FF_DrawnGroundLevel(float x, float y);
                        const float ffDrawn = FF_DrawnGroundLevel(position.x, position.y);
                        const float ffAcc = renderer->viewpoint->GetGroundLevel(position.x, position.y);

                        fprintf(stderr, "[BLDGZ] pos=(%.0f,%.0f) placedZ=%.1f "
                                "interpGL=%.1f drawn=%.1f buriedBy=%.1f\n",
                                position.x, position.y, position.z,
                                ffAcc, ffDrawn,
                                position.z - ffDrawn);   // >0 = below drawn surface (z is DOWN)
                        fflush(stderr);
                    }
                }
            }
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
            static int s_rwyDbg2 = -1;

            if (s_rwyDbg2 < 0) s_rwyDbg2 = getenv("FF_DEBUG_RUNWAY") ? 1 : 0;

            if (s_rwyDbg2)
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

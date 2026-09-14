/***************************************************************************\
    DrawRdbd.h    Scott Randolph
    July 29, 1997

    Derived class to do special position and containment processing for
 sections of bridges.
\***************************************************************************/
#ifndef _DRAWRDBD_H_
#define _DRAWRDBD_H_

#include "Edge.h"
#include "drawbldg.h"


class DrawableRoadbed : public DrawableBuilding
{
public:
    DrawableRoadbed(int IDbase, int IDtop, Tpoint *pos, float heading, float height, float angle, float s = 1.0f);
    virtual ~DrawableRoadbed();

    virtual void Draw(class RenderOTW *renderer, int LOD);
    void         DrawSuperstructure(class RenderOTW *renderer, int LOD);
    void         DrawSuperstructure(class Render3D *renderer);

    BOOL OnRoadbed(Tpoint *pos, Tpoint *normal);

#ifdef FF_LINUX
    /* RECON-3 S6: the bridge census needs to know whether a segment carries a superstructure,
       because "base + superstructure" is one of the two explanations for the two rows of spans
       the recon capture shows. */
    BOOL HasSuperstructure(void) const
    {
        return superStructure ? TRUE : FALSE;
    }
#endif

    // This one is for internal use only.  Don't use it or you'll break things...
    void ForceZ(float z)
    {
        position.z = z;

        if (superStructure) superStructure->ForceZ(z);
    };

protected:
    DrawableBSP *superStructure;

    float start;
    float length;

    float cosInvYaw;
    float sinInvYaw;

    float tanRampAngle;
    Edge ramp;

#ifdef USE_SH_POOLS
public:
    // Overload new/delete to use a SmartHeap fixed size pool
    void *operator new(size_t size)
    {
        ShiAssert(size == sizeof(DrawableRoadbed));
        return MemAllocFS(pool);
    };
    void operator delete(void *mem)
    {
        if (mem) MemFreeFS(mem);
    };
    static void InitializeStorage()
    {
        pool = MemPoolInitFS(sizeof(DrawableRoadbed), 10, 0);
    };
    static void ReleaseStorage()
    {
        MemPoolFree(pool);
    };
    static MEM_POOL pool;
#endif
};

#endif // _DRAWRDBD_H_

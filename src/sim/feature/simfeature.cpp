#include "graphics/include/drawbldg.h"
#include "stdhdr.h"
#include "simfeat.h"
#include "initdata.h"
#include "otwdrive.h"
#include "classtbl.h"
#include "entity.h"
#include "atcbrain.h"
#include "simdrive.h"
#include "Objectiv.h"
#include "ptdata.h"
#include "entity.h"
#include "playerop.h"
#include "Feature.h"
#include "sfx.h"
#include "graphics/include/rviewpnt.h"
#include "atcbrain.h"

/* 2001-03-06 S.G. FOR RADAR RANGE TO RADAR FEATURE */
#include "radardata.h"

#ifdef USE_SH_POOLS
MEM_POOL SimFeatureClass::pool;
#endif

void CalcTransformMatrix(SimBaseClass* theObject);
int GetTextureIdxFromHeading(int hdg);

SimFeatureClass::SimFeatureClass(VU_BYTE** stream, long *rem) : SimStaticClass(stream, rem)
{
    InitLocalData();
}

SimFeatureClass::SimFeatureClass(FILE* filePtr) : SimStaticClass(filePtr)
{
    InitLocalData();
}

SimFeatureClass::SimFeatureClass(int type) : SimStaticClass(type)
{
    InitLocalData();
}

SimFeatureClass::~SimFeatureClass(void)
{
    CleanupLocalData();
}

void SimFeatureClass::InitData()
{
    SimStaticClass::InitData();
    InitLocalData();
}

#ifdef FF_LINUX
// FF_LINUX (TERRAIN-Z): deferred feature ground re-snap (see Wake()).
// Serviced from the sim loop via FF_ServiceFeatureResnaps().
#include <vector>
#include <mutex>
#include "rviewpnt.h"   // RViewPoint/TViewPoint: LOD-aware GetGroundLevel

namespace
{
struct FFResnapEntry
{
    VU_ID id;
    float lastZ;
    int tries;
};

std::vector<FFResnapEntry> ffResnapQueue;
std::mutex ffResnapLock;

bool ffResnapEnabled(void)
{
    static int cached = -1;

    if (cached == -1)
        cached = getenv("FF_NO_FEATURE_RESNAP") ? 0 : 1;

    return cached != 0;
}
}

void FF_QueueFeatureResnap(VU_ID id, float z)
{
    if (not ffResnapEnabled())
        return;

    std::lock_guard<std::mutex> g(ffResnapLock);
    ffResnapQueue.push_back({ id, z, 0 });
}

// Called about once a second from the sim loop. Each entry is re-queried until
// two consecutive answers agree (terrain streamed in and stabilised) or the
// attempt cap is hit.
void FF_ServiceFeatureResnaps(void)
{
    if (not ffResnapEnabled())
        return;

    std::vector<FFResnapEntry> work;
    {
        std::lock_guard<std::mutex> g(ffResnapLock);

        if (ffResnapQueue.empty())
            return;

        work.swap(ffResnapQueue);
    }

    static int ffDbg = -1;

    if (ffDbg == -1)
        ffDbg = getenv("FF_DEBUG_RESNAP") ? 1 : 0;

    int resnapped = 0, settled = 0, dropped = 0;
    std::vector<FFResnapEntry> keep;
    keep.reserve(work.size());

    for (FFResnapEntry &e : work)
    {
        SimFeatureClass *feat = (SimFeatureClass*)vuDatabase->Find(e.id);

        if (not feat or not feat->IsStatic() or feat->IsDead())
        {
            dropped++;
            continue;
        }

        // Measured (TE-02): "same answer twice" is NOT a safe settle test --
        // during the terrain-streaming transient the query answers 0 at coarse
        // LOD repeatedly, so two ticks inside the transient look "stable".
        // Settle only when the answer actually came from fine terrain.
        RViewPoint *vp = OTWDriver.GetViewpoint();
        int lod = 99;
        float z = e.lastZ;

        if (vp)
            z = vp->GetGroundLevel(feat->XPos(), feat->YPos(), NULL, &lod);

        if (z != e.lastZ)
        {
            feat->SetPosition(feat->XPos(), feat->YPos(), z);

            // Statics have no per-frame draw sync, so move the drawable too --
            // repositioning only the entity would fix physics and leave the
            // pixels where they were.
            if (feat->drawPointer)
            {
                Tpoint p;
                p.x = feat->XPos();
                p.y = feat->YPos();
                p.z = feat->ZPos();
                feat->drawPointer->SetPosition(&p);
            }

            e.lastZ = z;
            resnapped++;
        }

        if (lod <= 1)
        {
            settled++;   // fine terrain answered -- this feature is done
            continue;
        }

        e.tries++;

        if (e.tries < 30)
            keep.push_back(e);
        else
            dropped++;
    }

    if (not keep.empty())
    {
        std::lock_guard<std::mutex> g(ffResnapLock);

        for (FFResnapEntry &e : keep)
            ffResnapQueue.push_back(e);
    }

    if (ffDbg and (resnapped or settled or dropped))
    {
        fprintf(stderr, "[RESNAP] moved=%d settled=%d dropped=%d pending=%d\n",
                resnapped, settled, dropped, (int)keep.size());
        fflush(stderr);
    }
}

#endif

void SimFeatureClass::InitLocalData()
{
    Falcon4EntityClassType* classPtr;
    FeatureClassDataType *fc;

    classPtr = &Falcon4ClassTable[ Type() - VU_LAST_ENTITY_TYPE];
    fc = (FeatureClassDataType *)classPtr->dataPtr;

    strength = maxStrength = (float)fc->HitPoints;
    pctStrength = 1.0;
    dyingTimer = -1.0f;
    theBrain = NULL;
    baseObject = NULL;
    featureFlags = 0;

    // 2001-03-06 ADDED BY S.G. SO RADAR FEATURES HAVE A specialData.rdrNominalRange,,,
    if (fc->RadarType)
    {
        specialData.rdrNominalRng = RadarDataTable[fc->RadarType].NominalRange;
    }
}

void SimFeatureClass::CleanupLocalData()
{
    // sfr: @todo remove this
    if (baseObject)
    {
        // WARNING:  This is unsafe.  RemoveObject should ONLY be called from the Sim thread
        // but the destructor can happen on any thread...
        OTWDriver.RemoveObject(baseObject, TRUE);
        baseObject = NULL;
    }
}

void SimFeatureClass::CleanupData()
{
    CleanupLocalData();
    SimStaticClass::CleanupData();
}

void SimFeatureClass::Init(SimInitDataClass* initData)
{
    Falcon4EntityClassType* classPtr;
    FeatureClassDataType *fc;

    classPtr = (Falcon4EntityClassType *)EntityType();
    fc = (FeatureClassDataType *)classPtr->dataPtr;

    strength = maxStrength = (float)fc->HitPoints;
    pctStrength = 1.0;
    dyingTimer = -1.0f;
    sfxTimer = 0.0f;

    SetFlag(ON_GROUND);

    if ( not initData)
        return;

    featureFlags = initData->specialFlags;

    SetDelta(0.0F, 0.0F, 0.0F);
    SetYPR(initData->heading, 0.0F, 0.0F);
    SetYPRDelta(0.0F, 0.0F, 0.0F);
    CalcTransformMatrix(this);

    SimStaticClass::Init(initData);
}

int SimFeatureClass::Wake()
{
    int texIdx = 0;
    int index;
    int rwyHeading;
    float yaw, z;
    int retval = 0;

    // KCK: Sets up this object to become sim aware
    if (IsAwake())
    {
        return retval;
    }

    SimStaticClass::Wake();

    // KCK: Set our Z position to an approximation of the ground level.
    // Hopefully this will be close enough.
    //RViewPoint* viewpoint = OTWDriver.GetViewpoint(); // JB 010616 safer
    //if (viewpoint)
    z = OTWDriver.GetApproxGroundLevel(XPos(), YPos());
    //else
    // z = 0.0;
    SetPosition(XPos(), YPos(), z);
#ifdef FF_LINUX
    // FF_LINUX (TERRAIN-Z): "hopefully close enough" is not close enough. At
    // sim entry, features Wake() in a burst BEFORE the terrain around the
    // viewpoint has streamed in, so the approximation runs out of LODs and
    // answers 0 (measured: the wake burst logs "RAN OUT OF LODs -> elevation=0"
    // while every query a moment later answers -14.0 at lod 0). The bad z is
    // then BAKED: nothing ever re-snaps a feature, so the whole airbase sits
    // ~11-14 ft below the terrain mesh that finishes streaming moments later --
    // the PO's "physics terrain seems to be a few meters below the graphics
    // terrain" during takeoff, landing and bombing.
    //
    // Measured on TE-02 (FF_DEBUG_RESNAP): 500 features settle on their FIRST
    // recheck -- the wake-time query is usually already right. The durable bug
    // is the line above: VuEntity::SetPosition moves the ENTITY only, and
    // nothing ever tells the drawable. The DrawableBSP keeps the position it
    // was created with (simdata.z = 0 from objectiv.cpp, i.e. sea level), so
    // the whole airbase RENDERS ~11-14 ft below the terrain mesh while the
    // entity/physics sit correctly on it -- the PO's "physics terrain a few
    // meters off the graphics terrain". Sync the drawable here, at the same
    // moment the entity gets its ground snap.
    if (drawPointer)
    {
        Tpoint ffP;
        ffP.x = XPos();
        ffP.y = YPos();
        ffP.z = ZPos();
        drawPointer->SetPosition(&ffP);
    }

    {
        static int s_wk = -1;

        if (s_wk == -1)
            s_wk = getenv("FF_DEBUG_RESNAP") ? 1 : 0;

        if (s_wk)
        {
            static int s_wc = 0;

            if (s_wc++ < 20)
            {
                fprintf(stderr, "[WAKE] feature id=%d pos=(%.0f,%.0f) snapZ=%.2f drawPtr=%s\n",
                        (int)Id().num_, XPos(), YPos(), z, drawPointer ? "yes" : "NULL");
                fflush(stderr);
            }
        }
    }

    // Queue this feature for convergent re-snapping: re-query about once a
    // second and re-position until the answer stops moving (terrain finished
    // streaming here) or a generous attempt cap is hit. No LOD plumbing needed,
    // and features waking with fine terrain already available converge on the
    // first check. FF_NO_FEATURE_RESNAP=1 reverts to the old bake. This also
    // covers the wake-during-streaming case, where the first answer really is
    // the transient ("RAN OUT OF LODs -> elevation=0", seen once per run).
    FF_QueueFeatureResnap(Id(), z);
#endif

    if (drawPointer)
    {
        DrawableBSP *bsp;
        int i, num;

        SimDriver.AddToFeatureList(this);
        bsp = (DrawableBSP *)drawPointer;

        // See if we need to add smoke to smoke stacks
        if (IsSetCampaignFlag(FEAT_HAS_SMOKE_STACK))
        {
            num = bsp->GetNumSlots();

            for (i = 0; i < num; i++)
            {
                // TODO:  Shouldn't this stay tied to the feature so it lives until reaggregation???
                SfxClass *sfx = new SfxClass(
                    SFX_SMOKING_FEATURE, // type
                    i, // slot #
                    this, // world pos
                    2.0f, // time to live
                    40.0f  // scale
                );
                OTWDriver.AddSfxRequest(sfx);
            }
        }

        // Is this something that needs lights turned on at night/off during the day?
        if (IsSetCampaignFlag(FEAT_HAS_LIGHT_SWITCH))
        {
            #ifdef FF_LINUX
            // FF_LINUX (OTWTHREAD-1): report the calling thread EVERY time, not only
            // when the assertion in otwlist.cpp happens to fire. The assertion is
            // one-hit-per-process and did not reproduce in ~40 runs, so it is a poor
            // sampler. If these callers routinely run off the sim thread, the missing
            // objectCriticalSection is a live exposure regardless. FF_DEBUG_OTWTHREAD=1.
            {
                extern DWORD gSimThreadID;
                static int ffTd = -1;
                static long ffN = 0, ffOff = 0;

                if (ffTd < 0)
                    ffTd = getenv("FF_DEBUG_OTWTHREAD") ? 1 : 0;

                if (ffTd)
                {
                    ffN++;

                    if (GetCurrentThreadId() not_eq gSimThreadID)
                        ffOff++;

                    if (ffN <= 3 or (ffN % 500) == 0)
                    {
                        fprintf(stderr, "[OTWCALL] simfeature cur=%lu sim=%lu offThread=%ld/%ld\n",
                                (unsigned long)GetCurrentThreadId(),
                                (unsigned long)gSimThreadID, ffOff, ffN);
                        fflush(stderr);
                    }
                }
            }
#endif
            OTWDriver.AddToLitList(bsp);
        }

        // Is This a runway number?
        if (EntityType()->classInfo_[VU_TYPE] == TYPE_RUNWAY and EntityType()->classInfo_[VU_STYPE] == STYPE_RUNWAY_NUM)
        {
            ShiAssert(GetCampaignObject());

            if ((Objective)GetCampaignObject())
            {
                ShiAssert(((Objective)GetCampaignObject())->brain);

                if (((Objective)GetCampaignObject())->brain)
                {
                    index = ((Objective)GetCampaignObject())->GetComponentIndex(this);
                    texIdx = ((Objective)GetCampaignObject())->brain->GetRunwayTexture(index);
                    ((DrawableBSP*)drawPointer)->SetTextureSet(texIdx);
                }
            }
        }

        // Is This a taxiway sign?
        if (EntityType()->classInfo_[VU_TYPE] == TYPE_TAXIWAY and 
            (EntityType()->classInfo_[VU_STYPE] == STYPE_THP or EntityType()->classInfo_[VU_STYPE] == STYPE_THPX))
        {
            // NOTE: Runway pieces are defined upside down. a heading of 0 means runway 18
            yaw = Yaw() * RTD + 180.0F;

            if (yaw > 360.0F)
            {
                yaw -= 360.0F;
            }

            rwyHeading = FloatToInt32((float)floor(yaw * 0.1F + 0.5F));

            texIdx = GetTextureIdxFromHeading(rwyHeading);

            ((DrawableBSP*)drawPointer)->SetTextureSet(texIdx);
        }
    }

    return retval;
}

int SimFeatureClass::Sleep(void)
{
    if ( not IsAwake())
    {
        return 0;
    }

    // Is this something we were managing in our time of day list?
    if (IsSetCampaignFlag(FEAT_HAS_LIGHT_SWITCH) and drawPointer)
    {
        OTWDriver.RemoveFromLitList((DrawableBSP *)drawPointer);
    }

    // If we're a bridge or platform with a "base" drawable, put that on the kill queue first (so it dies last)
    if (baseObject)
    {
        OTWDriver.RemoveObject(baseObject, TRUE);
        baseObject = NULL;
    }

    SimBaseClass::Sleep();

    SimDriver.RemoveFromFeatureList(this);

    return 0;
}

int SimFeatureClass::GetRadarType()
{
    FeatureClassDataType *fc;
    fc = (FeatureClassDataType *)Falcon4ClassTable[ Type() - VU_LAST_ENTITY_TYPE ].dataPtr;

    return fc->RadarType;
}

#if 0
/* sfr: not used
typedef struct {
   SimBaseClass* object;
   struct TmpObject* next;
} TmpObjectList;
*/
#endif

void SimFeatureClass::JoinFlight(void)
{
    /*
    VuEntityType* classPtr;

    if (GetCampaignObject()->GetComponentLead() == this)
    {
      classPtr = GetCampaignObject()->EntityType();
      // Should I have a brain?
      if (classPtr->classInfo_[VU_TYPE] == TYPE_AIRBASE or
          classPtr->classInfo_[VU_TYPE] == TYPE_AIRSTRIP)
      {
         theBrain = new ATCBrain (this);
         SimDriver.atcList->ForcedInsert(this);
      }
    }
    */
}

VU_ERRCODE SimFeatureClass::InsertionCallback(void)
{
    /* KCK: Not sure if this code is still relevant - I highly doubt it
       if ( not IsLocal())
       {
          // KCK: If this is an element of a campaign unit or objective, it was sent
          // to us during startup - I need to synthisize a deaggregation event.
          if (GetCampaignObject() > (VuEntity*)MAX_IA_CAMP_UNIT)  // KCK: Leon's hack - assumes 1-100 is dogfight slot #
        {
           // Synthisize a deaggregation (NOTE: not a Wake())
           ((CampEntity)GetCampaignObject())->SetAggregate(0);
           if ( not ((CampEntity)GetCampaignObject())->components)
        {
           DeaggregateList->ForcedInsert(GetCampaignObject());
           ((CampEntity)GetCampaignObject())->components = new TailInsertList();
           ((CampEntity)GetCampaignObject())->components->Init();
        }
           // Add the element to the components list
           ((CampEntity)GetCampaignObject())->components->ForcedInsert(this);
        }
          else
        {
           // KCK: All other objects will get woken now if possible, or during sim startup.
           if (OTWDriver.IsActive())
           Wake();
        }
       }
    */
    return SimStaticClass::InsertionCallback();
}

int GetTextureIdxFromHeading(int hdg)
{
    int texIdx = 0;

    switch (hdg)
    {
        case 0:
            texIdx = 37;
            break;

        case 1:
            texIdx = 0;
            break;

        case 2:
            texIdx = 1;
            break;

        case 3:
        case 4:
            texIdx = 4;
            break;

        case 5:
        case 6:
            texIdx = 5;
            break;

        case 7:
        case 8:
            texIdx = 6;
            break;

        case 9:
            texIdx = 7;
            break;

        case 10:
        case 11:
            texIdx = 8;
            break;

        case 12:
            texIdx = 9;
            break;

        case 13:
        case 14:
            texIdx = 10;
            break;

        case 16:
            texIdx = 13;
            break;

        case 15:
        case 17:
            texIdx = 16;
            break;

        case 18:
            texIdx = 17;
            break;

        case 19:
            texIdx = 20;
            break;

        case 20:
            texIdx = 21;
            break;

        case 22:
        case 21:
            texIdx = 24;
            break;

        case 23:
        case 24:
            texIdx = 25;
            break;

        case 25:
        case 26:
            texIdx = 26;
            break;

        case 27:
            texIdx = 27;
            break;

        case 28:
        case 29:
            texIdx = 28;
            break;

        case 30:
            texIdx = 29;
            break;

        case 32:
            texIdx = 30;
            break;

        case 34:
            texIdx = 33;
            break;

        case 33:
        case 35:
            texIdx = 36;
            break;

        case 36:
            texIdx = 37;
            break;
    }

    return texIdx;
}

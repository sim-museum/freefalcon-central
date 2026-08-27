#include <cstdlib>   // atexit (UAF-1 probe)
#include <float.h>
#include "vu_priv.h"
#include "vu2.h"

#if VU_ALL_FILTERED

VuGridTree::VuGridTree(VuBiKeyFilter* filter, unsigned int res) :
    VuCollection(filter), res_(res), suspendUpdates_(FALSE), nextgrid_(0)
{
    table_        = new VuRedBlackTree*[res_];

    for (unsigned int i = 0; i < res_; ++i)
    {
        table_[i] = new VuRedBlackTree(filter);
    }

    ffMagic_ = 0x56475254u;   // 'VGRT' -- FF_LINUX UAF-1 liveness sentinel
    vuCollectionManager->GridRegister(this);
}

VuGridTree::~VuGridTree()
{
    // FF_LINUX: deregister FIRST. This used to free table_ and only afterwards call
    // GridDeRegister, leaving a window in which a grid still reachable from
    // gridcoll_ had a dangling table_. VuCollectionManager::HandleMove iterates
    // gridcoll_ under gridsMutex_ and calls Move(), which indexes table_ and calls
    // filter_->RemoveTest() -- and the teardown below does NOT hold gridsMutex_, so
    // the two can overlap. Same shape as the ORDER-1 family: the step that makes the
    // object unreachable was sequenced after the object had already been torn down.
    ffMagic_ = 0xDEADDEADu;   // FF_LINUX UAF-1: poison BEFORE any teardown
    vuCollectionManager->GridDeRegister(this);
    Purge();
    delete [] table_;
}

// FF_LINUX (UAF-1): probe counters, reported once at exit so the hot path does no I/O.
long ffGridMoves_ = 0, ffGridLive_ = 0;
static void ffGridReport(void)
{
    if (ffGridMoves_)
        fprintf(stderr, "[GRIDMAGIC-EXIT] %ld Move() calls, %ld on live grids, %ld dangling\n",
                ffGridMoves_, ffGridLive_, ffGridMoves_ - ffGridLive_);
}
static int ffGridReportOnce = (atexit(ffGridReport), 0);

VU_ERRCODE VuGridTree::PrivateInsert(VuEntity* entity)
{
    VuBiKeyFilter *bkf = GetBiKeyFilter();
    VuRedBlackTree *row = table_[bkf->Key1(entity)];
    return row->ForcedInsert(entity);
}

VU_ERRCODE VuGridTree::PrivateRemove(VuEntity *entity)
{
    VuBiKeyFilter *bkf = GetBiKeyFilter();
    VuRedBlackTree* row = table_[bkf->Key1(entity)];
    VU_ERRCODE res = row->Remove(entity);
    return res;
}

bool VuGridTree::PrivateFind(VuEntity* entity) const
{
    VuBiKeyFilter *bkf = GetBiKeyFilter();
    const VuRedBlackTree* row = table_[bkf->Key1(entity)];
    return row->Find(entity);
}

VU_ERRCODE VuGridTree::Move(VuEntity *ent, BIG_SCALAR coord1, BIG_SCALAR coord2)
{
    // FF_DEBUG_GRIDMAGIC=1 (UAF-1): is the grid we were handed still alive?
    // Control counter first: "0 dangling grids" and "Move never ran" are otherwise
    // indistinguishable, which has already produced a false reading this session.
    {
        static int dbgOk = -1;
        if (dbgOk < 0) dbgOk = getenv("FF_DEBUG_GRIDMAGIC") ? 1 : 0;

        if (dbgOk)
        {
            // Counters only -- no I/O on this path. It runs ~40,000 times per mission
            // and the periodic fprintf was a plausible source of timing perturbation
            // (10 instrumented runs clean against a 2-in-9 uninstrumented baseline).
            // Totals are reported once, at exit.
            ffGridMoves_++;
            if (ffMagic_ == 0x56475254u) ffGridLive_++;
        }
    }

    if (ffMagic_ not_eq 0x56475254u)
    {
        static int dbg = -1;
        if (dbg < 0) dbg = getenv("FF_DEBUG_GRIDMAGIC") ? 1 : 0;
        static int n = 0;
        if (dbg and ++n <= 8)
            fprintf(stderr, "[GRIDMAGIC] Move() on grid %p with magic=0x%08x (expected 0x56475254)\n",
                    (void*)this, ffMagic_);
    }

    VuScopeLock l(GetMutex());
    VuBiKeyFilter *bkf = GetBiKeyFilter();

    if ((ent not_eq NULL) and (ent->VuState() == VU_MEM_ACTIVE) and bkf->RemoveTest(ent))
    {
        VuEntityBin safe(ent);
        VU_KEY ck1 = bkf->Key1(ent);
        VU_KEY nk1 = bkf->CoordToKey(coord1);
        VU_KEY ck2 = bkf->Key2(ent);
        VU_KEY nk2 = bkf->CoordToKey(coord2);

        if (ck1 not_eq nk1 or ck2 not_eq nk2)
        {
            // keys changed... have to remove and insert again
            table_[ck1]->Remove(ent);
            table_[nk1]->Insert(ent);
        }

        return VU_SUCCESS;
    }

    return VU_NO_OP;
}

VuBiKeyFilter *VuGridTree::GetBiKeyFilter() const
{
    return static_cast<VuBiKeyFilter*>(GetFilter());
}

#else

VuGridTree::VuGridTree(
    VuBiKeyFilter* filter, unsigned int numrows, BIG_SCALAR center, BIG_SCALAR radius
) :
    VuCollection(), rowcount_(numrows), suspendUpdates_(FALSE), nextgrid_(0)
{
    filter_       = static_cast<VuBiKeyFilter*>(filter->Copy());
    ulong icenter = filter->CoordToKey1(center);
    ulong iradius = filter->Distance1(radius);
    bottom_       = icenter - iradius;
    top_          = icenter + iradius;
    rowheight_    = 1.0f;//(top_ - bottom_)/rowcount_;
    invrowheight_ = 1.0f;//1.0f/(float)((top_ - bottom_)/rowcount_);

    table_        = new VuRedBlackTree*[numrows];

    for (unsigned int i = 0; i < numrows; ++i)
    {
        table_[i] = new VuRedBlackTree(filter_);
    }

    ffMagic_ = 0x56475254u;   // 'VGRT' -- FF_LINUX UAF-1 liveness sentinel
    vuCollectionManager->GridRegister(this);
}

VuGridTree::~VuGridTree()
{
    // FF_LINUX: deregister FIRST -- see the sibling destructor above. This variant is
    // worse: it also freed filter_ and nulled it while the grid was still registered,
    // and VuGridTree::Move calls filter_->RemoveTest(entity) on every visited grid.
    ffMagic_ = 0xDEADDEADu;   // FF_LINUX UAF-1: poison BEFORE any teardown
    vuCollectionManager->GridDeRegister(this);
    Purge();
    delete [] table_;
    delete filter_;
    filter_ = 0;
}

unsigned int VuGridTree::Row(VU_KEY key1) const
{
    VuScopeLock l(GetMutex());
    // if this goes off - the FPU is rounding to nearest rather than down.
    _controlfp(_RC_CHOP, MCW_RC);

    unsigned int index = 0;

    // compute index
    if (key1 > bottom_)
    {
        index = FTOL((key1 - bottom_) * invrowheight_);
    }

    // upper limit
    if (index >= rowcount_)
    {
        index = rowcount_ - 1;
    }

    return index;
}

VU_ERRCODE VuGridTree::ForcedInsert(VuEntity* entity)
{
    if (entity == NULL)
    {
        return VU_NO_OP;
    }

    VuScopeLock l(GetMutex());

    if ( not filter_->RemoveTest(entity)) return VU_NO_OP;

    VuRedBlackTree *row = table_[Row(filter_->Key1(entity))];
    return row->ForcedInsert(entity);
}

VU_ERRCODE VuGridTree::Insert(VuEntity *entity)
{
    if (entity == NULL)
    {
        return VU_NO_OP;
    }

    VuScopeLock l(GetMutex());

    if ( not filter_->Test(entity)) return VU_NO_OP;

    VuRedBlackTree *row = table_[Row(filter_->Key1(entity))];
    return row->Insert(entity);
}

VU_ERRCODE VuGridTree::Remove(VuEntity *entity)
{
    VuScopeLock l(GetMutex());

    if (filter_->RemoveTest(entity))
    {
        VuRedBlackTree* row = table_[Row(filter_->Key1(entity))];
        VU_ERRCODE res = row->Remove(entity);
        return res;
    }

    return VU_NO_OP;
}

VU_ERRCODE VuGridTree::Remove(VU_ID entityId)
{
    // since filter is responsible for keying, we cannot use ID as key
    VuEntity *ent = vuDatabase->Find(entityId);

    if (ent)
    {
        return Remove(ent);
    }

    return VU_NO_OP;
}

VuEntity *VuGridTree::Find(VU_ID entityId) const
{
    VuEntity* ent = vuDatabase->Find(entityId);
    return Find(ent);
}

VuEntity *VuGridTree::Find(VuEntity* ent) const
{
    if ( not ent)
    {
        return NULL;
    }

    VuScopeLock l(GetMutex());
    const VuRedBlackTree* row = table_[Row(filter_->Key1(ent))];
    return row->Find(ent);
}

VU_ERRCODE VuGridTree::Move(VuEntity *ent, BIG_SCALAR coord1, BIG_SCALAR coord2)
{
    // FF_DEBUG_GRIDMAGIC=1 (UAF-1): is the grid we were handed still alive?
    // Control counter first: "0 dangling grids" and "Move never ran" are otherwise
    // indistinguishable, which has already produced a false reading this session.
    {
        static int dbgOk = -1;
        if (dbgOk < 0) dbgOk = getenv("FF_DEBUG_GRIDMAGIC") ? 1 : 0;

        if (dbgOk)
        {
            // Counters only -- no I/O on this path. It runs ~40,000 times per mission
            // and the periodic fprintf was a plausible source of timing perturbation
            // (10 instrumented runs clean against a 2-in-9 uninstrumented baseline).
            // Totals are reported once, at exit.
            ffGridMoves_++;
            if (ffMagic_ == 0x56475254u) ffGridLive_++;
        }
    }

    if (ffMagic_ not_eq 0x56475254u)
    {
        static int dbg = -1;
        if (dbg < 0) dbg = getenv("FF_DEBUG_GRIDMAGIC") ? 1 : 0;
        static int n = 0;
        if (dbg and ++n <= 8)
            fprintf(stderr, "[GRIDMAGIC] Move() on grid %p with magic=0x%08x (expected 0x56475254)\n",
                    (void*)this, ffMagic_);
    }

    VuScopeLock l(GetMutex());

    if ((ent not_eq NULL) and (ent->VuState() == VU_MEM_ACTIVE) and filter_->RemoveTest(ent))
    {
        VuEntityBin safe(ent);
        VU_KEY ck1 = filter_->Key1(ent);
        VU_KEY nk1 = filter_->CoordToKey1(coord1);
        VU_KEY ck2 = filter_->Key2(ent);
        VU_KEY nk2 = filter_->CoordToKey2(coord2);

        if (ck1 not_eq nk1 or ck2 not_eq nk2)
        {
            // keys changed... have to remove and insert again
            table_[Row(ck1)]->Remove(ent);
            table_[Row(nk1)]->Insert(ent);
        }

        return VU_SUCCESS;
    }

    return VU_NO_OP;
}


#endif


unsigned int VuGridTree::Purge(VU_BOOL all)
{
    int retval = 0;
    VuScopeLock l(GetMutex());

    for (unsigned int i = 0; i < res_; i++)
    {
        retval += table_[i]->Purge(all);
    }

    return retval;
}

unsigned int VuGridTree::Count() const
{
    VuScopeLock l(GetMutex());
    int count = 0;

    for (unsigned int i = 0; i < res_; i++)
    {
        count += table_[i]->Count();
    }

    return count;
}

VU_COLL_TYPE VuGridTree::Type() const
{
    return VU_GRID_TREE_COLLECTION;
}

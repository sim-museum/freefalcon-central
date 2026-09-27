#include <windows.h>
#include <process.h>
#include <cstdint>  // FF_LINUX: For uint32_t
#include "TimerThread.h"
#include "FalcSess.h"
#include "Cmpclass.h"
#include "ui/include/uicomms.h"
#include "sim/include/simdrive.h"
#include "falcgame.h"
#include <cstdio>
#include <cstdlib>

// Time compression globals
uint32_t            lastStartTime;  // FF_LINUX: Use uint32_t for binary compat
int                 gameCompressionRatio = 0;
int                 targetGameCompressionRatio = 0;
int                 targetCompressionRatio = 0;

// Requests bit array
int remoteCompressionRequests;

#ifndef NO_TIMER_THREAD

#define     RT_FUNCTION_INTERVAL    20

static HANDLE       timerHandle;
static BOOL         timerRunning = FALSE;
static unsigned     timerThreadID;

static unsigned __stdcall timerThread(void)
{
    DWORD   delta;
    DWORD   timerRTFunction;

    timerRTFunction = GetTickCount();

    while (timerRunning)
    {
        vuxRealTime = GetTickCount();
        delta = (vuxRealTime - lastStartTime);

        if (delta > MAX_TIME_DELTA)
            delta = MAX_TIME_DELTA;

        //ShiAssert(vuxGameTime + SimLibMajorFrameTime >= SimLibElapsedTime);
        if ( not gCompressTillTime or vuxGameTime + delta * gameCompressionRatio < gCompressTillTime)
        {
            vuxGameTime += delta * gameCompressionRatio; // Normal time advance
        }
        else if (vuxGameTime < gCompressTillTime)
            vuxGameTime = gCompressTillTime; // We don't want to advance time past here

        //ShiAssert(vuxGameTime + SimLibMajorFrameTime >= SimLibElapsedTime);
        lastStartTime = vuxRealTime;

        if (timerRTFunction < vuxRealTime)
        {
            timerRTFunction += RT_FUNCTION_INTERVAL;
            RealTimeFunction(vuxRealTime, NULL);
        }

        Sleep(THREAD_TIME_SLICE);
    }

    return 0;
}

void beginTimer(void)
{
    ShiAssert(timerRunning == FALSE);
    timerRunning = TRUE;
    timerHandle = (HANDLE) _beginthreadex(
                      NULL, 0, (unsigned int (__stdcall *)(void *)) timerThread, NULL, 0, &timerThreadID
                  );
    ShiAssert(timerHandle);
    SetThreadPriority(timerHandle, THREAD_PRIORITY_ABOVE_NORMAL);
}

void endTimer(void)
{
    timerRunning = FALSE;
    WaitForSingleObject(timerHandle, INFINITE);
    CloseHandle(timerHandle);
}

#endif

#ifdef FF_LINUX
/* MP-CLOCK-1 (PO 2026-09-19: "client campaign clock is frozen while host game continues to advance").
   On Windows the sim Loop thread ran in RunningSim mode behind the UI and executed the NO_TIMER_THREAD
   time block in simloop.cpp, whose CLIENT branch turns the host's FalconTimingMessage
   (vuxTargetGameTime / targetGameCompressionRatio) into a local compression via
   SetOnlineTimeCompression(). The Linux port idles that thread between missions and advances the
   clock from the UI timer tick instead -- with only the single-player rule
   `vuxGameTime += delta * gameCompressionRatio`. A remote client never sets gameCompressionRatio
   itself (SetTimeCompression only files a REQUEST when online), so in the campaign UI the joiner's
   clock sat at ratio 0 for good. This is the client branch of that block, verbatim in its
   arithmetic, for the UI tick to call. Returns false when this session is not a remote client (or
   has not heard a timing message yet), leaving the caller's own rule in force.
   FF_NO_MPCLOCK_FIX=1 reverts; FF_DEBUG_MPCLOCK=1 traces it every 5 s. */
extern CampaignTime gLaunchTime;

bool FF_RemoteClientTimeStep(void)
{
    static int s_off = -1, s_dbg = -1;

    if (s_off < 0)
    {
        s_off = getenv("FF_NO_MPCLOCK_FIX") ? 1 : 0;
        s_dbg = getenv("FF_DEBUG_MPCLOCK") ? 1 : 0;
    }

    if (s_off or not FalconLocalGame or FalconLocalGame->IsLocal() or lastTimingMessage == 0)
        return false;

    if (vuPlayerPoolGroup and FalconLocalGame->Id() == vuPlayerPoolGroup->Id())
        return false;

    vuxRealTime = GetTickCount();
    DWORD real_delta = (DWORD)(vuxRealTime - lastStartTime);
    DWORD delta = real_delta;
    const int lookahead = 2000;
    int ratio;
    int y = (int)(vuxTargetGameTime + (targetGameCompressionRatio * lookahead) - vuxGameTime);

    if (y < 0)
    {
        ratio = 0;
    }
    else if (y <= lookahead + 2000)
    {
        if ((y >= lookahead) and (delta))
            delta = delta * (min(10, (y - lookahead) / 10) + 100) / 100;
        else if ((y <= lookahead) and (delta))
            delta = delta * ((100 - min(10, (lookahead - y) / 10)) / 100);

        ratio = 1;
    }
    else
    {
        ratio = y / lookahead;

        if (ratio < 4)
            ratio = 2;
        else if (ratio < 8)
            ratio = 4;
    }

    if (vuxRealTime > vuxDeadReconTime)
        ratio = 0;

    SetOnlineTimeCompression(ratio);

    uint32_t tmpTime = vuxGameTime + delta * gameCompressionRatio;

    if (vuxRealTime < vuxDeadReconTime)
    {
        int compress = targetGameCompressionRatio;

        if (compress > 4)
            compress = 4;

        vuxTargetGameTime = vuxTargetGameTime + real_delta * compress; // dead-recon the host's clock
    }

    if (FalconLocalSession->GetFlyState() not_eq FLYSTATE_FLYING and gCompressTillTime and tmpTime > gLaunchTime + 1000)
    {
        if (vuxGameTime < gCompressTillTime)
            tmpTime = gCompressTillTime;
        else
            tmpTime = vuxGameTime;
    }

    vuxGameTime = tmpTime;
    lastStartTime = vuxRealTime;

    if (s_dbg)
    {
        static DWORD s_last = 0;

        if (vuxRealTime - s_last >= 5000)
        {
            s_last = vuxRealTime;
            fprintf(stderr, "[mpclock] client step: game=%u target=%u y=%d hostRatio=%d -> ratio=%d gameComp=%d deadrecon_in=%dms lastTiming=%ums ago\n",
                    (unsigned)vuxGameTime, (unsigned)vuxTargetGameTime, y, targetGameCompressionRatio, ratio,
                    gameCompressionRatio, (int)(vuxDeadReconTime - vuxRealTime), (unsigned)(vuxRealTime - lastTimingMessage));
            fflush(stderr);
        }
    }

    return true;
}
#endif

void ResyncTimes();

// This is the main routine to change our time compression ratio
void SetTimeCompression(int newComp)
{
#ifdef FF_LINUX
    // FF_LINUX (ATO-1 harness): FF_CAMP_TIMECOMP=N forces the campaign clock rate
    // so a test run can cover campaign HOURS in wall-clock minutes -- at x1 the
    // clock advances ~2.5 min in a 150s run, which is far too little for any
    // mission TOT to expire. A request of 0 is left alone so pause still pauses.
    if (newComp > 0)
    {
        static int ffComp = -1;

        if (ffComp == -1)
        {
            const char *e = getenv("FF_CAMP_TIMECOMP");
            ffComp = e ? atoi(e) : 0;
        }

        if (ffComp > 0)
            newComp = ffComp;
    }

#endif

    if (newComp < 0)
        newComp = 0;

    if (newComp > 1024)
        newComp = 1024;

    // Force zero compression when game is over
    if (TheCampaign.EndgameResult)
        newComp = 0;

    if (gCommsMgr and gCommsMgr->Online() and FalconLocalGame)
    {
        // For online games, we only set our session's requested time compression.
        FalconLocalSession->SetReqCompression((short)newComp);
        // The resync is now done is SetReqCompression
        // ResyncTimes(); // Resync immediately
        return;
    }
    else
    {
        // Otherwise, set our compression directly
        lastStartTime = vuxRealTime;

        if ( not gameCompressionRatio and newComp)
            SimDriver.lastRealTime = vuxGameTime;

        gameCompressionRatio = newComp;
        targetCompressionRatio = newComp;
        FalconLocalSession->SetReqCompression((short)newComp);
    }
}

// This is the way we set time compression from remote (without effecting our
// requested compression, essentially.
void SetOnlineTimeCompression(int newComp)
{
    if (FalconLocalSession->GetFlyState() == FLYSTATE_FLYING)
    {
        if (newComp < 0)
            newComp = 0;

        if (newComp > 4)
            newComp = 4;
    }
    else
    {
        if (newComp < 0)
            newComp = 0;

        if (newComp > 128)
            newComp = 128;
    }

    lastStartTime = vuxRealTime;

    if ( not gameCompressionRatio and newComp)
        SimDriver.lastRealTime = vuxGameTime;

    gameCompressionRatio = newComp;
    targetCompressionRatio = newComp;
}

// This routine will temporarily change our time compression
// Used to override player's requested compression in time critical places
void SetTemporaryCompression(int newComp)
{
    if (newComp < 0)
        newComp = 0;

    if (newComp > 1024)
        newComp = 1024;

    lastStartTime = vuxRealTime;

    if ( not gameCompressionRatio and newComp)
        SimDriver.lastRealTime = vuxGameTime;

    gameCompressionRatio = newComp;
}

// This is the one and only way to change time
void SetTime(uint32_t currentTime)  // FF_LINUX: Use uint32_t for binary compat
{
    vuxGameTime = currentTime;
    vuxDeadReconTime = 0;
    vuxLastTargetGameTime = 0;
    vuxTargetGameTime = currentTime;

    //ShiAssert(vuxGameTime + SimLibMajorFrameTime >= SimLibElapsedTime);
    lastStartTime = vuxRealTime;
    TheCampaign.CurrentTime = currentTime;
    SimLibFrameElapsed = (float)currentTime;
    SimLibElapsedTime = currentTime;
    UPDATE_SIM_ELAPSED_SECONDS; // COBRA - RED - Scale Elapsed Seconds
    SimDriver.lastRealTime = currentTime;

    // ShiAssert
    // (
    // (gameCompressionRatio == 0) or
    // (TheCampaign.IsSuspended ())
    // );
}


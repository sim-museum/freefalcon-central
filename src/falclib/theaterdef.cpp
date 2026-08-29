#include <ctype.h>
#include <cstdio>
#include "TheaterDef.h"
#include "stdhdr.h"
#include "datafile.h"
#include "simfile.h"
#include "codelib/resources/reslib/src/resmgr.h"
#include "FalcSnd/voicemapper.h"
#include "entity.h"
#include "vu2/include/vuentity.h"   // FF_LINUX UAF-1: re-point entity type ptrs
#include "vu2/include/vucoll.h"     // FF_LINUX UAF-1: VuDatabaseIterator

#include "f4find.h"
#include "tactics.h"
#include "AIInput.h"
#include "Fsound.h"
#include "dispcfg.h"
#include "Falcsnd/voicefilter.h"
#include "campstr.h"

static const char THEATERLIST[] = "theater.lst";
extern char FalconDataDirectory[];
extern char FalconCampaignSaveDirectory[];
extern char FalconCampUserSaveDirectory[];
extern char FalconUIArtThrDirectory[];
extern char FalconTerrainDataDir[];
extern char FalconMovieDirectory[];
extern char FalconUISoundDirectory[];
extern char FalconObjectDataDir[];
extern char FalconMiscTexDataDir[];
extern char Falcon3DDataDir[]; // for Korea.* files
extern char FalconSoundThrDirectory[];
// RV - Biker - Make cockpits, zips, tacref and splash files also switchable with theater
extern char FalconCockpitThrDirectory[];
extern char FalconZipsThrDirectory[];
extern char FalconTacrefThrDirectory[];
extern char FalconSplashThrDirectory[];
extern char FalconPictureDirectory[];

// RV - Biker - Theater switching stuff
extern int numZips;
extern int* resourceHandle;
extern int SimPathHandle;
#define ZIPFILE_NAME    "ziplist.lst"

extern int g_nMinTacanChannel;

extern int g_nSoundSwitchFix;

extern void ClearCampCache();
extern void TheaterReload(char *theater, char *loddata);
extern void LoadTrails();
void ResetVoices(void);

void ReadAllAirframeData(void);
void ReadAllMissileData(void);
void ReadAllBombData(void);
void ReadDigitalBrainData(void);

void FreeAllAirframeData(void);
void FreeAllMissileData(void);
void FreeAllBombData(void);
void FreeDigitalBrainData(void);

TheaterList g_theaters;

TheaterDef *TheaterList::GetTheater(int n)
{
    TheaterDef *ptr;

    for (ptr = m_first; n > 0; n--)
        ptr = ptr->m_next;

    return ptr;
}


void TheaterList::AddTheater(TheaterDef *nt)
{
    TheaterDef **ptr;

    for (ptr = &m_first; *ptr; ptr = &(*ptr)->m_next)
        continue;

    *ptr = nt;
    m_count ++;
}

#define OFFSET(x) offsetof(TheaterDef, x)

static const InputDataDesc theaterdesc[] =
{
    {"name", InputDataDesc::ID_STRING, OFFSET(m_name), ""},
    {"desc", InputDataDesc::ID_STRING, OFFSET(m_description), ""},
    {"campaigndir", InputDataDesc::ID_STRING, OFFSET(m_campaign), ""},
    {"terraindir", InputDataDesc::ID_STRING, OFFSET(m_terrain), ""},
    {"artdir", InputDataDesc::ID_STRING, OFFSET(m_artdir), ""},
    {"moviedir", InputDataDesc::ID_STRING, OFFSET(m_moviedir), ""},
    {"uisounddir", InputDataDesc::ID_STRING, OFFSET(m_uisounddir), ""},
    {"objectdir", InputDataDesc::ID_STRING, OFFSET(m_objectdir), ""},
    {"3ddatadir", InputDataDesc::ID_STRING, OFFSET(m_3ddatadir), ""},
    {"misctexdir", InputDataDesc::ID_STRING, OFFSET(m_misctexdir), ""},
    {"bitmap", InputDataDesc::ID_STRING, OFFSET(m_bitmap), ""},
    {"mintacan", InputDataDesc::ID_INT, OFFSET(m_mintacan), "70"},
    {"sounddir", InputDataDesc::ID_STRING, OFFSET(m_sounddir), ""},
    {"cockpitdir", InputDataDesc::ID_STRING, OFFSET(m_cockpitdir), "art\\ckptart"},
    {"zipsdir", InputDataDesc::ID_STRING, OFFSET(m_zipsdir), ""},
    {"tacrefdir", InputDataDesc::ID_STRING, OFFSET(m_tacrefdir), ""},
    {"splashdir", InputDataDesc::ID_STRING, OFFSET(m_splashdir), "art\\splash"},
    { NULL,},
};

void LoadTheaterDef(char *name)
{
    SimlibFileClass* theaterfile;

    theaterfile = SimlibFileClass::Open(name, SIMLIB_READ);

    if (theaterfile == NULL) return;

    TheaterDef *theater = new TheaterDef;
    memset(theater, 0, sizeof * theater);

    if (ParseSimlibFile(theater, theaterdesc, theaterfile) == false)
    {
        delete theater;
    }
    else
    {
        g_theaters.AddTheater(theater);
    }

    theaterfile->Close();
    delete theaterfile;
}

void LoadTheaterList()
{
    char tlist[_MAX_PATH];
    sprintf(tlist, "%s/%s", FalconDataDirectory, THEATERLIST);
    FILE *fp = fopen(tlist, "r");

    if (fp == NULL)   // yikes- make something up
    {
        return;
    }

    char line[1024];

    while (fgets(line, sizeof line, fp))
    {
        if (line[0] == '\r' or line[0] == '#' or line[0] == ';' or line[0] == '\n')
            continue;

        char *cp;

        if (cp = strchr(line, '\n'))
            * cp = '\0';

        if (cp = strchr(line, '\r'))
            * cp = '\0';

        // Convert backslashes to forward slashes for Linux
        for (cp = line; *cp; cp++) {
            if (*cp == '\\') *cp = '/';
        }

        LoadTheaterDef(line);
    }

    fclose(fp);

}

TheaterList::~TheaterList()
{
    TheaterDef *ptr, *p2;

    for (ptr = m_first; ptr; ptr = p2)
    {
        p2 = ptr->m_next;
        delete ptr;
    }

    m_count = 0;
}

bool TheaterList::ChooseTheaterByName(const char *name)
{
    TheaterDef *ptr = FindTheaterByName(name);

    if (ptr)
        return SetNewTheater(ptr);

    return false;
}

// do all required to swap theaters
bool TheaterList::SetNewTheater(TheaterDef *td)
{
    FreeIndex();
    ClearCampCache();

    if (numZips)
    {
        for (int i = 0; i < numZips; i++)
        {
            if (resourceHandle[i])
                ResDetach(resourceHandle[i]);
            else
                break;
        }

        if (resourceHandle[0] < 0)
            delete [] resourceHandle;

        numZips = 0;
    }

    // Reinitialize res manager...
    ResInit(NULL);
    ResCreatePath(FalconDataDirectory, FALSE);

    //========
    if (strlen(td->m_campaign) > 0)
        SetPathName(FalconCampUserSaveDirectory, td->m_campaign, FalconDataDirectory);
    else
        SetPathName(FalconCampUserSaveDirectory, "Campaign\\Save", FalconDataDirectory);

    //========
    if (strlen(td->m_campaign) > 0)
        SetPathName(FalconCampaignSaveDirectory, td->m_campaign, FalconDataDirectory);
    else
        SetPathName(FalconCampaignSaveDirectory, "Campaign\\Save", FalconDataDirectory);

#ifdef FF_LINUX
    // FF_LINUX (THEATERS-1): the TE list loads Korea's missions in every theater
    // while the campaign path reaches the theater directory. Only two places assign
    // these globals -- here and init_game_paths -- and the ORDER cannot be read from
    // the log, because init_game_paths prints with printf (stdout, block-buffered
    // when redirected) while these traces use stderr. Report on stderr from BOTH
    // sites so one run settles it. FF_DEBUG_PATHS=1.
    if (getenv("FF_DEBUG_PATHS"))
    {
        fprintf(stderr, "[PATHS] SetNewTheater('%s') campaigndir='%s' -> CampUserSave='%s'\n",
                td->m_name ? td->m_name : "(null)",
                td->m_campaign ? td->m_campaign : "(empty)",
                FalconCampUserSaveDirectory);
        fflush(stderr);
    }
#endif

    //========
    if (strlen(td->m_terrain) > 0)
        SetPathName(FalconTerrainDataDir, td->m_terrain, FalconDataDirectory);
    else
        SetPathName(FalconTerrainDataDir, "terrdata\\Korea", FalconDataDirectory);

    //========
    if (strlen(td->m_moviedir) > 0)
        SetPathName(FalconMovieDirectory, td->m_moviedir, FalconDataDirectory);
    else
        SetPathName(FalconMovieDirectory, "movies", FalconDataDirectory);

    //========
    if (strlen(td->m_splashdir) > 0)
        SetPathName(FalconSplashThrDirectory, td->m_splashdir, FalconDataDirectory);
    else
        SetPathName(FalconSplashThrDirectory, "art\\splash", FalconDataDirectory);

    //========
    if (strlen(td->m_objectdir) > 0)
        SetPathName(FalconObjectDataDir, td->m_objectdir, FalconDataDirectory);
    else
        SetPathName(FalconObjectDataDir, "terrdata\\objects", FalconDataDirectory);

    //========
    if (strlen(td->m_3ddatadir) > 0)
        SetPathName(Falcon3DDataDir, td->m_3ddatadir, FalconDataDirectory);
    else
        SetPathName(Falcon3DDataDir, "terrdata\\objects", FalconDataDirectory);

    //========
    if (strlen(td->m_misctexdir) > 0)
        SetPathName(FalconMiscTexDataDir, td->m_misctexdir, FalconDataDirectory);
    else
        SetPathName(FalconMiscTexDataDir, "terrdata\\misctex", FalconDataDirectory);

    //========
    if (strlen(td->m_uisounddir) > 0)
        SetPathName(FalconUISoundDirectory, td->m_uisounddir, FalconDataDirectory);
    else
        strcpy(FalconUISoundDirectory, FalconDataDirectory);

    //========
    if (strlen(td->m_artdir) > 0)
        SetPathName(FalconUIArtThrDirectory, td->m_artdir, FalconDataDirectory);
    else
        strcpy(FalconUISoundDirectory, FalconDataDirectory);

    //========
    if (strlen(td->m_tacrefdir) > 0)
        SetPathName(FalconTacrefThrDirectory, td->m_tacrefdir, FalconDataDirectory);
    else
        strcpy(FalconTacrefThrDirectory, FalconDataDirectory);

    //========
    if (strlen(td->m_zipsdir) > 0)
        SetPathName(FalconZipsThrDirectory, td->m_zipsdir, FalconDataDirectory);
    else
        SetPathName(FalconZipsThrDirectory, "Zips", FalconDataDirectory);

    //========
    if (strlen(td->m_cockpitdir) > 0)
        SetPathName(FalconCockpitThrDirectory, td->m_cockpitdir, FalconDataDirectory);
    else
        SetPathName(FalconCockpitThrDirectory, "art\\ckptart", FalconDataDirectory);

    //========
    if (strlen(td->m_sounddir) > 0)
        SetPathName(FalconSoundThrDirectory, td->m_sounddir, FalconDataDirectory);
    else
        SetPathName(FalconSoundThrDirectory, "sounds", FalconDataDirectory);

    // We need some variables to store the diff. paths
    char tmpPath1[_MAX_PATH];
    sprintf(tmpPath1, "%s\\config", FalconDataDirectory);
    ResAddPath(tmpPath1, FALSE);

    sprintf(tmpPath1, "%s\\art", FalconDataDirectory);
    ResAddPath(tmpPath1, TRUE);

    sprintf(tmpPath1, "%s", FalconPictureDirectory);  // JB 010623
    ResAddPath(tmpPath1, FALSE);  // JB 010623

    //======================================================
    // Cobra - Add "sim" path if Korea theater
    if (( not strnicmp(td->m_name, "Korea", 5)) or ( not strnicmp(td->m_name, "Eurowar", 7)))
    {
        char tmpPath[256];
        sprintf(tmpPath, "%s\\sim", FalconDataDirectory); // JPO - so we can find raw sim files

        if (SimPathHandle == -1)
            SimPathHandle = ResAddPath(tmpPath, TRUE);
    }
    else if (SimPathHandle >= 0)
    {
        ResDetach(SimPathHandle);
        SimPathHandle = -1;
    }

    // Cobra - Setup theater's Zip folder files
    char tmpPath[_MAX_PATH];
    FILE* zipFile;

    sprintf(tmpPath, "%s/%s", FalconDataDirectory, ZIPFILE_NAME);
    zipFile = fopen(tmpPath, "r");

    if ( not zipFile)
    {
        char string[300];
        sprintf(string, "Failed to open %s\n", tmpPath);
        OutputDebugString(string);
        ShiWarning(string);  // Changed from ShiError to ShiWarning - not fatal
        numZips = 0;
    }
    else
    {
        fscanf(zipFile, "%d", &numZips);
        resourceHandle = new int[numZips];

        for (int i = 0; i < numZips; i++)
        {
            char tmp[256];
            fscanf(zipFile, "%*c%s", tmp);
            // Convert backslashes in zip path
            for (char *cp = tmp; *cp; cp++) {
                if (*cp == '\\') *cp = '/';
            }
            sprintf(tmpPath, "%s/%s", FalconZipsThrDirectory, tmp);

            if ( not strnicmp(td->m_name, "Korea", 5))
                resourceHandle[i] = ResAttach(FalconDataDirectory, tmpPath, FALSE);
            else
                // FRB - Must use FalconDataDirectory and not FalconZipsThrDirectory (don't know why?)
                //resourceHandle[i] = ResAttach (FalconZipsThrDirectory, tmpPath, TRUE);
                resourceHandle[i] = ResAttach(FalconDataDirectory, tmpPath, TRUE);
        }

        fclose(zipFile);
    }

    //======================================================

    g_nMinTacanChannel = td->m_mintacan;
    ResAddPath(FalconCampaignSaveDirectory, FALSE);
    ResAddPath(FalconUIArtThrDirectory, TRUE);
    ResAddPath(FalconZipsThrDirectory, FALSE);
    ResAddPath(FalconMovieDirectory, FALSE);
    ResAddPath(FalconUISoundDirectory, FALSE);
    ResAddPath(FalconObjectDataDir, FALSE);
    ResAddPath(FalconMiscTexDataDir, FALSE);
    ResAddPath(Falcon3DDataDir, FALSE);
    ResAddPath(FalconSoundThrDirectory, TRUE);
    ResAddPath(FalconCockpitThrDirectory, TRUE);
    ResAddPath(FalconTacrefThrDirectory, FALSE);

    fprintf(stderr, "  [SetNewTheater] ReadIndex...\n");
    ReadIndex("Strings");
    fprintf(stderr, "  [SetNewTheater] LoadPriorityTables...\n");
    LoadPriorityTables();
    fprintf(stderr, "  [SetNewTheater] TheaterReload...\n");
    TheaterReload(FalconTerrainDataDir, Falcon3DDataDir);

    fprintf(stderr, "  [SetNewTheater] ResSetDirectory...\n");
    ResSetDirectory(FalconDataDirectory);
    fprintf(stderr, "  [SetNewTheater] UnloadClassTable...\n");
    UnloadClassTable();
    fprintf(stderr, "  [SetNewTheater] LoadClassTable...\n");
    LoadClassTable("Falcon4");

    // FF_LINUX (UAF-1): every live entity caches entityTypePtr_, and for types beyond
    // the static vuTypeTable that pointer is &Falcon4ClassTable[...] -- i.e. it points
    // INTO the array UnloadClassTable() just freed above. LoadClassTable then allocates
    // a new block at a different address, leaving every pre-existing entity holding a
    // dangling type pointer. Any later ent->EntityType()->classInfo_[..] read is then a
    // use-after-free; UnitProxFilter::RemoveTest via VuGridTree::Move during
    // AircraftClass::SetDead is the reliable one (~2 in 5 runs of TE-29).
    // SetEntityType() recomputes the pointer from the stored type id, so re-pointing
    // every live entity here closes the window. FF_NO_TYPEPTR_REFRESH=1 reverts.
    {
        static int ffNoTypePtrRefresh = -1;
        if (ffNoTypePtrRefresh < 0)
            ffNoTypePtrRefresh = getenv("FF_NO_TYPEPTR_REFRESH") ? 1 : 0;

        // CRITICAL: main_linux.cpp calls SetNewTheater BEFORE InitVU, so vuDatabase is
        // NULL on the startup pass. VuDatabaseIterator's constructor uses the global
        // with no null check (and dereferences vuDatabase->dbHash_ under
        // VU_ALL_FILTERED), so constructing one here would fault on every launch.
        // There are no entities to re-point before the database exists anyway.
        if ( not ffNoTypePtrRefresh and vuDatabase not_eq NULL)
        {
            VuDatabaseIterator iter;
            int nRepointed = 0;

            for (VuEntity *e = iter.GetFirst(); e not_eq NULL; e = iter.GetNext())
            {
                e->SetEntityType(e->Type());
                nRepointed++;
            }

            fprintf(stderr, "  [SetNewTheater] re-pointed %d entity type pointers\n", nRepointed);
        }
    }
    fprintf(stderr, "  [SetNewTheater] ReadCampAIInputs...\n");
    ReadCampAIInputs("Falcon4");
    fprintf(stderr, "  [SetNewTheater] FreeTactics...\n");
    FreeTactics();
    fprintf(stderr, "  [SetNewTheater] LoadTactics...\n");
    LoadTactics("Falcon4");
    fprintf(stderr, "  [SetNewTheater] LoadTrails...\n");
    LoadTrails();

    fprintf(stderr, "  [SetNewTheater] Freeing SIM data...\n");
    // RV - Biker - Reload SIM stuff
    FreeAllAirframeData();
    FreeAllMissileData();
    FreeAllBombData();
    FreeDigitalBrainData();

    fprintf(stderr, "  [SetNewTheater] Reading SIM data...\n");
    ReadAllAirframeData();
    ReadAllMissileData();
    ReadAllBombData();
    ReadDigitalBrainData();

    fprintf(stderr, "  [SetNewTheater] SetCurrentTheater...\n");
    SetCurrentTheater(td);
    fprintf(stderr, "  [SetNewTheater] Complete!\n");

#ifdef FF_LINUX
    // FF_LINUX (THEATER-1): the PO reports that switching to Balkans leaves the
    // menu background stale, shows a Korea map and generates no missions, while
    // the Wine build switches cleanly. Path resolution has been ruled out by
    // inspection -- SetPathName converts backslashes, and every directory the
    // Balkans .tdf names exists with matching case -- so dump what the globals
    // ACTUALLY hold once the switch has finished, rather than reasoning about
    // what they should hold. FF_DEBUG_THEATER=1.
    if (getenv("FF_DEBUG_THEATER"))
    {
        fprintf(stderr, "[THEATER] after switch to '%s':\n", td->m_name);
        fprintf(stderr, "[THEATER]   campaign  = %s\n", FalconCampaignSaveDirectory);
        fprintf(stderr, "[THEATER]   terrain   = %s\n", FalconTerrainDataDir);
        fprintf(stderr, "[THEATER]   objects   = %s\n", FalconObjectDataDir);
        fprintf(stderr, "[THEATER]   3ddata    = %s\n", Falcon3DDataDir);
        fprintf(stderr, "[THEATER]   uiart     = %s\n", FalconUIArtThrDirectory);
        fprintf(stderr, "[THEATER]   cockpit   = %s\n", FalconCockpitThrDirectory);
        fprintf(stderr, "[THEATER]   misctex   = %s\n", FalconMiscTexDataDir);
        fprintf(stderr, "[THEATER]   splash    = %s\n", FalconSplashThrDirectory);
        fflush(stderr);
    }

#endif
    return true;
}

void TheaterList::DoSoundSetup() // must be done after END_UI 
{
    if (g_nSoundSwitchFix bitand 0x01)
    {
        ExitSoundManager();
        InitSoundManager(FalconDisplay.appWin, 0, FalconDataDirectory);
        ResetVoices();
        F4HearVoices(); // a try... sometimes voices come not up after this code is run - who knows why...?
    }

    // 2002-04-08 MN Moved here in, as InitSoundManager destroys the voice flag list -
    // need to rebuild voicemapping here
    g_voicemap.LoadVoices();

}


// helper function so we can have full pathnames if we want.
void TheaterList::SetPathName(char *dest, char *src, char *reldir)
{
    //if (src == NULL or src[0] == 0) return; // leave alone
    if (src == NULL or src[0] == 0)
        strcpy(dest, reldir);
    else if ((src[1] == ':' and isalpha(src[0])) or
             (src[0] == '\\' and src[1] == '\\') or
             (src[0] == '/')) // probably full pathname
        strcpy(dest, src);
    else
        sprintf(dest, "%s/%s", reldir, src);

    // Convert backslashes to forward slashes for Linux compatibility
    for (char *cp = dest; *cp; cp++) {
        if (*cp == '\\') *cp = '/';
    }
}

void TheaterList::SetCurrentTheater(TheaterDef *td)
{
    HKEY theKey;
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, FALCON_REGISTRY_KEY, 0, KEY_ALL_ACCESS, &theKey);

    // strlen + 1: REG_SZ includes the terminator, and FF_RegQueryValue copies
    // exactly the stored byte count without adding one. Writing strlen left the
    // reader's stack buffer unterminated -- it only ever worked because the
    // next stack byte happened to be zero.
    RegSetValueEx(theKey, "curTheater", 0, REG_SZ, (LPBYTE)td->m_name, strlen(td->m_name) + 1);

    if (getenv("FF_DEBUG_THEATER"))
    {
        fprintf(stderr, "[THEATER] SetCurrentTheater wrote '%s' as %d bytes (incl NUL)\n",
                td->m_name, (int)strlen(td->m_name) + 1);
        fflush(stderr);
    }

    RegCloseKey(theKey);
}

TheaterDef * TheaterList::GetCurrentTheater()
{
    char TheaterName[_MAX_PATH];
    HKEY theKey;
    DWORD size, type;


    RegOpenKeyEx(HKEY_LOCAL_MACHINE, FALCON_REGISTRY_KEY, 0, KEY_ALL_ACCESS, &theKey);

    size = sizeof(TheaterName);

    if (RegQueryValueEx(theKey, "curTheater", 0, &type, (LPBYTE)TheaterName, &size) not_eq ERROR_SUCCESS)
    {
        //TheaterName[0] = '\0';
        strcpy(TheaterName, "Korea");
        RegSetValueEx(theKey, "curTheater", 0, REG_SZ, (LPBYTE)TheaterName, strlen(TheaterName) + 1);
    }

    //if (strnicmp(TheaterName, "Korea", 5) == 0)
    // strcpy(TheaterName, "Korea");

    RegCloseKey(theKey);

    if (getenv("FF_DEBUG_THEATER"))
    {
        int i;
        fprintf(stderr, "[THEATER] GetCurrentTheater read size=%u bytes:", (unsigned)size);

        for (i = 0; i < (int)size and i < 16; i++)
            fprintf(stderr, " %02X", (unsigned char)TheaterName[i]);

        fprintf(stderr, "  byte[size]=%02X\n", (unsigned char)TheaterName[size]);
        {
            TheaterDef *found = FindTheaterByName(TheaterName);
            fprintf(stderr, "[THEATER] name='%s' -> matched '%s'%s\n",
                    TheaterName, found ? found->m_name : "<null>",
                    (found and stricmp(found->m_name, TheaterName) not_eq 0) ? "  <-- FELL BACK" : "");
            fflush(stderr);
            return found;
        }
    }

    return FindTheaterByName(TheaterName);
}

TheaterDef * TheaterList::FindTheaterByName(const char *name)
{
    TheaterDef *ptr;

    for (ptr = m_first; ptr; ptr = ptr->m_next)
    {
        //if (strnicmp(ptr->m_name, "Korea", 5) == 0)
        //   return ptr;
        if (stricmp(ptr->m_name, name) == 0)
        {
            return ptr;
        }
    }

    // RV - Biker - If we did not find this theater use first in list
    //return NULL;
    // Silent until now: an unrecognised name loads m_first (Korea) and the
    // player sees their theater choice quietly ignored. Callers assume a
    // non-NULL return, so the fallback stays -- but it no longer hides.
    fprintf(stderr, "[THEATER] WARNING: no theater named '%s'; falling back to '%s'\n",
            name ? name : "<null>", m_first ? m_first->m_name : "<none>");
    fflush(stderr);
    return m_first;
}

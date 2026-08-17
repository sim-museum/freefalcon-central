#ifndef _VOICE_FILTER_H_
#define _VOICE_FILTER_H_

#include "FileMemMap.h"

#define SLOT_IN_USE 1
#define VOICE_INITIALIZE -1
#define SPEAKER_INIT 0

//flags for COMM_FILE_INFO structure
enum
{
    cfiPositionData = 0x01,
};

//radio bitflags
typedef enum
{
    TOFROM_FLIGHT = 0x01,
    TO_PACKAGE = 0x02,
    TOFROM_PACKAGE = 0x04,
    TO_TEAM = 0x08,
    IN_PROXIMITY = 0x10,
    TO_WORLD = 0x20,
    TOFROM_TOWER = 0x40,
} PlayBits;

//radio channel filters
typedef enum
{
    rcfOff,
    rcfFlight1,
    rcfFlight2,
    rcfFlight3,
    rcfFlight4,
    rcfFlight5,
    rcfPackage1,
    rcfPackage2,
    rcfPackage3,
    rcfPackage4,
    rcfPackage5,
    rcfFromPackage,
    rcfProx,
    rcfTeam,
    rcfAll,
    rcfTower
} RadioChannels;

typedef enum
{
    rpLifeThreatening,
    rpGoDefensive,
    rpGoOffensive,
    rpInterFlightInfo,
    rpSectionInfo,
    rpDefault
} RadioPriorities;

enum
{
    EVAL_BY_VALUE,
    EVAL_BY_INDEX,
};

/*
 * the following 3 structures are the layouts of the Frag,
 * eval and conversation database files.
 */
typedef struct
{
    short speaker;
    short fileNbr;
} SPEAKER_TO_FILE;

/* FF_LINUX: these are FILE layouts (see the comment above), written by a 32-bit
 * Windows build where long is 4 bytes. Left as `long` they are 16 bytes here
 * instead of 8, because the 8-byte long also forces 4 bytes of padding after the
 * two shorts. That breaks the frag/eval databases twice over: GetFragInfo()
 * strides by sizeof(FRAG_FILE_INFO), so every record after index 0 is read from
 * the wrong offset, and FragFile::Initialise() computes
 * maxfrags = fragOffset / sizeof(*f1), which comes out HALF its true value.
 * That is the source of the "fragNumber < fragfile.MaxFrags()" and
 * "F4IsBadReadPtr(eEvalElem)" assertions seen in every session. */
typedef struct
{
    short fragHdrNbr;
    short totalSpeakers;
    int32_t fragOffset;
} FRAG_FILE_INFO;

typedef struct
{
    short evalHdrNbr;
    short numEvals;
    int32_t evalOffset;
} EVAL_FILE_INFO;

typedef struct
{
    short evalElem;
    short evalFrag;
} EVAL_ELEM;

/*
#pragma pack( push, pack1, 1 )
typedef struct {
 short commHdrNbr;
 short totalElements;
 short totalEvals;
 long commOffset;
} COMM_FILE_INFO;
#pragma pack( pop, pack1 )
*/

#pragma pack(1)
typedef struct
{
    short commHdrNbr;
    unsigned short warp;
    unsigned char priority;
    char positionElement;
    short bullseye;
    unsigned char totalElements;
    unsigned char totalEvals;
    int32_t commOffset;   /* FF_LINUX: file layout - 4 bytes, not sizeof(long) */
} COMM_FILE_INFO;
#pragma pack()

/* FF_LINUX: lock the on-disk sizes down so this cannot drift again. These are
 * the 32-bit Windows sizes the data files were written with. */
static_assert(sizeof(FRAG_FILE_INFO) == 8,  "FRAG_FILE_INFO must match the file layout");
static_assert(sizeof(EVAL_FILE_INFO) == 8,  "EVAL_FILE_INFO must match the file layout");
static_assert(sizeof(COMM_FILE_INFO) == 14, "COMM_FILE_INFO must match the file layout");

typedef struct
{
    int speaker;
    int voice;
    int status;
} SPEAKER_VOICE_LOOKUP;

typedef struct
{
    int mesgID;
    int timeCalled;
} MESG_REPEAT_LOOKUP;

class FragFile : public FileMemMap
{
    int maxfrags;
    int maxvoices;
public:
    FragFile() : maxfrags(-1), maxvoices(-1) {};
    void Initialise()
    {
        FRAG_FILE_INFO *f1 = GetFragInfo(0);
        maxfrags = f1->fragOffset / sizeof(*f1);
        maxvoices = f1->totalSpeakers;
    };
    int MaxFrags()
    {
        return maxfrags;
    };
    int MaxVoices()
    {
        return maxvoices;
    };
    FRAG_FILE_INFO *GetFragInfo(int fragid)
    {
        return (FRAG_FILE_INFO*)GetData(sizeof(FRAG_FILE_INFO) * fragid, sizeof(FRAG_FILE_INFO));
    };
    SPEAKER_TO_FILE *GetSpeaker(FRAG_FILE_INFO *ff)
    {
        return (SPEAKER_TO_FILE *)GetData(ff->fragOffset, sizeof(SPEAKER_TO_FILE) * ff->totalSpeakers);
    };
};

class EvalFile : public FileMemMap
{
    int maxevals;
public:
    EvalFile() : maxevals(-1) {};
    void Initialise()
    {
        EVAL_FILE_INFO *e1 = GetEval(0);
        maxevals = e1->evalOffset / sizeof(*e1);
    };
    int MaxEvals()
    {
        return maxevals;
    };
    EVAL_FILE_INFO *GetEval(int eindex)
    {
        return (EVAL_FILE_INFO*)GetData(eindex * sizeof(EVAL_FILE_INFO), sizeof(EVAL_FILE_INFO));
    };
    EVAL_ELEM *GetEvalElem(EVAL_FILE_INFO *efi)
    {
        return (EVAL_ELEM*) GetData(efi->evalOffset, sizeof(EVAL_ELEM) * efi->numEvals);
    };
};

class CommFile : public FileMemMap
{
    int maxcomms;
public:
    CommFile() : maxcomms(-1) {};
    void Initialise()
    {
        COMM_FILE_INFO *c0 = GetComm(0);
        maxcomms = c0->commOffset / sizeof * c0;
    };
    int MaxComms()
    {
        return maxcomms;
    };
    COMM_FILE_INFO *GetComm(int commid)
    {
        return (COMM_FILE_INFO*)GetData(commid * sizeof(COMM_FILE_INFO), sizeof(COMM_FILE_INFO));
    };
    int GetWarp(int commid)
    {
        COMM_FILE_INFO *cp = GetComm(commid);
        ShiAssert(cp not_eq NULL);

        if (cp) return cp->warp;

        return 0;
    };
    short *GetCommInd(COMM_FILE_INFO *cid)
    {
        return (short *)GetData(cid->commOffset, sizeof(short) * cid->totalElements);
    };
};

#ifndef BINARY_TOOL

class VoiceFilter
{
    friend void SetupNewMsg(void);
    friend void UpdateVoiceDialog(HWND hwnd);

public:
    VoiceFilter(void);
    ~VoiceFilter(void);
    void StartVoiceManager(void);
    void EndVoiceManager(void);
    void ResetVoiceManager(void);
    // void ResumeVoiceStreams( void );
    void HearVoices();
    void SilenceVoices();
    void SetUpVoiceFilter(void);
    void CleanUpVoiceFilter(void);
    void PlayRadioMessage(char talker, short msgid, short *data = NULL, VU_TIME playTime = vuxGameTime, char radiofilter = TO_TEAM, char channel = 0, VU_ID from = FalconNullId, int evalby = EVAL_BY_VALUE, VU_ID to = FalconNullId);
    char CanUserHearThisMessage(const char radiofilter, const VU_ID, const VU_ID); // Retro 21Dec2003
    int GetBullseyeComm(int *mesgID, short *data);
    int GetWarp(int mesgID);
    short IndexElement(short evalHdrNumber, short evalElement);
    short FragToFile(int talker, short fragNumber);

private:
    // MESG_REPEAT_LOOKUP *mesgTable;
    // char *commData; //, *evalData; //, *fragData;
    CommFile commfile;
    FragFile fragfile;
    EvalFile evalfile;
    void LoadCommFile(void);
    void DisposeCommData(void);
    void LoadEvalFile(void);
    void DisposeEvalData(void);
    void LoadFragFile(void);
    void DisposeFragData(void);
    short KilometersToNauticalMiles(short km);
    short FeetToAngel(int feet);
    short FeetToThousands(int feet);
    short DegreesToElement(int degree);
    short KnotsToElement(int knots);
    short KnotsToReduceIncreaseToElement(int knots);
    short EvaluateElement(short evalHdrNumber, short evalElement);
    // for the dialog tool
    friend void InitDialogData(void);
    friend double CalcCombinations(void);
    friend void IncDecMsgToPlay(int delta);
    friend void IncDecIndex(int index, int delta);
    friend void IncDecDataToPlay(int delta);
    friend void IncDecFragToPlay(int delta);
    friend void PlayRandomMessage(int channel);
    friend int PlayToolMessage(HWND hwnd);
    friend LRESULT CALLBACK PlayVoicesProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    friend int InitSoundManager(HWND hWnd, int, char *falconDataDir);
};
#endif

extern char *RadioStrings[16];

#endif

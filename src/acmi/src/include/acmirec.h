/*
** Name: ACMIREC.H
** Description:
** Recorder class for writing an ACMI recording in raw data format.
** Types of records are defined here.
** History:
** 13-oct-97 (edg)
** We go dancing in.....
*/
#ifndef _ACMIREC_H_
#define _ACMIREC_H_

#include "f4thread.h"
#include "tchar.h"
#include "acmi/src/include/acmihash.h"

#define RECORD_DIR "acmibin\\"
//#define RECORD_DIR "campaign\\save\\fltfiles\\"


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/*
** These are the enumerated record types
*/
enum
{
    ACMIRecGenPosition = 0,
    ACMIRecMissilePosition,
    ACMIRecFeaturePosition,
    ACMIRecAircraftPosition,
    ACMIRecTracerStart,
    ACMIRecStationarySfx,
    ACMIRecMovingSfx,
    ACMIRecSwitch,
    ACMIRecDOF,
    ACMIRecChaffPosition,
    ACMIRecFlarePosition,
    ACMIRecTodOffset,
    ACMIRecFeatureStatus,
    ACMICallsignList,
    ACMIRecMaxTypes
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/*
** Record structure typedefs for each type of record
*/

//
// ACMIRecHeader
// this struct is common thru all record types as a record header
//
#pragma pack (push, pack1, 1)
typedef struct
{
    BYTE type; // one of the ennumerated types
    float time; // time stamp
} ACMIRecHeader;
#pragma pack (pop, pack1)

//
// ACMIGenPositionData
// General position data
//
#pragma pack (push, pack1, 1)
typedef struct
{
    int type; // base type for creating simbase object
    long uniqueID; // identifier of instance
    float x;
    float y;
    float z;
    float yaw;
    float pitch;
    float  roll;
} ACMIGenPositionData;
#pragma pack (pop, pack1)

//
// ACMIFeaturePositionData
// General position data
//
#pragma pack (push, pack1, 1)
typedef struct
{
    int type; // base type for creating simbase object
    long uniqueID; // identifier of instance
    long leadUniqueID; // id of lead component (for bridges. bases etc)
    int slot; // slot number in component list
    int specialFlags;   // campaign feature flag
    float x;
    float y;
    float z;
    float yaw;
    float pitch;
    float  roll;
} ACMIFeaturePositionData;
#pragma pack (pop, pack1)

/*
** ACMI Text event (strings parsed from event file)
*/
#pragma pack (push, pack1, 1)
typedef struct
{
    long    intTime;
    _TCHAR timeStr[20];
    _TCHAR msgStr[100];
} ACMITextEvent;
#pragma pack (pop, pack1)

//
// ACMISwitchData
// General position data
//
#pragma pack (push, pack1, 1)
typedef struct
{
    int type; // base type for creating simbase object
    long uniqueID; // identifier of instance
    int switchNum;
    int switchVal;
    int prevSwitchVal;
} ACMISwitchData;
#pragma pack (pop, pack1)

//
// ACMIFeatureStatusData
// Feature status change data
//
#pragma pack (push, pack1, 1)
typedef struct
{
    long uniqueID; // identifier of instance
    int newStatus;
    int prevStatus;
} ACMIFeatureStatusData;
#pragma pack (pop, pack1)

//
// ACMIDOFData
// General position data
//
#pragma pack (push, pack1, 1)
typedef struct
{
    int type; // base type for creating simbase object
    long uniqueID; // identifier of instance
    int DOFNum;
    float DOFVal;
    float prevDOFVal;
} ACMIDOFData;
#pragma pack (pop, pack1)

//
// ACMITracerStartData
// Starting pos and velocity of tracer rounds
//
#pragma pack (push, pack1, 1)
typedef struct
{
    // initial values
    float x;
    float y;
    float z;
    float dx;
    float dy;
    float  dz;
} ACMITracerStartData;
#pragma pack (pop, pack1)

//
// ACMIStationarySfxData
// Starting pos of a staionay special sfx
//
#pragma pack (push, pack1, 1)
typedef struct
{
    int type; // sfx type
    float x; // position
    float y;
    float z;
    float timeToLive;
    float scale;
} ACMIStationarySfxData;
#pragma pack (pop, pack1)

//
// ACMIMovingSfxData
// Starting pos of a staionay special sfx
//
#pragma pack (push, pack1, 1)
typedef struct
{
    int type; // sfx type
    int user; // misc data
    int flags;
    float x; // position
    float y;
    float z;
    float dx; // vector
    float dy;
    float dz;
    float timeToLive;
    float scale;
} ACMIMovingSfxData;
#pragma pack (pop, pack1)

// these are the actual I/O records
#pragma pack (push, pack1, 1)

typedef struct
{
    ACMIRecHeader hdr;
    ACMIMovingSfxData data;
} ACMIMovingSfxRecord;

typedef struct
{
    ACMIRecHeader hdr;
    ACMIStationarySfxData data;
} ACMIStationarySfxRecord;

typedef struct
{
    ACMIRecHeader hdr;
    ACMIGenPositionData data;
} ACMIGenPositionRecord;

typedef struct
{
    ACMIRecHeader hdr;
    ACMIGenPositionData data;
} ACMIMissilePositionRecord;

typedef struct
{
    ACMIRecHeader hdr;
} ACMITodOffsetRecord;

typedef struct
{
    ACMIRecHeader hdr;
    ACMIGenPositionData data;
} ACMIChaffPositionRecord;

typedef struct
{
    ACMIRecHeader hdr;
    ACMIGenPositionData data;
} ACMIFlarePositionRecord;

typedef struct
{
    ACMIRecHeader hdr;
    ACMIGenPositionData data;
    long RadarTarget;

} ACMIAircraftPositionRecord;

typedef struct
{
    ACMIRecHeader hdr;
    ACMIFeaturePositionData data;
} ACMIFeaturePositionRecord;

typedef struct
{
    ACMIRecHeader hdr;
    ACMIFeatureStatusData data;
} ACMIFeatureStatusRecord;

typedef struct
{
    ACMIRecHeader hdr;
    ACMITracerStartData data;
} ACMITracerStartRecord;

typedef struct
{
    ACMIRecHeader hdr;
    ACMISwitchData data;
} ACMISwitchRecord;

typedef struct
{
    ACMIRecHeader hdr;
    ACMIDOFData data;
} ACMIDOFRecord;
#pragma pack (pop, pack1)





////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class ACMIRecorder
{
public:

    // Constructors.
    ACMIRecorder(void);

    // Destructor.
    ~ACMIRecorder();

    void StartRecording(void);
    void StopRecording(void);
    void ToggleRecording(void);

    inline BOOL IsRecording(void)
    {
        return _recording;
    };

    void TracerRecord(ACMITracerStartRecord *recp);
    void GenPositionRecord(ACMIGenPositionRecord *recp);
    void AircraftPositionRecord(ACMIAircraftPositionRecord *recp);
    void MissilePositionRecord(ACMIMissilePositionRecord *recp);
    void ChaffPositionRecord(ACMIChaffPositionRecord *recp);
    void FlarePositionRecord(ACMIFlarePositionRecord *recp);
    void FeaturePositionRecord(ACMIFeaturePositionRecord *recp);
    void StationarySfxRecord(ACMIStationarySfxRecord *recp);
    void MovingSfxRecord(ACMIMovingSfxRecord *recp);
    void SwitchRecord(ACMISwitchRecord *recp);
    void DOFRecord(ACMIDOFRecord *recp);
    void TodOffsetRecord(ACMITodOffsetRecord *recp);
    void FeatureStatusRecord(ACMIFeatureStatusRecord *recp);

    int  PercentTapeFull(void);


private:
    FILE  *_fd;

    // we need synchronization for writes
    F4CSECTIONHANDLE* _csect;

    BOOL _recording;

    float _bytesWritten;
    float _maxBytesToWrite;

};

#include <cstdint>

#pragma pack (1)
struct ACMI_CallRec
{
    char label[16];
    // ACMI-1 (2026-08-09): this record is read verbatim out of the .vhs, which is
    // written by a 32-bit Windows build where `long` is 4 bytes. Declaring it
    // `long` made the record 24 bytes here instead of 20, so the memcpy stride was
    // wrong and every callsign after the first was garbage. Same class as the
    // acmitape.h sweep -- this one lives in a different header and was missed.
    int32_t teamColor;
};

#pragma pack()

static_assert(sizeof(ACMI_CallRec) == 20, "ACMI_CallRec must stay 32-bit-Windows sized");

extern ACMI_CallRec *ACMI_Callsigns;
// ACMI-1: how many entries ACMI_Callsigns actually has. Without this the load
// path indexed the array by entity uniqueID (which runs to 477 on real tapes)
// with no bound at all.
extern int32_t ACMI_NumCallsigns;
extern ACMIRecorder gACMIRec;
extern ACMI_Hash    *ACMIIDTable;

#endif  // _ACMIREC_H_


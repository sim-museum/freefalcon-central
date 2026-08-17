/* ------------------------------------------------------------------------

  LHSP.cpp

 Lernout bitand Hauspie Speech compression

   Version 1.02

 Written by Jim DiZoglio (x257)       (c) 1997 Microprose
 Rewritten by Dave Power

------------------------------------------------------------------------ */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "fsound.h"
#include "FalcVoice.h"
#include "debuggr.h"
#include "F4Find.h"
#pragma pack(1)
//#include "landh/include/st80.h"
#pragma pack()
#include "LHSP.h"

void *map_file(char *filename);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

LHSP::LHSP(void)
{
    // FF_LINUX: these were never initialised. The only assignments to PMSIZE and
    // CODESIZE are the commented-out CodecInfoExStruct lines below, because the
    // ST80 codec is stubbed out on this port -- so ReadLHSPFile() ran its decode
    // loop on garbage:
    //
    //   * outputCodedSize = PMSIZE (garbage) was accumulated into the RETURN
    //     value, which the caller stores as dataInWaveBuffer. AddNoise() then
    //     walks that many bytes over an 80960-byte buffer and WRITES to them
    //     (*pos = level) -- heap corruption whose damage lands in whatever
    //     allocation happens to sit after the voice buffer;
    //   * a garbage CODESIZE <= 0 makes the loop subtract 0 from loopCount
    //     forever -- a hang, not just corruption.
    //
    // PMSIZE = 0 is the honest value while the codec is stubbed: no samples are
    // produced, so no samples are reported.
    hAccess = NULL;
    PMSIZE = 0;
    CODESIZE = MAXCODESIZE;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

LHSP::~LHSP(void)
{
    CleanupLHSP();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void LHSP::InitializeLHSP(void)
{
    //CODECINFOEX CodecInfoExStruct;
    //
    //ST80_GetCodecInfoEx( &CodecInfoExStruct, sizeof( CODECINFOEX ) );
    //PMSIZE = CodecInfoExStruct.wInputBufferSize;
    //CODESIZE = CodecInfoExStruct.wCodedBufferSize;
    //
    //// lpInputUncoded = new unsigned char[ MAXCODESIZE ];
    ////lpInputUncoded = new unsigned char[ MAX_INDECODE_SIZE ];
    //
    //if ( ( hAccess = ST80_Open_Decoder( LINEAR_PCM_16_BIT ) ) == NULL )
    //{
    // return;
    //}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

long LHSP::ReadLHSPFile(COMPRESSION_DATA *input, unsigned char **buffer)
{
    unsigned char *outputPtr;
    //unsigned long errorCode;
    long outputCodedSize;
    long loopCount, decodeSize, compDecodeSize = 0;

    if (input->bytesRead >= input->compFileLength)
        return 0;

    loopCount = input->compFileLength - input->bytesRead;

    if (loopCount > MAX_INDECODE_SIZE)
    {
        loopCount = MAX_INDECODE_SIZE;
    }

    outputCodedSize = PMSIZE;
    outputPtr = *buffer;

    // FF_LINUX: never let the loop stall (decodeSize <= 0 would spin forever) and
    // never report more output than the caller's buffer can hold.
    if (CODESIZE <= 0)
        CODESIZE = MAXCODESIZE;

    if (outputCodedSize < 0)
        outputCodedSize = 0;

    while (loopCount > 0)
    {
        decodeSize = loopCount;

        if (decodeSize > CODESIZE)
            decodeSize = CODESIZE;

        if (compDecodeSize + outputCodedSize > MAX_OUTDECODE_SIZE)
            break;

        /* I must check if Decode adjusts the output buffer size to use for channel struct */
        //errorCode = ST80_Decode
        //(
        // hAccess,
        // (unsigned char *)input->dataPtr,
        // ( LPWORD )&decodeSize,
        // outputPtr,
        // ( LPWORD )&outputCodedSize
        //);

        /* if ( errorCode == LH_EBADARG )
         {
         return 0;
         }
         if ( errorCode == LH_BADHANDLE )
         {
         return 0;
         }*/

        input->dataPtr += decodeSize;
        outputPtr += outputCodedSize;
        loopCount -= decodeSize;
        input->bytesRead += decodeSize;
        compDecodeSize += outputCodedSize;
    }

    return(compDecodeSize);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void LHSP::CleanupLHSP(void)
{
    //delete lpInputUncoded;

    //ST80_Close_Decoder( hAccess );
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/*
void LHSP::VoiceClose( FILE *falconVoiceFile )
{
 //if (falconVoiceFile)
 // fclose( falconVoiceFile );
}*/

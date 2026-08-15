#include "MsgInc/SendChatMessage.h"
#include "mesg.h"
#include "falclib.h"
#include "falcmesg.h"
#include "falcgame.h"
#include "falcsess.h"
#include "InvalidBufferException.h"


void ReceiveChatString(VU_ID from, _TCHAR *message); // from ui\src\ui_comms.cpp

UI_SendChatMessage::UI_SendChatMessage(VU_ID entityId, VuTargetEntity *target, VU_BOOL loopback) : FalconEvent(SendChatMessage, FalconEvent::SimThread, entityId, target, loopback)
{
    dataBlock.size = 0; dataBlock.message = NULL;  // FF_LINUX: don't let the dtor read uninitialized size / free a garbage pointer
    RequestOutOfBandTransmit();
    RequestReliableTransmit();
}

UI_SendChatMessage::UI_SendChatMessage(VU_MSG_TYPE type, VU_ID senderid, VU_ID target) : FalconEvent(SendChatMessage, FalconEvent::SimThread, senderid, target)
{
    dataBlock.size = 0; dataBlock.message = NULL;  // FF_LINUX: don't let the dtor read uninitialized size / free a garbage pointer
    RequestOutOfBandTransmit();
    RequestReliableTransmit();
    // Your Code Goes Here
    type;
}

UI_SendChatMessage::~UI_SendChatMessage(void)
{
    // FF_LINUX: dataBlock.message is void*, so `delete []` on it is ill-formed and
    // -fpermissive silently degrades it to a scalar operator delete -- an
    // alloc-dealloc mismatch, since every assignment site uses new char[]/new
    // VU_BYTE[]/new _TCHAR[]. Cast to a byte pointer so the array form is emitted.
    if (dataBlock.size > 0)
        delete [] static_cast<VU_BYTE *>(dataBlock.message);
}

int UI_SendChatMessage::Decode(VU_BYTE **buf, long *rem)
{
    long init = *rem;

    FalconEvent::Decode(buf, rem);
    memcpychk(&dataBlock.from, buf, sizeof(VU_ID), rem);
    memcpychk(&dataBlock.size, buf, sizeof(short), rem);
    ShiAssert(dataBlock.size > 0);
    dataBlock.message = new VU_BYTE[dataBlock.size];
    memcpychk(dataBlock.message, buf, dataBlock.size, rem);
    return init - *rem;
}

int UI_SendChatMessage::Encode(VU_BYTE **buf)
{
    int size;

    ShiAssert(dataBlock.size > 0);
    size = FalconEvent::Encode(buf);
    memcpy(*buf, &dataBlock.from, sizeof(VU_ID));
    *buf += sizeof(VU_ID);
    size += sizeof(VU_ID);
    memcpy(*buf, &dataBlock.size, sizeof(short));
    *buf += sizeof(short);
    size += sizeof(short);
    memcpy(*buf, dataBlock.message, dataBlock.size);
    *buf += dataBlock.size;
    size += dataBlock.size;
    return size;
}


int UI_SendChatMessage::Process(uchar autodisp)
{
    // Abort if this is autodispatching (the UI is probably shutdown)
    if (autodisp)
        return 0;

    ReceiveChatString(dataBlock.from, (_TCHAR *)dataBlock.message);
    return 0;
}

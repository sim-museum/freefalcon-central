// Sfr: vu address part which needs commapi
#include "vutypes.h"
#include "InvalidBufferException.h"
#include "comms/capi.h"

///////////////
// VU_ADDRESS //
///////////////
bool VU_ADDRESS::IsPrivate() const
{
    return (com_API_private_IP(this->ip)) ? true : false;
}

void VU_ADDRESS::Decode(VU_BYTE **stream, long *rem)
{
    memcpychk(&recvPort, stream, sizeof(unsigned short), rem);
    memcpychk(&reliableRecvPort, stream, sizeof(unsigned short), rem);
    memcpychk(&ip, stream, sizeof(uint32_t), rem);  // FF_LINUX: 4 bytes
}

int VU_ADDRESS::Encode(VU_BYTE **stream)
{
    VU_BYTE *init = *stream;
    memcpy(*stream, &recvPort, sizeof(unsigned short));
    *stream += sizeof(unsigned short);
    memcpy(*stream, &reliableRecvPort, sizeof(unsigned short));
    *stream += sizeof(unsigned short);
    // FF_LINUX (VUADDR-1): 4 bytes, matching the uint32_t field AND Decode above.
    // This wrote sizeof(unsigned long) = 8 on LP64 (4 on Win32) while Decode had
    // already been corrected to sizeof(uint32_t). Two consequences, both live:
    //   - VuSessionEntity::LocalSize counts sizeof(address_) = 8, so Encode wrote
    //     4 bytes past the buffer VuCreateEvent::Encode allocated -- ASAN caught a
    //     heap-buffer-overflow WRITE in FalconSessionEntity::Save on BOTH peers of
    //     a two-peer run, during OpenSession.
    //   - encoder and decoder disagreed by 4 bytes, so every field after the
    //     address decoded misaligned.
    memcpy(*stream, &ip, sizeof(ip));
    *stream += sizeof(ip);
    // how much we wrote
    return *stream - init;
}


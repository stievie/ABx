#include "stdafx.h"
#include "NetworkMessage.h"

void NetworkMessage::AddPaddingBytes(uint32_t n)
{
    if (!CanAdd(n))
        return;

    memset((void*)&buffer_[readPos_], 0x33, n);
    size_ += n;
}

void NetworkMessage::AddBytes(const char* bytes, uint32_t size)
{
    if (!CanAdd(size) || size > 8192)
        return;

    memcpy_s(buffer_ + readPos_, NETWORKMESSAGE_MAXSIZE, bytes, size);
    readPos_ += size;
    size_ += size;
}

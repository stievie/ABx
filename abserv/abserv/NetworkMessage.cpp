#include "stdafx.h"
#include "NetworkMessage.h"
#include <string>

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

void NetworkMessage::AddString(const char* value)
{
    uint32_t len = static_cast<uint32_t>(strlen(value));
    if (!CanAdd(len + 2) || len > 8192)
        return;

    AddU16(len);
    strcpy_s((char*)(buffer_ + readPos_), NETWORKMESSAGE_MAXSIZE, value);
    readPos_ += len;
    size_ += len;
}

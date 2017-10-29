#include "stdafx.h"
#include "NetworkMessage.h"

#include "DebugNew.h"

namespace Net {

std::string NetworkMessage::GetString(uint16_t len /* = 0 */)
{
    if (len == 0)
    {
        len = Get<uint16_t>();
        if (len >= (NETWORKMESSAGE_MAXSIZE - info_.position))
            return std::string();
    }
    if (!CanRead(len))
        return std::string();

    char* v = reinterpret_cast<char*>(buffer_) + info_.position;
    info_.position += len;
    return std::string(v, len);
}

void NetworkMessage::AddPaddingBytes(uint32_t n)
{
    if (!CanAdd(n))
        return;

    memset((void*)&buffer_[info_.position], 0x33, n);
    info_.length += n;
}

void NetworkMessage::AddBytes(const char* bytes, uint32_t size)
{
    if (!CanAdd(size) || size > 8192)
        return;

    memcpy_s(buffer_ + info_.position, NETWORKMESSAGE_MAXSIZE, bytes, size);
    info_.position += size;
    info_.length += size;
}

void NetworkMessage::AddString(const char* value)
{
    uint32_t len = static_cast<uint32_t>(strlen(value));
    if (!CanAdd(len + 2) || len > 8192)
        return;

    Add<uint16_t>(len);
    memcpy_s(buffer_ + info_.position, NETWORKMESSAGE_MAXSIZE, value, len);
    info_.position += len;
    info_.length += len;
}

}

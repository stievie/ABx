#include "stdafx.h"
#include "InputMessage.h"
#include "Utils.h"
#include <abcrypto.hpp>
#include <AB/ProtocolCodes.h>

#include "DebugNew.h"

namespace Client {

InputMessage::InputMessage()
{
    Reset();
}

bool InputMessage::ReadChecksum()
{
    uint32_t receivedCheck = Get<uint32_t>();
    int size = GetUnreadSize();
    uint32_t checksum = AdlerChecksum(buffer_ + pos_, size);
    return receivedCheck == checksum;
}

void InputMessage::SetBuffer(const std::string& buffer)
{
    size_t len = buffer.size();
    CheckWrite(len);
    memcpy_s(buffer_ + MaxHeaderSize, MaxBufferSize, buffer.c_str(), len);
    pos_ = MaxHeaderSize;
    headerPos_ = MaxHeaderSize;
    size_ = (uint16_t)len;
}

std::string InputMessage::GetString()
{
    uint16_t len = Get<uint16_t>();
    CheckRead(len);
    char* v = (char*)(buffer_ + pos_);
    pos_ += len;
    return std::string(v, len);
}

std::string InputMessage::GetStringEncrypted()
{
    std::string encString = GetString();
    if (encString.empty())
        return encString;

    size_t len = encString.length();
    char* buff = new char[len + 1];
    memset(buff, 0, len + 1);
    memcpy_s((char*)(buff), len, encString.data(), len);
    uint32_t* buffer = (uint32_t*)(buff);
    xxtea_dec(buffer, (uint32_t)(len / 4), AB::ENC_KEY);
    std::string result(buff);
    delete[] buff;
    return result;
}

void InputMessage::CheckWrite(size_t size)
{
    if (size > MaxBufferSize)
        throw std::exception("InputMessage max buffer size reached");
}

void InputMessage::CheckRead(size_t size)
{
    if (!CanRead(size))
        throw std::exception("InputMessage max buffer size reached");
}

bool InputMessage::CanRead(size_t size)
{
    if ((pos_ - headerPos_ + size > size_) || (pos_ + size > MaxBufferSize))
        return false;
    return true;
}

void InputMessage::FillBuffer(uint8_t* buffer, uint16_t size)
{
    CheckWrite(pos_ + size);
    memcpy_s(buffer_ + pos_, MaxBufferSize - pos_, buffer, size);
    size_ += size;
}

}

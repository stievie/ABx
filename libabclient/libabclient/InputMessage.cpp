#include "stdafx.h"
#include "InputMessage.h"
#include "Utils.h"
#include <abcrypto.hpp>
#include <AB/ProtocolCodes.h>
#include <lz4.h>

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
#ifdef _MSC_VER
    memcpy_s(buffer_ + MaxHeaderSize, MaxBufferSize, buffer.c_str(), len);
#else
    memcpy(buffer_ + MaxHeaderSize, buffer.c_str(), len);
#endif // _MSC_VER
    pos_ = MaxHeaderSize;
    headerPos_ = MaxHeaderSize;
    size_ = static_cast<uint16_t>(len);
}

std::string InputMessage::GetString()
{
    uint16_t len = Get<uint16_t>();
    CheckRead(len);
    const char* v = reinterpret_cast<const char*>(buffer_ + pos_);
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
#ifdef _MSC_VER
    memcpy_s((char*)(buff), len, encString.data(), len);
#else
    memcpy((char*)(buff), encString.data(), len);
#endif
    uint32_t* buffer = (uint32_t*)(buff);
    xxtea_dec(buffer, (uint32_t)(len / 4), AB::ENC_KEY);
    std::string result(buff);
    delete[] buff;
    return result;
}

void InputMessage::CheckWrite(size_t size)
{
    if (size > MaxBufferSize)
        throw std::runtime_error("InputMessage max buffer size reached");
}

void InputMessage::CheckRead(size_t size)
{
    if (!CanRead(size))
        throw std::runtime_error("InputMessage max buffer size reached");
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
#ifdef _MSC_VER
    memcpy_s(buffer_ + pos_, MaxBufferSize - pos_, buffer, size);
#else
    memcpy(buffer_ + pos_, buffer, size);
#endif
    size_ += size;
}

bool InputMessage::Uncompress()
{
    char buff[MaxBufferSize];
    const char* src = reinterpret_cast<const char*>(buffer_ + MaxHeaderSize);
    int size = LZ4_decompress_safe(src, buff, size_, MaxBufferSize);
    if (size > 0)
    {
        /*
#ifdef _MSC_VER
        memcpy_s(buffer_ + MaxHeaderSize, MaxBufferSize, buff, size);
#else
        memcpy(buffer_ + MaxHeaderSize, buff, size);
#endif
        size_ = static_cast<uint16_t>(size);
        pos_ = MaxHeaderSize;
        headerPos_ = MaxHeaderSize;
        */
        return true;
    }
    return true;
}

}

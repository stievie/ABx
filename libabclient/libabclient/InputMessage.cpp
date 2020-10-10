/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "InputMessage.h"
#include "Utils.h"
#include <abcrypto.hpp>
#include <AB/ProtocolCodes.h>
#include <sa/Assert.h>

namespace Client {

InputMessage::InputMessage()
{
    Reset();
}

bool InputMessage::ReadChecksum()
{
    uint32_t receivedCheck = Get<uint32_t>();
    size_t size = GetUnreadSize();
    uint32_t checksum = AdlerChecksum(buffer_ + pos_, static_cast<int>(size));
    return receivedCheck == checksum;
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
    memcpy_s(buff, len, encString.data(), len);
#else
    memcpy(buff, encString.data(), len);
#endif
    uint32_t* buffer = reinterpret_cast<uint32_t*>(buff);
    xxtea_dec(buffer, static_cast<uint32_t>(len / 4), AB::ENC_KEY);
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

void InputMessage::FillBuffer(uint8_t* buffer, size_t size)
{
    CheckWrite(static_cast<size_t>(pos_) + static_cast<size_t>(size));
#ifdef _MSC_VER
    memcpy_s(buffer_ + pos_, MaxBufferSize - pos_, buffer, size);
#else
    memcpy(buffer_ + pos_, buffer, size);
#endif
    size_ += size;
}

}

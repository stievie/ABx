/**
 * Copyright 2020 Stefan Ascher
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

#pragma once

#include <AB/ProtocolCodes.h>
#include <sa/Compiler.h>

namespace Net {

class MessageDecoder
{
private:
    const uint8_t* ptr_;
    size_t size_;
    size_t pos_;
    bool CanRead(size_t size) const
    {
        return pos_ + size <= size_;
    }
    std::string GetString();
public:
    MessageDecoder(const uint8_t* ptr, size_t size);
    template <typename T>
    T Get()
    {
        if (!CanRead(sizeof(T)))
            return { };
        T v;
#ifdef SA_MSVC
        memcpy_s(&v, sizeof(T), ptr_ + pos_, sizeof(T));
#else
        memcpy(&v, ptr_ + pos_, sizeof(T));
#endif
        pos_ += sizeof(T);
        return v;
    }

    AB::GameProtocol::ServerPacketType GetNext();
};

template <>
inline std::string MessageDecoder::Get<std::string>()
{
    return GetString();
}
template <>
inline bool MessageDecoder::Get<bool>()
{
    return Get<uint8_t>() != 0;
}

}

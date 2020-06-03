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

#include "MessageDecoder.h"
#include <abscommon/NetworkMessage.h>

namespace Net {

MessageDecoder::MessageDecoder(const uint8_t* ptr, size_t size) :
    ptr_(ptr),
    size_(size),
    pos_(0)
{ }

AB::GameProtocol::ServerPacketType MessageDecoder::GetNext()
{
    if (!CanRead(sizeof(AB::GameProtocol::ServerPacketType)))
        return AB::GameProtocol::ServerPacketType::__Last;
    return Get< AB::GameProtocol::ServerPacketType>();
}

std::string MessageDecoder::GetString()
{
    size_t len = Get<uint16_t>();
    if (len >= (NetworkMessage::NETWORKMESSAGE_BUFFER_SIZE - pos_))
        return std::string();
    if (!CanRead(len))
        return std::string();

    const char* v = reinterpret_cast<const char*>(ptr_) + pos_;
    pos_ += len;
    return std::string(v, len);
}

}

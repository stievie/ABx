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

namespace Net {

MessageDecoder::MessageDecoder(const NetworkMessage& message) :
    ptr_(message.GetBuffer()),
    size_(message.GetSize() + NetworkMessage::INITIAL_BUFFER_POSITION),
    pos_(NetworkMessage::INITIAL_BUFFER_POSITION)
{ }

std::optional<AB::GameProtocol::ServerPacketType> MessageDecoder::GetNext()
{
    if (!CanRead(1))
        return {};
    return static_cast<AB::GameProtocol::ServerPacketType>(ptr_[pos_++]);
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

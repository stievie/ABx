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

#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <queue>
#include "PropStream.h"

namespace Net {

enum class MessageType : uint8_t
{
    Unknown = 0,
    /// Send the server the ID of this server. Body contains the UUID as string.
    ServerJoined = 1,
    ServerLeft,
    /// Shutdown server. Body contains UUID of server to shutdown.
    Shutdown,
    /// Tell a game server to spawn a new instance. Body contains UUID of the server which should spawn a new server.
    Spawn,
    ClearCache,

    GuildChat,
    TradeChat,
    Whipser,
    NewMail,
    PlayerChanged,

    // Queues
    QueueAdd,
    QueueRemove,
    PlayerAddedToQueue,
    PlayerRemovedFromQueue,
    TeamsEnterMatch,
    CreateGameInstance,

    Last
};

/// Message server message
class MessageMsg
{
public:
    // BodyLength (2B), MessageType (1B), aligned to 4B so last byte is 0
    enum { HeaderLength = 4 };
    enum { MaxBodyLength = 512 };
private:
    static constexpr size_t BufferSize = HeaderLength + MaxBodyLength;
    size_t bodyLength_;
    uint8_t data_[BufferSize];
public:
    MessageMsg() :
        bodyLength_(0),
        data_{},
        type_(MessageType::Unknown)
    { }
    MessageMsg(const MessageMsg& other) :
        bodyLength_(other.bodyLength_),
        type_(other.type_)
    {
        std::copy(other.data_, other.data_ + HeaderLength + other.bodyLength_, data_);
    }
    ~MessageMsg() = default;

    MessageType type_;
    const uint8_t* Data() const
    {
        return data_;
    }
    uint8_t* Data()
    {
        return data_;
    }
    size_t Length()
    {
        return HeaderLength + bodyLength_;
    }
    bool IsEmpty() const
    {
        return bodyLength_ == 0;
    }
    void Empty()
    {
        bodyLength_ = 0;
        type_ = MessageType::Unknown;
    }
    const uint8_t* Body() const
    {
        return data_ + HeaderLength;
    }
    uint8_t* Body()
    {
        return data_ + HeaderLength;
    }
    size_t BodyLength() const
    {
        return bodyLength_;
    }
    std::string GetBodyString() const
    {
        return std::string(reinterpret_cast<char const*>(Body()), BodyLength());
    }
    void SetBodyString(const std::string& str)
    {
        SetBodyLength(str.length());
        memcpy(Body(), str.data(), bodyLength_);
        EncodeHeader();
    }
    void SetBodyLength(size_t newLength)
    {
        bodyLength_ = newLength;
        if (bodyLength_ > MaxBodyLength)
            bodyLength_ = MaxBodyLength;
    }
    bool SetPropStream(const IO::PropWriteStream& stream)
    {
        size_t size = 0;
        const char* data = stream.GetStream(size);
        SetBodyLength(size);
        if (bodyLength_ != size)
            return false;
        memcpy(Body(), data, bodyLength_);
        EncodeHeader();
        return true;
    }
    bool GetPropStream(IO::PropReadStream& stream) const
    {
        stream.Init(reinterpret_cast<char const*>(Body()), bodyLength_);
        return true;
    }
    bool DecodeHeader()
    {
        bodyLength_ = static_cast<size_t>(data_[0] | data_[1] << 8);
        type_ = static_cast<MessageType>(data_[2]);
        if (bodyLength_ > MaxBodyLength)
        {
            bodyLength_ = 0;
            return false;
        }
        if (type_ > MessageType::Last)
        {
            type_ = MessageType::Unknown;
            return false;
        }
        return true;
    }
    void EncodeHeader()
    {
        data_[0] = static_cast<uint8_t>(bodyLength_);
        data_[1] = static_cast<uint8_t>(bodyLength_ >> 8);
        data_[2] = static_cast<uint8_t>(type_);
    }
};

typedef std::deque<MessageMsg> MessageQueue;

}

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
    ServerId = 1,
    /// Shutdown server. Body contains UUID of server to shutdown.
    Shutdown,

    GuildChat,
    TradeChat,
    Whipser,
    NewMail,

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
    size_t bodyLength_;
    uint8_t data_[HeaderLength + MaxBodyLength];
public:
    MessageMsg() :
        bodyLength_(0),
        type_(MessageType::Unknown)
    { }
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
        bodyLength_ = (size_t)(data_[0] | data_[1] << 8);
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

#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <queue>

namespace Net {

enum MessageType : uint8_t
{
    MessageTypeUnknown = 0,
    MessageTypeServerId = 1,

    MessageTypeLast = MessageTypeServerId
};

/// Message server message
class MessageMsg
{
public:
    // BodyLength (2B), MessageType (1B)
    enum { HeaderLength = 4 };
    enum { MaxBodyLength = 512 };
private:
    size_t bodyLength_;
    uint8_t data_[HeaderLength + MaxBodyLength];
public:
    MessageMsg() :
        bodyLength_(0)
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
    bool DecodeHeader()
    {
        bodyLength_ = (size_t)(data_[0] | data_[1] << 8);
        type_ = static_cast<MessageType>(data_[2]);
        if (bodyLength_ > MaxBodyLength)
        {
            bodyLength_ = 0;
            return false;
        }
        if (type_ > MessageTypeLast)
        {
            type_ = MessageTypeUnknown;
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

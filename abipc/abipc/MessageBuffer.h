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

#include <deque>
#include <string>

namespace IPC {

class MessageBuffer
{
public:
    enum { HeaderLength = 2 + sizeof(size_t) };
    enum { MaxBodyLength = 512 };
private:
    static constexpr size_t BufferSize = HeaderLength + MaxBodyLength;
    size_t bodyLength_;
    uint8_t data_[BufferSize];
    size_t pos_{ 0 };
    bool CanRead(size_t size) const
    {
        return pos_ + size < bodyLength_;
    }
    bool CanWrite(size_t size) const
    {
        return pos_ + size < MaxBodyLength;
    }
    void AddString(const std::string& value);
    std::string GetString();
public:
    MessageBuffer() :
        bodyLength_(0),
        type_(0),
        data_{}
    { }
    size_t Length() const
    {
        return HeaderLength + bodyLength_;
    }
    const uint8_t* Data() const
    {
        return data_;
    }
    uint8_t* Data()
    {
        return data_;
    }
    const uint8_t* Body() const
    {
        return data_ + HeaderLength;
    }
    uint8_t* Body()
    {
        return data_ + HeaderLength;
    }
    size_t BodyLength() const { return bodyLength_; }
    bool DecodeHeader();
    void EncodeHeader();
    bool IsEmpty() const
    {
        return bodyLength_ == 0;
    }
    void Empty()
    {
        bodyLength_ = 0;
        type_ = 0;
    }

    template <typename T>
    T Get()
    {
        if (!CanRead(sizeof(T)))
            return 0;
        T v;
#ifdef _MSC_VER
        memcpy_s(&v, sizeof(T), data_ + pos_, sizeof(T));
#else
        memcpy(&v, data_ + pos_, sizeof(T));
#endif
        pos_ += sizeof(T);
        return v;
    }
    template <typename T>
    void Add(const T& value)
    {
        if (!CanWrite(sizeof(T)))
            return;

#ifdef _MSC_VER
        memcpy_s(data_ + pos_, sizeof(T), &value, sizeof(T));
#else
        memcpy(data_ + pos_, &value, sizeof(T));
#endif
        pos_ += sizeof(T);
        bodyLength_ += sizeof(T);
    }

    size_t type_;
};

template <>
inline void MessageBuffer::Add<std::string>(const std::string& value)
{
    AddString(value);
}
template <>
inline void MessageBuffer::Add<bool>(const bool& value)
{
    Add<uint8_t>(value ? 1 : 0);
}
template <>
inline std::string MessageBuffer::Get<std::string>()
{
    return GetString();
}
template <>
inline bool MessageBuffer::Get<bool>()
{
    return Get<uint8_t>() != 0;
}

typedef std::deque<MessageBuffer> BufferQueue;

}

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

#include <string>
#include <stdint.h>
#include <sa/Assert.h>

namespace Client {

class Protocol;

/// Message read from network
class InputMessage
{
public:
    enum {
        MaxBufferSize = 65536,
        MaxHeaderSize = 8
    };
private:
    uint8_t buffer_[MaxBufferSize];
    uint16_t size_;
    uint16_t pos_;
    uint16_t headerPos_;

    void CheckWrite(size_t size);
    void CheckRead(size_t size);
    bool CanRead(size_t size);
protected:
    friend class Protocol;

    void Reset()
    {
        size_ = 0;
        pos_ = MaxHeaderSize;
        headerPos_ = MaxHeaderSize;
    }
    void SetHeaderSize(uint16_t size)
    {
        ASSERT(MaxHeaderSize - size >= 0);
        headerPos_ = MaxHeaderSize - size;
        pos_ = headerPos_;
    }
    void FillBuffer(uint8_t *buffer, uint16_t size);
    void SetMessageSize(uint16_t size) { size_ = size; }
    std::string GetString();
    std::string GetStringEncrypted();
public:
    InputMessage();

    uint16_t ReadSize() { return Get<uint16_t>(); }
    bool ReadChecksum();

    size_t GetUnreadSize() const { return size_ - (pos_ - headerPos_); }
    size_t GetMessageSize() const { return size_; }

    uint8_t* GetReadBuffer() { return buffer_ + pos_; }
    uint8_t* GetHeaderBuffer() { return buffer_ + headerPos_; }
    uint8_t* GetDataBuffer() { return buffer_ + MaxHeaderSize; }
    size_t GetHeaderSize() const { return (MaxHeaderSize - headerPos_); }
    bool Eof() const { return (pos_ - headerPos_) >= size_; }

    void SetBuffer(const std::string& buffer);

    template <typename T>
    T Get()
    {
        CheckRead(sizeof(T));
        unsigned p = pos_;
        pos_ += sizeof(T);
        return *reinterpret_cast<T*>(buffer_ + p);
    }
    template <typename T>
    T GetDecrypted()
    {
        // Only strings
        return Get<T>();
    }
};

template<>
inline std::string InputMessage::Get<std::string>()
{
    return GetString();
}
template<>
inline std::string InputMessage::GetDecrypted<std::string>()
{
    return GetStringEncrypted();
}
template<>
inline bool InputMessage::Get<bool>()
{
    return Get<uint8_t>() != 0;
}

}

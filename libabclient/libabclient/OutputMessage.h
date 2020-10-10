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

#include <limits>
#include <stdint.h>
#include <string>

namespace Client {

class Protocol;

// Make sizeof(OutputMessage) == 1KB
static constexpr uint16_t OUTPUTMESSAGE_HEADER_SIZE = 8;

struct OutputMessageInfo
{
    uint16_t headerPos = OUTPUTMESSAGE_HEADER_SIZE;
    uint16_t pos = OUTPUTMESSAGE_HEADER_SIZE;
    uint16_t size = 0;
};

static constexpr size_t OUTPUTMESSAGE_BUFFER_SIZE = 4096 - sizeof(OutputMessageInfo);
static constexpr size_t OUTPUTMESSAGE_MAX_STRING_LEN = OUTPUTMESSAGE_BUFFER_SIZE - OUTPUTMESSAGE_HEADER_SIZE - sizeof(uint16_t);

/// Message to write to the network
class OutputMessage
{
    friend class Protocol;
private:
    OutputMessageInfo info_;
    uint8_t buffer_[OUTPUTMESSAGE_BUFFER_SIZE];
    bool CanWrite(int bytes);
    void CheckWrite(int bytes);
    void AddString(const std::string& value);
    void AddStringEncrypted(const std::string& value);
protected:
    void WriteChecksum();
    void WriteMessageSize();
    uint8_t* GetWriteBuffer() { return buffer_ + info_.pos; }
    uint8_t* GetHeaderBuffer() { return buffer_ + info_.headerPos; }
    uint8_t* GetDataBuffer() { return buffer_ + OUTPUTMESSAGE_HEADER_SIZE; }
public:
    OutputMessage();

    size_t GetSize() const { return info_.size; }
    uint16_t GetPos() const { return info_.pos; }
    void AddPaddingBytes(int bytes, uint8_t byte = 0);
    template <typename T>
    void Add(const T& value)
    {
        if (!CanWrite(sizeof(T)))
            return;
        *reinterpret_cast<T*>(buffer_ + info_.pos) = value;
        info_.pos += sizeof(T);
        info_.size += sizeof(T);
    }
    template <typename T>
    void AddEncryted(const T& value)
    {
        // Only strings
        Add<T>(value);
    }
    template <typename T>
    void Set(uint16_t pos, T value)
    {
        uint16_t p = info_.pos;
        info_.pos = pos;
        if (!CanWrite(sizeof(T)))
            return;
        *reinterpret_cast<T*>(buffer_ + info_.pos) = value;
        info_.pos = p;
    }
};

template<>
inline void OutputMessage::Add<bool>(const bool& value)
{
    Add<uint8_t>(value ? 1 : 0);
}
template<>
inline void OutputMessage::Add<std::string>(const std::string& value)
{
    AddString(value);
}
template<>
inline void OutputMessage::AddEncryted<std::string>(const std::string& value)
{
    AddStringEncrypted(value);
}

}

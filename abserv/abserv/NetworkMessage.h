#pragma once

#include <stdint.h>
#include <string>

#define NETWORKMESSAGE_MAXSIZE 15340

namespace Net {

class NetworkMessage
{
public:
    enum { HeaderLength = 2 };
    enum { CryptoLength = 4 };
    enum { XteaMultiple = 8 };
    enum { MaxBodyLength = NETWORKMESSAGE_MAXSIZE - HeaderLength - CryptoLength - XteaMultiple };
private:
    int32_t readPos_;
protected:
    int32_t size_;
    uint8_t buffer_[NETWORKMESSAGE_MAXSIZE];
    void Reset()
    {
        size_ = 0;
        readPos_ = 0;
    }
    bool CanAdd(uint32_t size)
    {
        return (size + readPos_ < MaxBodyLength);
    }
public:
    NetworkMessage()
    {
        Reset();
    }
    ~NetworkMessage()
    {}

    int32_t GetMessageLength() const { return size_; }

    /// Read functions
    uint8_t GetByte() { return buffer_[readPos_++]; }
    std::string GetString();
    uint16_t GetU16()
    {
        uint16_t v = *(uint32_t*)(buffer_ + readPos_);
        readPos_ += 2;
        return v;
    }
    uint32_t GetU32()
    {
        uint32_t v = *(uint32_t*)(buffer_ + readPos_);
        readPos_ += 4;
        return v;
    }
    uint32_t PeekU32()
    {
        uint32_t v = *(uint32_t*)(buffer_ + readPos_);
        return v;
    }

    /// Add function
    void AddPaddingBytes(uint32_t n);
    void AddBytes(const char* bytes, uint32_t size);
    void AddByte(uint8_t value)
    {
        if (!CanAdd(1))
            return;
        buffer_[readPos_++] = value;
        size_++;
    }
    void AddString(const std::string& value)
    {
        AddString(value.c_str());
    }
    void AddString(const char* value);
    void AddU16(uint16_t value)
    {
        if (!CanAdd(sizeof(value)))
            return;
        *(uint16_t*)(buffer_ + readPos_) = value;
        readPos_ += 2;
        size_ += 2;
    }

    int32_t DecodeHeader()
    {
        size_ = (int32_t)(buffer_[0] | buffer_[1] << 8);
        return size_;
    }
    /// Other function
    void Skip(int bytes)
    {
        readPos_ += bytes;
    }

    char* GetBuffer() { return (char*)&buffer_[0]; }
    char* GetBodyBuffer()
    {
        readPos_ = 2;
        return (char*)&buffer_[HeaderLength];
    }
    int32_t GetSize() const { return size_; }
    void SetSize(int32_t size) { size_ = size; }
    int32_t GetReadPos() const { return readPos_; }
};

}

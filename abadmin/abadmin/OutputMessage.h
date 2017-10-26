#pragma once

#include <stdint.h>

class Protocol;

class OutputMessage
{
public:
    enum {
        MaxBufferSize = 65536,
        MaxStringLength = 65536,
        MaxHeaderSize = 8
    };
private:
    uint16_t headerPos_;
    uint16_t pos_;
    uint16_t size_;
    uint8_t buffer_[MaxBufferSize];
    bool CanWrite(int bytes);
    void CheckWrite(int bytes);
protected:
    friend class Protocol;

    void Reset()
    {
        pos_ = (uint16_t)MaxHeaderSize;
        headerPos_ = (uint16_t)MaxBufferSize;
        size_ = 0;
    }
    void WriteChecksum();
    void WriteMessageSize();
    uint8_t* GetWriteBuffer() { return buffer_ + pos_; }
    uint8_t* GetHeaderBuffer() { return buffer_ + headerPos_; }
    uint8_t* GetDataBuffer() { return buffer_ + MaxHeaderSize; }

public:
    OutputMessage();

    uint16_t GetSize() const { return size_; }
    uint16_t GetPos() const { return pos_; }
    void AddPaddingBytes(int bytes, uint8_t byte = 0);
    template <typename T>
    void Add(T value)
    {
        if (!CanWrite(sizeof(T)))
            return;
        *(T*)(buffer_ + pos_) = value;
        pos_ += sizeof(T);
        size_ += sizeof(T);
    }
    template <typename T>
    void Set(uint16_t pos, T value)
    {
        uint16_t p = pos_;
        pos_ = pos;
        if (!CanWrite(sizeof(T)))
            return;
        *(T*)(buffer_ + pos_) = value;
        pos_ = p;
    }
};


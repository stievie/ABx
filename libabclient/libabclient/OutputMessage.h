#pragma once

#include <limits>
#include <stdint.h>

namespace Client {

class Protocol;

static constexpr size_t BUFFER_SIZE = 4096;
static constexpr size_t MESSAGEPOOL_COUNT = 60;

/// Message to write to the network
class OutputMessage
{
public:
    enum {
        MaxHeaderSize = 8,
        MaxBufferSize = BUFFER_SIZE,
        MaxStringLength = BUFFER_SIZE - MaxHeaderSize,
    };
private:
    static std::vector<std::unique_ptr<OutputMessage>> pool_;
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
        headerPos_ = (uint16_t)MaxHeaderSize;
        size_ = 0;
    }
    void WriteChecksum();
    void WriteMessageSize();
    uint8_t* GetWriteBuffer() { return buffer_ + pos_; }
    uint8_t* GetHeaderBuffer() { return buffer_ + headerPos_; }
    uint8_t* GetDataBuffer() { return buffer_ + MaxHeaderSize; }
public:
    static std::shared_ptr<OutputMessage> New();
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

    void AddString(const std::string& value);
    void AddStringEncrypted(const std::string& value);

    bool Compress();
};

}

#pragma once

#include <stdint.h>
#include <string>

namespace Client {

class Protocol;

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
        assert(MaxHeaderSize - size >= 0);
        headerPos_ = MaxHeaderSize - size;
        pos_ = headerPos_;
    }
    void FillBuffer(uint8_t *buffer, uint16_t size);
    void SetMessageSize(uint16_t size) { size_ = size; }
public:
    InputMessage();

    int GetUnreadSize() { return size_ - (pos_ - headerPos_); }
    uint16_t ReadSize() { return Get<uint16_t>(); }
    bool ReadChecksum();

    uint16_t GetMessageSize() { return size_; }

    uint8_t* GetReadBuffer() { return buffer_ + pos_; }
    uint8_t* GetHeaderBuffer() { return buffer_ + headerPos_; }
    uint8_t* GetDataBuffer() { return buffer_ + MaxHeaderSize; }
    uint16_t GetHeaderSize() { return (MaxHeaderSize - headerPos_); }
    bool Eof() const { return (pos_ - headerPos_) >= size_; }

    void SetBuffer(const std::string& buffer);

    std::string GetString();
    template <typename T>
    T Get()
    {
        CheckRead(sizeof(T));
        T v = *(T*)(buffer_ + pos_);
        pos_ += sizeof(T);
        return v;
    }
};

}

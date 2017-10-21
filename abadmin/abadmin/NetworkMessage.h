#pragma once

#include "Definitions.h"

#define RETRY_TIME 10000

enum SocketCode
{
    SocketCodeOK,
    SocketCodeTimeout,
    SocketCodeError
};

class NetworkMessage
{
private:
    uint8_t buffer_[NETWORKMESSAGE_MAXSIZE];
    int size_;
    int readPos_;
    inline bool CanAdd(int size)
    {
        return (size + readPos_ < NETWORKMESSAGE_MAXSIZE - 16);
    }
public:
    NetworkMessage();
    ~NetworkMessage() {}

    // socket functions
    SocketCode ReadFromSocket(SOCKET socket, int timeout = RETRY_TIME);
    SocketCode WriteToSocket(SOCKET socket, int timeout = RETRY_TIME);

    uint8_t PeekByte()
    {
        return buffer_[readPos_];
    }
    uint16_t GetByte()
    {
        return buffer_[readPos_++];
    }
    template <typename T>
    T Get()
    {
        T v = *(T*)(buffer_ + readPos_);
        readPos_ += sizeof(T);
        return v;
    }

    void AddByte(uint8_t value)
    {
        if (!CanAdd(1))
            return;
        buffer_[readPos_++] = value;
        size_++;
    }
    template <typename T>
    void Add(T value)
    {
        if (!CanAdd(sizeof(T)))
            return;
        *(T*)(buffer_ + readPos_) = value;
        readPos_ += sizeof(T);
        size_ += sizeof(T);
    }

    void Reset();

    static bool encryptionEnabled;
    static bool keySet;
    static uint32_t key[4];

};


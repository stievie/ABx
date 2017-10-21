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

    void Reset();
};


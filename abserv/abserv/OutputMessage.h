#pragma once

#include <memory>
#include <stdint.h>
#include <list>
#include <mutex>
#include "NetworkMessage.h"
#include "Connection.h"
#include "Protocol.h"
#include "Utils.h"
#include "Logger.h"

#define OUTPUT_POOL_SIZE 100

namespace Net {

class OutputMessage : public NetworkMessage
{
private:
    uint32_t outputBufferStart_;

    friend class OutputMessagePool;

    template <typename T>
    inline void AddHeader(T add)
    {
        if ((int32_t)outputBufferStart_ < (int32_t)sizeof(T))
        {
            LOG_ERROR << "outputBufferStart_(" << outputBufferStart_ << ") < " <<
                sizeof(T) << std::endl;
            return;
        }
        outputBufferStart_ -= sizeof(T);
        memcpy(buffer_ + outputBufferStart_, &add, sizeof(T));
        info_.length += sizeof(T);
    }
public:
    OutputMessage() = default;
    OutputMessage(const OutputMessage&) = delete;
    ~OutputMessage() {}

    uint8_t* GetOutputBuffer() { return buffer_ + outputBufferStart_; }
    void AddCryptoHeader(bool addChecksum)
    {
        if (addChecksum)
            AddHeader<uint32_t>(Utils::AdlerChecksum((uint8_t*)(buffer_ - outputBufferStart_), info_.length));
        WriteMessageLength();
    }
    void WriteMessageLength()
    {
        AddHeader<uint16_t>(info_.length);
    }

    void Append(const NetworkMessage& msg)
    {
        int32_t msgLen = msg.GetSize();
        memcpy_s(buffer_ + info_.position, NETWORKMESSAGE_MAXSIZE, (msg.GetBuffer() + 8), msgLen);
        info_.length += msgLen;
        info_.position += msgLen;
    }
    void Append(const std::shared_ptr<OutputMessage>& msg)
    {
        int32_t msgLen = msg->GetSize();
        memcpy_s(buffer_ + info_.position, NETWORKMESSAGE_MAXSIZE, (msg->GetBuffer() + 8), msgLen);
        info_.length += msgLen;
        info_.position += msgLen;
    }
};

class OutputMessagePool
{
private:
    OutputMessagePool() = default;
public:
    OutputMessagePool(const OutputMessagePool&) = delete;
    OutputMessagePool& operator=(const OutputMessagePool&) = delete;

    static OutputMessagePool* Instance()
    {
        static OutputMessagePool instance;
        return &instance;
    }

    void SendAll();
    void ScheduleSendAll();
    static std::shared_ptr<OutputMessage> GetOutputMessage();
    void AddToAutoSend(std::shared_ptr<Protocol> protocol);
    void RemoveFromAutoSend(const std::shared_ptr<Protocol>& protocol);
private:
    //NOTE: A vector is used here because this container is mostly read
    //and relatively rarely modified (only when a client connects/disconnects)
    std::vector<std::shared_ptr<Protocol>> bufferedProtocols_;
};

}

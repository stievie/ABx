#pragma once

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
    uint32_t outputBufferStart_ = INITIAL_BUFFER_POSITION;

    friend class OutputMessagePool;

    template <typename T>
    inline void AddHeader(T add)
    {
        if (static_cast<int32_t>(outputBufferStart_) < static_cast<int32_t>(sizeof(T)))
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
        {
            uint32_t checksum = Utils::AdlerChecksum((uint8_t*)(buffer_ + outputBufferStart_), info_.length);
            AddHeader<uint32_t>(checksum);
        }
        WriteMessageLength();
    }
    void WriteMessageLength()
    {
        AddHeader<uint16_t>(info_.length);
    }

    void Append(const NetworkMessage& msg)
    {
        int32_t msgLen = msg.GetSize();
#ifdef _MSC_VER
        memcpy_s(buffer_ + info_.position, NetworkMessage::NETWORKMESSAGE_BUFFER_SIZE, (msg.GetBuffer() + 8), msgLen);
#else
        memcpy(buffer_ + info_.position, (msg.GetBuffer() + 8), msgLen);
#endif
        info_.length += static_cast<MsgSize_t>(msgLen);
        info_.position += static_cast<MsgSize_t>(msgLen);
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

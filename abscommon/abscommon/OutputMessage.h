#pragma once

#include "NetworkMessage.h"
#include "Utils.h"
#include "Logger.h"
#include <sa/PoolAllocator.h>
#include <sa/SharedPtr.h>
#include <cstring>

namespace Net {

class Protocol;

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

constexpr size_t OUTPUTMESSAGE_SIZE = sizeof(OutputMessage);
constexpr size_t OUTPUTMESSAGE_POOL_COUNT = 1024;

struct PoolWrapper
{
    using MessagePool = sa::PoolAllocator<OutputMessage, OUTPUTMESSAGE_SIZE* OUTPUTMESSAGE_POOL_COUNT, OUTPUTMESSAGE_SIZE>;
    // Must be instantiated in one single cpp file
    static MessagePool sOutputMessagePool;
};

}

namespace sa {

/*
template <>
struct DefaultDelete<::Net::OutputMessage>
{
    DefaultDelete() = default;
    void operator()(::Net::OutputMessage* p) const noexcept
    {
        Net::PoolWrapper::sOutputMessagePool.deallocate(p, 1);
    }
};

template <>
inline SharedPtr<::Net::OutputMessage> MakeShared()
{
    auto* ptr = Net::PoolWrapper::sOutputMessagePool.allocate(1, nullptr);
    assert(ptr);
    ptr->Reset();
    return sa::SharedPtr<::Net::OutputMessage>(ptr);
}
*/

}

namespace Net {

class OutputMessagePool
{
private:
    OutputMessagePool() = default;
public:
#ifdef DEBUG_POOLALLOCATOR
    static sa::PoolInfo GetPoolInfo();
#endif
    static sa::SharedPtr<OutputMessage> GetOutputMessage();

    OutputMessagePool(const OutputMessagePool&) = delete;
    OutputMessagePool& operator=(const OutputMessagePool&) = delete;

    static OutputMessagePool* Instance()
    {
        static OutputMessagePool instance;
        return &instance;
    }

    void SendAll();
    void ScheduleSendAll();
    void AddToAutoSend(std::shared_ptr<Protocol> protocol);
    void RemoveFromAutoSend(const std::shared_ptr<Protocol>& protocol);
private:
    //NOTE: A vector is used here because this container is mostly read
    //and relatively rarely modified (only when a client connects/disconnects)
    std::vector<std::shared_ptr<Protocol>> bufferedProtocols_;
};

}

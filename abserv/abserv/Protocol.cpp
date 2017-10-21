#include "stdafx.h"
#include "Protocol.h"
#include "NetworkMessage.h"
#include "OutputMessage.h"
#include "Logger.h"
#include "Scheduler.h"

#include "DebugNew.h"

namespace Net {

void Protocol::XTEAEncrypt(OutputMessage& message)
{
    uint32_t k[4];
    k[0] = xteaKey_[0]; k[1] = xteaKey_[1]; k[2] = xteaKey_[2]; k[3] = xteaKey_[3];

    int32_t msgLength = message.GetMessageLength();

    uint32_t n;
    // Add bytes until we reach a multiple of 8
    if ((msgLength % 8) != 0)
    {
        n = 8 - (msgLength % 8);
        message.AddPaddingBytes(n);
        msgLength += n;
    }

    int readPos = 0;
    uint32_t* buffer = (uint32_t*)message.GetOutputBuffer();
    while (readPos < msgLength / 4)
    {
        uint32_t v0 = buffer[readPos], v1 = buffer[readPos + 1];
        uint32_t delta = 0x61C88647;
        uint32_t sum = 0;

        for (int32_t i = 0; i < 32; ++i)
        {
            v0 += ((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + k[sum & 3]);
            sum -= delta;
            v1 += ((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + k[sum >> 11 & 3]);
        }
        buffer[readPos] = v0; buffer[readPos + 1] = v1;
        readPos += 2;
    }
}

bool Protocol::XTEADecrypt(NetworkMessage& message)
{
    if ((message.GetMessageLength() - 6) % 8 != 0)
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Invalid encrypted message size" << std::endl;
#endif
        return false;
    }

    uint32_t k[4];
    k[0] = xteaKey_[0]; k[1] = xteaKey_[1]; k[2] = xteaKey_[2]; k[3] = xteaKey_[3];

    uint32_t* buffer = (uint32_t*)(message.GetBuffer() + message.GetReadPos());
    int readPos = 0;
    int32_t messageLength = message.GetMessageLength() - 6;
    while (readPos < messageLength / 4)
    {
        uint32_t v0 = buffer[readPos], v1 = buffer[readPos + 1];
        uint32_t delta = 0x61C88647;
        uint32_t sum = 0xC6EF3720;

        for (int32_t i = 0; i < 32; ++i)
        {
            v1 -= ((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + k[sum >> 11 & 3]);
            sum += delta;
            v0 -= ((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + k[sum & 3]);
        }
        buffer[readPos] = v0; buffer[readPos + 1] = v1;
        readPos += 2;
    }

    int tmp = message.Get<uint16_t>();
    if (tmp > message.GetMessageLength() - 8)
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Invalid unencrypted message size" << std::endl;
#endif
        return false;
    }

    return true;
}

void Protocol::OnSendMessage(std::shared_ptr<OutputMessage> message)
{
#ifdef DEBUG_NET
    LOG_DEBUG << "Sending message" << std::endl;
#endif

    if (rawMessages_)
    {
        message->WriteMessageLength();
        if (encryptionEnabled_)
        {
#ifdef DEBUG_NET
            LOG_DEBUG << "Encrypting" << std::endl;
#endif
            XTEAEncrypt(*message);
            message->AddCryptoHeader(checksumEnabled_);
        }
        else if (checksumEnabled_)
            message->AddCryptoHeader(true);
    }
    if (message == outputBuffer_)
        outputBuffer_.reset();
}

void Protocol::OnRecvMessage(NetworkMessage& message)
{
#ifdef DEBUG_NET
    LOG_DEBUG << "Receiving message" << std::endl;
#endif
    if (encryptionEnabled_)
    {
#ifdef DEBUG_NET
        LOG_DEBUG << "Decrypting" << std::endl;
#endif
        XTEADecrypt(message);
    }
    ParsePacket(message);
}

std::shared_ptr<OutputMessage> Protocol::GetOutputBuffer()
{
    if (outputBuffer_ && outputBuffer_->GetMessageLength() < NETWORKMESSAGE_MAXSIZE - 4096)
        return outputBuffer_;
    if (connection_)
    {
        outputBuffer_ = OutputMessagePool::Instance()->GetOutputMessage(this);
        return outputBuffer_;
    }
    return std::shared_ptr<OutputMessage>();
}

void Protocol::Send(std::shared_ptr<OutputMessage> message)
{
    OutputMessagePool::Instance()->Send(message);
}

void Protocol::Disconnect()
{
    GetConnection()->Close();
}

void Protocol::Release()
{
    if (refCount_ > 0)
    {
        Asynch::Scheduler::Instance.Add(
            Asynch::CreateScheduledTask(SCHEDULER_MINTICKS,
                std::bind(&Protocol::Release, this))
        );
    }
    else
        DeleteProtocolTask();
}

void Protocol::DeleteProtocolTask()
{
    assert(refCount_ == 0);
    SetConnection(std::shared_ptr<Connection>());

    delete this;
}

}

#include "stdafx.h"
#include "Protocol.h"
#include "NetworkMessage.h"
#include "OutputMessage.h"
#include "Scheduler.h"
#include "Aes.h"

#include "DebugNew.h"

namespace Net {

void Protocol::XTEAEncrypt(OutputMessage& message) const
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

bool Protocol::XTEADecrypt(NetworkMessage& message) const
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

void Protocol::AESEncrypt(OutputMessage& message)
{
    int32_t msgLength = message.GetMessageLength();

    uint32_t n;
    // Add bytes until we reach a multiple of 16
    if ((msgLength % AES_BLOCK_SIZE) != 0)
    {
        n = AES_BLOCK_SIZE - (msgLength % AES_BLOCK_SIZE);
        message.AddPaddingBytes(n);
        msgLength += n;
    }
    uint8_t* buffer = (uint8_t*)(message.GetBuffer() + message.GetReadPos());
    uint8_t* buff = new uint8_t[msgLength + AES_IV_SIZE];
    size_t len = Crypto::Aes::AesEncrypt(buffer, msgLength, buff, msgLength + AES_IV_SIZE, dhKey_);
    delete[] buff;
}

bool Protocol::AESDecrypt(NetworkMessage& message)
{
    int32_t msgLength = message.GetMessageLength();
    uint8_t* buffer = (uint8_t*)(message.GetBuffer() + message.GetReadPos());
    uint8_t* buff = new uint8_t[msgLength + AES_IV_SIZE];
    size_t len = Crypto::Aes::AesDecrypt(buffer, msgLength, buff, msgLength + AES_IV_SIZE, dhKey_);
    delete[] buff;
    return true;
}

void Protocol::OnSendMessage(const std::shared_ptr<OutputMessage>& message) const
{
#ifdef DEBUG_NET
    LOG_DEBUG << "Sending message" << std::endl;
#endif

    if (!rawMessages_)
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
        {
            message->AddCryptoHeader(true);
        }
    }
}

void Protocol::OnRecvMessage(NetworkMessage& message)
{
#ifdef DEBUG_NET
    LOG_DEBUG << "Receiving message with size " << message.GetMessageLength() << std::endl;
#endif
    if (encryptionEnabled_)
    {
#ifdef DEBUG_NET
        LOG_DEBUG << "Decrypting" << std::endl;
#endif
        if (!XTEADecrypt(message))
            return;
    }
    ParsePacket(message);
}

std::shared_ptr<OutputMessage> Protocol::GetOutputBuffer(int32_t size)
{
    // Dispatcher Thread
    if (!outputBuffer_)
    {
        outputBuffer_ = OutputMessagePool::GetOutputMessage();
    }
    else if ((outputBuffer_->GetSize() + size) > NetworkMessage::MaxProtocolBodyLength)
    {
        Send(outputBuffer_);
        outputBuffer_ = OutputMessagePool::GetOutputMessage();
    }
    return outputBuffer_;
}

}

#include "stdafx.h"
#include "Protocol.h"
#include "NetworkMessage.h"
#include "OutputMessage.h"
#include "Scheduler.h"
#include <AB/ProtocolCodes.h>
#include <abcrypto.hpp>

namespace Net {

void Protocol::XTEAEncrypt(OutputMessage& msg) const
{
    // The message must be a multiple of 8
    size_t paddingBytes = msg.GetSize() % 8;
    if (paddingBytes != 0)
        msg.AddPaddingBytes((uint32_t)(8 - paddingBytes));

    uint32_t* buffer = (uint32_t*)(msg.GetOutputBuffer());
    const size_t messageLength = msg.GetSize();
    xxtea_enc(buffer, (uint32_t)(messageLength / 4), reinterpret_cast<const uint32_t*>(&encKey_));
}

bool Protocol::XTEADecrypt(NetworkMessage& msg) const
{
    int32_t length = msg.GetSize() - 6;
    if ((length & 7) != 0)
        return false;

    uint32_t* buffer = (uint32_t*)(msg.GetBuffer() + 6);
    xxtea_dec(buffer, length / 4, reinterpret_cast<const uint32_t*>(&encKey_));

    return true;
}

void Protocol::OnSendMessage(OutputMessage& message) const
{
#ifdef DEBUG_NET
//    LOG_DEBUG << "Sending message" << std::endl;
#endif
    if (encryptionEnabled_)
    {
        XTEAEncrypt(message);
    }
    if (compressionEnabled_)
        message.Compress();
    if (encryptionEnabled_ || checksumEnabled_)
    {
        message.AddCryptoHeader(checksumEnabled_);
    }
}

void Protocol::OnRecvMessage(NetworkMessage& message)
{
#ifdef DEBUG_NET
//    LOG_DEBUG << "Receiving message with size " << message.GetMessageLength() << std::endl;
#endif
    if (compressionEnabled_)
        message.Uncompress();

    if (encryptionEnabled_)
    {
        if (!XTEADecrypt(message))
        {
#ifdef DEBUG_NET
            LOG_DEBUG << "Failed to decrypt message." << std::endl;
#endif
            return;
        }
    }
    ParsePacket(message);
}

sa::SharedPtr<OutputMessage> Protocol::GetOutputBuffer(int32_t size)
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

void Protocol::ResetOutputBuffer()
{
    outputBuffer_ = OutputMessagePool::GetOutputMessage();
}

}

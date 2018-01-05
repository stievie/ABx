#include "stdafx.h"
#include "Protocol.h"
#include "NetworkMessage.h"
#include "OutputMessage.h"
#include "Scheduler.h"
#include <AB/ProtocolCodes.h>
#include <abcrypto.hpp>

#include "DebugNew.h"

namespace Net {

void Protocol::XTEAEncrypt(OutputMessage& msg) const
{
    // The message must be a multiple of 8
    size_t paddingBytes = msg.GetSize() % 8;
    if (paddingBytes != 0)
    {
        msg.AddPaddingBytes((uint32_t)(8 - paddingBytes));
    }

    uint32_t* buffer = (uint32_t*)(msg.GetOutputBuffer());
    const size_t messageLength = msg.GetSize();
    xxtea_enc(buffer, (uint32_t)(messageLength / 4), AB::ENC_KEY);
}

bool Protocol::XTEADecrypt(NetworkMessage& msg) const
{
    int32_t length = msg.GetSize() - 6;
    if ((length & 7) != 0)
        return false;

    uint32_t* buffer = (uint32_t*)(msg.GetBuffer() + 6);
    xxtea_dec(buffer, length / 4, AB::ENC_KEY);

    return true;
}

void Protocol::OnSendMessage(const std::shared_ptr<OutputMessage>& message) const
{
#ifdef DEBUG_NET
//    LOG_DEBUG << "Sending message" << std::endl;
#endif
//    message->WriteMessageLength();
    if (encryptionEnabled_)
    {
        XTEAEncrypt(*message);
        message->AddCryptoHeader(checksumEnabled_);
    }
    else if (checksumEnabled_)
    {
        message->AddCryptoHeader(true);
    }
}

void Protocol::OnRecvMessage(NetworkMessage& message)
{
#ifdef DEBUG_NET
//    LOG_DEBUG << "Receiving message with size " << message.GetMessageLength() << std::endl;
#endif

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

void Protocol::ResetOutputBuffer()
{
    outputBuffer_ = OutputMessagePool::GetOutputMessage();
}

}

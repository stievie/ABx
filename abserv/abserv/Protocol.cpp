#include "stdafx.h"
#include "Protocol.h"
#include "NetworkMessage.h"
#include "OutputMessage.h"
#include "Scheduler.h"
#include "Aes.h"

#include "DebugNew.h"

namespace Net {

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
//    LOG_DEBUG << "Sending message" << std::endl;
#endif

    if (checksumEnabled_)
    {
        message->AddCryptoHeader(true);
    }
}

void Protocol::OnRecvMessage(NetworkMessage& message)
{
#ifdef DEBUG_NET
//    LOG_DEBUG << "Receiving message with size " << message.GetMessageLength() << std::endl;
#endif
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

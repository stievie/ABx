/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "Protocol.h"
#include "NetworkMessage.h"
#include "OutputMessage.h"
#include "Scheduler.h"
#include <AB/ProtocolCodes.h>
#include <abcrypto.hpp>
#include "Connection.h"
#include "OutputMessage.h"

namespace Net {

Protocol::Protocol(std::shared_ptr<Connection> connection) :
    connection_(connection),
    checksumEnabled_(false),
    compressionEnabled_(false),
    encryptionEnabled_(false)
{ }

Protocol::~Protocol() = default;

void Protocol::Disconnect() const
{
    if (auto conn = GetConnection())
        conn->Close();
}

uint32_t Protocol::GetIP()
{
    if (auto c = GetConnection())
        return c->GetIP();
    return 0;
}

void Protocol::Send(sa::SharedPtr<OutputMessage>&& message)
{
    if (auto conn = GetConnection())
    {
        conn->Send(std::move(message));
        return;
    }
    LOG_ERROR << "Connection expired" << std::endl;
    Release();
}

sa::SharedPtr<OutputMessage>& Protocol::GetCurrentBuffer()
{
    return outputBuffer_;
}

void Protocol::XTEAEncrypt(OutputMessage& msg) const
{
    // The message must be a multiple of 8
    size_t paddingBytes = msg.GetSize() % 8;
    if (paddingBytes != 0)
        msg.AddPaddingBytes(static_cast<uint32_t>(8 - paddingBytes));

    uint32_t* buffer = reinterpret_cast<uint32_t*>(msg.GetOutputBuffer());
    const uint32_t messageLength = static_cast<uint32_t>(msg.GetSize());
    xxtea_enc(buffer, (messageLength / 4), reinterpret_cast<const uint32_t*>(&encKey_));
}

bool Protocol::XTEADecrypt(NetworkMessage& msg) const
{
    int32_t length = msg.GetSize() - 6;
    if ((length & 7) != 0)
        return false;

    uint32_t* buffer = reinterpret_cast<uint32_t*>(msg.GetBuffer() + 6);
    xxtea_dec(buffer, static_cast<uint32_t>(length / 4), reinterpret_cast<const uint32_t*>(&encKey_));

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
        Send(std::move(outputBuffer_));
        outputBuffer_ = OutputMessagePool::GetOutputMessage();
    }
    return outputBuffer_;
}

void Protocol::ResetOutputBuffer()
{
    outputBuffer_ = OutputMessagePool::GetOutputMessage();
}

}

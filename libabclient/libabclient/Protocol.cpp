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
#include <ctime>
#include <AB/ProtocolCodes.h>
#include <abcrypto.hpp>

namespace Client {

Protocol::Protocol(Crypto::DHKeys& keys, asio::io_service& ioService) :
    inputMessage_(std::make_shared<InputMessage>()),
    ioService_(ioService),
    connection_(nullptr),
    encryptEnabled_(false),
    keys_(keys),
    errorCallback_(nullptr),
    protocolErrorCallback_(nullptr)
{ }

Protocol::~Protocol()
{
    Disconnect();
}

void Protocol::Connect(const std::string& host, uint16_t port)
{
    connection_ = std::make_shared<Connection>(ioService_);
    connection_->SetErrorCallback(std::bind(&Protocol::OnError, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2));
    connection_->Connect(host, port, std::bind(&Protocol::OnConnect, shared_from_this()));
}

void Protocol::Connect(const std::string& host, uint16_t port,
    std::function<void()>&& onConnect)
{
    connection_ = std::make_shared<Connection>(ioService_);
    connection_->SetErrorCallback(std::bind(&Protocol::OnError, shared_from_this(),
        std::placeholders::_1, std::placeholders::_2));
    connection_->Connect(host, port, std::move(onConnect));
}

void Protocol::Disconnect()
{
    if (connection_)
        connection_->Close();
}

void Protocol::Send(OutputMessage& message)
{
    if (encryptEnabled_)
        XTEAEncrypt(message);
    message.WriteChecksum();
    message.WriteMessageSize();

    if (connection_)
        connection_->Write(message.GetHeaderBuffer(), message.GetSize());
}

void Protocol::Receive()
{
    inputMessage_->Reset();

    // first update message header size
    size_t headerSize = 2; // 2 bytes for message size
    headerSize += 4; // 4 bytes for checksum
    if (encryptEnabled_)
        headerSize += 2;
    inputMessage_->SetHeaderSize(headerSize);

    // read the first 2 bytes which contain the message size
    if (connection_)
        connection_->Read(2, std::bind(&Protocol::InternalRecvHeader, shared_from_this(),
            std::placeholders::_1, std::placeholders::_2));
}

void Protocol::InternalRecvHeader(uint8_t* buffer, size_t size)
{
    if (!IsConnected())
        return;

    inputMessage_->FillBuffer(buffer, size);
    size_t remainingSize = inputMessage_->ReadSize();

    // read remaining message data
    if (connection_)
        connection_->Read(remainingSize, std::bind(&Protocol::InternalRecvData,
            shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void Protocol::InternalRecvData(uint8_t* buffer, size_t size)
{
    if (!IsConnected())
        return;

    inputMessage_->FillBuffer(buffer, size);

    if (!inputMessage_->ReadChecksum())
        return;
    if (encryptEnabled_)
    {
        if (!XTEADecrypt(*inputMessage_))
            return;
    }

    OnReceive(*inputMessage_);
}

bool Protocol::XTEADecrypt(InputMessage& inputMessage)
{
    size_t encryptedSize = inputMessage.GetUnreadSize();
    if (encryptedSize % 8 != 0)
        return false;

    uint32_t* buffer = reinterpret_cast<uint32_t*>(inputMessage.GetReadBuffer());
    xxtea_dec(buffer, static_cast<uint32_t>(encryptedSize / 4), reinterpret_cast<const uint32_t*>(&encKey_));

    return true;
}

void Protocol::XTEAEncrypt(OutputMessage& outputMessage)
{
    size_t encryptedSize = outputMessage.GetSize();

    //add bytes until reach 8 multiple
    if ((encryptedSize % 8) != 0)
    {
        uint16_t n = 8 - (encryptedSize % 8);
        outputMessage.AddPaddingBytes(n);
        encryptedSize += n;
    }

    uint32_t* buffer = reinterpret_cast<uint32_t*>(outputMessage.GetDataBuffer());
    xxtea_enc(buffer, static_cast<uint32_t>(encryptedSize / 4), reinterpret_cast<const uint32_t*>(&encKey_));
}

void Protocol::OnError(ConnectionError connectionError, const asio::error_code& err)
{
    if (errorCallback_)
        errorCallback_(connectionError, err);
}

}

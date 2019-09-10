#include "stdafx.h"
#include "Protocol.h"
#include <ctime>
#include <AB/ProtocolCodes.h>
#include <abcrypto.hpp>

#include "DebugNew.h"

namespace Client {

Protocol::Protocol(Crypto::DHKeys& keys, asio::io_service& ioService) :
    inputMessage_(std::make_shared<InputMessage>()),
    ioService_(ioService),
    connection_(nullptr),
    checksumEnabled_(false),
    compressionEnabled_(false),
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
    connection_->SetErrorCallback(std::bind(&Protocol::OnError, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    connection_->Connect(host, port, std::bind(&Protocol::OnConnect, shared_from_this()));
}

void Protocol::Disconnect()
{
    if (connection_)
    {
        connection_->Close();
    }
}

void Protocol::Send(std::shared_ptr<OutputMessage>&& message)
{
    if (encryptEnabled_)
        XTEAEncrypt(*message);
    if (compressionEnabled_)
        message->Compress();
    if (checksumEnabled_)
        message->WriteChecksum();
    message->WriteMessageSize();

    if (connection_)
        connection_->Write(message->GetHeaderBuffer(), message->GetSize());

    message->Reset();
}

void Protocol::Receive()
{
    inputMessage_->Reset();

    // first update message header size
    int headerSize = 2; // 2 bytes for message size
    if (checksumEnabled_)
        headerSize += 4; // 4 bytes for checksum
    if (encryptEnabled_)
        headerSize += 2;
    inputMessage_->SetHeaderSize(static_cast<uint16_t>(headerSize));

    // read the first 2 bytes which contain the message size
    if (connection_)
        connection_->Read(2, std::bind(&Protocol::InternalRecvHeader, shared_from_this(),
            std::placeholders::_1, std::placeholders::_2));
}

void Protocol::InternalRecvHeader(uint8_t* buffer, uint16_t size)
{
    if (!IsConnected())
        return;

    inputMessage_->FillBuffer(buffer, size);
    uint16_t remainingSize = inputMessage_->ReadSize();

    // read remaining message data
    if (connection_)
        connection_->Read(remainingSize, std::bind(&Protocol::InternalRecvData,
            shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void Protocol::InternalRecvData(uint8_t* buffer, uint16_t size)
{
    if (!IsConnected())
        return;

    inputMessage_->FillBuffer(buffer, size);

    if (checksumEnabled_ && !inputMessage_->ReadChecksum())
    {
        return;
    }
    if (compressionEnabled_)
    {
        if (!inputMessage_->Uncompress())
            return;
    }
    if (encryptEnabled_)
    {
        if (!XTEADecrypt(*inputMessage_))
        {
            return;
        }
    }

    OnReceive(*inputMessage_);
}

bool Protocol::XTEADecrypt(InputMessage& inputMessage)
{
    uint16_t encryptedSize = (uint16_t)inputMessage.GetUnreadSize();
    if (encryptedSize % 8 != 0)
    {
        return false;
    }

    uint32_t* buffer = (uint32_t*)(inputMessage.GetReadBuffer());
    xxtea_dec(buffer, encryptedSize / 4, reinterpret_cast<const uint32_t*>(&encKey_));

    return true;
}

void Protocol::XTEAEncrypt(OutputMessage& outputMessage)
{
    uint16_t encryptedSize = outputMessage.GetSize();

    //add bytes until reach 8 multiple
    if ((encryptedSize % 8) != 0)
    {
        uint16_t n = 8 - (encryptedSize % 8);
        outputMessage.AddPaddingBytes(n);
        encryptedSize += n;
    }

    uint32_t* buffer = (uint32_t*)(outputMessage.GetDataBuffer());
    xxtea_enc(buffer, encryptedSize / 4, reinterpret_cast<const uint32_t*>(&encKey_));
}

void Protocol::OnError(ConnectionError connectionError, const asio::error_code& err)
{
    if (errorCallback_)
        errorCallback_(connectionError, err);
    Disconnect();
}

}

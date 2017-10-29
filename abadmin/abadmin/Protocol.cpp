#include "stdafx.h"
#include "Protocol.h"
#include <random>
#include <ctime>
#include "Logger.h"

Protocol::Protocol() :
    connection_(nullptr),
    checksumEnabled_(false),
    xteaEnabled_(false)
{
    inputMessage_ = std::make_shared<InputMessage>();
}

Protocol::~Protocol()
{
    Disconnect();
}

void Protocol::Connect(const std::string& host, uint16_t port)
{
    connection_ = std::make_shared<Connection>();
    connection_->SetErrorCallback(std::bind(&Protocol::OnError, shared_from_this(), std::placeholders::_1));
    connection_->Connect(host, port, std::bind(&Protocol::OnConnect, shared_from_this()));
}

void Protocol::Disconnect()
{
    if (connection_)
    {
        connection_->Close();
        connection_.reset();
    }
}

void Protocol::GenerateXteaKey()
{
    std::mt19937 eng((unsigned)std::time(NULL));
    std::uniform_int_distribution<uint32_t> unif(0, 0xFFFFFFFF);
    xteaKey_[0] = unif(eng);
    xteaKey_[1] = unif(eng);
    xteaKey_[2] = unif(eng);
    xteaKey_[3] = unif(eng);
}

void Protocol::SetXteaKey(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    xteaKey_[0] = a;
    xteaKey_[1] = b;
    xteaKey_[2] = c;
    xteaKey_[3] = d;
}

void Protocol::Send(const std::shared_ptr<OutputMessage>& message)
{
    if (xteaEnabled_)
        XteaEncrypt(message);
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
    if (xteaEnabled_)
        headerSize += 2; // 2 bytes for XTEA encrypted message size
    inputMessage_->SetHeaderSize(headerSize);

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
#ifdef _DEBUG
    LOG_DEBUG << "size = " << size << ", remaining size = " << remainingSize << std::endl;
#endif

    // read remaining message data
    if (connection_)
        connection_->Read(remainingSize, std::bind(&Protocol::InternalRecvData,
            shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void Protocol::InternalRecvData(uint8_t* buffer, uint16_t size)
{
    if (!IsConnected())
        return;

#ifdef _DEBUG
    LOG_DEBUG << "size = " << size << std::endl;
#endif

    inputMessage_->FillBuffer(buffer, size);

    if (checksumEnabled_ && !inputMessage_->ReadChecksum())
    {
        // Invalid checksum
        return;
    }

    if (xteaEnabled_)
    {
        if (!XteaDecrypt(inputMessage_))
        {
            return;
        }
    }

    OnReceive(inputMessage_);
}

bool Protocol::XteaDecrypt(const std::shared_ptr<InputMessage>& message)
{
    uint16_t encryptedSize = message->GetUnreadSize();
    if (encryptedSize % 8 != 0) {
        return false;
    }

    uint32_t *buffer = (uint32_t*)(message->GetReadBuffer());
    int readPos = 0;

    while (readPos < encryptedSize / 4)
    {
        uint32_t v0 = buffer[readPos], v1 = buffer[readPos + 1];
        uint32_t delta = 0x61C88647;
        uint32_t sum = 0xC6EF3720;

        for (int32_t i = 0; i < 32; i++)
        {
            v1 -= ((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + xteaKey_[sum >> 11 & 3]);
            sum += delta;
            v0 -= ((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + xteaKey_[sum & 3]);
        }
        buffer[readPos] = v0; buffer[readPos + 1] = v1;
        readPos = readPos + 2;
    }

    uint16_t decryptedSize = message->Get<uint16_t>() + 2;
    int sizeDelta = decryptedSize - encryptedSize;
    if (sizeDelta > 0 || -sizeDelta > encryptedSize)
    {
        return false;
    }

    message->SetMessageSize(message->GetMessageSize() + sizeDelta);
    return true;
}

void Protocol::XteaEncrypt(const std::shared_ptr<OutputMessage>& message)
{
    message->WriteMessageSize();
    uint16_t encryptedSize = message->GetSize();

    //add bytes until reach 8 multiple
    if ((encryptedSize % 8) != 0)
    {
        uint16_t n = 8 - (encryptedSize % 8);
        message->AddPaddingBytes(n);
        encryptedSize += n;
    }

    int readPos = 0;
    uint32_t *buffer = (uint32_t*)(message->GetDataBuffer() - 2);
    while (readPos < encryptedSize / 4)
    {
        uint32_t v0 = buffer[readPos], v1 = buffer[readPos + 1];
        uint32_t delta = 0x61C88647;
        uint32_t sum = 0;

        for (int32_t i = 0; i < 32; i++)
        {
            v0 += ((v1 << 4 ^ v1 >> 5) + v1) ^ (sum + xteaKey_[sum & 3]);
            sum -= delta;
            v1 += ((v0 << 4 ^ v0 >> 5) + v0) ^ (sum + xteaKey_[sum >> 11 & 3]);
        }
        buffer[readPos] = v0; buffer[readPos + 1] = v1;
        readPos = readPos + 2;
    }
}

void Protocol::OnError(const asio::error_code& err)
{
    Disconnect();
}

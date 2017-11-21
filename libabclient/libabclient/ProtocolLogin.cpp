#include "stdafx.h"
#include "ProtocolLogin.h"

namespace Client {

ProtocolLogin::ProtocolLogin() :
    Protocol()
{
    checksumEnabled_ = ProtocolLogin::UseChecksum;
}

ProtocolLogin::~ProtocolLogin()
{
}

void ProtocolLogin::Login(std::string& host, uint16_t port,
    const std::string& account, const std::string& password)
{
    accountName_ = account;
    password_ = password;
    Connect(host, port);
}

void ProtocolLogin::SendLoginPacket()
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(ProtocolLogin::ProtocolIdentifier);
    msg->Add<uint16_t>(1);   // Client OS
    msg->Add<uint16_t>(1);   // Client Version
    msg->AddString(accountName_);
    msg->AddString(password_);
    Send(msg);
    Receive();
}

void ProtocolLogin::ParseMessage(const std::shared_ptr<InputMessage>& message)
{
    uint8_t recvByte = message->Get<uint8_t>();
    switch (recvByte)
    {
    case 0x64:
    {
        characters_.clear();
        uint32_t id = message->Get<uint32_t>();
        uint16_t level = message->Get<uint16_t>();
        std::string name = message->GetString();
        characters_.push_back({ id, level, name });
        break;
    }
    }
}

void ProtocolLogin::OnConnect()
{
    firstRecv_ = true;
    Protocol::OnConnect();

    SendLoginPacket();
}

void ProtocolLogin::OnReceive(const std::shared_ptr<InputMessage>& message)
{
    if (firstRecv_)
    {
        firstRecv_ = false;
    }

    ParseMessage(message);
}

}

#include "stdafx.h"
#include "ProtocolLogin.h"
#include <AB/ProtocolCodes.h>

namespace Client {

ProtocolLogin::ProtocolLogin() :
    Protocol(),
    charlistCallback(nullptr)
{
    checksumEnabled_ = ProtocolLogin::UseChecksum;
}

ProtocolLogin::~ProtocolLogin()
{
}

void ProtocolLogin::Login(std::string& host, uint16_t port,
    const std::string& account, const std::string& password, const CharlistCallback& callback)
{
    accountName_ = account;
    password_ = password;
    charlistCallback = callback;
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
}

void ProtocolLogin::ParseMessage(const std::shared_ptr<InputMessage>& message)
{
    uint8_t recvByte = message->Get<uint8_t>();
    switch (recvByte)
    {
    case AB::LoginProtocol::CharacterList:
    {
        gameHost_ = message->GetString();
        gamePort_ = message->Get<uint16_t>();
        CharList chars;
        int count = message->Get<uint8_t>();
        for (int i = 0; i < count; i++)
        {
            uint32_t id = message->Get<uint32_t>();
            uint16_t level = message->Get<uint16_t>();
            std::string name = message->GetString();
            std::string lastMap = message->GetString();
            chars.push_back({ id, level, name, lastMap });
        }
        if (charlistCallback)
            charlistCallback(chars);
        break;
    }
    case AB::LoginProtocol::LoginError:
    {
        uint8_t error = message->Get<uint8_t>();
        ProtocolError(error);
        break;
    }
    }
}

void ProtocolLogin::OnConnect()
{
    firstRecv_ = true;
    Protocol::OnConnect();

    SendLoginPacket();
    Receive();
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

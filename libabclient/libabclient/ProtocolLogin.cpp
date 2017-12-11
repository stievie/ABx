#include "stdafx.h"
#include "ProtocolLogin.h"
#include <AB/ProtocolCodes.h>
#include "DHKeys.h"

namespace Client {

ProtocolLogin::ProtocolLogin() :
    Protocol(),
    charlistCallback(nullptr)
{
    checksumEnabled_ = ProtocolLogin::UseChecksum;
}

void ProtocolLogin::Login(std::string& host, uint16_t port,
    const std::string& account, const std::string& password, const CharlistCallback& callback)
{
    accountName_ = account;
    password_ = password;
    charlistCallback = callback;
    Connect(host, port);
}

void ProtocolLogin::SendKeyExchange()
{
    // TODO:
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(ProtocolLogin::ProtocolIdentifier);
    msg->Add<uint16_t>(1);   // Client OS
    msg->Add<uint16_t>(AB::PROTOCOL_VERSION);   // Protocol Version
    const DH_KEY& clientKey = Crypto::DHKeys::Instance.GetPublicKey();
    for (int i = 0; i < DH_KEY_LENGTH; i++)
        msg->Add<uint8_t>(clientKey[i]);
    Send(msg);
}

void ProtocolLogin::SendLoginPacket()
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(ProtocolLogin::ProtocolIdentifier);
    msg->Add<uint16_t>(1);   // Client OS
    msg->Add<uint16_t>(AB::PROTOCOL_VERSION);   // Protocol Version
    msg->AddString(accountName_);
    msg->AddString(password_);
    Send(msg);
}

void ProtocolLogin::ParseMessage(const std::shared_ptr<InputMessage>& message)
{
    uint8_t recvByte = message->Get<uint8_t>();
    switch (recvByte)
    {
    case AB::LoginProtocol::KeyExchange:
    {
        DH_KEY serverPublic;
        for (int i = 0; i < DH_KEY_LENGTH; i++)
            serverPublic[i] = message->Get<uint8_t>();
        // Calculate shared key from server pubic key and our key
        Crypto::DHKeys::Instance.GetSharedKey(serverPublic, sharedKey_);
        break;
    }
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

#include "stdafx.h"
#include "ProtocolGame.h"

namespace Client {

ProtocolGame::ProtocolGame() :
    Protocol(),
    enterWorldCallback_(nullptr)
{
}

ProtocolGame::~ProtocolGame()
{
}

void ProtocolGame::Login(const std::string& accountName,
    const std::string& accountPass, std::string charName,
    const std::string& host, uint16_t port, const EnterWorldCallback& callback)
{
    accountName_ = accountName;
    accountPass_ = accountPass;
    charName_ = charName;
    enterWorldCallback_ = callback;

    Connect(host, port);
}

void ProtocolGame::OnConnect()
{
    firstRevc_ = true;
    Protocol::OnConnect();

    SendLoginPacket();

    Receive();
}

void ProtocolGame::OnReceive(const std::shared_ptr<InputMessage>& message)
{
    if (firstRevc_)
    {
        firstRevc_ = false;
    }
    ParseMessage(message);
    Receive();
}

void ProtocolGame::OnError(const asio::error_code& err)
{
    Protocol::OnError(err);
}

void ProtocolGame::ParseMessage(const std::shared_ptr<InputMessage>& message)
{
    uint8_t opCode = 0;
    while (!message->Eof())
    {
        opCode = message->Get<uint8_t>();

        switch (opCode)
        {
        case GameServerLoginOrPendingState:
            break;
        case GameServerChallenge:
            break;
        case GameServerLoginSuccess:
            break;
        case GameServerEnterGame:
            ParseEnterWorld(message);
            break;
        }
    }
}

void ProtocolGame::ParseEnterWorld(const std::shared_ptr<InputMessage>& message)
{
    std::string map = message->GetString();
    if (enterWorldCallback_)
        enterWorldCallback_(map);
}

void ProtocolGame::SendLoginPacket()
{
    std::shared_ptr<OutputMessage> msg = std::make_shared<OutputMessage>();
    msg->Add<uint8_t>(ProtocolGame::ProtocolIdentifier);
    msg->Add<uint16_t>(1);   // Client OS
    msg->Add<uint16_t>(1);   // Client Version
    msg->AddString(accountName_);
    msg->AddString(accountPass_);
    msg->AddString(charName_);
    Send(msg);
}


}

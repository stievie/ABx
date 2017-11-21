#include "stdafx.h"
#include "ProtocolGame.h"

namespace Client {

ProtocolGame::ProtocolGame() :
    Protocol()
{
}

ProtocolGame::~ProtocolGame()
{
}

void ProtocolGame::Login(const std::string& accountName,
    const std::string& accountPass, std::string charName, const std::string& host, uint16_t port)
{
    accountName_ = accountName;
    accountPass_ = accountPass;
    charName_ = charName;

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

bool ProtocolGame::ParseMessage(const std::shared_ptr<InputMessage>& message)
{
    return false;
}

void ProtocolGame::SendLoginPacket()
{
}


}

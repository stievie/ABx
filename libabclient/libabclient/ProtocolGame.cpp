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

void ProtocolGame::OnConnect()
{
}

void ProtocolGame::OnReceive(const std::shared_ptr<InputMessage>& message)
{
}

bool ProtocolGame::ParsePacket(uint8_t cmd, const std::shared_ptr<InputMessage>& message)
{
    return false;
}

}

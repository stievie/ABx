#include "stdafx.h"
#include "ProtocolGame.h"


ProtocolGame::ProtocolGame(std::shared_ptr<Connection> connection) :
    Protocol(connection)
{
}


ProtocolGame::~ProtocolGame()
{
}

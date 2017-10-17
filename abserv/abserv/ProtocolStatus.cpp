#include "stdafx.h"
#include "ProtocolStatus.h"


ProtocolStatus::ProtocolStatus(std::shared_ptr<Connection> connection) :
    Protocol(connection)
{
}


ProtocolStatus::~ProtocolStatus()
{
}

void ProtocolStatus::OnRecvFirstMessage(NetworkMessage& msg)
{
}

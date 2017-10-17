#include "stdafx.h"
#include "ProtocolStatus.h"

namespace Net {

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

}

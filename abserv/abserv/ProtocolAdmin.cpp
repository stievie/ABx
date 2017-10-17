#include "stdafx.h"
#include "ProtocolAdmin.h"

namespace Net {

ProtocolAdmin::ProtocolAdmin(std::shared_ptr<Connection> connection) :
    Protocol(connection)
{
}


ProtocolAdmin::~ProtocolAdmin()
{
}

void ProtocolAdmin::OnRecvFirstMessage(NetworkMessage& msg)
{
}

}

#include "stdafx.h"
#include "ProtocolLogin.h"
#include "OutputMessage.h"

namespace Net {

ProtocolLogin::ProtocolLogin(std::shared_ptr<Connection> connection) :
    Protocol(connection)
{
}


ProtocolLogin::~ProtocolLogin()
{
}

void ProtocolLogin::OnRecvFirstMessage(NetworkMessage& msg)
{
}

void ProtocolLogin::DisconnectClient(uint8_t error, const char* message)
{
    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage(this, false);
    if (output)
    {
        output->AddByte(error);
        output->AddString(message);
        OutputMessagePool::Instance()->Send(output);
    }
    GetConnection()->Close();
}

bool ProtocolLogin::ParseFirstPacket(NetworkMessage& message)
{
    return false;
}

}

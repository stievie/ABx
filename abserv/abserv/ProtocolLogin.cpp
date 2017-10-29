#include "stdafx.h"
#include "ProtocolLogin.h"
#include "OutputMessage.h"

#include "DebugNew.h"

namespace Net {

ProtocolLogin::ProtocolLogin(std::shared_ptr<Connection> connection) :
    Protocol(connection)
{
}


ProtocolLogin::~ProtocolLogin()
{
}

void ProtocolLogin::OnRecvFirstMessage(NetworkMessage& message)
{
    ParseFirstPacket(message);
}

void ProtocolLogin::DisconnectClient(uint8_t error, const char* message)
{
    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();
    if (output)
    {
        output->AddByte(error);
        output->AddString(message);
        Send(output);
    }
    Disconnect();
}

bool ProtocolLogin::ParseFirstPacket(NetworkMessage& message)
{
    return false;
}

}

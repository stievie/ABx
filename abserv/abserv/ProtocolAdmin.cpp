#include "stdafx.h"
#include "ProtocolAdmin.h"
#include "ConfigManager.h"
#include "Logger.h"
#include "Utils.h"
#include "OutputMessage.h"

#include "DebugNew.h"

namespace Net {

ProtocolAdmin::ProtocolAdmin(std::shared_ptr<Connection> connection) :
    Protocol(connection),
    loginTries_(0),
    lastCommand_(0),
    state_(NotConnected)
{
    startTime_ = std::time(nullptr);
}

void ProtocolAdmin::OnRecvFirstMessage(NetworkMessage& msg)
{
    if (!ConfigManager::Instance[ConfigManager::Key::AdminEnabled])
    {
        Disconnect();
        return;
    }

    state_ = NotConnected;

    if (!AllowIP(GetIP()))
    {
        Disconnect();
        return;
    }

    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage(this, false);

    Send(output);

    lastCommand_ = std::time(nullptr);
    state_ = EncryptionToSet;
}

void ProtocolAdmin::ParsePacket(NetworkMessage& message)
{
    uint8_t recvByte = message.GetByte();

    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage(this, false);
    if (output)
    {
        switch (state_)
        {
        case EncryptionToSet:
            break;
        case NotloggedIn:
            break;
        case LooggedIn:
            // Can execute commands
            break;
        default:
            Disconnect();
            return;
        }

        lastCommand_ = std::time(nullptr);

        switch (recvByte)
        {
        case ApMsg::ApMsgLogin:
            break;
        default:
            break;
        }

        if (output->GetMessageLength() > 0)
            Send(output);
    }
}

bool ProtocolAdmin::AllowIP(uint32_t clientIP)
{
    if (ConfigManager::Instance[ConfigManager::Key::AdminLocalhostOnly])
    {
        if (clientIP == 0x0100007F)
            // 127.0.0.1
            return true;

        LOG_WARNING << "Forbidden connection try from " << Utils::ConvertIPToString(clientIP) << std::endl;
        return false;
    }
    return true;
}

}

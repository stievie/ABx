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
    requireLogin_ = ConfigManager::Instance[ConfigManager::Key::AdminRequireLogin];
    requireEncryption_ = ConfigManager::Instance[ConfigManager::Key::AdminRequireEncryption];
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
    if (output)
    {
        output->AddByte(AP_MSG_HELLO);
        output->Add<uint32_t>(1);  // Version
//        output->AddString("ABADMIN");
        output->Add<uint16_t>(GetProtocolPolicy());
        output->Add<uint32_t>(GetProtocolOptions());
        Send(output);
    }

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
        {
            if (ConfigManager::Instance[ConfigManager::Key::AdminRequireEncryption])
            {
                if (std::time(nullptr) - startTime_ > 30000)
                {
                    Disconnect();
                    LOG_WARNING << "Encryption timeout" << std::endl;
                    return;
                }

                if (recvByte != AP_MSG_ENCRYPTION && recvByte != AP_MSG_KEY_EXCHANGE)
                {
                    output->AddByte(AP_MSG_ERROR);
                    output->AddString("Encryption required");
                    Send(output);
                    Disconnect();
                    LOG_WARNING << "Wrong command while EncryptionToSet" << std::endl;
                }
            }
            else
                state_ = NotloggedIn;
            break;
        }
        case NotloggedIn:
        {
            if (ConfigManager::Instance[ConfigManager::Key::AdminRequireLogin])
            {

            }
            break;
        }
        case LoggedIn:
            // Can execute commands
            break;
        default:
            Disconnect();
            return;
        }

        lastCommand_ = std::time(nullptr);

        switch (recvByte)
        {
        case AP_MSG_LOGIN:
            HandleMsgLogin(message, output.get());
            break;
        case AP_MSG_ENCRYPTION:
            HandleMsgEncryption(message, output.get());
            break;
        case AP_MSG_KEY_EXCHANGE:
            HandleMsgKeyExchange(message, output.get());
            break;
        case AP_MSG_COMMAND:
            HandleMsgCommand(message, output.get());
            break;
        case AP_MSG_PING:
            HandleMsgPing(message, output.get());
            break;
        case AP_MSG_KEEP_ALIVE:
            // Do nothing
            break;
        default:
            output->AddByte(AP_MSG_ERROR);
            output->AddString("Unknown command byte");
            break;
        }

        if (output->GetMessageLength() > 0)
            Send(output);
    }
}

void ProtocolAdmin::HandleMsgLogin(NetworkMessage& message, OutputMessage* output)
{
}

void ProtocolAdmin::HandleMsgEncryption(NetworkMessage& message, OutputMessage* output)
{

}

void ProtocolAdmin::HandleMsgKeyExchange(NetworkMessage& message, OutputMessage* output)
{

}

void ProtocolAdmin::HandleMsgCommand(NetworkMessage& message, OutputMessage* output)
{

}

void ProtocolAdmin::HandleMsgPing(NetworkMessage& message, OutputMessage* output)
{
    output->AddByte(AP_MSG_PING_OK);
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

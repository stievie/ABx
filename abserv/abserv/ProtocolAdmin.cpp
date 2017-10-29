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
#ifdef DEBUG_NET
        LOG_DEBUG << std::endl;
#endif
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

    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();
    if (output)
    {
#ifdef DEBUG_NET
        LOG_DEBUG << "Sending HELLO" << std::endl;
#endif
        output->AddByte(AP_MSG_HELLO);                  // 1 Byte
        output->Add<uint32_t>(1);                       // Version 4 Byte
        output->AddString("ABADMIN");                   // Server string
        output->Add<uint16_t>(GetProtocolPolicy());     // 2 Byte
        output->Add<uint32_t>(GetProtocolOptions());    // 4 Byte
        Send(output);
    }

    lastCommand_ = std::time(nullptr);
    state_ = EncryptionToSet;
}

void ProtocolAdmin::ParsePacket(NetworkMessage& message)
{
#ifdef DEBUG_NET
        LOG_DEBUG << std::endl;
#endif
    uint8_t recvByte = message.GetByte();

    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();
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
                if ((std::time(nullptr) - startTime_) > 30000)
                {
                    // Login timeout
                    Disconnect();
                    LOG_WARNING << "Login timeout" << std::endl;
                    return;
                }

                if (loginTries_ > 3)
                {
                    output->AddByte(AP_MSG_ERROR);
                    output->AddString("Too many login tries");
                    Send(output);
                    Disconnect();
                    LOG_WARNING << "Too many login tries" << std::endl;
                    return;
                }

                if (recvByte != AP_MSG_LOGIN)
                {
                    output->AddByte(AP_MSG_ERROR);
                    output->AddString("You are not logged in");
                    Send(output);
                    Disconnect();
                    LOG_WARNING << "Wrong command while NotloggedIn" << std::endl;
                    return;
                }
                break;
            }
            else
                state_ = LoggedIn;
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
#ifdef DEBUG_NET
    LOG_DEBUG << std::endl;
#endif
    if (state_ == NotloggedIn && ConfigManager::Instance[ConfigManager::Key::AdminRequireLogin].GetBool())
    {
        std::string password = message.GetString();
        if (password.compare(ConfigManager::Instance[ConfigManager::Key::AdminPassword].GetString()) == 0)
        {
            state_ = LoggedIn;
            output->AddByte(AP_MSG_LOGIN_OK);
            LOG_INFO << "Login OK" << std::endl;
        }
        else
        {
            loginTries_++;
            output->AddByte(AP_MSG_LOGIN_FAILED);
            output->AddString("Wrong password");
            LOG_WARNING << "Login failed, password: " << password << std::endl;
        }
    }
    else
    {
        output->AddByte(AP_MSG_LOGIN_FAILED);
        output->AddString("Can not login");
        LOG_WARNING << "Wrong state at login" << std::endl;
    }
}

void ProtocolAdmin::HandleMsgEncryption(NetworkMessage& message, OutputMessage* output)
{
#ifdef DEBUG_NET
    LOG_DEBUG << std::endl;
#endif

}

void ProtocolAdmin::HandleMsgKeyExchange(NetworkMessage& message, OutputMessage* output)
{
#ifdef DEBUG_NET
    LOG_DEBUG << std::endl;
#endif

}

void ProtocolAdmin::HandleMsgCommand(NetworkMessage& message, OutputMessage* output)
{
#ifdef DEBUG_NET
    LOG_DEBUG << std::endl;
#endif
    if (state_ != LoggedIn)
    {
        LOG_ERROR << "Got AP_MSG_COMMAND while not logged in" << std::endl;
        return;
    }

    uint8_t command = message.GetByte();
}

void ProtocolAdmin::HandleMsgPing(NetworkMessage& message, OutputMessage* output)
{
#ifdef DEBUG_NET
    LOG_DEBUG << std::endl;
#endif
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

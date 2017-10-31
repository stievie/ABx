#include "stdafx.h"
#include "ProtocolLogin.h"
#include "OutputMessage.h"
#include "Bans.h"
#include "Dispatcher.h"
#include <functional>
#include "Account.h"

#include "DebugNew.h"

namespace Net {

void ProtocolLogin::OnRecvFirstMessage(NetworkMessage& message)
{
    // if not game == running return

    message.Skip(2);    // Client OS
    uint16_t version = message.Get<uint16_t>();

    Auth::BanInfo banInfo;
    std::shared_ptr<Connection> conn = GetConnection();
    if (Auth::BanManager::Instance.IsIpBanned(conn->GetIP(), banInfo))
    {
        //        DisconnectClient()
        return;
    }

    std::string accountName = message.GetString();
    if (accountName.empty())
    {
        return;
    }

    std::string password = message.GetString();
    if (password.empty())
    {
        return;
    }

    std::string authToken = message.GetString();

    std::shared_ptr<ProtocolLogin> thisPtr = std::static_pointer_cast<ProtocolLogin>(shared_from_this());
    Asynch::Dispatcher::Instance.Add(
        Asynch::CreateTask(std::bind(
            &ProtocolLogin::GetCharacterList, thisPtr,
            accountName, password, authToken
        ))
    );
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

void ProtocolLogin::GetCharacterList(const std::string& accountName, const std::string& password,
    const std::string& token)
{
    Auth::Account account;
    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();

    Send(output);
    Disconnect();
}

}

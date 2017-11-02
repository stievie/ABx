#include "stdafx.h"
#include "ProtocolLogin.h"
#include "OutputMessage.h"
#include "Bans.h"
#include "Dispatcher.h"
#include <functional>
#include "Account.h"
#include "IOAccount.h"

#include "DebugNew.h"

namespace Net {

void ProtocolLogin::OnRecvFirstMessage(NetworkMessage& message)
{
    // if not game == running return

    message.Skip(2);    // Client OS
    uint16_t version = message.Get<uint16_t>();

    Auth::BanInfo banInfo;
    std::shared_ptr<Connection> conn = GetConnection();
    if (Auth::BanManager::Instance.IsIpBanned(conn->GetIP()))
    {
        DisconnectClient(0x0A, "IP is banned");
        return;
    }
    if (Auth::BanManager::Instance.IsIpDisabled(conn->GetIP()))
    {
        DisconnectClient(0x0A, "Too many connections from this IP");
        return;
    }

    std::string accountName = message.GetString();
    if (accountName.empty())
    {
        DisconnectClient(0x0A, "Invalid account name");
        return;
    }

    std::string password = message.GetString();
    if (password.empty())
    {
        DisconnectClient(0x0A, "Invalid password");
        return;
    }

    std::shared_ptr<ProtocolLogin> thisPtr = std::static_pointer_cast<ProtocolLogin>(shared_from_this());
    Asynch::Dispatcher::Instance.Add(
        Asynch::CreateTask(std::bind(
            &ProtocolLogin::GetCharacterList, thisPtr,
            accountName, password
        ))
    );
}

void ProtocolLogin::GetCharacterList(const std::string& accountName, const std::string& password)
{
    Account account;
    bool res = DB::IOAccount::LoginServerAuth(accountName, password, account);
    if (!res)
    {
        DisconnectClient(0x0A, "Account name or password not correct");
        Auth::BanManager::Instance.AddLoginAttempt(GetIP(), false);
        return;
    }

    Auth::BanManager::Instance.AddLoginAttempt(GetIP(), true);

    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();

    output->AddByte(0x64);
    for (const AccountCharacter& character : account.characters_)
    {
        output->Add<uint32_t>(character.id);
        output->Add<uint16_t>(character.level);
        output->AddString(character.name);
    }

    Send(output);
    Disconnect();
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

}

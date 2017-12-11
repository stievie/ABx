#include "stdafx.h"
#include "ProtocolLogin.h"
#include "OutputMessage.h"
#include "Bans.h"
#include "Dispatcher.h"
#include <functional>
#include "Account.h"
#include "IOAccount.h"
#include "ConfigManager.h"
#include <AB/ProtocolCodes.h>
#include "PlayerManager.h"

#include "DebugNew.h"

namespace Net {

void ProtocolLogin::OnRecvFirstMessage(NetworkMessage& message)
{
    // if not game == running return

    message.Skip(2);    // Client OS
    uint16_t version = message.Get<uint16_t>();
    if (version != AB::PROTOCOL_VERSION)
    {
        DisconnectClient(AB::Errors::WrongProtocolVersion);
        return;
    }

    Auth::BanInfo banInfo;
    std::shared_ptr<Connection> conn = GetConnection();
    if (Auth::BanManager::Instance.IsIpBanned(conn->GetIP()))
    {
        DisconnectClient(AB::Errors::IPBanned);
        return;
    }
    if (Auth::BanManager::Instance.IsIpDisabled(conn->GetIP()))
    {
        DisconnectClient(AB::Errors::TooManyConnectionsFromThisIP);
        return;
    }

    std::string accountName = message.GetString();
    if (accountName.empty())
    {
        DisconnectClient(AB::Errors::InvalidAccountName);
        return;
    }

    std::string password = message.GetString();
    if (password.empty())
    {
        DisconnectClient(AB::Errors::InvalidPassword);
        return;
    }

    std::shared_ptr<ProtocolLogin> thisPtr = std::static_pointer_cast<ProtocolLogin>(shared_from_this());
    Asynch::Dispatcher::Instance.Add(
        Asynch::CreateTask(std::bind(
            &ProtocolLogin::SendCharacterList, thisPtr,
            accountName, password
        ))
    );
}

void ProtocolLogin::SendKeyExchange()
{

}

void ProtocolLogin::SendCharacterList(const std::string& accountName, const std::string& password)
{
    Account account;
    bool res = DB::IOAccount::LoginServerAuth(accountName, password, account);
    if (!res)
    {
        DisconnectClient(AB::Errors::NamePasswordMismatch);
        Auth::BanManager::Instance.AddLoginAttempt(GetIP(), false);
        return;
    }
    const auto player = Game::PlayerManager::Instance.GetPlayerByAccountId(account.id_);
    if (player)
    {
        DisconnectClient(AB::Errors::AlreadyLoggedIn);
        Auth::BanManager::Instance.AddLoginAttempt(GetIP(), false);
        return;
    }

    Auth::BanManager::Instance.AddLoginAttempt(GetIP(), true);

    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();

    output->AddByte(AB::LoginProtocol::CharacterList);
    output->AddString(ConfigManager::Instance[ConfigManager::GameHost].GetString());
    output->Add<uint16_t>(static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::GamePort].GetInt()));
    output->AddByte(static_cast<uint8_t>(account.characters_.size()));
    for (const AccountCharacter& character : account.characters_)
    {
        output->Add<uint32_t>(character.id);
        output->Add<uint16_t>(character.level);
        output->AddString(character.name);
        output->AddString(character.lastMap);
    }

    Send(output);
    Disconnect();
}

void ProtocolLogin::DisconnectClient(uint8_t error)
{
    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();
    if (output)
    {
        output->AddByte(AB::LoginProtocol::LoginError);
        output->AddByte(error);
        Send(output);
    }
    Disconnect();
}

}

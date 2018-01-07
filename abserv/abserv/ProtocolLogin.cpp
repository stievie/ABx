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
#include "SkillManager.h"
#include "IOPlayer.h"
#include "IOGame.h"
#include "GameManager.h"

#include "DebugNew.h"

namespace Net {

void ProtocolLogin::OnRecvFirstMessage(NetworkMessage& message)
{
    if (Game::GameManager::Instance.GetState() != Game::GameManager::ManagerStateRunning)
    {
        Disconnect();
        return;
    }

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

    uint8_t recvByte = message.GetByte();
    switch (recvByte)
    {
    case AB::LoginProtocol::LoginLogin:
        HandleLoginPacket(message);
        break;
    case AB::LoginProtocol::LoginCreateAccount:
        HandleCreateAccountPacket(message);
        break;
    case AB::LoginProtocol::LoginCreateCharacter:
        HandleCreateCharacterPacket(message);
        break;
    case AB::LoginProtocol::LoginDeleteCharacter:
        HandleDeleteCharacterPacket(message);
        break;
    }
}

void ProtocolLogin::HandleLoginPacket(NetworkMessage& message)
{
    std::string accountName = message.GetStringEncrypted();
    if (accountName.empty())
    {
        DisconnectClient(AB::Errors::InvalidAccountName);
        return;
    }
    std::string password = message.GetStringEncrypted();
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

void ProtocolLogin::HandleCreateAccountPacket(NetworkMessage& message)
{
    std::string accountName = message.GetStringEncrypted();
    if (accountName.empty())
    {
        DisconnectClient(AB::Errors::InvalidAccountName);
        return;
    }
    if (accountName.length() < 6 || accountName.length() > 32)
    {
        DisconnectClient(AB::Errors::InvalidAccountName);
        return;
    }
    std::string password = message.GetStringEncrypted();
    if (password.empty())
    {
        DisconnectClient(AB::Errors::InvalidPassword);
        return;
    }
    std::string email = message.GetStringEncrypted();
    if (password.empty())
    {
        DisconnectClient(AB::Errors::InvalidEmail);
        return;
    }
    std::string accKey = message.GetStringEncrypted();
    if (accKey.empty())
    {
        DisconnectClient(AB::Errors::InvalidAccountKey);
        return;
    }

    std::shared_ptr<ProtocolLogin> thisPtr = std::static_pointer_cast<ProtocolLogin>(shared_from_this());
    Asynch::Dispatcher::Instance.Add(
        Asynch::CreateTask(std::bind(
            &ProtocolLogin::CreateAccount, thisPtr,
            accountName, password,
            email, accKey
        ))
    );
}

void ProtocolLogin::HandleCreateCharacterPacket(NetworkMessage& message)
{
    std::string accountName = message.GetStringEncrypted();
    if (accountName.empty())
    {
        DisconnectClient(AB::Errors::InvalidAccountName);
        return;
    }
    std::string password = message.GetStringEncrypted();
    if (password.empty())
    {
        DisconnectClient(AB::Errors::InvalidPassword);
        return;
    }
    std::string charName = message.GetStringEncrypted();
    if (charName.empty())
    {
        DisconnectClient(AB::Errors::InvalidCharacterName);
        return;
    }
    if (charName.length() < 6 || charName.length() > 20)
    {
        DisconnectClient(AB::Errors::InvalidCharacterName);
        return;
    }

    Game::PlayerSex sex = static_cast<Game::PlayerSex>(message.GetByte());
    if (sex == Game::PlayerSexUnknown || sex > Game::PlayerSexMale)
    {
        DisconnectClient(AB::Errors::InvalidPlayerSex);
        return;
    }
    std::string prof = message.GetString();
    if (prof.empty())
    {
        DisconnectClient(AB::Errors::InvalidProfession);
        return;
    }
    uint32_t profId = Game::SkillManager::Instance.GetProfessionId(prof);
    if (profId == 0)
    {
        DisconnectClient(AB::Errors::InvalidProfession);
        return;
    }
    bool isPvp = message.GetByte() != 0;

    std::shared_ptr<ProtocolLogin> thisPtr = std::static_pointer_cast<ProtocolLogin>(shared_from_this());
    Asynch::Dispatcher::Instance.Add(
        Asynch::CreateTask(std::bind(
            &ProtocolLogin::CreatePlayer, thisPtr,
            accountName, password,
            charName, prof, sex, isPvp
        ))
    );
}

void ProtocolLogin::HandleDeleteCharacterPacket(NetworkMessage& message)
{
    std::string accountName = message.GetStringEncrypted();
    if (accountName.empty())
    {
        DisconnectClient(AB::Errors::InvalidAccountName);
        return;
    }
    std::string password = message.GetStringEncrypted();
    if (password.empty())
    {
        DisconnectClient(AB::Errors::InvalidPassword);
        return;
    }
    uint32_t charId = message.Get<uint32_t>();
    if (charId == 0)
    {
        DisconnectClient(AB::Errors::InvalidCharacter);
        return;
    }

    std::shared_ptr<ProtocolLogin> thisPtr = std::static_pointer_cast<ProtocolLogin>(shared_from_this());
    Asynch::Dispatcher::Instance.Add(
        Asynch::CreateTask(std::bind(
            &ProtocolLogin::DeletePlayer, thisPtr,
            accountName, password,
            charId
        ))
    );
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

    LOG_INFO << Utils::ConvertIPToString(GetIP()) << ": "
        << accountName << " logged in" << std::endl;

    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();

    output->AddByte(AB::LoginProtocol::CharacterList);
    output->AddString(ConfigManager::Instance[ConfigManager::GameHost].GetString());
    output->Add<uint16_t>(static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::GamePort].GetInt()));
    output->AddByte(static_cast<uint8_t>(account.characters_.size()));
    for (const AccountCharacter& character : account.characters_)
    {
        output->Add<uint32_t>(character.id);
        output->Add<uint16_t>(character.level);
        output->AddStringEncrypted(character.name);
        output->AddStringEncrypted(character.prof);
        output->AddStringEncrypted(character.prof2);
        output->AddStringEncrypted(character.lastMap);
    }

    Send(output);
    Disconnect();
}

void ProtocolLogin::CreateAccount(const std::string& accountName, const std::string& password,
    const std::string& email, const std::string& accKey)
{
    DB::IOAccount::CreateAccountResult res = DB::IOAccount::CreateAccount(accountName, password, email, accKey);

    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();

    if (res == DB::IOAccount::ResultOK)
    {
        output->AddByte(AB::LoginProtocol::CreateAccountSuccess);
    }
    else
    {
        output->AddByte(AB::LoginProtocol::CreateAccountError);
        switch (res)
        {
        case DB::IOAccount::ResultNameExists:
            output->AddByte(AB::Errors::AccountNameExists);
            break;
        case DB::IOAccount::ResultInvalidAccountKey:
            output->AddByte(AB::Errors::InvalidAccountKey);
            break;
        default:
            output->AddByte(AB::Errors::UnknownError);
            break;
        }
    }

    LOG_INFO << Utils::ConvertIPToString(GetIP()) << ": "
        << accountName << " created" << std::endl;

    Send(output);
    Disconnect();
}

void ProtocolLogin::CreatePlayer(const std::string& accountName, const std::string& password,
    std::string& name, const std::string& prof, Game::PlayerSex sex, bool isPvp)
{
    Account account;
    bool authRes = DB::IOAccount::LoginServerAuth(accountName, password, account);
    if (!authRes)
    {
        DisconnectClient(AB::Errors::NamePasswordMismatch);
        Auth::BanManager::Instance.AddLoginAttempt(GetIP(), false);
        return;
    }

    DB::IOPlayer::CreatePlayerResult res = DB::IOPlayer::CreatePlayer(
        account.id_, name, prof, sex, isPvp
    );

    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();

    if (res == DB::IOPlayer::ResultOK)
    {
        output->AddByte(AB::LoginProtocol::CreatePlayerSuccess);
        output->AddStringEncrypted(name);
        output->AddStringEncrypted(DB::IOGame::GetLandingGame());
    }
    else
    {
        output->AddByte(AB::LoginProtocol::CreatePlayerError);
        switch (res)
        {
        case DB::IOPlayer::ResultNameExists:
            output->AddByte(AB::Errors::PlayerNameExists);
            break;
        case DB::IOPlayer::ResultInvalidAccount:
            output->AddByte(AB::Errors::InvalidAccount);
            break;
        default:
            output->AddByte(AB::Errors::UnknownError);
            break;
        }
    }

    LOG_INFO << Utils::ConvertIPToString(GetIP()) << ": "
        << accountName << " created character " << name << std::endl;

    Send(output);
    Disconnect();
}

void ProtocolLogin::DeletePlayer(const std::string& accountName, const std::string& password,
    uint32_t playerId)
{
    Account account;
    bool authRes = DB::IOAccount::LoginServerAuth(accountName, password, account);
    if (!authRes)
    {
        DisconnectClient(AB::Errors::NamePasswordMismatch);
        Auth::BanManager::Instance.AddLoginAttempt(GetIP(), false);
        return;
    }

    bool res = DB::IOPlayer::DeletePlayer(
        account.id_, playerId
    );

    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();

    if (res)
    {
        output->AddByte(AB::LoginProtocol::DeletePlayerSuccess);
    }
    else
    {
        output->AddByte(AB::LoginProtocol::DeletePlayerError);
        output->AddByte(AB::Errors::InvalidCharacter);
    }

    LOG_INFO << Utils::ConvertIPToString(GetIP()) << ": "
        << accountName << " deleted character with ID " << playerId << std::endl;

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

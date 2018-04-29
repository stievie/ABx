#include "stdafx.h"
#include "ProtocolLogin.h"
#include "OutputMessage.h"
#include "Bans.h"
#include "Dispatcher.h"
#include <functional>
#include "IOAccount.h"
#include "ConfigManager.h"
#include <AB/ProtocolCodes.h>
#include "PlayerManager.h"
#include "SkillManager.h"
#include "IOPlayer.h"
#include "IOGame.h"
#include "GameManager.h"
#include <AB/Entities/Account.h>
#include <AB/Entities/Game.h>

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
    uint32_t clientIp = conn->GetIP();
    if (Auth::BanManager::Instance.IsIpBanned(clientIp))
    {
        DisconnectClient(AB::Errors::IPBanned);
        return;
    }
    if (Auth::BanManager::Instance.IsIpDisabled(clientIp))
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
    case AB::LoginProtocol::LoginAddAccountKey:
        HandleAddAccountKeyPacket(message);
        break;
    case AB::LoginProtocol::LoginGetGameList:
        HandleGetGameListPacket(message);
        break;
    default:
        LOG_ERROR << Utils::ConvertIPToString(clientIp) << ": Unknown packet header: 0x" <<
            std::hex << static_cast<uint16_t>(recvByte) << std::dec << std::endl;
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
    std::transform(accKey.begin(), accKey.end(), accKey.begin(), ::tolower);

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

    AB::Entities::CharacterSex sex = static_cast<AB::Entities::CharacterSex>(message.GetByte());
    if (sex < AB::Entities::CharacterSex::CharacterSexFemale || sex > AB::Entities::CharacterSex::CharacterSexMale)
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
    const std::string charUuid = message.GetStringEncrypted();
    if (charUuid.empty() || uuids::uuid(charUuid).nil())
    {
        DisconnectClient(AB::Errors::InvalidCharacter);
        return;
    }

    std::shared_ptr<ProtocolLogin> thisPtr = std::static_pointer_cast<ProtocolLogin>(shared_from_this());
    Asynch::Dispatcher::Instance.Add(
        Asynch::CreateTask(std::bind(
            &ProtocolLogin::DeletePlayer, thisPtr,
            accountName, password,
            charUuid
        ))
    );
}

void ProtocolLogin::HandleAddAccountKeyPacket(NetworkMessage& message)
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
    std::string accKey = message.GetStringEncrypted();
    if (accKey.empty())
    {
        DisconnectClient(AB::Errors::InvalidAccountKey);
        return;
    }
    std::transform(accKey.begin(), accKey.end(), accKey.begin(), ::tolower);

    std::shared_ptr<ProtocolLogin> thisPtr = std::static_pointer_cast<ProtocolLogin>(shared_from_this());
    Asynch::Dispatcher::Instance.Add(
        Asynch::CreateTask(std::bind(
            &ProtocolLogin::AddAccountKey, thisPtr,
            accountName, password,
            accKey
        ))
    );
}

void ProtocolLogin::HandleGetGameListPacket(NetworkMessage& message)
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
            &ProtocolLogin::SendGameList, thisPtr,
            accountName, password
        ))
    );
}

void ProtocolLogin::SendCharacterList(const std::string& accountName, const std::string& password)
{
    AB::Entities::Account account;
    bool res = IO::IOAccount::LoginServerAuth(accountName, password, account);
    if (!res)
    {
        DisconnectClient(AB::Errors::NamePasswordMismatch);
        Auth::BanManager::Instance.AddLoginAttempt(GetIP(), false);
        return;
    }
    const auto player = Game::PlayerManager::Instance.GetPlayerByAccountId(account.uuid);
    if (player)
    {
        DisconnectClient(AB::Errors::AlreadyLoggedIn);
        Auth::BanManager::Instance.AddLoginAttempt(GetIP(), false);
        return;
    }

    Auth::BanManager::Instance.AddLoginAttempt(GetIP(), true);

    LOG_INFO << Utils::ConvertIPToString(GetIP(), true) << ": " << accountName << " logged in" << std::endl;
    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();

    output->AddByte(AB::LoginProtocol::CharacterList);
    output->AddString(ConfigManager::Instance[ConfigManager::GameHost].GetString());
    output->Add<uint16_t>(static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::GamePort].GetInt()));
    output->Add<uint16_t>(static_cast<uint16_t>(account.charSlots));
    output->Add<uint16_t>(static_cast<uint16_t>(account.characterUuids.size()));
    for (const std::string& characterUuid : account.characterUuids)
    {
        AB::Entities::Character character;
        character.uuid = characterUuid;
        if (!IO::IOPlayer::LoadCharacter(character))
            continue;

        output->AddStringEncrypted(character.uuid);
        output->Add<uint8_t>(character.level);
        output->AddStringEncrypted(character.name);
        output->AddStringEncrypted(character.profession);
        output->AddStringEncrypted(character.profession2);
        output->AddByte(static_cast<uint8_t>(character.sex));
        output->AddStringEncrypted(character.lastMap);
    }

    Send(output);
    Disconnect();
}

void ProtocolLogin::SendGameList(const std::string& accountName, const std::string& password)
{
    AB::Entities::Account account;
    bool res = IO::IOAccount::LoginServerAuth(accountName, password, account);
    if (!res)
    {
        DisconnectClient(AB::Errors::NamePasswordMismatch);
        Auth::BanManager::Instance.AddLoginAttempt(GetIP(), false);
        return;
    }
    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();
    output->AddByte(AB::LoginProtocol::GameList);
    const std::vector<AB::Entities::Game> games = IO::IOGame::GetGameList();
    output->Add<uint16_t>(static_cast<uint16_t>(games.size()));
    for (const AB::Entities::Game& game : games)
    {
        if (game.type == AB::Entities::GameType::GameTypeOutpost)
        {
            output->AddStringEncrypted(game.uuid);
            output->AddStringEncrypted(game.name);
            output->AddByte(static_cast<uint8_t>(game.type));
        }
    }

    Send(output);
    Disconnect();
}

void ProtocolLogin::CreateAccount(const std::string& accountName, const std::string& password,
    const std::string& email, const std::string& accKey)
{
    IO::IOAccount::Result res = IO::IOAccount::CreateAccount(accountName, password, email, accKey);

    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();

    if (res == IO::IOAccount::ResultOK)
    {
        output->AddByte(AB::LoginProtocol::CreateAccountSuccess);
    }
    else
    {
        output->AddByte(AB::LoginProtocol::CreateAccountError);
        switch (res)
        {
        case IO::IOAccount::ResultNameExists:
            output->AddByte(AB::Errors::AccountNameExists);
            break;
        case IO::IOAccount::ResultInvalidAccountKey:
            output->AddByte(AB::Errors::InvalidAccountKey);
            break;
        default:
            output->AddByte(AB::Errors::UnknownError);
            break;
        }
    }

    Send(output);
    Disconnect();
}

void ProtocolLogin::CreatePlayer(const std::string& accountName, const std::string& password,
    std::string& name, const std::string& prof, AB::Entities::CharacterSex sex, bool isPvp)
{
    AB::Entities::Account account;
    bool authRes = IO::IOAccount::LoginServerAuth(accountName, password, account);
    if (!authRes)
    {
        DisconnectClient(AB::Errors::NamePasswordMismatch);
        Auth::BanManager::Instance.AddLoginAttempt(GetIP(), false);
        return;
    }

    IO::IOPlayer::CreatePlayerResult res = IO::IOPlayer::CreatePlayer(
        account.uuid, name, prof, sex, isPvp
    );

    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();

    if (res == IO::IOPlayer::ResultOK)
    {
        output->AddByte(AB::LoginProtocol::CreatePlayerSuccess);
        output->AddStringEncrypted(name);
        output->AddStringEncrypted(IO::IOGame::GetLandingGame());
    }
    else
    {
        output->AddByte(AB::LoginProtocol::CreatePlayerError);
        switch (res)
        {
        case IO::IOPlayer::ResultNameExists:
            output->AddByte(AB::Errors::PlayerNameExists);
            break;
        case IO::IOPlayer::ResultInvalidAccount:
            output->AddByte(AB::Errors::InvalidAccount);
            break;
        case IO::IOPlayer::ResultNoMoreCharSlots:
            output->AddByte(AB::Errors::NoMoreCharSlots);
            break;
        default:
            output->AddByte(AB::Errors::UnknownError);
            break;
        }
    }

    Send(output);
    Disconnect();
}

void ProtocolLogin::AddAccountKey(const std::string& accountName, const std::string& password,
    const std::string& accKey)
{
    AB::Entities::Account account;
    bool authRes = IO::IOAccount::LoginServerAuth(accountName, password, account);
    if (!authRes)
    {
        DisconnectClient(AB::Errors::NamePasswordMismatch);
        Auth::BanManager::Instance.AddLoginAttempt(GetIP(), false);
        return;
    }

    IO::IOAccount::Result res = IO::IOAccount::AddAccountKey(accountName, password, accKey);
    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();

    if (res == IO::IOAccount::ResultOK)
    {
        output->AddByte(AB::LoginProtocol::AddAccountKeySuccess);
    }
    else
    {
        output->AddByte(AB::LoginProtocol::AddAccountKeyError);
        switch (res)
        {
        case IO::IOAccount::ResultNameExists:
            output->AddByte(AB::Errors::AccountNameExists);
            break;
        case IO::IOAccount::ResultInvalidAccountKey:
            output->AddByte(AB::Errors::InvalidAccountKey);
            break;
        case IO::IOAccount::ResultInvalidAccount:
            output->AddByte(AB::Errors::InvalidAccount);
            break;
        default:
            output->AddByte(AB::Errors::UnknownError);
            break;
        }
    }

    Send(output);
    Disconnect();
}

void ProtocolLogin::DeletePlayer(const std::string& accountName, const std::string& password,
    const std::string& playerUuid)
{
    AB::Entities::Account account;
    bool authRes = IO::IOAccount::LoginServerAuth(accountName, password, account);
    if (!authRes)
    {
        DisconnectClient(AB::Errors::NamePasswordMismatch);
        Auth::BanManager::Instance.AddLoginAttempt(GetIP(), false);
        return;
    }

    bool res = IO::IOPlayer::DeletePlayer(
        account.uuid, playerUuid
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
        << accountName << " deleted character with UUID " << playerUuid << std::endl;

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

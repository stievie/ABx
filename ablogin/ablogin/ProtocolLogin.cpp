#include "stdafx.h"
#include "ProtocolLogin.h"
#include "OutputMessage.h"
#include "BanManager.h"
#include "Dispatcher.h"
#include <functional>
#include "IOAccount.h"
#include <AB/ProtocolCodes.h>
#include <AB/Entities/Account.h>
#include <AB/Entities/Game.h>
#include <uuid.h>
#include "StringUtils.h"
#include "IOService.h"
#include "IOGame.h"
#include "Subsystems.h"
#include "UuidUtils.h"

namespace Net {

void ProtocolLogin::OnRecvFirstMessage(NetworkMessage& message)
{
    message.Skip(2);    // Client OS
    uint16_t version = message.Get<uint16_t>();
    if (version != AB::PROTOCOL_VERSION)
    {
        DisconnectClient(AB::Errors::WrongProtocolVersion);
        return;
    }

    std::shared_ptr<Connection> conn = GetConnection();
    uint32_t clientIp = conn->GetIP();
    auto* banMan = GetSubsystem<Auth::BanManager>();
    if (banMan->IsIpBanned(clientIp))
    {
        DisconnectClient(AB::Errors::IPBanned);
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
    case AB::LoginProtocol::LoginGetOutposts:
        HandleGetOutpostsPacket(message);
        break;
    case AB::LoginProtocol::LoginGetGameServers:
        HandleGetServersPacket(message);
        break;
    default:
        LOG_ERROR << Utils::ConvertIPToString(clientIp) << ": Unknown packet header: 0x" <<
            std::hex << static_cast<uint16_t>(recvByte) << std::dec << std::endl;
        break;
    }
}

void ProtocolLogin::HandleLoginPacket(NetworkMessage& message)
{
    const std::string accountName = message.GetStringEncrypted();
    if (accountName.empty())
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Invalid account name " << accountName << std::endl;
#endif
        DisconnectClient(AB::Errors::InvalidAccountName);
        return;
    }
    const std::string password = message.GetStringEncrypted();
    if (password.empty())
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Invalid password " << password << std::endl;
#endif
        DisconnectClient(AB::Errors::InvalidPassword);
        return;
    }

    std::shared_ptr<ProtocolLogin> thisPtr = std::static_pointer_cast<ProtocolLogin>(shared_from_this());
    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(std::bind(
            &ProtocolLogin::AuthenticateSendCharacterList, thisPtr,
            accountName, password
        ))
    );
}

void ProtocolLogin::HandleCreateAccountPacket(NetworkMessage& message)
{
    const std::string accountName = message.GetStringEncrypted();
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
    const std::string password = message.GetStringEncrypted();
    if (password.empty())
    {
        DisconnectClient(AB::Errors::InvalidPassword);
        return;
    }
    const std::string email = message.GetStringEncrypted();
    if (password.empty())
    {
        DisconnectClient(AB::Errors::InvalidEmail);
        return;
    }
    const std::string accKey = message.GetStringEncrypted();
    if (accKey.empty())
    {
        DisconnectClient(AB::Errors::InvalidAccountKey);
        return;
    }

    std::shared_ptr<ProtocolLogin> thisPtr = std::static_pointer_cast<ProtocolLogin>(shared_from_this());
    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(std::bind(
            &ProtocolLogin::CreateAccount, thisPtr,
            accountName, password,
            email, accKey
        ))
    );
}

void ProtocolLogin::HandleCreateCharacterPacket(NetworkMessage& message)
{
    const std::string accountUuid = message.GetStringEncrypted();
    if (accountUuid.empty())
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Invalid account UUID " << accountUuid << std::endl;
#endif
        DisconnectClient(AB::Errors::InvalidAccount);
        return;
    }
    const std::string password = message.GetStringEncrypted();
    if (password.empty())
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Invalid password " << password << std::endl;
#endif
        DisconnectClient(AB::Errors::InvalidPassword);
        return;
    }
    const std::string charName = message.GetStringEncrypted();
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

    uint32_t modelIndex = message.Get<uint32_t>();

    AB::Entities::CharacterSex sex = static_cast<AB::Entities::CharacterSex>(message.GetByte());
    if (sex < AB::Entities::CharacterSex::CharacterSexFemale || sex > AB::Entities::CharacterSex::CharacterSexMale)
    {
        DisconnectClient(AB::Errors::InvalidPlayerSex);
        return;
    }
    const std::string prof = message.GetString();
    if (Utils::Uuid::IsEmpty(prof))
    {
        DisconnectClient(AB::Errors::InvalidProfession);
        return;
    }
    bool isPvp = message.GetByte() != 0;

    std::shared_ptr<ProtocolLogin> thisPtr = std::static_pointer_cast<ProtocolLogin>(shared_from_this());
    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(std::bind(
            &ProtocolLogin::CreatePlayer, thisPtr,
            accountUuid, password,
            charName, prof, modelIndex, sex, isPvp
        ))
    );
}

void ProtocolLogin::HandleDeleteCharacterPacket(NetworkMessage& message)
{
    const std::string accountUuid = message.GetStringEncrypted();
    if (accountUuid.empty())
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Invalid account UUID " << accountUuid << std::endl;
#endif
        DisconnectClient(AB::Errors::InvalidAccount);
        return;
    }
    const std::string password = message.GetStringEncrypted();
    if (password.empty())
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Invalid password " << password << std::endl;
#endif
        DisconnectClient(AB::Errors::InvalidPassword);
        return;
    }
    const std::string charUuid = message.GetStringEncrypted();
    if (Utils::Uuid::IsEmpty(charUuid))
    {
        DisconnectClient(AB::Errors::InvalidCharacter);
        return;
    }

    std::shared_ptr<ProtocolLogin> thisPtr = std::static_pointer_cast<ProtocolLogin>(shared_from_this());
    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(std::bind(
            &ProtocolLogin::DeletePlayer, thisPtr,
            accountUuid, password,
            charUuid
        ))
    );
}

void ProtocolLogin::HandleAddAccountKeyPacket(NetworkMessage& message)
{
    const std::string accountUuid = message.GetStringEncrypted();
    if (accountUuid.empty())
    {
        DisconnectClient(AB::Errors::InvalidAccount);
        return;
    }
    const std::string token = message.GetStringEncrypted();
    if (token.empty())
    {
        DisconnectClient(AB::Errors::InvalidPassword);
        return;
    }
    const std::string accKey = message.GetStringEncrypted();
    if (accKey.empty())
    {
        DisconnectClient(AB::Errors::InvalidAccountKey);
        return;
    }

    std::shared_ptr<ProtocolLogin> thisPtr = std::static_pointer_cast<ProtocolLogin>(shared_from_this());
    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(std::bind(
            &ProtocolLogin::AddAccountKey, thisPtr,
            accountUuid, token,
            accKey
        ))
    );
}

void ProtocolLogin::HandleGetOutpostsPacket(NetworkMessage& message)
{
    const std::string accountUuid = message.GetStringEncrypted();
    if (accountUuid.empty())
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Invalid account UUID " << accountUuid << std::endl;
#endif
        DisconnectClient(AB::Errors::InvalidAccount);
        return;
    }
    const std::string token = message.GetStringEncrypted();
    if (token.empty())
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Invalid token" << std::endl;
#endif
        DisconnectClient(AB::Errors::InvalidPassword);
        return;
    }

    std::shared_ptr<ProtocolLogin> thisPtr = std::static_pointer_cast<ProtocolLogin>(shared_from_this());
    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(std::bind(
            &ProtocolLogin::SendOutposts, thisPtr,
            accountUuid, token
        ))
    );
}

void ProtocolLogin::HandleGetServersPacket(NetworkMessage& message)
{
#ifdef DEBUG_NET
    LOG_DEBUG << "Sending server list" << std::endl;
#endif
    const std::string accountUuid = message.GetStringEncrypted();
    if (accountUuid.empty())
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Invalid account UUID " << accountUuid << std::endl;
#endif
        DisconnectClient(AB::Errors::InvalidAccount);
        return;
    }
    const std::string token = message.GetStringEncrypted();
    if (token.empty())
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Invalid token" << std::endl;
#endif
        DisconnectClient(AB::Errors::InvalidPassword);
        return;
    }

    std::shared_ptr<ProtocolLogin> thisPtr = std::static_pointer_cast<ProtocolLogin>(shared_from_this());
    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(std::bind(
            &ProtocolLogin::SendServers, thisPtr,
            accountUuid, token
        ))
    );
}

void ProtocolLogin::AuthenticateSendCharacterList(const std::string& accountName, const std::string& password)
{
    AB::Entities::Account account;
    account.name = accountName;
    IO::IOAccount::PasswordAuthResult res = IO::IOAccount::PasswordAuth(password, account);
    auto banMan = GetSubsystem<Auth::BanManager>();
    switch (res)
    {
    case IO::IOAccount::PasswordAuthResult::InvalidAccount:
        DisconnectClient(AB::Errors::InvalidAccount);
        banMan->AddLoginAttempt(GetIP(), false);
        return;
    case IO::IOAccount::PasswordAuthResult::PasswordMismatch:
        DisconnectClient(AB::Errors::NamePasswordMismatch);
        banMan->AddLoginAttempt(GetIP(), false);
        return;
    case IO::IOAccount::PasswordAuthResult::AlreadyLoggedIn:
        DisconnectClient(AB::Errors::AlreadyLoggedIn);
        banMan->AddLoginAttempt(GetIP(), false);
        break;
    case IO::IOAccount::PasswordAuthResult::InternalError:
        DisconnectClient(AB::Errors::UnknownError);
        break;
    default:
        break;
    }

    AB::Entities::Service gameServer;
    if (!IO::IOService::GetService(
        AB::Entities::ServiceTypeGameServer,
        gameServer,
        account.currentServerUuid))
    {
        DisconnectClient(AB::Errors::AllServersFull);
        return;
    }

    AB::Entities::Service fileServer;
    if (!IO::IOService::GetService(
        AB::Entities::ServiceTypeFileServer,
        fileServer))
    {
        DisconnectClient(AB::Errors::AllServersFull);
        return;
    }

    banMan->AddLoginAttempt(GetIP(), true);

    LOG_INFO << Utils::ConvertIPToString(GetIP(), true) << ": " << accountName << " logged in" << std::endl;

    auto output = OutputMessagePool::GetOutputMessage();

    output->AddByte(AB::LoginProtocol::CharacterList);

    output->AddStringEncrypted(account.uuid);
    output->AddStringEncrypted(account.authToken);

    output->AddString(gameServer.host);
    output->Add<uint16_t>(gameServer.port);
    output->AddString(fileServer.host);
    output->Add<uint16_t>(fileServer.port);

    const std::string landingGame = IO::IOGame::GetLandingGameUuid();
    output->Add<uint16_t>(static_cast<uint16_t>(account.charSlots));
    output->Add<uint16_t>(static_cast<uint16_t>(account.characterUuids.size()));
    for (const std::string& characterUuid : account.characterUuids)
    {
        AB::Entities::Character character;
        character.uuid = characterUuid;
        if (!IO::IOAccount::LoadCharacter(character))
            continue;

        output->AddStringEncrypted(character.uuid);
        output->Add<uint8_t>(character.level);
        output->AddStringEncrypted(character.name);
        output->AddStringEncrypted(character.profession);
        output->AddStringEncrypted(character.profession2);
        output->AddByte(static_cast<uint8_t>(character.sex));
        output->Add<uint32_t>(character.modelIndex);
        if (!Utils::Uuid::IsEmpty(character.lastOutpostUuid))
            output->AddStringEncrypted(character.lastOutpostUuid);
        else
            output->AddStringEncrypted(landingGame);
    }

    Send(output);
    Disconnect();
}

void ProtocolLogin::SendOutposts(const std::string& accountUuid, const std::string& token)
{
    AB::Entities::Account account;
    account.uuid = accountUuid;
    bool res = IO::IOAccount::TokenAuth(token, account);
    if (!res)
    {
        DisconnectClient(AB::Errors::TokenAuthFailure);
        return;
    }

    auto output = OutputMessagePool::GetOutputMessage();
    output->AddByte(AB::LoginProtocol::OutpostList);
    const std::vector<AB::Entities::Game> games = IO::IOGame::GetGameList(AB::Entities::GameType::GameTypeOutpost);
    output->Add<uint16_t>(static_cast<uint16_t>(games.size()));
    for (const AB::Entities::Game& game : games)
    {
        if (game.type == AB::Entities::GameType::GameTypeOutpost)
        {
            output->AddStringEncrypted(game.uuid);
            output->AddStringEncrypted(game.name);
            output->AddByte(static_cast<uint8_t>(game.type));
            output->AddByte(game.partySize);
            output->Add<int32_t>(game.mapCoordX);
            output->Add<int32_t>(game.mapCoordY);
        }
    }

    Send(output);
    Disconnect();
}

void ProtocolLogin::SendServers(const std::string& accountUuid, const std::string& token)
{
    AB::Entities::Account account;
    account.uuid = accountUuid;
    bool res = IO::IOAccount::TokenAuth(token, account);
    if (!res)
    {
        DisconnectClient(AB::Errors::TokenAuthFailure);
        return;
    }

    auto output = OutputMessagePool::GetOutputMessage();
    output->AddByte(AB::LoginProtocol::ServerList);

    std::vector<AB::Entities::Service> services;
    int count = IO::IOService::GetServices(AB::Entities::ServiceTypeGameServer, services);

    output->Add<uint16_t>(static_cast<uint16_t>(count));
    for (const AB::Entities::Service& service : services)
    {
        output->Add<AB::Entities::ServiceType>(service.type);
        output->AddStringEncrypted(service.uuid);
        output->AddStringEncrypted(service.host);
        output->Add<uint16_t>(service.port);
        output->AddStringEncrypted(service.location);
        output->AddStringEncrypted(service.name);
    }

    Send(output);
    Disconnect();
}

void ProtocolLogin::CreateAccount(const std::string& accountName, const std::string& password,
    const std::string& email, const std::string& accKey)
{
    IO::IOAccount::CreateAccountResult res = IO::IOAccount::CreateAccount(accountName, password, email, accKey);

    auto output = OutputMessagePool::GetOutputMessage();

    if (res == IO::IOAccount::CreateAccountResult::OK)
    {
        output->AddByte(AB::LoginProtocol::CreateAccountSuccess);
    }
    else
    {
        output->AddByte(AB::LoginProtocol::CreateAccountError);
        switch (res)
        {
        case IO::IOAccount::CreateAccountResult::NameExists:
            output->AddByte(AB::Errors::AccountNameExists);
            break;
        case IO::IOAccount::CreateAccountResult::InvalidAccountKey:
            output->AddByte(AB::Errors::InvalidAccountKey);
            break;
        case IO::IOAccount::CreateAccountResult::PasswordError:
            output->AddByte(AB::Errors::InvalidPassword);
            break;
        case IO::IOAccount::CreateAccountResult::EmailError:
            output->AddByte(AB::Errors::InvalidEmail);
            break;
        default:
            output->AddByte(AB::Errors::UnknownError);
            break;
        }
    }

    Send(output);
    Disconnect();
}

void ProtocolLogin::CreatePlayer(const std::string& accountUuid, const std::string& token,
    const std::string& name, const std::string& prof,
    uint32_t modelIndex,
    AB::Entities::CharacterSex sex, bool isPvp)
{
    AB::Entities::Account account;
    account.uuid = accountUuid;
    bool ses = IO::IOAccount::TokenAuth(token, account);
    if (!ses)
    {
        DisconnectClient(AB::Errors::TokenAuthFailure);
        return;
    }

    std::string uuid;
    IO::IOAccount::CreatePlayerResult res = IO::IOAccount::CreatePlayer(
        account.uuid, name, prof, modelIndex, sex, isPvp, uuid
    );

    auto output = OutputMessagePool::GetOutputMessage();

    if (res == IO::IOAccount::CreatePlayerResult::OK)
    {
        output->AddByte(AB::LoginProtocol::CreatePlayerSuccess);
        output->AddStringEncrypted(uuid);
        output->AddStringEncrypted(IO::IOGame::GetLandingGameUuid());
    }
    else
    {
        output->AddByte(AB::LoginProtocol::CreatePlayerError);
        switch (res)
        {
        case IO::IOAccount::CreatePlayerResult::NameExists:
            output->AddByte(AB::Errors::PlayerNameExists);
            break;
        case IO::IOAccount::CreatePlayerResult::InvalidAccount:
            output->AddByte(AB::Errors::InvalidAccount);
            break;
        case IO::IOAccount::CreatePlayerResult::NoMoreCharSlots:
            output->AddByte(AB::Errors::NoMoreCharSlots);
            break;
        case IO::IOAccount::CreatePlayerResult::InvalidProfession:
            output->AddByte(AB::Errors::InvalidProfession);
            break;
        case IO::IOAccount::CreatePlayerResult::InvalidName:
            output->AddByte(AB::Errors::InvalidCharacterName);
            break;
        default:
            output->AddByte(AB::Errors::UnknownError);
            break;
        }
    }

    Send(output);
    Disconnect();
}

void ProtocolLogin::AddAccountKey(const std::string& accountUuid, const std::string& token,
    const std::string& accKey)
{
    AB::Entities::Account account;
    account.uuid = accountUuid;
    bool authres = IO::IOAccount::TokenAuth(token, account);
    if (!authres)
    {
        DisconnectClient(AB::Errors::TokenAuthFailure);
        return;
    }

    IO::IOAccount::CreateAccountResult res = IO::IOAccount::AddAccountKey(account, accKey);
    auto output = OutputMessagePool::GetOutputMessage();

    if (res == IO::IOAccount::CreateAccountResult::OK)
    {
        output->AddByte(AB::LoginProtocol::AddAccountKeySuccess);
    }
    else
    {
        output->AddByte(AB::LoginProtocol::AddAccountKeyError);
        switch (res)
        {
        case IO::IOAccount::CreateAccountResult::NameExists:
            output->AddByte(AB::Errors::AccountNameExists);
            break;
        case IO::IOAccount::CreateAccountResult::InvalidAccountKey:
            output->AddByte(AB::Errors::InvalidAccountKey);
            break;
        case IO::IOAccount::CreateAccountResult::InvalidAccount:
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

void ProtocolLogin::DeletePlayer(const std::string& accountUuid, const std::string& token,
    const std::string& playerUuid)
{
    AB::Entities::Account account;
    account.uuid = accountUuid;
    bool authRes = IO::IOAccount::TokenAuth(token, account);
    if (!authRes)
    {
        DisconnectClient(AB::Errors::TokenAuthFailure);
        return;
    }

    bool res = IO::IOAccount::DeletePlayer(
        account.uuid, playerUuid
    );

    auto output = OutputMessagePool::GetOutputMessage();

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
        << accountUuid << " deleted character with UUID " << playerUuid << std::endl;

    Send(output);
    Disconnect();
}

void ProtocolLogin::DisconnectClient(uint8_t error)
{
    auto output = OutputMessagePool::GetOutputMessage();
    output->AddByte(AB::LoginProtocol::LoginError);
    output->AddByte(error);
    Send(output);
    Disconnect();
}

}

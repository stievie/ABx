#include "stdafx.h"
#include "ProtocolLogin.h"
#include "OutputMessage.h"
#include "Bans.h"
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
    auto banMan = GetSubsystem<Auth::BanManager>();
    if (banMan->IsIpBanned(clientIp))
    {
        DisconnectClient(AB::Errors::IPBanned);
        return;
    }
/*    if (Auth::BanManager::Instance.IsIpDisabled(clientIp))
    {
        DisconnectClient(AB::Errors::TooManyConnectionsFromThisIP);
        return;
    }*/

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
            &ProtocolLogin::SendCharacterList, thisPtr,
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
    if (prof.empty() || uuids::uuid(prof).nil())
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
    if (charUuid.empty() || uuids::uuid(charUuid).nil())
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
    const std::string password = message.GetStringEncrypted();
    if (password.empty())
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
            accountUuid, password,
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
            &ProtocolLogin::SendOutposts, thisPtr,
            accountUuid, password
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
            &ProtocolLogin::SendServers, thisPtr,
            accountUuid, password
        ))
    );
}

void ProtocolLogin::SendCharacterList(const std::string& accountName, const std::string& password)
{
    AB::Entities::Account account;
    account.name = accountName;
    IO::IOAccount::LoginError res = IO::IOAccount::LoginServerAuth(password, account);
    auto banMan = GetSubsystem<Auth::BanManager>();
    switch (res)
    {
    case IO::IOAccount::LoginError::InvalidAccount:
        DisconnectClient(AB::Errors::InvalidAccount);
        banMan->AddLoginAttempt(GetIP(), false);
        return;
    case IO::IOAccount::LoginError::PasswordMismatch:
        DisconnectClient(AB::Errors::NamePasswordMismatch);
        banMan->AddLoginAttempt(GetIP(), false);
        return;
    case IO::IOAccount::LoginError::AlreadyLoggedIn:
        DisconnectClient(AB::Errors::AlreadyLoggedIn);
        banMan->AddLoginAttempt(GetIP(), false);
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

    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();

    output->AddByte(AB::LoginProtocol::CharacterList);

    output->AddStringEncrypted(account.uuid);

    output->AddString(gameServer.host);
    output->Add<uint16_t>(gameServer.port);
    output->AddString(fileServer.host);
    output->Add<uint16_t>(fileServer.port);

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
        output->AddStringEncrypted(character.currentMapUuid);
    }

    Send(output);
    Disconnect();
}

void ProtocolLogin::SendOutposts(const std::string& accountUuid, const std::string& password)
{
    AB::Entities::Account account;
    account.uuid = accountUuid;
    IO::IOAccount::LoginError res = IO::IOAccount::LoginServerAuth(password, account);
    switch (res)
    {
    case IO::IOAccount::LoginError::InvalidAccount:
        DisconnectClient(AB::Errors::InvalidAccount);
        return;
    case IO::IOAccount::LoginError::PasswordMismatch:
        DisconnectClient(AB::Errors::NamePasswordMismatch);
        return;
    }

    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();
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

void ProtocolLogin::SendServers(const std::string& accountUuid, const std::string& password)
{
    AB::Entities::Account account;
    account.uuid = accountUuid;
    IO::IOAccount::LoginError res = IO::IOAccount::LoginServerAuth(password, account);
    switch (res)
    {
    case IO::IOAccount::LoginError::InvalidAccount:
#ifdef DEBUG_NET
        LOG_ERROR << "Invalid account UUID " << accountUuid << std::endl;
#endif
        DisconnectClient(AB::Errors::InvalidAccount);
        return;
    case IO::IOAccount::LoginError::PasswordMismatch:
        DisconnectClient(AB::Errors::NamePasswordMismatch);
        return;
    }

    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();
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
    IO::IOAccount::Result res = IO::IOAccount::CreateAccount(accountName, password, email, accKey);

    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();

    if (res == IO::IOAccount::Result::OK)
    {
        output->AddByte(AB::LoginProtocol::CreateAccountSuccess);
    }
    else
    {
        output->AddByte(AB::LoginProtocol::CreateAccountError);
        switch (res)
        {
        case IO::IOAccount::Result::NameExists:
            output->AddByte(AB::Errors::AccountNameExists);
            break;
        case IO::IOAccount::Result::InvalidAccountKey:
            output->AddByte(AB::Errors::InvalidAccountKey);
            break;
        case IO::IOAccount::Result::PasswordError:
            output->AddByte(AB::Errors::InvalidPassword);
            break;
        case IO::IOAccount::Result::EmailError:
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

void ProtocolLogin::CreatePlayer(const std::string& accountUuid, const std::string& password,
    const std::string& name, const std::string& prof,
    uint32_t modelIndex,
    AB::Entities::CharacterSex sex, bool isPvp)
{
    AB::Entities::Account account;
    account.uuid = accountUuid;
    IO::IOAccount::LoginError authRes = IO::IOAccount::LoginServerAuth(password, account);
    auto banMan = GetSubsystem<Auth::BanManager>();
    switch (authRes)
    {
    case IO::IOAccount::LoginError::InvalidAccount:
        DisconnectClient(AB::Errors::InvalidAccount);
        banMan->AddLoginAttempt(GetIP(), false);
        return;
    case IO::IOAccount::LoginError::PasswordMismatch:
        DisconnectClient(AB::Errors::NamePasswordMismatch);
        banMan->AddLoginAttempt(GetIP(), false);
        return;
    case IO::IOAccount::LoginError::AlreadyLoggedIn:
        DisconnectClient(AB::Errors::AlreadyLoggedIn);
        banMan->AddLoginAttempt(GetIP(), false);
        break;
    }

    std::string uuid;
    IO::IOAccount::CreatePlayerResult res = IO::IOAccount::CreatePlayer(
        account.uuid, name, prof, modelIndex, sex, isPvp, uuid
    );

    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();

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

void ProtocolLogin::AddAccountKey(const std::string& accountUuid, const std::string& password,
    const std::string& accKey)
{
    AB::Entities::Account account;
    account.uuid = accountUuid;
    IO::IOAccount::LoginError authRes = IO::IOAccount::LoginServerAuth(password, account);
    auto banMan = GetSubsystem<Auth::BanManager>();
    switch (authRes)
    {
    case IO::IOAccount::LoginError::InvalidAccount:
        DisconnectClient(AB::Errors::InvalidAccount);
        banMan->AddLoginAttempt(GetIP(), false);
        return;
    case IO::IOAccount::LoginError::PasswordMismatch:
        DisconnectClient(AB::Errors::NamePasswordMismatch);
        banMan->AddLoginAttempt(GetIP(), false);
        return;
    case IO::IOAccount::LoginError::AlreadyLoggedIn:
        DisconnectClient(AB::Errors::AlreadyLoggedIn);
        banMan->AddLoginAttempt(GetIP(), false);
        break;
    }

    IO::IOAccount::Result res = IO::IOAccount::AddAccountKey(accountUuid, password, accKey);
    std::shared_ptr<OutputMessage> output = OutputMessagePool::Instance()->GetOutputMessage();

    if (res == IO::IOAccount::Result::OK)
    {
        output->AddByte(AB::LoginProtocol::AddAccountKeySuccess);
    }
    else
    {
        output->AddByte(AB::LoginProtocol::AddAccountKeyError);
        switch (res)
        {
        case IO::IOAccount::Result::NameExists:
            output->AddByte(AB::Errors::AccountNameExists);
            break;
        case IO::IOAccount::Result::InvalidAccountKey:
            output->AddByte(AB::Errors::InvalidAccountKey);
            break;
        case IO::IOAccount::Result::InvalidAccount:
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

void ProtocolLogin::DeletePlayer(const std::string& accountUuid, const std::string& password,
    const std::string& playerUuid)
{
    AB::Entities::Account account;
    account.uuid = accountUuid;
    IO::IOAccount::LoginError authRes = IO::IOAccount::LoginServerAuth(password, account);
    auto banMan = GetSubsystem<Auth::BanManager>();
    switch (authRes)
    {
    case IO::IOAccount::LoginError::InvalidAccount:
        DisconnectClient(AB::Errors::InvalidAccount);
        banMan->AddLoginAttempt(GetIP(), false);
        return;
    case IO::IOAccount::LoginError::PasswordMismatch:
        DisconnectClient(AB::Errors::NamePasswordMismatch);
        banMan->AddLoginAttempt(GetIP(), false);
        return;
    case IO::IOAccount::LoginError::AlreadyLoggedIn:
        DisconnectClient(AB::Errors::AlreadyLoggedIn);
        banMan->AddLoginAttempt(GetIP(), false);
        break;
    }

    bool res = IO::IOAccount::DeletePlayer(
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
        << accountUuid << " deleted character with UUID " << playerUuid << std::endl;

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

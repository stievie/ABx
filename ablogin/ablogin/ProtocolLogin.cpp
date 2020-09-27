/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "ProtocolLogin.h"
#include "IOAccount.h"
#include "IOGame.h"
#include "IOService.h"
#include <AB/CommonConfig.h>
#include <AB/Entities/Account.h>
#include <AB/Entities/Game.h>
#include <AB/Packets/Packet.h>
#include <AB/ProtocolCodes.h>
#include <abscommon/BanManager.h>
#include <abscommon/Dispatcher.h>
#include <abscommon/Connection.h>
#include <abscommon/OutputMessage.h>
#include <abscommon/StringUtils.h>
#include <abscommon/Subsystems.h>
#include <abscommon/UuidUtils.h>
#include <functional>
#include <uuid.h>

//#define DEBUG_NET

namespace Net {

void ProtocolLogin::OnRecvFirstMessage(NetworkMessage& message)
{
    std::shared_ptr<Connection> conn = GetConnection();
    uint32_t clientIp = conn->GetIP();

    message.Skip(2);    // Client OS

    uint16_t version = message.Get<uint16_t>();
    if (version != AB::PROTOCOL_VERSION)
    {
        LOG_WARNING << "Wrong protocol version from client " << Utils::ConvertIPToString(clientIp) << std::endl;
        DisconnectClient(AB::ErrorCodes::WrongProtocolVersion);
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
    auto packet = AB::Packets::Get<AB::Packets::Client::Login::Login>(message);
    if (packet.accountName.empty())
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Account name is empty" << std::endl;
#endif
        DisconnectClient(AB::ErrorCodes::InvalidAccountName);
        return;
    }
    if (packet.password.empty())
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Password is empty" << std::endl;
#endif
        DisconnectClient(AB::ErrorCodes::InvalidPassword);
        return;
    }

    std::shared_ptr<ProtocolLogin> thisPtr = std::static_pointer_cast<ProtocolLogin>(shared_from_this());
    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(std::bind(
            &ProtocolLogin::AuthenticateSendCharacterList, thisPtr,
            packet
        ))
    );
}

void ProtocolLogin::HandleCreateAccountPacket(NetworkMessage& message)
{
    auto packet = AB::Packets::Get<AB::Packets::Client::Login::CreateAccount>(message);
    if (packet.accountName.empty())
    {
        DisconnectClient(AB::ErrorCodes::InvalidAccountName);
        return;
    }
    if (packet.accountName.length() < ACCOUNT_NAME_MIN || packet.accountName.length() > ACCOUNT_NAME_MAX)
    {
        DisconnectClient(AB::ErrorCodes::InvalidAccountName);
        return;
    }
    if (packet.password.empty())
    {
        DisconnectClient(AB::ErrorCodes::InvalidPassword);
        return;
    }
    if (packet.password.length() < PASSWORD_LENGTH_MIN)
    {
        DisconnectClient(AB::ErrorCodes::InvalidPassword);
        return;
    }
    if (packet.password.length() > PASSWORD_LENGTH_MAX)
    {
        DisconnectClient(AB::ErrorCodes::InvalidPassword);
        return;
    }
#ifdef EMAIL_MANDATORY
    if (packet.email.empty())
    {
        DisconnectClient(AB::ErrorCodes::InvalidEmail);
        return;
    }
    if (packet.email.length() < 3)
    {
        DisconnectClient(AB::ErrorCodes::InvalidEmail);
        return;
    }
    if (packet.email.length() > EMAIL_LENGTH_MAX)
    {
        DisconnectClient(AB::ErrorCodes::InvalidEmail);
        return;
    }
#endif
    if (packet.accountKey.empty())
    {
        DisconnectClient(AB::ErrorCodes::InvalidAccountKey);
        return;
    }

    std::shared_ptr<ProtocolLogin> thisPtr = std::static_pointer_cast<ProtocolLogin>(shared_from_this());
    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(std::bind(
            &ProtocolLogin::CreateAccount, thisPtr,
            packet
        ))
    );
}

void ProtocolLogin::HandleCreateCharacterPacket(NetworkMessage& message)
{
    auto packet = AB::Packets::Get<AB::Packets::Client::Login::CreatePlayer>(message);
    if (packet.accountUuid.empty())
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Account UUID id empty" << std::endl;
#endif
        DisconnectClient(AB::ErrorCodes::InvalidAccount);
        return;
    }
    if (packet.authToken.empty())
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Auth token is empty" << std::endl;
#endif
        DisconnectClient(AB::ErrorCodes::TokenAuthFailure);
        return;
    }
    if (packet.charName.empty())
    {
        DisconnectClient(AB::ErrorCodes::InvalidCharacterName);
        return;
    }
    if (packet.charName.length() < CHARACTER_NAME_NIM || packet.charName.length() > CHARACTER_NAME_MAX)
    {
        DisconnectClient(AB::ErrorCodes::InvalidCharacterName);
        return;
    }
    if (packet.charName.find_first_of(RESTRICTED_NAME_CHARS) != std::string::npos)
    {
        DisconnectClient(AB::ErrorCodes::InvalidCharacterName);
        return;
    }

    if (packet.sex < static_cast<uint8_t>(AB::Entities::CharacterSex::Female) ||
        packet.sex > static_cast<uint8_t>(AB::Entities::CharacterSex::Male))
    {
        DisconnectClient(AB::ErrorCodes::InvalidPlayerSex);
        return;
    }
    if (Utils::Uuid::IsEmpty(packet.profUuid))
    {
        DisconnectClient(AB::ErrorCodes::InvalidProfession);
        return;
    }

    std::shared_ptr<ProtocolLogin> thisPtr = std::static_pointer_cast<ProtocolLogin>(shared_from_this());
    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(std::bind(
            &ProtocolLogin::CreatePlayer, thisPtr,
            packet
        ))
    );
}

void ProtocolLogin::HandleDeleteCharacterPacket(NetworkMessage& message)
{
    auto packet = AB::Packets::Get<AB::Packets::Client::Login::DeleteCharacter>(message);
    if (packet.accountUuid.empty())
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Account UUID is empy" << std::endl;
#endif
        DisconnectClient(AB::ErrorCodes::InvalidAccount);
        return;
    }
    if (packet.authToken.empty())
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Auth token is empty" << std::endl;
#endif
        DisconnectClient(AB::ErrorCodes::TokenAuthFailure);
        return;
    }
    if (Utils::Uuid::IsEmpty(packet.charUuid))
    {
        DisconnectClient(AB::ErrorCodes::InvalidCharacter);
        return;
    }

    std::shared_ptr<ProtocolLogin> thisPtr = std::static_pointer_cast<ProtocolLogin>(shared_from_this());
    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(std::bind(
            &ProtocolLogin::DeletePlayer, thisPtr,
            packet
        ))
    );
}

void ProtocolLogin::HandleAddAccountKeyPacket(NetworkMessage& message)
{
    auto packet = AB::Packets::Get<AB::Packets::Client::Login::AddAccountKey>(message);
    if (packet.accountUuid.empty())
    {
        DisconnectClient(AB::ErrorCodes::InvalidAccount);
        return;
    }
    if (packet.authToken.empty())
    {
        DisconnectClient(AB::ErrorCodes::TokenAuthFailure);
        return;
    }
    if (packet.accountKey.empty())
    {
        DisconnectClient(AB::ErrorCodes::InvalidAccountKey);
        return;
    }

    std::shared_ptr<ProtocolLogin> thisPtr = std::static_pointer_cast<ProtocolLogin>(shared_from_this());
    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(std::bind(
            &ProtocolLogin::AddAccountKey, thisPtr,
            packet
        ))
    );
}

void ProtocolLogin::HandleGetOutpostsPacket(NetworkMessage& message)
{
    auto packet = AB::Packets::Get<AB::Packets::Client::Login::GetOutposts>(message);
    if (packet.accountUuid.empty())
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Account UUID is empty" << std::endl;
#endif
        DisconnectClient(AB::ErrorCodes::InvalidAccount);
        return;
    }
    if (packet.authToken.empty())
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Auth Token is empty" << std::endl;
#endif
        DisconnectClient(AB::ErrorCodes::TokenAuthFailure);
        return;
    }

    std::shared_ptr<ProtocolLogin> thisPtr = std::static_pointer_cast<ProtocolLogin>(shared_from_this());
    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(std::bind(
            &ProtocolLogin::SendOutposts, thisPtr,
            packet
        ))
    );
}

void ProtocolLogin::HandleGetServersPacket(NetworkMessage& message)
{
#ifdef DEBUG_NET
    LOG_DEBUG << "Sending server list" << std::endl;
#endif
    auto packet = AB::Packets::Get<AB::Packets::Client::Login::GetServers>(message);
    if (packet.accountUuid.empty())
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Account UUID is empty" << std::endl;
#endif
        DisconnectClient(AB::ErrorCodes::InvalidAccount);
        return;
    }
    if (packet.authToken.empty())
    {
#ifdef DEBUG_NET
        LOG_ERROR << "Auth Token is empty" << std::endl;
#endif
        DisconnectClient(AB::ErrorCodes::InvalidPassword);
        return;
    }

    std::shared_ptr<ProtocolLogin> thisPtr = std::static_pointer_cast<ProtocolLogin>(shared_from_this());
    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(std::bind(
            &ProtocolLogin::SendServers, thisPtr,
            packet
        ))
    );
}

void ProtocolLogin::AuthenticateSendCharacterList(AB::Packets::Client::Login::Login request)
{
    AB::Entities::Account account;
    account.name = request.accountName;
    IO::IOAccount::PasswordAuthResult res = IO::IOAccount::PasswordAuth(request.password, account);
    auto banMan = GetSubsystem<Auth::BanManager>();
    switch (res)
    {
    case IO::IOAccount::PasswordAuthResult::InvalidAccount:
        DisconnectClient(AB::ErrorCodes::InvalidAccount);
        banMan->AddLoginAttempt(GetIP(), false);
        return;
    case IO::IOAccount::PasswordAuthResult::PasswordMismatch:
        DisconnectClient(AB::ErrorCodes::NamePasswordMismatch);
        banMan->AddLoginAttempt(GetIP(), false);
        return;
    case IO::IOAccount::PasswordAuthResult::AlreadyLoggedIn:
        DisconnectClient(AB::ErrorCodes::AlreadyLoggedIn);
        banMan->AddLoginAttempt(GetIP(), false);
        return;
    case IO::IOAccount::PasswordAuthResult::AccountBanned:
        DisconnectClient(AB::ErrorCodes::AlreadyLoggedIn);
        banMan->AddLoginAttempt(GetIP(), false);
        return;
    case IO::IOAccount::PasswordAuthResult::InternalError:
        DisconnectClient(AB::ErrorCodes::UnknownError);
        return;
    default:
        break;
    }

    AB::Entities::Service gameServer;
    if (!IO::IOService::EnsureService(
        AB::Entities::ServiceTypeGameServer,
        gameServer,
        account.currentServerUuid))
    {
        DisconnectClient(AB::ErrorCodes::AllServersFull);
        return;
    }

    AB::Entities::Service fileServer;
    if (!IO::IOService::EnsureService(
        AB::Entities::ServiceTypeFileServer,
        fileServer))
    {
        DisconnectClient(AB::ErrorCodes::AllServersFull);
        return;
    }

    banMan->AddLoginAttempt(GetIP(), true);

    LOG_INFO << Utils::ConvertIPToString(GetIP(), true) << ": " << request.accountName << " logged in" << std::endl;

    auto output = OutputMessagePool::GetOutputMessage();

    output->AddByte(AB::LoginProtocol::CharacterList);

    AB::Packets::Server::Login::CharacterList packet;
    packet.accountUuid = account.uuid;
    packet.authToken = account.authToken;
    packet.accountType = static_cast<uint8_t>(account.type);
    packet.serverHost = gameServer.host;
    packet.serverPort = gameServer.port;
    packet.fileHost = fileServer.host;
    packet.filePort = fileServer.port;
    packet.charSlots = static_cast<uint16_t>(account.charSlots);
    packet.charCount = static_cast<uint16_t>(account.characterUuids.size());
    packet.characters.reserve(packet.charCount);

    const std::string landingGame = IO::IOGame::GetLandingGameUuid();
    for (const std::string& characterUuid : account.characterUuids)
    {
        AB::Entities::Character character;
        character.uuid = characterUuid;
        if (!IO::IOAccount::LoadCharacter(character))
            continue;
        packet.characters.push_back({
            character.uuid,
            character.level,
            character.name,
            character.profession,
            character.profession2,
            static_cast<uint8_t>(character.sex),
            character.modelIndex,
            (Utils::Uuid::IsEmpty(character.lastOutpostUuid) ? landingGame : character.lastOutpostUuid)
        });
    }
    AB::Packets::Add(packet, *output);

    Send(std::move(output));
    Disconnect();
}

void ProtocolLogin::SendOutposts(AB::Packets::Client::Login::GetOutposts request)
{
    AB::Entities::Account account;
    account.uuid = request.accountUuid;
    bool res = IO::IOAccount::TokenAuth(request.authToken, account);
    if (!res)
    {
        DisconnectClient(AB::ErrorCodes::TokenAuthFailure);
        return;
    }

    auto output = OutputMessagePool::GetOutputMessage();
    output->AddByte(AB::LoginProtocol::OutpostList);
    const std::vector<AB::Entities::Game> games = IO::IOGame::GetGameList({
        AB::Entities::GameType::GameTypeOutpost,
        AB::Entities::GameType::GameTypeTown });

    AB::Packets::Server::Login::OutpostList packet;
    packet.count = static_cast<uint16_t>(games.size());
    packet.outposts.reserve(packet.count);
    for (const AB::Entities::Game& game : games)
    {
        packet.outposts.push_back({
            game.uuid,
            game.name,
            game.type,
            game.partySize,
            game.mapCoordX,
            game.mapCoordY
        });
    }
    AB::Packets::Add(packet, *output);

    Send(std::move(output));
    Disconnect();
}

void ProtocolLogin::SendServers(AB::Packets::Client::Login::GetServers request)
{
    AB::Entities::Account account;
    account.uuid = request.accountUuid;
    bool res = IO::IOAccount::TokenAuth(request.authToken, account);
    if (!res)
    {
        DisconnectClient(AB::ErrorCodes::TokenAuthFailure);
        return;
    }

    auto output = OutputMessagePool::GetOutputMessage();
    output->AddByte(AB::LoginProtocol::ServerList);

    std::vector<AB::Entities::Service> services;
    AB::Packets::Server::Login::ServerList packet;
    packet.count = static_cast<uint16_t>(IO::IOService::GetServices(AB::Entities::ServiceTypeGameServer, services));
    packet.servers.reserve(packet.count);
    for (const AB::Entities::Service& service : services)
    {
        packet.servers.push_back({
            service.type,
            service.uuid,
            service.host,
            service.port,
            service.location,
            service.name
        });
    }
    AB::Packets::Add(packet, *output);

    Send(std::move(output));
    Disconnect();
}

void ProtocolLogin::CreateAccount(AB::Packets::Client::Login::CreateAccount request)
{
    IO::IOAccount::CreateAccountResult res = IO::IOAccount::CreateAccount(
        request.accountName, request.password, request.email, request.accountKey);

    auto output = OutputMessagePool::GetOutputMessage();

    if (res == IO::IOAccount::CreateAccountResult::OK)
    {
        output->AddByte(AB::LoginProtocol::CreateAccountSuccess);
    }
    else
    {
        output->AddByte(AB::LoginProtocol::CreateAccountError);
        AB::Packets::Server::Login::Error packet;
        switch (res)
        {
        case IO::IOAccount::CreateAccountResult::NameExists:
            packet.code = static_cast<uint8_t>(AB::ErrorCodes::AccountNameExists);
            break;
        case IO::IOAccount::CreateAccountResult::InvalidAccountKey:
            packet.code = static_cast<uint8_t>(AB::ErrorCodes::InvalidAccountKey);
            break;
        case IO::IOAccount::CreateAccountResult::PasswordError:
            packet.code = static_cast<uint8_t>(AB::ErrorCodes::InvalidPassword);
            break;
        case IO::IOAccount::CreateAccountResult::EmailError:
            packet.code = static_cast<uint8_t>(AB::ErrorCodes::InvalidEmail);
            break;
        default:
            packet.code = static_cast<uint8_t>(AB::ErrorCodes::UnknownError);
            break;
        }
        AB::Packets::Add(packet, *output);
    }

    Send(std::move(output));
    Disconnect();
}

void ProtocolLogin::CreatePlayer(AB::Packets::Client::Login::CreatePlayer request)
{
    AB::Entities::Account account;
    account.uuid = request.accountUuid;
    bool ses = IO::IOAccount::TokenAuth(request.authToken, account);
    if (!ses)
    {
        DisconnectClient(AB::ErrorCodes::TokenAuthFailure);
        return;
    }

    std::string uuid;
    IO::IOAccount::CreatePlayerResult res = IO::IOAccount::CreatePlayer(
        account.uuid, request.charName, request.profUuid, request.itemIndex,
        static_cast<AB::Entities::CharacterSex>(request.sex), request.pvp, uuid
    );

    auto output = OutputMessagePool::GetOutputMessage();

    if (res == IO::IOAccount::CreatePlayerResult::OK)
    {
        output->AddByte(AB::LoginProtocol::CreatePlayerSuccess);
        AB::Packets::Server::Login::CreateCharacterSuccess packet = {
            uuid,
            IO::IOGame::GetLandingGameUuid()
        };
        AB::Packets::Add(packet, *output);
    }
    else
    {
        output->AddByte(AB::LoginProtocol::CreatePlayerError);
        AB::Packets::Server::Login::Error packet;
        switch (res)
        {
        case IO::IOAccount::CreatePlayerResult::NameExists:
            packet.code = static_cast<uint8_t>(AB::ErrorCodes::PlayerNameExists);
            break;
        case IO::IOAccount::CreatePlayerResult::InvalidAccount:
            packet.code = static_cast<uint8_t>(AB::ErrorCodes::InvalidAccount);
            break;
        case IO::IOAccount::CreatePlayerResult::NoMoreCharSlots:
            packet.code = static_cast<uint8_t>(AB::ErrorCodes::NoMoreCharSlots);
            break;
        case IO::IOAccount::CreatePlayerResult::InvalidProfession:
            packet.code = static_cast<uint8_t>(AB::ErrorCodes::InvalidProfession);
            break;
        case IO::IOAccount::CreatePlayerResult::InvalidName:
            packet.code = static_cast<uint8_t>(AB::ErrorCodes::InvalidCharacterName);
            break;
        default:
            packet.code = static_cast<uint8_t>(AB::ErrorCodes::UnknownError);
            break;
        }
        AB::Packets::Add(packet, *output);
    }

    Send(std::move(output));
    Disconnect();
}

void ProtocolLogin::AddAccountKey(AB::Packets::Client::Login::AddAccountKey request)
{
    AB::Entities::Account account;
    account.uuid = request.accountUuid;
    bool authres = IO::IOAccount::TokenAuth(request.authToken, account);
    if (!authres)
    {
        DisconnectClient(AB::ErrorCodes::TokenAuthFailure);
        return;
    }

    IO::IOAccount::CreateAccountResult res = IO::IOAccount::AddAccountKey(account, request.accountKey);
    auto output = OutputMessagePool::GetOutputMessage();

    if (res == IO::IOAccount::CreateAccountResult::OK)
    {
        output->AddByte(AB::LoginProtocol::AddAccountKeySuccess);
    }
    else
    {
        output->AddByte(AB::LoginProtocol::AddAccountKeyError);
        AB::Packets::Server::Login::Error packet;
        switch (res)
        {
        case IO::IOAccount::CreateAccountResult::NameExists:
            packet.code = static_cast<uint8_t>(AB::ErrorCodes::AccountNameExists);
            break;
        case IO::IOAccount::CreateAccountResult::InvalidAccountKey:
            packet.code = static_cast<uint8_t>(AB::ErrorCodes::InvalidAccountKey);
            break;
        case IO::IOAccount::CreateAccountResult::InvalidAccount:
            packet.code = static_cast<uint8_t>(AB::ErrorCodes::InvalidAccount);
            break;
        case IO::IOAccount::CreateAccountResult::AlreadyAdded:
            packet.code = static_cast<uint8_t>(AB::ErrorCodes::AccountKeyAlreadyAdded);
            break;
        default:
            packet.code = static_cast<uint8_t>(AB::ErrorCodes::UnknownError);
            break;
        }
        AB::Packets::Add(packet, *output);
    }

    Send(std::move(output));
    Disconnect();
}

void ProtocolLogin::DeletePlayer(AB::Packets::Client::Login::DeleteCharacter request)
{
    AB::Entities::Account account;
    account.uuid = request.accountUuid;
    bool authRes = IO::IOAccount::TokenAuth(request.authToken, account);
    if (!authRes)
    {
        DisconnectClient(AB::ErrorCodes::TokenAuthFailure);
        return;
    }

    bool res = IO::IOAccount::DeletePlayer(
        account.uuid, request.charUuid
    );

    auto output = OutputMessagePool::GetOutputMessage();

    if (res)
    {
        output->AddByte(AB::LoginProtocol::DeletePlayerSuccess);
        AB::Packets::Server::Login::CharacterDeleted packet = {
            request.charUuid
        };
        AB::Packets::Add(packet, *output);
    }
    else
    {
        output->AddByte(AB::LoginProtocol::DeletePlayerError);
        AB::Packets::Server::Login::Error packet =  {
            static_cast<uint8_t>(AB::ErrorCodes::InvalidCharacter)
        };
        AB::Packets::Add(packet, *output);
    }

    LOG_INFO << Utils::ConvertIPToString(GetIP()) << ": "
        << request.accountUuid << " deleted character with UUID " << request.charUuid << std::endl;

    Send(std::move(output));
    Disconnect();
}

void ProtocolLogin::DisconnectClient(AB::ErrorCodes error)
{
    auto output = OutputMessagePool::GetOutputMessage();
    output->AddByte(AB::LoginProtocol::LoginError);
    AB::Packets::Server::Login::Error packet = {
        static_cast<uint8_t>(error)
    };
    AB::Packets::Add(packet, *output);
    Send(std::move(output));
    Disconnect();
}

}

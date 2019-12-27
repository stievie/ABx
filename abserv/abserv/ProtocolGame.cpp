#include "stdafx.h"
#include "ProtocolGame.h"
#include "Player.h"
#include "PlayerManager.h"
#include "OutputMessage.h"
#include "IOPlayer.h"
#include "IOAccount.h"
#include "BanManager.h"
#include "StringUtils.h"
#include "GameManager.h"
#include "Logger.h"
#include "Game.h"
#include "Random.h"
#include "Variant.h"
#include "IOGame.h"
#include "stdafx.h"
#include "ConfigManager.h"
#include "Subsystems.h"
#include <AB/DHKeys.hpp>
#include "UuidUtils.h"
#include <AB/Entities/FriendList.h>
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>
#include <AB/Packets/ClientPackets.h>

namespace Net {

std::string ProtocolGame::serverId_ = Utils::Uuid::EMPTY_UUID;

inline void ProtocolGame::AddPlayerInput(Game::InputType type, Utils::VariantMap&& data)
{
    if (auto p = player_.lock())
        p->AddInput(type, std::move(data));
}

inline void ProtocolGame::AddPlayerInput(Game::InputType type)
{
    if (auto p = player_.lock())
        p->AddInput(type);
}

void ProtocolGame::Login(const std::string& playerUuid, const uuids::uuid& accountUuid,
    const std::string& mapUuid, const std::string& instanceUuid)
{
#ifdef DEBUG_NET
    LOG_DEBUG << "Player " << playerUuid << " logging in" << std::endl;
#endif
    auto* playerMan = GetSubsystem<Game::PlayerManager>();
    std::shared_ptr<Game::Player> foundPlayer = playerMan->GetPlayerByUuid(playerUuid);
    if (foundPlayer)
    {
#ifdef DEBUG_NET
        LOG_DEBUG << "Player " << foundPlayer->GetName() << " already logged in" << std::endl;
#endif
        DisconnectClient(AB::Errors::AlreadyLoggedIn);
        return;
    }

    if (GetSubsystem<Auth::BanManager>()->IsAccountBanned(accountUuid))
    {
        LOG_INFO << "Login attempt from banned account " << accountUuid.to_string() << std::endl;
        DisconnectClient(AB::Errors::AccountBanned);
        return;
    }

    std::shared_ptr<Game::Player> player = playerMan->CreatePlayer(GetPtr());
    assert(player);

    // Load player and account data from DB
    if (!IO::IOPlayer::LoadPlayerByUuid(*player, playerUuid))
    {
        LOG_ERROR << "Error loading player " << playerUuid << std::endl;
        DisconnectClient(AB::Errors::ErrorLoadingCharacter);
        return;
    }
    // Account and player is loaded create the index for the player
    playerMan->UpdatePlayerIndex(*player);

    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    // Check if game exists.
    AB::Entities::Game g;
    g.uuid = mapUuid;
    if (!client->Read(g))
    {
        LOG_ERROR << "Invalid game with map " << mapUuid << std::endl;
        DisconnectClient(AB::Errors::InvalidGame);
        return;
    }

    player->account_.onlineStatus = AB::Entities::OnlineStatus::OnlineStatusOnline;
    player->account_.currentCharacterUuid = player->data_.uuid;
    player->account_.currentServerUuid = ProtocolGame::serverId_;
    client->Update(player->account_);

    player->Initialize();
    player->data_.currentMapUuid = mapUuid;
    player->data_.lastLogin = Utils::Tick();
    if (!uuids::uuid(instanceUuid).nil())
        player->data_.instanceUuid = instanceUuid;
    client->Update(player->data_);
    OutputMessagePool::Instance()->AddToAutoSend(shared_from_this());
    LOG_INFO << "User " << player->account_.name << " logged in with " << player->data_.name << " entering " << g.name << std::endl;
    player_ = player;
    Connect();
}

void ProtocolGame::Logout()
{
    auto player = GetPlayer();
    if (!player)
        return;

    player->logoutTime_ = Utils::Tick();
    IO::IOPlayer::SavePlayer(*player);
    IO::IOAccount::AccountLogout(player->data_.accountUuid);
    GetSubsystem<Game::PlayerManager>()->RemovePlayer(player->id_);
    Disconnect();
    OutputMessagePool::Instance()->RemoveFromAutoSend(shared_from_this());
    LOG_INFO << "User " << player->account_.name << " logged out" << std::endl;
    player_.reset();
}

void ProtocolGame::ParsePacket(NetworkMessage& message)
{
    if (!acceptPackets_ ||
        GetSubsystem<Game::GameManager>()->GetState() != Game::GameManager::ManagerStateRunning ||
        message.GetSize() == 0)
        return;

    const uint8_t recvByte = message.GetByte();

    switch (recvByte)
    {
    case AB::GameProtocol::PacketTypePing:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::Ping>(message);
        AddPlayerTask(&Game::Player::CRQPing, packet.tick);
        break;
    }
    case AB::GameProtocol::PacketTypeLogout:
    {
        /* auto packet = */ AB::Packets::Get<AB::Packets::Client::Logout>(message);
        AddPlayerTask(&Game::Player::CRQLogout);
        break;
    }
    case AB::GameProtocol::PacketTypeChangeMap:
    {
        // Called by the client when clicking on the map
        auto packet = AB::Packets::Get<AB::Packets::Client::ChangeMap>(message);
        AddPlayerTask(&Game::Player::CRQChangeMap, packet.mapUuid);
        break;
    }
    case AB::GameProtocol::PacketTypeSendMail:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::SendMail>(message);
        AddPlayerTask(&Game::Player::CRQSendMail, packet.recipient, packet.subject, packet.body);
        break;
    }
    case AB::GameProtocol::PacketTypeGetMailHeaders:
    {
        /* auto packet = */ AB::Packets::Get<AB::Packets::Client::GetMailHeaders>(message);
        AddPlayerTask(&Game::Player::CRQGetMailHeaders);
        break;
    }
    case AB::GameProtocol::PacketTypeGetMail:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::GetMail>(message);
        AddPlayerTask(&Game::Player::CRQGetMail, packet.mailUuid);
        break;
    }
    case AB::GameProtocol::PacketTypeDeleteMail:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::DeleteMail>(message);
        AddPlayerTask(&Game::Player::CRQDeleteMail, packet.mailUuid);
        break;
    }
    case AB::GameProtocol::PacketTypeGetInventory:
    {
        /* auto packet = */ AB::Packets::Get<AB::Packets::Client::GetInventory>(message);
        AddPlayerTask(&Game::Player::CRQGetInventory);
        break;
    }
    case AB::GameProtocol::PacketTypeInventoryDestroyItem:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::InventoryDestroyItem>(message);
        AddPlayerTask(&Game::Player::CRQDestroyInventoryItem, packet.pos);
        break;
    }
    case AB::GameProtocol::PacketTypeInventoryDropItem:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::InventoryDropItem>(message);
        AddPlayerTask(&Game::Player::CRQDropInventoryItem, packet.pos);
        break;
    }
    case AB::GameProtocol::PacketTypeInventoryStoreInChest:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::InventoryStoreItem>(message);
        AddPlayerTask(&Game::Player::CRQStoreInChest, packet.pos);
        break;
    }
    case AB::GameProtocol::PacketTypeGetChest:
    {
        /* auto packet = */ AB::Packets::Get<AB::Packets::Client::GetChest>(message);
        AddPlayerTask(&Game::Player::CRQGetChest);
        break;
    }
    case AB::GameProtocol::PacketTypeChestDestroyItem:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::ChestDestroyItem>(message);
        AddPlayerTask(&Game::Player::CRQDestroyChestItem, packet.pos);
        break;
    }
    case AB::GameProtocol::PacketTypePartyInvitePlayer:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::PartyInvitePlayer>(message);
        AddPlayerTask(&Game::Player::CRQPartyInvitePlayer, packet.targetId);
        break;
    }
    case AB::GameProtocol::PacketTypePartyKickPlayer:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::PartyKickPlayer>(message);
        AddPlayerTask(&Game::Player::CRQPartyKickPlayer, packet.targetId);
        break;
    }
    case AB::GameProtocol::PacketTypePartyLeave:
    {
        /* auto packet = */ AB::Packets::Get<AB::Packets::Client::PartyLeave>(message);
        AddPlayerTask(&Game::Player::CRQPartyLeave);
        break;
    }
    case AB::GameProtocol::PacketTypePartyAcceptInvite:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::PartyAcceptInvite>(message);
        AddPlayerTask(&Game::Player::CRQPartyAccept, packet.inviterId);
        break;
    }
    case AB::GameProtocol::PacketTypePartyRejectInvite:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::PartyRejectInvite>(message);
        AddPlayerTask(&Game::Player::CRQPartyRejectInvite, packet.inviterId);
        break;
    }
    case AB::GameProtocol::PacektTypeGetPartyMembers:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::PartyGetMembers>(message);
        AddPlayerTask(&Game::Player::CRQPartyGetMembers, packet.partyId);
        break;
    }
    case AB::GameProtocol::PacketTypeMove:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::Move>(message);
        Utils::VariantMap data;
        data[Game::InputDataDirection] = packet.direction;
        AddPlayerInput(Game::InputType::Move, std::move(data));
        break;
    }
    case AB::GameProtocol::PacketTypeTurn:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::Turn>(message);
        Utils::VariantMap data;
        data[Game::InputDataDirection] = packet.direction;   // None | Left | Right
        AddPlayerInput(Game::InputType::Turn, std::move(data));
        break;
    }
    case AB::GameProtocol::PacketTypeSetDirection:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::SetDirection>(message);
        Utils::VariantMap data;
        data[Game::InputDataDirection] = packet.rad;   // World angle Rad
        AddPlayerInput(Game::InputType::Direction, std::move(data));
        break;
    }
    case AB::GameProtocol::PacketTypeSetState:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::SetPlayerState>(message);
        Utils::VariantMap data;
        data[Game::InputDataState] = packet.newState;
        AddPlayerInput(Game::InputType::SetState, std::move(data));
        break;
    }
    case AB::GameProtocol::PacketTypeGoto:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::GotoPos>(message);
        Utils::VariantMap data;
        data[Game::InputDataVertexX] = packet.pos[0];
        data[Game::InputDataVertexY] = packet.pos[1];
        data[Game::InputDataVertexZ] = packet.pos[2];
        AddPlayerInput(Game::InputType::Goto, std::move(data));
        break;
    }
    case AB::GameProtocol::PacketTypeFollow:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::Follow>(message);
        Utils::VariantMap data;
        data[Game::InputDataObjectId] = packet.targetId;
        data[Game::InputDataPingTarget] = packet.ping;
        AddPlayerInput(Game::InputType::Follow, std::move(data));
        break;
    }
    case AB::GameProtocol::PacketTypeUseSkill:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::UseSkill>(message);
        // 1 based -> convert to 0 based
        const uint8_t index = packet.index;
        if (index > 0 && index <= Game::PLAYER_MAX_SKILLS)
        {
            Utils::VariantMap data;
            data[Game::InputDataSkillIndex] = index - 1;
            data[Game::InputDataPingTarget] = packet.ping;
            AddPlayerInput(Game::InputType::UseSkill, std::move(data));
        }
        break;
    }
    case AB::GameProtocol::PacketTypeCancel:
    {
        /* auto packet = */ AB::Packets::Get<AB::Packets::Client::Cancel>(message);
        AddPlayerInput(Game::InputType::Cancel);
        break;
    }
    case AB::GameProtocol::PacketTypeAttack:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::Attack>(message);
        Utils::VariantMap data;
        data[Game::InputDataPingTarget] = packet.ping;
        AddPlayerInput(Game::InputType::Attack, std::move(data));
        break;
    }
    case AB::GameProtocol::PacketTypeSelect:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::SelectObject>(message);
        Utils::VariantMap data;
        data[Game::InputDataObjectId] = packet.sourceId;    // Source
        data[Game::InputDataObjectId2] = packet.targetId;   // Target
        AddPlayerInput(Game::InputType::Select, std::move(data));
        break;
    }
    case AB::GameProtocol::PacketTypeClickObject:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::ClickObject>(message);
        Utils::VariantMap data;
        data[Game::InputDataObjectId] = packet.sourceId;    // Source
        data[Game::InputDataObjectId2] = packet.targetId;   // Target
        AddPlayerInput(Game::InputType::ClickObject, std::move(data));
        break;
    }
    case AB::GameProtocol::PacketTypeQueue:
    {
        /* auto packet = */ AB::Packets::Get<AB::Packets::Client::QueueMatch>(message);
        AddPlayerTask(&Game::Player::CRQQueueForMatch);
        break;
    }
    case AB::GameProtocol::PacketTypeUnqueue:
    {
        /* auto packet = */ AB::Packets::Get<AB::Packets::Client::UnqueueMatch>(message);
        AddPlayerTask(&Game::Player::CRQUnqueueForMatch);
        break;
    }
    case AB::GameProtocol::PacketTypeCommand:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::Command>(message);
        Utils::VariantMap data;
        data[Game::InputDataCommandType] = packet.type;
        data[Game::InputDataCommandData] = packet.data;
        AddPlayerInput(Game::InputType::Command, std::move(data));
        break;
    }
    case AB::GameProtocol::PacketTypeGetFriendList:
    {
        /* auto packet = */ AB::Packets::Get<AB::Packets::Client::UpdateFriendList>(message);
        AddPlayerTask(&Game::Player::CRQGetFriendList);
        break;
    }
    case AB::GameProtocol::PacketTypeAddFriend:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::AddFriend>(message);
        const AB::Entities::FriendRelation rel = static_cast<AB::Entities::FriendRelation>(packet.relation);
        AddPlayerTask(&Game::Player::CRQAddFriend, packet.name, rel);
        break;
    }
    case AB::GameProtocol::PacketTypeRemoveFriend:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::RemoveFriend>(message);
        AddPlayerTask(&Game::Player::CRQRemoveFriend, packet.accountUuid);
        break;
    }
    case AB::GameProtocol::PacketTypeRenameFriend:
    {
        // TODO:
        std::string accountUuid = message.GetString();
        std::string newName = message.GetString();
        AddPlayerTask(&Game::Player::CRQChangeFriendNick, accountUuid, newName);
        break;
    }
    case AB::GameProtocol::PacketTypeGetPlayerInfoByAccount:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::GetPlayerInfoByAccount>(message);
        AddPlayerTask(&Game::Player::CRQGetPlayerInfoByAccount, packet.accountUuid, packet.fields);
        break;
    }
    case AB::GameProtocol::PacketTypeGetPlayerInfoByName:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::GetPlayerInfoByName>(message);
        AddPlayerTask(&Game::Player::CRQGetPlayerInfoByName, packet.name, packet.fields);
        break;
    }
    case AB::GameProtocol::PacketTypeGetGuildInfo:
    {
        // TODO
        AddPlayerTask(&Game::Player::CRQGetGuildInfo);
        break;
    }
    case AB::GameProtocol::PacketTypeGetGuildMembers:
    {
        // TODO
        AddPlayerTask(&Game::Player::CRQGetGuildMembers);
        break;
    }
    case AB::GameProtocol::PacketTypeSetOnlineStatus:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::SetOnlineStatus>(message);
        const AB::Entities::OnlineStatus status = static_cast<AB::Entities::OnlineStatus>(packet.newStatus);
        AddPlayerTask(&Game::Player::CRQSetOnlineStatus, status);
        break;
    }
    default:
    {
        auto player = GetPlayer();
        LOG_ERROR << Utils::ConvertIPToString(GetIP()) << ": Player " << (player ? player->GetName() : "null") <<
            " sent an unknown packet header: 0x" <<
            std::hex << static_cast<uint16_t>(recvByte) << std::dec << std::endl;
        break;
    }
    }
}

void ProtocolGame::OnRecvFirstMessage(NetworkMessage& msg)
{
    if (encryptionEnabled_)
    {
        if (!XTEADecrypt(msg))
        {
            Disconnect();
            return;
        }
    }

    msg.Skip(2);    // Client OS
    uint16_t version = msg.Get<uint16_t>();
    if (version != AB::PROTOCOL_VERSION)
    {
        DisconnectClient(AB::Errors::WrongProtocolVersion);
        return;
    }
    for (int i = 0; i < DH_KEY_LENGTH; ++i)
        clientKey_[i] = msg.GetByte();
    auto keys = GetSubsystem<Crypto::DHKeys>();
    // Switch now to the shared key
    keys->GetSharedKey(clientKey_, encKey_);

    const std::string accountUuid = msg.GetString();
    if (Utils::Uuid::IsEmpty(accountUuid))
    {
        LOG_ERROR << "Invalid account " << accountUuid << std::endl;
        DisconnectClient(AB::Errors::InvalidAccount);
        return;
    }
    const std::string authToken = msg.GetString();
    const std::string characterUuid = msg.GetString();
    const std::string map = msg.GetString();
    const std::string instance = msg.GetString();

    const uint32_t ip = GetIP();
    if (GetSubsystem<Auth::BanManager>()->IsIpBanned(ip))
    {
        LOG_ERROR << "Connection attempt from banned IP " << Utils::ConvertIPToString(ip, true) << std::endl;
        DisconnectClient(AB::Errors::IPBanned);
        return;
    }

    if (!IO::IOAccount::GameWorldAuth(accountUuid, authToken, characterUuid))
    {
        DisconnectClient(AB::Errors::NamePasswordMismatch);
        return;
    }

    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(
            std::bind(&ProtocolGame::Login, GetPtr(), characterUuid, uuids::uuid(accountUuid), map, instance)
        )
    );
}

void ProtocolGame::OnConnect()
{
    auto output = OutputMessagePool::GetOutputMessage();
    output->AddByte(AB::GameProtocol::KeyExchange);
    auto keys = GetSubsystem<Crypto::DHKeys>();
    output->AddBytes((const char*)&keys->GetPublickKey(), DH_KEY_LENGTH);
    Send(output);
}

void ProtocolGame::DisconnectClient(uint8_t error)
{
    auto output = OutputMessagePool::GetOutputMessage();
    output->AddByte(AB::GameProtocol::Error);
    AB::Packets::Server::ProtocolError packet = { error };
    AB::Packets::Add(packet, *output);
    Send(output);
    Disconnect();
}

void ProtocolGame::Connect()
{
    if (IsConnectionExpired())
    {
        // ProtocolGame::release() has been called at this point and the Connection object
        // no longer exists, so we return to prevent leakage of the Player.
        LOG_ERROR << "Connection expired" << std::endl;
        return;
    }
    auto player = player_.lock();
    if (!player)
    {
        LOG_ERROR << "Player expired" << std::endl;
        return;
    }

    player->loginTime_ = Utils::Tick();

    acceptPackets_ = true;

    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(
            std::bind(&ProtocolGame::EnterGame, GetPtr())
        )
    );
}

void ProtocolGame::WriteToOutput(const NetworkMessage& message)
{
    GetOutputBuffer(message.GetSize())->Append(message);
}

void ProtocolGame::EnterGame()
{
    auto player = GetPlayer();
    if (!player)
    {
        LOG_ERROR << "GetPlayer returned null" << std::endl;
        DisconnectClient(AB::Errors::CannotEnterGame);
        return;
    }
    auto* gameMan = GetSubsystem<Game::GameManager>();
    bool success = false;
    std::shared_ptr<Game::Game> instance;
    if (!Utils::Uuid::IsEmpty(player->data_.instanceUuid))
    {
        // Enter an existing instance
        instance = gameMan->GetInstance(player->data_.instanceUuid);
        if (instance)
        {
            instance->PlayerJoin(player->id_);
            success = true;
        }
        else
            LOG_ERROR << "Game instance not found " << player->data_.instanceUuid << std::endl;
    }
    else if (gameMan->AddPlayer(player->data_.currentMapUuid, player))
    {
        // Create new instance
        success = true;
        instance = gameMan->GetInstance(player->data_.instanceUuid);
    }

    if (success)
    {
        auto output = OutputMessagePool::GetOutputMessage();
        output->AddByte(AB::GameProtocol::GameEnter);
        AB::Packets::Server::EnterWorld packet = {
            ProtocolGame::serverId_,
            player->data_.currentMapUuid,
            player->data_.instanceUuid,
            player->id_,
            static_cast<uint8_t>(instance->data_.type),
            instance->data_.partySize
        };
        AB::Packets::Add(packet, *output);
        Send(output);
    }
    else
        DisconnectClient(AB::Errors::CannotEnterGame);
}

void ProtocolGame::ChangeServerInstance(const std::string& serverUuid, const std::string& mapUuid, const std::string& instanceUuid)
{
    auto player = GetPlayer();
    if (!player)
    {
        LOG_ERROR << "Player == null" << std::endl;
        return;
    }

#ifdef DEBUG_NET
    LOG_DEBUG << "Player changing instance to " << mapUuid << std::endl;
#endif

    auto output = OutputMessagePool::GetOutputMessage();
    output->AddByte(AB::GameProtocol::ChangeInstance);
    AB::Packets::Server::ChangeInstance packet = {
        serverUuid, mapUuid, instanceUuid, player->data_.uuid
    };
    AB::Packets::Add(packet, *output);
    Send(output);
}

void ProtocolGame::ChangeInstance(const std::string& mapUuid, const std::string& instanceUuid)
{
    ChangeServerInstance(ProtocolGame::serverId_, mapUuid, instanceUuid);
}

}

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

    std::shared_ptr<Game::Player> player = playerMan->CreatePlayer(GetThis());
    assert(player);

    // Load player and account data from DB
    if (!IO::IOPlayer_LoadPlayerByUuid(*player, playerUuid))
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
    IO::IOPlayer_SavePlayer(*player);
    IO::IOAccount_AccountLogout(player->data_.accountUuid);
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
        const int64_t clientTick = message.Get<int64_t>();
        AddPlayerTask(&Game::Player::CRQPing, clientTick);
        break;
    }
    case AB::GameProtocol::PacketTypeLogout:
        AddPlayerTask(&Game::Player::CRQLogout);
        break;
    case AB::GameProtocol::PacketTypeChangeMap:
    {
        // Called by the client when clicking on the map
        const std::string mapUuid = message.GetString();
        AddPlayerTask(&Game::Player::CRQChangeMap, mapUuid);
        break;
    }
    case AB::GameProtocol::PacketTypeSendMail:
    {
        const std::string recipient = message.GetString();
        const std::string subject = message.GetString();
        const std::string body = message.GetString();
        AddPlayerTask(&Game::Player::CRQSendMail, recipient, subject, body);
        break;
    }
    case AB::GameProtocol::PacketTypeGetMailHeaders:
        AddPlayerTask(&Game::Player::CRQGetMailHeaders);
        break;
    case AB::GameProtocol::PacketTypeGetMail:
    {
        const std::string mailUuid = message.GetString();
        AddPlayerTask(&Game::Player::CRQGetMail, mailUuid);
        break;
    }
    case AB::GameProtocol::PacketTypeDeleteMail:
    {
        const std::string mailUuid = message.GetString();
        AddPlayerTask(&Game::Player::CRQDeleteMail, mailUuid);
        break;
    }
    case AB::GameProtocol::PacketTypeGetInventory:
        AddPlayerTask(&Game::Player::CRQGetInventory);
        break;
    case AB::GameProtocol::PacketTypeInventoryDestroyItem:
    {
        uint16_t pos = message.Get<uint16_t>();
        AddPlayerTask(&Game::Player::CRQDestroyInventoryItem, pos);
        break;
    }
    case AB::GameProtocol::PacketTypeInventoryDropItem:
    {
        uint16_t pos = message.Get<uint16_t>();
        AddPlayerTask(&Game::Player::CRQDropInventoryItem, pos);
        break;
    }
    case AB::GameProtocol::PacketTypeInventoryStoreInChest:
    {
        uint16_t pos = message.Get<uint16_t>();
        AddPlayerTask(&Game::Player::CRQStoreInChest, pos);
        break;
    }
    case AB::GameProtocol::PacketTypeGetChest:
        AddPlayerTask(&Game::Player::CRQGetChest);
        break;
    case AB::GameProtocol::PacketTypeChestDestroyItem:
    {
        uint16_t pos = message.Get<uint16_t>();
        AddPlayerTask(&Game::Player::CRQDestroyChestItem, pos);
        break;
    }
    case AB::GameProtocol::PacketTypePartyInvitePlayer:
    {
        uint32_t playerId = message.Get<uint32_t>();
        AddPlayerTask(&Game::Player::CRQPartyInvitePlayer, playerId);
        break;
    }
    case AB::GameProtocol::PacketTypePartyKickPlayer:
    {
        uint32_t playerId = message.Get<uint32_t>();
        AddPlayerTask(&Game::Player::CRQPartyKickPlayer, playerId);
        break;
    }
    case AB::GameProtocol::PacketTypePartyLeave:
    {
        AddPlayerTask(&Game::Player::CRQPartyLeave);
        break;
    }
    case AB::GameProtocol::PacketTypePartyAcceptInvite:
    {
        uint32_t inviterId = message.Get<uint32_t>();
        AddPlayerTask(&Game::Player::CRQPartyAccept, inviterId);
        break;
    }
    case AB::GameProtocol::PacketTypePartyRejectInvite:
    {
        uint32_t inviterId = message.Get<uint32_t>();
        AddPlayerTask(&Game::Player::CRQPartyRejectInvite, inviterId);
        break;
    }
    case AB::GameProtocol::PacektTypeGetPartyMembers:
    {
        uint32_t partyId = message.Get<uint32_t>();
        AddPlayerTask(&Game::Player::CRQPartyGetMembers, partyId);
        break;
    }
    case AB::GameProtocol::PacketTypeMove:
    {
        Utils::VariantMap data;
        data[Game::InputDataDirection] = message.Get<uint8_t>();
        AddPlayerInput(Game::InputType::Move, std::move(data));
        break;
    }
    case AB::GameProtocol::PacketTypeTurn:
    {
        Utils::VariantMap data;
        data[Game::InputDataDirection] = message.Get<uint8_t>();   // None | Left | Right
        AddPlayerInput(Game::InputType::Turn, std::move(data));
        break;
    }
    case AB::GameProtocol::PacketTypeSetDirection:
    {
        Utils::VariantMap data;
        data[Game::InputDataDirection] = message.Get<float>();   // World angle Rad
        AddPlayerInput(Game::InputType::Direction, std::move(data));
        break;
    }
    case AB::GameProtocol::PacketTypeSetState:
    {
        Utils::VariantMap data;
        data[Game::InputDataState] = message.Get<uint8_t>();
        AddPlayerInput(Game::InputType::SetState, std::move(data));
        break;
    }
    case AB::GameProtocol::PacketTypeGoto:
    {
        Utils::VariantMap data;
        data[Game::InputDataVertexX] = message.Get<float>();
        data[Game::InputDataVertexY] = message.Get<float>();
        data[Game::InputDataVertexZ] = message.Get<float>();
        AddPlayerInput(Game::InputType::Goto, std::move(data));
        break;
    }
    case AB::GameProtocol::PacketTypeFollow:
    {
        Utils::VariantMap data;
        data[Game::InputDataObjectId] = message.Get<uint32_t>();
        data[Game::InputDataPingTarget] = (message.Get<uint8_t>() != 0);
        AddPlayerInput(Game::InputType::Follow, std::move(data));
        break;
    }
    case AB::GameProtocol::PacketTypeUseSkill:
    {
        // 1 based -> convert to 0 based
        uint8_t index = message.Get<uint8_t>();
        bool ping = message.Get<uint8_t>() != 0;
        if (index > 0 && index <= Game::PLAYER_MAX_SKILLS)
        {
            Utils::VariantMap data;
            data[Game::InputDataSkillIndex] = index - 1;
            data[Game::InputDataPingTarget] = ping;
            AddPlayerInput(Game::InputType::UseSkill, std::move(data));
        }
        break;
    }
    case AB::GameProtocol::PacketTypeCancel:
    {
        AddPlayerInput(Game::InputType::Cancel);
        break;
    }
    case AB::GameProtocol::PacketTypeAttack:
    {
        Utils::VariantMap data;
        bool ping = message.Get<uint8_t>() != 0;
        data[Game::InputDataPingTarget] = ping;
        AddPlayerInput(Game::InputType::Attack, std::move(data));
        break;
    }
    case AB::GameProtocol::PacketTypeSelect:
    {
        Utils::VariantMap data;
        data[Game::InputDataObjectId] = message.Get<uint32_t>();    // Source
        data[Game::InputDataObjectId2] = message.Get<uint32_t>();   // Target
        AddPlayerInput(Game::InputType::Select, std::move(data));
        break;
    }
    case AB::GameProtocol::PacketTypeClickObject:
    {
        Utils::VariantMap data;
        data[Game::InputDataObjectId] = message.Get<uint32_t>();    // Source
        data[Game::InputDataObjectId2] = message.Get<uint32_t>();   // Target
        AddPlayerInput(Game::InputType::ClickObject, std::move(data));
        break;
    }
    case AB::GameProtocol::PacketTypeQueue:
        AddPlayerTask(&Game::Player::CRQQueueForMatch);
        break;
    case AB::GameProtocol::PacketTypeUnqueue:
        AddPlayerTask(&Game::Player::CRQUnqueueForMatch);
        break;
    case AB::GameProtocol::PacketTypeCommand:
    {
        Utils::VariantMap data;
        data[Game::InputDataCommandType] = message.GetByte();
        data[Game::InputDataCommandData] = message.GetString();
        AddPlayerInput(Game::InputType::Command, std::move(data));
        break;
    }
    case AB::GameProtocol::PacketTypeGetFriendList:
        AddPlayerTask(&Game::Player::CRQGetFriendList);
        break;
    case AB::GameProtocol::PacketTypeAddFriend:
    {
        std::string playerName = message.GetString();
        AB::Entities::FriendRelation rel = static_cast<AB::Entities::FriendRelation>(message.Get<uint8_t>());
        AddPlayerTask(&Game::Player::CRQAddFriend, playerName, rel);
        break;
    }
    case AB::GameProtocol::PacketTypeRemoveFriend:
    {
        std::string accountUuid = message.GetString();
        AddPlayerTask(&Game::Player::CRQRemoveFriend, accountUuid);
        break;
    }
    case AB::GameProtocol::PacketTypeGetFriend:
    {
        std::string accountUuid = message.GetString();
        AddPlayerTask(&Game::Player::CRQGetFriend, accountUuid);
        break;
    }
    case AB::GameProtocol::PacketTypeGetGuildInfo:
        AddPlayerTask(&Game::Player::CRQGetGuildInfo);
        break;
    case AB::GameProtocol::PacketTypeGetGuildMembers:
        AddPlayerTask(&Game::Player::CRQGetGuildMembers);
        break;
    case AB::GameProtocol::PacketTypeGetGuildMember:
    {
        std::string accountUuid = message.GetString();
        AddPlayerTask(&Game::Player::CRQGetGuildMember, accountUuid);
        break;
    }
    case AB::GameProtocol::PacketTypeGetPlayerInfo:
    {
        std::string accountUuid = message.GetString();
        AddPlayerTask(&Game::Player::CRQGetPlayerInfo, accountUuid);
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

    if (!IO::IOAccount_GameWorldAuth(accountUuid, authToken, characterUuid))
    {
        DisconnectClient(AB::Errors::NamePasswordMismatch);
        return;
    }

    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(
            std::bind(&ProtocolGame::Login, GetThis(), characterUuid, uuids::uuid(accountUuid), map, instance)
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
    output->AddByte(error);
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
            std::bind(&ProtocolGame::EnterGame, GetThis())
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
    auto gameMan = GetSubsystem<Game::GameManager>();
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
    else if (gameMan->AddPlayer(player->data_.currentMapUuid, player->GetThis()))
    {
        // Create new instance
        success = true;
        instance = gameMan->GetInstance(player->data_.instanceUuid);
    }

    if (success)
    {
        auto output = OutputMessagePool::GetOutputMessage();
        output->AddByte(AB::GameProtocol::GameEnter);
        output->AddString(ProtocolGame::serverId_);
        output->AddString(player->data_.currentMapUuid);
        output->AddString(player->data_.instanceUuid);
        output->Add<uint32_t>(player->id_);
        output->Add<uint8_t>(static_cast<uint8_t>(instance->data_.type));
        output->Add<uint8_t>(instance->data_.partySize);
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
    output->AddString(serverUuid);               // Server UUID
    output->AddString(mapUuid);                  // Map UUID
    output->AddString(instanceUuid);             // Instance UUID
    output->AddString(player->data_.uuid);       // Character UUID
    Send(output);
}

void ProtocolGame::ChangeInstance(const std::string& mapUuid, const std::string& instanceUuid)
{
    ChangeServerInstance(ProtocolGame::serverId_, mapUuid, instanceUuid);
}

}

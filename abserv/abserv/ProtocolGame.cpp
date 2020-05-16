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

#include "stdafx.h"
#include "ProtocolGame.h"
#include "ConfigManager.h"
#include "Game.h"
#include "GameManager.h"
#include "IOAccount.h"
#include "IOGame.h"
#include "IOPlayer.h"
#include "Player.h"
#include "PlayerManager.h"
#include "stdafx.h"
#include <AB/DHKeys.hpp>
#include <AB/Entities/FriendList.h>
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>

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

void ProtocolGame::Login(AB::Packets::Client::GameLogin packet)
{
#ifdef DEBUG_NET
    LOG_DEBUG << "Player " << packet.charUuid << " logging in" << std::endl;
#endif
    auto* playerMan = GetSubsystem<Game::PlayerManager>();
    ea::shared_ptr<Game::Player> foundPlayer = playerMan->GetPlayerByUuid(packet.charUuid);
    if (foundPlayer)
    {
#ifdef DEBUG_NET
        LOG_DEBUG << "Player " << foundPlayer->GetName() << " already logged in" << std::endl;
#endif
        DisconnectClient(AB::ErrorCodes::AlreadyLoggedIn);
        return;
    }

    if (GetSubsystem<Auth::BanManager>()->IsAccountBanned(uuids::uuid(packet.accountUuid)))
    {
        LOG_INFO << "Login attempt from banned account " << packet.accountUuid << std::endl;
        DisconnectClient(AB::ErrorCodes::AccountBanned);
        return;
    }

    ea::shared_ptr<Game::Player> player = playerMan->CreatePlayer(GetPtr());
    assert(player);

    // Load player and account data from DB
    if (!IO::IOPlayer::LoadPlayerByUuid(*player, packet.charUuid))
    {
        LOG_ERROR << "Error loading player " << packet.charUuid << std::endl;
        DisconnectClient(AB::ErrorCodes::ErrorLoadingCharacter);
        return;
    }
    // Account and player is loaded create the index for the player
    playerMan->UpdatePlayerIndex(*player);

    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    // Check if game exists.
    AB::Entities::Game g;
    g.uuid = packet.mapUuid;
    if (!client->Read(g))
    {
        LOG_ERROR << "Invalid game with map " << packet.mapUuid << std::endl;
        DisconnectClient(AB::ErrorCodes::InvalidGame);
        return;
    }

    player->account_.onlineStatus = AB::Entities::OnlineStatus::OnlineStatusOnline;
    player->account_.currentCharacterUuid = player->data_.uuid;
    player->account_.currentServerUuid = ProtocolGame::serverId_;
    client->Update(player->account_);

    player->Initialize();
    player->data_.currentMapUuid = packet.mapUuid;
    player->data_.lastLogin = Utils::Tick();
    if (!uuids::uuid(packet.instanceUuid).nil())
        player->data_.instanceUuid = packet.instanceUuid;
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
        GetSubsystem<Game::GameManager>()->GetState() != Game::GameManager::State::Running ||
        message.GetSize() == 0)
        return;

    using namespace AB::GameProtocol;

    const ClientPacketTypes recvByte = static_cast<ClientPacketTypes>(message.GetByte());

    switch (recvByte)
    {
    case ClientPacketTypes::Ping:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::Ping>(message);
        AddPlayerTask(&Game::Player::CRQPing, packet.tick);
        break;
    }
    case ClientPacketTypes::Logout:
    {
        /* auto packet = */ AB::Packets::Get<AB::Packets::Client::Logout>(message);
        AddPlayerTask(&Game::Player::CRQLogout);
        break;
    }
    case ClientPacketTypes::ChangeMap:
    {
        // Called by the client when clicking on the map
        auto packet = AB::Packets::Get<AB::Packets::Client::ChangeMap>(message);
        AddPlayerTask(&Game::Player::CRQChangeMap, packet.mapUuid);
        break;
    }
    case ClientPacketTypes::SendMail:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::SendMail>(message);
        AddPlayerTask(&Game::Player::CRQSendMail, packet.recipient, packet.subject, packet.body);
        break;
    }
    case ClientPacketTypes::GetMailHeaders:
    {
        /* auto packet = */ AB::Packets::Get<AB::Packets::Client::GetMailHeaders>(message);
        AddPlayerTask(&Game::Player::CRQGetMailHeaders);
        break;
    }
    case ClientPacketTypes::GetMail:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::GetMail>(message);
        AddPlayerTask(&Game::Player::CRQGetMail, packet.mailUuid);
        break;
    }
    case ClientPacketTypes::DeleteMail:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::DeleteMail>(message);
        AddPlayerTask(&Game::Player::CRQDeleteMail, packet.mailUuid);
        break;
    }
    case ClientPacketTypes::GetInventory:
    {
        /* auto packet = */ AB::Packets::Get<AB::Packets::Client::GetInventory>(message);
        AddPlayerTask(&Game::Player::CRQGetInventory);
        break;
    }
    case ClientPacketTypes::InventoryDestroyItem:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::InventoryDestroyItem>(message);
        AddPlayerTask(&Game::Player::CRQDestroyInventoryItem, packet.pos);
        break;
    }
    case ClientPacketTypes::InventoryDropItem:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::InventoryDropItem>(message);
        AddPlayerTask(&Game::Player::CRQDropInventoryItem, packet.pos, packet.count);
        break;
    }
    case ClientPacketTypes::SetItemPos:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::SetItemPos>(message);
        AddPlayerTask(&Game::Player::CRQSetItemPos,
            static_cast<AB::Entities::StoragePlace>(packet.currentPlace),
            packet.currentPos,
            static_cast<AB::Entities::StoragePlace>(packet.soragePlace),
            packet.storagePos,
            packet.count);
        break;
    }
    case ClientPacketTypes::WithdrawMoney:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::DespositWithdrawMoney>(message);
        AddPlayerTask(&Game::Player::CRQWithdrawMoney, packet.amount);
        break;
    }
    case ClientPacketTypes::DepositMoney:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::DespositWithdrawMoney>(message);
        AddPlayerTask(&Game::Player::CRQDepositMoney, packet.amount);
        break;
    }
    case ClientPacketTypes::GetChest:
    {
        /* auto packet = */ AB::Packets::Get<AB::Packets::Client::GetChest>(message);
        AddPlayerTask(&Game::Player::CRQGetChest);
        break;
    }
    case ClientPacketTypes::ChestDestroyItem:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::ChestDestroyItem>(message);
        AddPlayerTask(&Game::Player::CRQDestroyChestItem, packet.pos);
        break;
    }
    case ClientPacketTypes::PartyInvitePlayer:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::PartyInvitePlayer>(message);
        AddPlayerTask(&Game::Player::CRQPartyInvitePlayer, packet.targetId);
        break;
    }
    case ClientPacketTypes::PartyKickPlayer:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::PartyKickPlayer>(message);
        AddPlayerTask(&Game::Player::CRQPartyKickPlayer, packet.targetId);
        break;
    }
    case ClientPacketTypes::PartyLeave:
    {
        /* auto packet = */ AB::Packets::Get<AB::Packets::Client::PartyLeave>(message);
        AddPlayerTask(&Game::Player::CRQPartyLeave);
        break;
    }
    case ClientPacketTypes::PartyAcceptInvite:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::PartyAcceptInvite>(message);
        AddPlayerTask(&Game::Player::CRQPartyAccept, packet.inviterId);
        break;
    }
    case ClientPacketTypes::PartyRejectInvite:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::PartyRejectInvite>(message);
        AddPlayerTask(&Game::Player::CRQPartyRejectInvite, packet.inviterId);
        break;
    }
    case ClientPacketTypes::GetPartyMembers:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::PartyGetMembers>(message);
        AddPlayerTask(&Game::Player::CRQPartyGetMembers, packet.partyId);
        break;
    }
    case ClientPacketTypes::Move:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::Move>(message);
        Utils::VariantMap data;
        data[Game::InputDataDirection] = packet.direction;
        AddPlayerInput(Game::InputType::Move, std::move(data));
        break;
    }
    case ClientPacketTypes::Turn:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::Turn>(message);
        Utils::VariantMap data;
        data[Game::InputDataDirection] = packet.direction;   // None | Left | Right
        AddPlayerInput(Game::InputType::Turn, std::move(data));
        break;
    }
    case ClientPacketTypes::SetDirection:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::SetDirection>(message);
        Utils::VariantMap data;
        data[Game::InputDataDirection] = packet.rad;   // World angle Rad
        AddPlayerInput(Game::InputType::Direction, std::move(data));
        break;
    }
    case ClientPacketTypes::SetState:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::SetPlayerState>(message);
        Utils::VariantMap data;
        data[Game::InputDataState] = packet.newState;
        AddPlayerInput(Game::InputType::SetState, std::move(data));
        break;
    }
    case ClientPacketTypes::Goto:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::GotoPos>(message);
        Utils::VariantMap data;
        data[Game::InputDataVertexX] = packet.pos[0];
        data[Game::InputDataVertexY] = packet.pos[1];
        data[Game::InputDataVertexZ] = packet.pos[2];
        AddPlayerInput(Game::InputType::Goto, std::move(data));
        break;
    }
    case ClientPacketTypes::Follow:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::Follow>(message);
        Utils::VariantMap data;
        data[Game::InputDataObjectId] = packet.targetId;
        data[Game::InputDataPingTarget] = packet.ping;
        AddPlayerInput(Game::InputType::Follow, std::move(data));
        break;
    }
    case ClientPacketTypes::UseSkill:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::UseSkill>(message);
        // 1 based -> convert to 0 based
        if (packet.index > 0 && packet.index <= Game::PLAYER_MAX_SKILLS)
        {
            Utils::VariantMap data;
            data[Game::InputDataSkillIndex] = packet.index - 1;
            data[Game::InputDataPingTarget] = packet.ping;
            AddPlayerInput(Game::InputType::UseSkill, std::move(data));
        }
        break;
    }
    case ClientPacketTypes::Cancel:
    {
        /* auto packet = */ AB::Packets::Get<AB::Packets::Client::Cancel>(message);
        AddPlayerInput(Game::InputType::Cancel);
        break;
    }
    case ClientPacketTypes::Attack:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::Attack>(message);
        Utils::VariantMap data;
        data[Game::InputDataPingTarget] = packet.ping;
        AddPlayerInput(Game::InputType::Attack, std::move(data));
        break;
    }
    case ClientPacketTypes::Select:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::SelectObject>(message);
        Utils::VariantMap data;
        data[Game::InputDataObjectId] = packet.sourceId;    // Source
        data[Game::InputDataObjectId2] = packet.targetId;   // Target
        AddPlayerInput(Game::InputType::Select, std::move(data));
        break;
    }
    case ClientPacketTypes::ClickObject:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::ClickObject>(message);
        Utils::VariantMap data;
        data[Game::InputDataObjectId] = packet.sourceId;    // Source
        data[Game::InputDataObjectId2] = packet.targetId;   // Target
        AddPlayerInput(Game::InputType::ClickObject, std::move(data));
        break;
    }
    case ClientPacketTypes::Queue:
    {
        /* auto packet = */ AB::Packets::Get<AB::Packets::Client::QueueMatch>(message);
        AddPlayerTask(&Game::Player::CRQQueueForMatch);
        break;
    }
    case ClientPacketTypes::Unqueue:
    {
        /* auto packet = */ AB::Packets::Get<AB::Packets::Client::UnqueueMatch>(message);
        AddPlayerTask(&Game::Player::CRQUnqueueForMatch);
        break;
    }
    case ClientPacketTypes::TradeRequest:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::TradeRequest>(message);
        AddPlayerTask(&Game::Player::CRQTradeRequest, packet.targetId);
        break;
    }
    case ClientPacketTypes::TradeCancel:
    {
        /* auto packet = */ AB::Packets::Get<AB::Packets::Client::TradeCancel>(message);
        AddPlayerTask(&Game::Player::CRQTradeCancel);
        break;
    }
    case ClientPacketTypes::TradeOffer:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::TradeOffer>(message);
        AddPlayerTask(&Game::Player::CRQTradeOffer, packet.money, packet.items);
        break;
    }
    case ClientPacketTypes::TradeAccept:
    {
        /* auto packet = */ AB::Packets::Get<AB::Packets::Client::TradeAccept>(message);
        AddPlayerTask(&Game::Player::CRQTradeAccept);
        break;
    }
    case ClientPacketTypes::Command:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::Command>(message);
        Utils::VariantMap data;
        data[Game::InputDataCommandType] = packet.type;
        data[Game::InputDataCommandData] = packet.data;
        AddPlayerInput(Game::InputType::Command, std::move(data));
        break;
    }
    case ClientPacketTypes::GetFriendList:
    {
        /* auto packet = */ AB::Packets::Get<AB::Packets::Client::UpdateFriendList>(message);
        AddPlayerTask(&Game::Player::CRQGetFriendList);
        break;
    }
    case ClientPacketTypes::AddFriend:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::AddFriend>(message);
        const AB::Entities::FriendRelation rel = static_cast<AB::Entities::FriendRelation>(packet.relation);
        AddPlayerTask(&Game::Player::CRQAddFriend, packet.name, rel);
        break;
    }
    case ClientPacketTypes::RemoveFriend:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::RemoveFriend>(message);
        AddPlayerTask(&Game::Player::CRQRemoveFriend, packet.accountUuid);
        break;
    }
    case ClientPacketTypes::RenameFriend:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::RenameFriend>(message);
        AddPlayerTask(&Game::Player::CRQChangeFriendNick, packet.accountUuid, packet.newName);
        break;
    }
    case ClientPacketTypes::GetPlayerInfoByAccount:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::GetPlayerInfoByAccount>(message);
        AddPlayerTask(&Game::Player::CRQGetPlayerInfoByAccount, packet.accountUuid, packet.fields);
        break;
    }
    case ClientPacketTypes::GetPlayerInfoByName:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::GetPlayerInfoByName>(message);
        AddPlayerTask(&Game::Player::CRQGetPlayerInfoByName, packet.name, packet.fields);
        break;
    }
    case ClientPacketTypes::GetGuildInfo:
    {
        /* auto packet = */ AB::Packets::Get<AB::Packets::Client::GuildInfo>(message);
        AddPlayerTask(&Game::Player::CRQGetGuildInfo);
        break;
    }
    case ClientPacketTypes::GetGuildMembers:
    {
        /* auto packet = */ AB::Packets::Get<AB::Packets::Client::GuildMembers>(message);
        AddPlayerTask(&Game::Player::CRQGetGuildMembers);
        break;
    }
    case ClientPacketTypes::SetOnlineStatus:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::SetOnlineStatus>(message);
        const AB::Entities::OnlineStatus status = static_cast<AB::Entities::OnlineStatus>(packet.newStatus);
        AddPlayerTask(&Game::Player::CRQSetOnlineStatus, status);
        break;
    }
    case ClientPacketTypes::SetSecondaryProfession:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::SetSecondaryProfession>(message);
        AddPlayerTask(&Game::Player::CRQSetSecondaryProfession, packet.profIndex);
        break;
    }
    case ClientPacketTypes::SetAttributeValue:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::SetAttributeValue>(message);
        AddPlayerTask(&Game::Player::CRQSetAttributeValue, packet.attribIndex, packet.value);
        break;
    }
    case ClientPacketTypes::EquipSkill:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::EquipSkill>(message);
        AddPlayerTask(&Game::Player::CRQEquipSkill, packet.skillIndex, packet.pos);
        break;
    }
    case ClientPacketTypes::LoadSkillTemplate:
    {
        auto packet = AB::Packets::Get<AB::Packets::Client::LoadSkillTemplate>(message);
        AddPlayerTask(&Game::Player::CRQLoadSkillTemplate, packet.templ);
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

    auto packet = AB::Packets::Get<AB::Packets::Client::GameLogin>(msg);
    if (packet.protocolVersion != AB::PROTOCOL_VERSION)
    {
        DisconnectClient(AB::ErrorCodes::WrongProtocolVersion);
        return;
    }
    for (int i = 0; i < DH_KEY_LENGTH; ++i)
        clientKey_[i] = packet.key[i];
    auto* keys = GetSubsystem<Crypto::DHKeys>();
    // Switch now to the shared key
    keys->GetSharedKey(clientKey_, encKey_);
    if (Utils::Uuid::IsEmpty(packet.accountUuid))
    {
        LOG_ERROR << "Invalid account " << packet.accountUuid << std::endl;
        DisconnectClient(AB::ErrorCodes::InvalidAccount);
        return;
    }

    const uint32_t ip = GetIP();
    if (GetSubsystem<Auth::BanManager>()->IsIpBanned(ip))
    {
        LOG_ERROR << "Connection attempt from banned IP " << Utils::ConvertIPToString(ip, true) << std::endl;
        DisconnectClient(AB::ErrorCodes::IPBanned);
        return;
    }

    if (!IO::IOAccount::GameWorldAuth(packet.accountUuid, packet.authToken, packet.charUuid))
    {
        DisconnectClient(AB::ErrorCodes::NamePasswordMismatch);
        return;
    }

    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(
            std::bind(&ProtocolGame::Login, GetPtr(), packet)
        )
    );
}

void ProtocolGame::OnConnect()
{
    auto output = OutputMessagePool::GetOutputMessage();
    output->AddByte(AB::GameProtocol::ServerPacketType::KeyExchange);
    auto* keys = GetSubsystem<Crypto::DHKeys>();
    output->AddBytes(reinterpret_cast<const char*>(&keys->GetPublickKey()), DH_KEY_LENGTH);
    Send(std::move(output));
}

void ProtocolGame::DisconnectClient(AB::ErrorCodes error)
{
    auto output = OutputMessagePool::GetOutputMessage();
    output->AddByte(AB::GameProtocol::ServerPacketType::Error);
    AB::Packets::Server::ProtocolError packet = { static_cast<uint8_t>(error) };
    AB::Packets::Add(packet, *output);
    Send(std::move(output));
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
        DisconnectClient(AB::ErrorCodes::CannotEnterGame);
        return;
    }
    auto* gameMan = GetSubsystem<Game::GameManager>();
    bool success = false;
    ea::shared_ptr<Game::Game> instance;
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
        output->AddByte(AB::GameProtocol::ServerPacketType::GameEnter);
        AB::Packets::Server::EnterWorld packet = {
            ProtocolGame::serverId_,
            player->data_.currentMapUuid,
            player->data_.instanceUuid,
            player->id_,
            static_cast<uint8_t>(instance->data_.type),
            instance->data_.partySize
        };
        AB::Packets::Add(packet, *output);
        Send(std::move(output));
    }
    else
        DisconnectClient(AB::ErrorCodes::CannotEnterGame);
}

void ProtocolGame::ChangeServerInstance(const std::string& serverUuid,
    const std::string& mapUuid, const std::string& instanceUuid)
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
    output->AddByte(AB::GameProtocol::ServerPacketType::ChangeInstance);
    AB::Packets::Server::ChangeInstance packet = {
        serverUuid, mapUuid, instanceUuid, player->data_.uuid
    };
    AB::Packets::Add(packet, *output);
    Send(std::move(output));
}

void ProtocolGame::ChangeInstance(const std::string& mapUuid, const std::string& instanceUuid)
{
    ChangeServerInstance(ProtocolGame::serverId_, mapUuid, instanceUuid);
}

}

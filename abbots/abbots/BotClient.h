/**
 * Copyright 2020 Stefan Ascher
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

#pragma once

#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/FriendList.h>
#include <AB/Packets/ServerPackets.h>
#include <AB/ProtocolCodes.h>
#include <Client.h>
#include <Receiver.h>
#include <sa/CircularQueue.h>
#include <AB/DHKeys.hpp>
#include <asio.hpp>
#include <memory>
#include <numeric>
#include "Errors.h"
#include "Receiver.h"
#include "Structs.h"
#include <memory>
#include <absmath/Transformation.h>

class Game;

class BotClient final : public Client::Receiver
{
public:
    enum class State
    {
        Disconnected,
        LoggingIn,
        World
    };
private:
    Client::Client client_;
    std::string authToken_;
    std::string accountUuid_;
    AB::Entities::AccountType accountType_{ AB::Entities::AccountType::Unknown };
    State state_{ State::Disconnected };
    AB::Entities::CharList chars_;
    std::vector<AB::Entities::Game> outposts_;
    std::unique_ptr<Game> game_;
    std::string currentName_;
    uint32_t playerId_{ 0 };
    void SpawnObject(AB::GameProtocol::GameObjectType type, uint32_t id,
        const Math::Vector3& pos, const Math::Vector3& scale, const Math::Quaternion& rot);
public:
    std::string username_;
    std::string password_;
    std::string characterName_;
    std::string script_;

    BotClient(std::shared_ptr<asio::io_service> ioService, const std::string& loginHost, uint16_t loginPort);
    ~BotClient() override;

    State GetState() const { return state_; }

    void Update(uint32_t timeElapsed);
    void Login();
    void Logout();

    void OnLog(const std::string& message) override;
    void OnNetworkError(Client::ConnectionError connectionError, const std::error_code& err) override;
    void OnProtocolError(AB::ErrorCodes err) override;
    void OnPong(int lastPing) override;

    void OnLoggedIn(const std::string& accountUuid,
        const std::string& authToken,
        AB::Entities::AccountType accType) override;
    void OnGetCharlist(const AB::Entities::CharList& chars) override;
    void OnGetOutposts(const std::vector<AB::Entities::Game>& games) override;
    void OnGetServices(const std::vector<AB::Entities::Service>& services) override;
    void OnAccountCreated() override;
    void OnPlayerCreated(const std::string& uuid, const std::string& mapUuid) override;
    void OnAccountKeyAdded() override;
    void OnCharacterDeleted(const std::string& uuid) override;

    void OnPacket(int64_t updateTick, const AB::Packets::Server::ServerJoined& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ServerLeft& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ChangeInstance& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::EnterWorld& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::PlayerAutorun& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSpawn& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSpawnExisting& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::MailHeaders& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::MailComplete& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectDespawn& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectPositionUpdate& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSpeedChanged& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::InventoryContent& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::InventoryItemUpdate& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::InventoryItemDelete& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ChestContent& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ChestItemUpdate& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ChestItemDelete& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectRotationUpdate& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectTargetSelected& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectStateChanged& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::PlayerError& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSkillFailure& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectUseSkill& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSkillSuccess& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectAttackFailure& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectPingTarget& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectEffectAdded& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectEffectRemoved& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectDamaged& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectHealed& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectProgress& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectDroppedItem& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectForcePosition& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectGroupMaskChanged& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSetAttackSpeed& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ServerMessage& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ChatMessage& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::PartyPlayerInvited& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::PartyPlayerRemoved& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::PartyPlayerAdded& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::PartyInviteRemoved& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::PartyResigned& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::PartyDefeated& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::PartyMembersInfo& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectResourceChanged& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::DialogTrigger& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::FriendList& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::PlayerInfo& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::FriendAdded& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::FriendRemoved& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::FriendRenamed& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::GuildInfo& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::GuildMemberList& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::QuestSelectionDialogTrigger& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::QuestDialogTrigger& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::NpcHasQuest& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::QuestDeleted& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::QuestRewarded& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::SetObjectAttributeValue& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSecProfessionChanged& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSetSkill& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::SkillTemplateLoaded& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::TradeDialogTrigger& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::TradeCancel& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::TradeOffer& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::TradeAccepted& packet) override;
};

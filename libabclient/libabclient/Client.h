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

#pragma once

#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/FriendList.h>
#include "Receiver.h"
#include "Structs.h"
#include <AB/ProtocolCodes.h>
#include "Structs.h"
#include <memory>
#include <AB/DHKeys.hpp>
#include "Errors.h"
#include <asio.hpp>
#include <sa/CircularQueue.h>
#include <numeric>
#include <AB/Packets/ServerPackets.h>

namespace httplib {
class SSLClient;
}

namespace Client {

class ProtocolLogin;
class ProtocolGame;

enum class State
{
    Disconnected,
    CreateAccount,
    CreateChar,
    SelectChar,
    World
};

class Client final : public Receiver
{
private:
    Receiver& receiver_;
    std::shared_ptr<asio::io_service> ioService_;
    std::shared_ptr<ProtocolLogin> protoLogin_;
    std::shared_ptr<ProtocolGame> protoGame_;
    std::string accountName_;
    sa::CircularQueue<int, 10> pings_;
    std::unique_ptr<httplib::SSLClient> httpClient_;
    int lastRun_{ 0 };
    int lastPing_{ 0 };
    int64_t lastPongTick_{ 0 };
    int64_t enterWorldMessage_{ 0 };
    bool gotPong_{ true };
    // Our (client) key pair
    Crypto::DHKeys dhKeys_;
    State state_{ State::Disconnected };
    ProtocolLogin& GetProtoLogin();
    void Terminate();

    // Receiver
    void OnLog(const std::string& message) override;
    void OnNetworkError(ConnectionError connectionError, const std::error_code& err) override;
    void OnProtocolError(AB::ErrorCodes err) override;
    void OnPong(int lastPing) override;

    void OnLoggedIn(const std::string& accountUui, const std::string& authTokend, AB::Entities::AccountType accType) override;
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
    void OnPacket(int64_t updateTick, const AB::Packets::Server::MerchantItems& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ItemPrice& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::CraftsmanItems& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::DropTargetChanged& packet) override;
public:
    static const char* GetProtocolErrorMessage(AB::ErrorCodes err);
    static const char* GetNetworkErrorMessage(ConnectionError connectionError);
    explicit Client(Receiver& receiver);
    Client(Receiver& receiver, std::shared_ptr<asio::io_service> ioSerive);
    ~Client() override;

    void ResetPoll();
    void Poll();
    void Run();

    /// Login to login server
    void Login(const std::string& name, const std::string& pass);
    void Logout();
    void GetOutposts();
    void GetServers();
    void CreateAccount(const std::string& name, const std::string& pass,
        const std::string& email, const std::string& accKey);
    void CreatePlayer(const std::string& charName, const std::string& profUuid,
        uint32_t modelIndex,
        AB::Entities::CharacterSex sex, bool isPvp);
    void DeleteCharacter(const std::string& uuid);
    void AddAccountKey(const std::string& newKey);

    /// Connect to game server -> authenticate -> enter game
    void EnterWorld(const std::string& charUuid, const std::string& mapUuid,
        const std::string& host = "", uint16_t port = 0, const std::string& instanceId = "");
    void Update(uint32_t timeElapsed, bool noRun);

    bool HttpRequest(const std::string& path, std::ostream& out);
    bool HttpRequest(const std::string& path, std::function<bool(const char* data, uint64_t size)>&& callback);
    bool HttpDownload(const std::string& path, const std::string& outFile);

    uint32_t GetIp() const;

    std::string accountUuid_;
    std::string password_;
    std::string authToken_;
    std::string loginHost_;
    uint16_t loginPort_;
    std::string fileHost_;
    uint16_t filePort_{ 0 };
    std::string gameHost_;
    uint16_t gamePort_{ 0 };
    // May return nullptr when fileHost_ is empty or filePort_ is 0
    httplib::SSLClient* GetHttpClient();
    State GetState() const { return state_; }
    void SetState(State value) { state_ = value; }
    int GetAvgPing() const
    {
        if (pings_.IsEmpty())
            return 0;
        return std::accumulate(pings_.begin(), pings_.end(), 0) / static_cast<int>(pings_.Size());
    }
    int GetLastPing() const
    {
        if (!pings_.IsEmpty())
            return pings_.Last();
        return 0;
    }
    int64_t GetClockDiff() const;

    /// Causes the server to change the map for the whole party
    void ChangeMap(const std::string& mapUuid);
    void GetMailHeaders();
    void GetMail(const std::string& mailUuid);
    void GetInventory();
    void InventoryDestroyItem(uint16_t pos);
    void InventoryDropItem(uint16_t pos, uint32_t count);
    void SetItemPos(AB::Entities::StoragePlace currentPlace, uint16_t currentPos,
        AB::Entities::StoragePlace place, uint16_t newPos, uint32_t count);
    void GetChest();
    void ChestDestroyItem(uint16_t pos);
    void DepositMoney(uint32_t amount);
    void WithdrawMoney(uint32_t amount);
    void SellItem(uint32_t npcId, uint16_t pos, uint32_t count);
    void BuyItem(uint32_t npcId, uint32_t id, uint32_t count);
    void GetMerchantItems(uint32_t npcId, uint16_t itemType, const std::string& searchName, uint32_t page);
    void GetCraftsmanItems(uint32_t npcId, uint16_t itemType, const std::string& searchName, uint32_t page);
    void CraftItem(uint32_t npcId, uint32_t index, uint32_t count, uint32_t attributeIndex);
    void SalvageItem(uint16_t pos);
    void DeleteMail(const std::string& mailUuid);
    void SendMail(const std::string& recipient, const std::string& subject, const std::string& body);
    void GetPlayerInfoByName(const std::string& name, uint32_t fields);
    void GetPlayerInfoByAccount(const std::string& accountUuid, uint32_t fields);
    void Move(uint8_t direction);
    void Turn(uint8_t direction);
    void SetDirection(float rad);
    void ClickObject(uint32_t sourceId, uint32_t targetId);
    void SelectObject(uint32_t sourceId, uint32_t targetId);
    void FollowObject(uint32_t targetId, bool ping);
    void UseSkill(uint32_t index, bool ping);
    void Attack(bool ping);
    void Command(AB::GameProtocol::CommandType type, const std::string& data);
    void GotoPos(const Vec3& pos);
    void PartyInvitePlayer(uint32_t targetId);
    /// Kick player from party or remove invitation
    void PartyKickPlayer(uint32_t targetId);
    void PartyAcceptInvite(uint32_t inviterId);
    void PartyRejectInvite(uint32_t inviterId);
    void PartyGetMembers(uint32_t partyId);
    void PartyLeave();
    void QueueMatch();
    void UnqueueMatch();
    void AddFriend(const std::string& name, AB::Entities::FriendRelation relation);
    void RemoveFriend(const std::string& accountUuid);
    void RenameFriend(const std::string& accountUuid, const std::string& newName);
    void UpdateFriendList();
    void Cancel();
    void SetPlayerState(AB::GameProtocol::CreatureState newState);
    void SetOnlineStatus(AB::Packets::Server::PlayerInfo::Status status);
    void SetSecondaryProfession(uint32_t profIndex);
    void SetAttributeValue(uint32_t attribIndex, uint8_t value);
    void EquipSkill(uint32_t skillIndex, uint8_t pos);
    void LoadSkillTemplate(const std::string& templ);
    void TradeRequest(uint32_t targetId);
    void TradeCancel();
    void TradeOffer(uint32_t money, std::vector<std::pair<uint16_t, uint32_t>>&& items);
    void TradeAccept();
    void GetItemPrice(const std::vector<uint16_t>& items);
};

}

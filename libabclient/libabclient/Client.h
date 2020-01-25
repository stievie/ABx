#pragma once

#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/FriendList.h>
#include "Receiver.h"
#include "PropStream.h"
#include "Structs.h"
#include <AB/ProtocolCodes.h>
#include "Structs.h"
#include <memory>
#include <AB/DHKeys.hpp>
#include "Errors.h"
#include <asio.hpp>
#include <sa/CircularQueue.h>
#include <numeric>

namespace Client {

class ProtocolLogin;
class ProtocolGame;
class HttpsClient;

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
    std::unique_ptr<ProtocolLogin> protoLogin_;
    std::unique_ptr<ProtocolGame> protoGame_;
    std::string accountName_;
    sa::CircularQueue<int, 10> pings_;
    int lastRun_{ 0 };
    int lastPing_{ 0 };
    bool gotPong_{ true };
    // Our (client) key pair
    Crypto::DHKeys dhKeys_;
    State state_{ State::Disconnected };
    ProtocolLogin& GetProtoLogin();
    void Terminate();
public:
    Client(Receiver& receiver);
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
    void AddAccountKey(const std::string& newKey);

    /// Connect to game server -> authenticate -> enter game
    void EnterWorld(const std::string& charUuid, const std::string& mapUuid,
        const std::string& host = "", uint16_t port = 0, const std::string& instanceId = "");
    void Update(int timeElapsed);

    bool HttpRequest(const std::string& path, std::ostream& out);
    bool HttpDownload(const std::string& path, const std::string& outFile);

    uint32_t GetIp() const;

    // Receiver
    void OnLog(const std::string& message) override;
    void OnNetworkError(ConnectionError connectionError, const std::error_code& err) override;
    void OnProtocolError(AB::ErrorCodes err) override;
    void OnPong(int lastPing) override;

    void OnLoggedIn(const std::string& accountUui, const std::string& authTokend) override;
    void OnGetCharlist(const AB::Entities::CharList& chars) override;
    void OnGetOutposts(const std::vector<AB::Entities::Game>& games) override;
    void OnGetServices(const std::vector<AB::Entities::Service>& services) override;
    void OnAccountCreated() override;
    void OnPlayerCreated(const std::string& uuid, const std::string& mapUuid) override;
    void OnAccountKeyAdded() override;

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
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectPosUpdate& packet) override;
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
    void OnPacket(int64_t updateTick, const AB::Packets::Server::GameError& packet) override;
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
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSetPosition& packet) override;
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
    void OnPacket(int64_t updateTick, const AB::Packets::Server::GuildInfo& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::GuildMemberList& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::QuestSelectionDialogTrigger& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::QuestDialogTrigger& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::NpcHasQuest& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::QuestDeleted& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::QuestRewarded& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::SetPlayerAttributeValue& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSecProfessionChanged& packet) override;
    void OnPacket(int64_t updateTick, const AB::Packets::Server::SetPlayerSkill& packet) override;

    std::string accountUuid_;
    std::string password_;
    std::string authToken_;
    std::string loginHost_;
    uint16_t loginPort_;
    std::string fileHost_;
    uint16_t filePort_{ 0 };
    std::string gameHost_;
    uint16_t gamePort_{ 0 };
    HttpsClient* httpClient_{ nullptr };
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
    void InventoryStoreItem(uint16_t pos);
    void InventoryDestroyItem(uint16_t pos);
    void InventoryDropItem(uint16_t pos);
    void GetChest();
    void ChestDestroyItem(uint16_t pos);
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
    void UpdateFriendList();
    void Cancel();
    void SetPlayerState(AB::GameProtocol::CreatureState newState);
    void SetOnlineStatus(AB::Packets::Server::PlayerInfo::Status status);
    void SetSecondaryProfession(uint32_t profIndex);
    void SetAttributeValue(uint32_t attribIndex, uint8_t value);
    void EquipSkill(uint32_t skillIndex, uint8_t pos);
};

}

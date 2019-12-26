#pragma once

#include "Client.h"
#include "Receiver.h"
#include <AB/ProtocolCodes.h>
#include "Structs.h"
#include <AB/Entities/Character.h>
#include <AB/Entities/Profession.h>
#include <AB/Entities/Skill.h>
#include <AB/Entities/Effect.h>
#include <AB/Entities/Music.h>
#include <AB/Entities/FriendList.h>
#include <map>
#include "Options.h"

struct EventItem
{
    StringHash eventId;
    VariantMap eventData;
};

struct InventoryItem
{
    AB::Entities::ItemType type;
    uint32_t index;
    AB::Entities::StoragePlace place;
    uint16_t pos;
    uint32_t count;
    uint16_t value;
};

class FwClient : public Object, public Client::Receiver
{
    URHO3D_OBJECT(FwClient, Object);
private:
    String currentLevel_;
    String currentMapUuid_;
    bool levelReady_{ false };
    Vector<EventItem> queuedEvents_;
    uint32_t playerId_{ 0 };
    Client::Client client_;
    Client::Client::ClientState lastState_;
    AB::Entities::CharList characters_;
    std::map<std::string, AB::Entities::Game> outposts_;
    std::map<std::string, AB::Entities::Game> games_;
    std::map<std::string, AB::Entities::Service> services_;
    std::vector<AB::Entities::MailHeader> mailHeaders_;
    std::vector<InventoryItem> inventory_;
    std::vector<InventoryItem> chest_;
    std::vector<std::string> friendList_;
    std::vector<std::string> guildMembers_;
    std::map<std::string, Client::RelatedAccount> relatedAccounts_;
    String currentServerId_;
    AB::Entities::Mail currentMail_;
    String currentCharacterUuid_;
    Mutex mutex_;
    HashMap<String, uint32_t> versions_;
    void LoadData();
    static bool IsOldData(uint32_t curVersion, XMLFile* file);
    void LoadGames(uint32_t curVersion);
    void LoadSkills(uint32_t curVersion);
    void LoadAttributes(uint32_t curVersion);
    void LoadProfessions(uint32_t curVersion);
    void LoadEffects(uint32_t curVersion);
    void LoadItems(uint32_t curVersion);
    void LoadMusic(uint32_t curVersion);
    bool MakeHttpRequest(const String& path, const String& outFile);
    bool loggedIn_{ false };
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleLevelReady(StringHash eventType, VariantMap& eventData);
    void QueueEvent(StringHash eventType, VariantMap& eventData);
    void UpdatePlayer(const Client::RelatedAccount& player);
public:
    static String GetProtocolErrorMessage(uint8_t err);
    static String GetSkillErrorMessage(AB::GameProtocol::SkillError err);
    static String GetAttackErrorMessage(AB::GameProtocol::AttackError err);
    static String GetGameErrorMessage(AB::GameProtocol::PlayerErrorValue err);

    FwClient(Context* context);
    ~FwClient();

    void SetEnvironment(const Environment* env);
    uint32_t GetIp() const
    {
        if (loggedIn_)
            return client_.GetIp();
        return 0;
    }
    /// Get difference of server and client clock
    int64_t GetClockDiff() const
    {
        if (loggedIn_)
            return client_.GetClockDiff();
        return 0;
    }
    void UpdateServers();
    bool Start();
    void Stop();
    void UnsubscribeUpdate();
    void SubscribeUpdate();
    void Update(float timeStep);
    void Login(const String& name, const String& pass);
    void CreateAccount(const String& name, const String& pass, const String& email, const String& accKey);
    void CreatePlayer(const String& name, const String& profUuid, uint32_t modelIndex,
        AB::Entities::CharacterSex sex, bool isPvp);
    void EnterWorld(const String& charUuid, const String& mapUuid);
    void ChangeWorld(const String& mapUuid);
    /// Causes the server to change the map for the whole party
    void ChangeMap(const String& mapUuid);
    void ChangeServer(const String& serverId);
    void Logout();
    void GetMailHeaders();
    void ReadMail(const std::string& uuid);
    void DeleteMail(const std::string& uuid);
    void SendMail(const std::string& recipient, const std::string& subject, const std::string& body);
    void GetPlayerInfoByName(const std::string& name, uint32_t fields);
    void GetPlayerInfoByAccount(const std::string& accountUuid, uint32_t fields);
    void UpdateInventory();
    void InventoryStoreItem(uint16_t pos);
    void InventoryDestroyItem(uint16_t pos);
    void InventoryDropItem(uint16_t pos);
    void UpdateChest();
    void ChestDestroyItem(uint16_t pos);
    void Move(uint8_t direction);
    void Turn(uint8_t direction);
    void SetDirection(float rad);
    void ClickObject(uint32_t sourceId, uint32_t targetId);
    void SelectObject(uint32_t sourceId, uint32_t targetId);
    void Command(AB::GameProtocol::CommandTypes type, const String& data);
    void GotoPos(const Vector3& pos);
    void FollowObject(uint32_t objectId);
    void SetPlayerState(AB::GameProtocol::CreatureState newState);
    void UseSkill(uint32_t index);
    void Attack();
    void Cancel();
    void AddFriend(const String& name, AB::Entities::FriendRelation relation);
    void RemoveFriend(const String& accountUuid);
    void UpdateFriendList();
    void SetOnlineStatus(Client::RelatedAccount::Status status);
    void PartyInvitePlayer(uint32_t objectId);
    /// Remove as party member or remove invitation
    void PartyKickPlayer(uint32_t objectId);
    void PartyAcceptInvite(uint32_t inviterId);
    void PartyRejectInvite(uint32_t inviterId);
    void PartyGetMembers(uint32_t partyId);
    void PartyLeave();
    void QueueMatch();
    void UnqueueMatch();

    void OnLog(const std::string& message) override;
    /// asio network error
    void OnNetworkError(Client::ConnectionError connectionError, const std::error_code& err) override;
    /// Protocol error, e.g. Login failed
    void OnProtocolError(uint8_t err) override;
    void OnPong(int) override { }

    void OnLoggedIn(const std::string&, const std::string&) override;
    void OnGetCharlist(const AB::Entities::CharList& chars) override;
    void OnGetOutposts(const std::vector<AB::Entities::Game>& games) override;
    void OnGetServices(const std::vector<AB::Entities::Service>& services) override;
    void OnAccountCreated() override;
    void OnPlayerCreated(const std::string& uuid, const std::string& mapUuid) override;

    void OnResourceChanged(int64_t updateTick, uint32_t id,
        AB::GameProtocol::ResourceType resType, int16_t value) override;
    void OnDialogTrigger(int64_t updateTick, uint32_t dialogId) override;
    void OnPlayerInfo(int64_t updateTick, const Client::RelatedAccount& player) override;
    void OnFriendList(int64_t updateTick, const std::vector<std::string>& list) override;
    void OnFriendAdded(int64_t updateTick, const std::string& accountUuid, Client::RelatedAccount::Relation relation) override;
    void OnFriendRemoved(int64_t updateTick, const std::string& accountUuid, Client::RelatedAccount::Relation relation) override;
    void OnGuildMemberList(int64_t updateTick, const std::vector<std::string>& list) override;
    void OnGuildInfo(int64_t updateTick, const AB::Entities::Guild& guild) override;
    void OnQuestSelectionDialogTrigger(int64_t updateTick, const std::set<uint32_t>& quests) override;
    void OnQuestDialogTrigger(int64_t updateTick, uint32_t questIndex) override;
    void OnNpcHasQuest(int64_t updateTick, uint32_t npcId, bool hasQuest) override;
    void OnQuestDeleted(int64_t updateTick, uint32_t index, bool deleted) override;
    void OnQuestRewarded(int64_t updateTick, uint32_t index, bool rewarded) override;

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

    void SetState(Client::Client::ClientState state)
    {
        if (state == Client::Client::ClientState::Disconnected || state == Client::Client::ClientState::CreateAccount)
            loggedIn_ = false;
        client_.state_ = state;
    }
    const std::map<std::string, AB::Entities::Service>& GetServices() const
    {
        return services_;
    }
    const String& GetCurrentServerId() const
    {
        return currentServerId_;
    }
    const String& GetCurrentCharacterUuid() const
    {
        return currentCharacterUuid_;
    }
    const AB::Entities::CharList& GetCharacters() const
    {
        return characters_;
    }
    const std::string& GetAccountUuid() const;
    const AB::Entities::Game* GetGame(const String& uuid) const
    {
        auto it = games_.find(std::string(uuid.CString()));
        if (it == games_.end())
            return nullptr;
        return &(*it).second;

    }
    const std::map<std::string, AB::Entities::Game>& GetOutposts() const
    {
        return outposts_;
    }
    String GetGameName(const String& uuid) const
    {
        auto it = games_.find(std::string(uuid.CString()));
        if (it == games_.end())
            return "";
        return String((*it).second.name.c_str());
    }
    uint8_t GetPartySize(const String& uuid) const
    {
        auto it = games_.find(std::string(uuid.CString()));
        if (it == games_.end())
            return 1;
        return (*it).second.partySize;
    }

    const std::vector<AB::Entities::MailHeader>& GetCurrentMailHeaders() const
    {
        return mailHeaders_;
    }
    const AB::Entities::Mail& GetCurrentMail() const
    {
        return currentMail_;
    }
    const std::vector<InventoryItem>& GetInventoryItems() const
    {
        return inventory_;
    }
    const InventoryItem& GetInventoryItem(uint16_t pos) const
    {
        const auto it = std::find_if(inventory_.begin(), inventory_.end(), [pos](const InventoryItem& current) -> bool
        {
            return current.pos == pos;
        });
        if (it == inventory_.end())
        {
            static InventoryItem empty;
            return empty;
        }

        return (*it);
    }

    const std::vector<std::string>& GetFriendList() const
    {
        return friendList_;
    }
    const std::vector<std::string>& GetGuildMembers() const
    {
        return guildMembers_;
    }
    const Client::RelatedAccount* GetRelatedAccount(const String& accountUuid) const;

    int GetAvgPing() const
    {
        return client_.GetAvgPing();
    }
    int GetLastPing() const
    {
        return client_.GetLastPing();
    }
    uint32_t GetPlayerId() const
    {
        return playerId_;
    }

    String accountName_;
    String accountPass_;
};


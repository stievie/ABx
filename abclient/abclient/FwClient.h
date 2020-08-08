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
#include <AB/Entities/Game.h>
#include <map>
#include "Options.h"
#include <AB/Packets/ServerPackets.h>
#include <sa/PropStream.h>
#include <abshared/Items.h>

struct EventItem
{
    StringHash eventId;
    VariantMap eventData;
};

using ItemStats = HashMap<Game::ItemStatIndex, Variant>;
struct ConcreteItem
{
    uint32_t id{ 0 };               // Server ID
    AB::Entities::ItemType type;
    uint32_t index;
    AB::Entities::StoragePlace place;
    uint16_t pos;
    uint32_t count;
    uint32_t value;
    ItemStats stats;
    uint32_t flags;
};


bool VariantMapRead(ItemStats& vMap, sa::PropReadStream& stream);
void VariantMapWrite(const ItemStats& vMap, sa::PropWriteStream& stream);
void LoadStatsFromString(ItemStats& stats, const std::string& value);
void LoadStatsFromString(ItemStats& stats, const String& value);
String SaveStatsToString(const ItemStats& stats);

namespace Urho3D {
template <>
inline unsigned MakeHash<Game::ItemStatIndex>(const Game::ItemStatIndex& value)
{
    return static_cast<unsigned>(value);
}
}

struct InventoryLimit
{
    uint32_t maxMoney{ 0 };
    uint32_t maxItems{ 0 };
};

class FwClient final : public Object, public Client::Receiver
{
    URHO3D_OBJECT(FwClient, Object)
    friend void PingServerWork(const WorkItem*, unsigned);
private:
    String currentLevel_;
    String currentMapUuid_;
    AB::Entities::GameType currentGameType_{ AB::Entities::GameTypeUnknown };
    bool levelReady_{ false };
    Vector<EventItem> queuedEvents_;
    uint32_t playerId_{ 0 };
    Client::Client client_;
    Client::State lastState_{ Client::State::Disconnected };
    AB::Entities::CharList characters_;
    std::map<std::string, AB::Entities::Game> outposts_;
    std::map<std::string, AB::Entities::Game> games_;
    std::map<std::string, AB::Entities::Service> services_;
    std::vector<AB::Entities::MailHeader> mailHeaders_;
    std::vector<ConcreteItem> inventory_;
    std::vector<ConcreteItem> chest_;
    std::vector<ConcreteItem> merchantItems_;
    std::vector<AB::Entities::ItemType> merchantItemTypes_;
    uint32_t merchantItemsPage_{ 1 };
    uint32_t merchantItemsPageCount_{ 0 };
    InventoryLimit inventoryLimit_;
    InventoryLimit chestLimit_;
    std::vector<std::string> friendList_;
    std::vector<std::string> guildMembers_;
    AB::Packets::Server::TradeOffer currentPartnerOffer_;
    std::map<std::string, AB::Packets::Server::PlayerInfo> relatedAccounts_;
    String currentServerId_;
    AB::Entities::Mail currentMail_;
    String currentCharacterUuid_;
    Mutex mutex_;
    HashMap<String, uint32_t> versions_;
    bool loggedIn_{ false };

    AB::Entities::AccountType accountType_{ AB::Entities::AccountType::Unknown };
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
    bool MakeHttpRequest(const String& path, PODVector<unsigned char>& buffer);
    bool MakeHttpRequest(const String& path, const std::function<void(unsigned size, const PODVector<unsigned char>&)>& onData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleLevelReady(StringHash eventType, VariantMap& eventData);
    void QueueEvent(StringHash eventType, VariantMap& eventData);
    void UpdatePlayer(const AB::Packets::Server::PlayerInfo& player);
    void HandleWorkCompleted(StringHash eventType, VariantMap& eventData);

    void OnLog(const std::string& message) override;
    /// asio network error
    void OnNetworkError(Client::ConnectionError connectionError, const std::error_code& err) override;
    /// Protocol error, e.g. Login failed
    void OnProtocolError(AB::ErrorCodes err) override;
    void OnPong(int) override { }

    void OnLoggedIn(const std::string&, const std::string&, AB::Entities::AccountType accType) override;
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
    static const char* GetItemTypeName(AB::Entities::ItemType type);
    static String GetProtocolErrorMessage(AB::ErrorCodes err);
    static String GetSkillErrorMessage(AB::GameProtocol::SkillError err);
    static String GetAttackErrorMessage(AB::GameProtocol::AttackError err);
    static String GetGameErrorMessage(AB::GameProtocol::PlayerErrorValue err);

    FwClient(Context* context);
    ~FwClient() override;

    void SetEnvironment(const Environment* env);
    void PingServer(const String& name, const String& host, uint16_t port);
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
    AB::Entities::AccountType GetAccountType() const { return accountType_; }
    void UpdateServers();
    void AddAccountKey(const String& newKey);
    void DeleteCharacter(const String& uuid);
    bool Start();
    void Stop();
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
    void GetPlayerInfoByAccount(const std::string& accountUuid, uint32_t fields, bool refresh = false);
    void UpdateInventory();
    void InventoryDestroyItem(uint16_t pos);
    void InventoryDropItem(uint16_t pos, uint32_t count);
    void SetItemPos(AB::Entities::StoragePlace currentPlace, uint16_t currentPos,
        AB::Entities::StoragePlace place, uint16_t newPos, uint32_t count);
    void UpdateChest();
    void ChestDestroyItem(uint16_t pos);
    void DepositMoney(uint32_t amount);
    void WithdrawMoney(uint32_t amount);
    void SellItem(uint32_t npcId, uint16_t pos, uint32_t count);
    void BuyItem(uint32_t npcId, uint32_t id, uint32_t count);
    void RequestMerchantItems(uint32_t npcId, uint16_t itemType = 0, const String& searchName = String::EMPTY, uint32_t page = 1);
    void RequestCrafsmanItems(uint32_t npcId, uint16_t itemType = 0, const String& searchName = String::EMPTY, uint32_t page = 1);
    void CraftItem(uint32_t npcId, uint32_t index, uint32_t count, uint32_t attributeIndex);
    void SalvageItem(uint16_t kitPos, uint16_t pos);
    void Move(uint8_t direction);
    void Turn(uint8_t direction);
    void SetDirection(float rad);
    void ClickObject(uint32_t sourceId, uint32_t targetId);
    void SelectObject(uint32_t sourceId, uint32_t targetId);
    void Command(AB::GameProtocol::CommandType type, const String& data);
    void GotoPos(const Vector3& pos);
    void SetPlayerState(AB::GameProtocol::CreatureState newState);
    void UseSkill(uint32_t index);
    void Interact();
    void Cancel();
    void AddFriend(const String& name, AB::Entities::FriendRelation relation);
    void RemoveFriend(const String& accountUuid);
    void RenameFriend(const String& accountUuid, const String& newName);
    void UpdateFriendList();
    void SetOnlineStatus(AB::Packets::Server::PlayerInfo::Status status);
    void PartyInvitePlayer(uint32_t objectId);
    /// Remove as party member or remove invitation
    void PartyKickPlayer(uint32_t objectId);
    void PartyAcceptInvite(uint32_t inviterId);
    void PartyRejectInvite(uint32_t inviterId);
    void PartyGetMembers(uint32_t partyId);
    void PartyLeave();
    void QueueMatch();
    void UnqueueMatch();
    void SetSecondaryProfession(uint32_t profIndex);
    void SetAttributeValue(uint32_t attribIndex, uint8_t value);
    void EquipSkill(uint32_t skillIndex, uint8_t pos);
    void LoadSkillTemplate(const std::string& templ);
    void TradeRequest(uint32_t targetId);
    void TradeCancel();
    void TradeOffer(uint32_t money, std::vector<std::pair<uint16_t, uint32_t>>&& items);
    void TradeAccept();
    void GetItemPrice(const std::vector<uint16_t>& items);

    void SetState(Client::State state)
    {
        if (state == Client::State::Disconnected || state == Client::State::CreateAccount)
            loggedIn_ = false;
        client_.SetState(state);
    }
    std::vector<AB::Entities::Service> GetServices() const;
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

    const AB::Packets::Server::TradeOffer& GetCurrentPartnersOffer() const
    {
        return currentPartnerOffer_;
    }
    const std::vector<AB::Entities::MailHeader>& GetCurrentMailHeaders() const
    {
        return mailHeaders_;
    }
    const AB::Entities::Mail& GetCurrentMail() const
    {
        return currentMail_;
    }
    const InventoryLimit& GetInventoryLimit() const { return inventoryLimit_; }
    const std::vector<ConcreteItem>& GetInventoryItems() const
    {
        return inventory_;
    }
    uint32_t GetInventoryMoney() const
    {
        const auto it = std::find_if(inventory_.begin(), inventory_.end(), [](const ConcreteItem& current) -> bool
        {
            return current.type == AB::Entities::ItemType::Money;
        });
        if (it == inventory_.end())
        {
            return 0;
        }

        return (*it).count;
    }

    const ConcreteItem& GetInventoryItem(uint16_t pos) const
    {
        const auto it = std::find_if(inventory_.begin(), inventory_.end(), [pos](const ConcreteItem& current) -> bool
        {
            return current.pos == pos;
        });
        if (it == inventory_.end())
        {
            static ConcreteItem empty;
            return empty;
        }

        return (*it);
    }
    const InventoryLimit& GetChestLimit() const { return chestLimit_; }
    const std::vector<ConcreteItem>& GetChestItems() const
    {
        return chest_;
    }
    uint32_t GetChestMoney() const
    {
        const auto it = std::find_if(chest_.begin(), chest_.end(), [](const ConcreteItem& current) -> bool
        {
            return current.type == AB::Entities::ItemType::Money;
        });
        if (it == chest_.end())
        {
            return 0;
        }

        return (*it).count;
    }
    const ConcreteItem& GetChestItem(uint16_t pos) const
    {
        const auto it = std::find_if(chest_.begin(), chest_.end(), [pos](const ConcreteItem& current) -> bool
        {
            return current.pos == pos;
        });
        if (it == inventory_.end())
        {
            static ConcreteItem empty;
            return empty;
        }

        return (*it);
    }
    const std::vector<ConcreteItem>& GetMerchantItems() const
    {
        return merchantItems_;
    }
    const std::vector<AB::Entities::ItemType>& GetMerchantItemTypes() const
    {
        return merchantItemTypes_;
    }
    uint32_t GetMerchantItemsPage() const { return merchantItemsPage_; }
    uint32_t GetMerchantItemsPageCount() const { return merchantItemsPageCount_; }

    const std::vector<std::string>& GetFriendList() const
    {
        return friendList_;
    }
    const std::vector<std::string>& GetGuildMembers() const
    {
        return guildMembers_;
    }
    const AB::Packets::Server::PlayerInfo* GetRelatedAccount(const String& accountUuid) const;

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

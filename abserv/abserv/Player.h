#pragma once

#include "ProtocolGame.h"
#include "GameObject.h"
#include "Game.h"
#include "Effect.h"
#include "Actor.h"
#include <AB/Entities/Character.h>
#include <AB/Entities/Account.h>
#include "Party.h"
#include "QuestComp.h"
#include <AB/Entities/FriendList.h>

namespace Game {

class PlayerManager;
class MailBox;
class FriendList;

class Player final : public Actor
{
    friend class PlayerManager;
private:
    // The Player and ConnectionManager owns the client. The client has a weak ref of the player.
    std::shared_ptr<Net::ProtocolGame> client_;
    std::unique_ptr<MailBox> mailBox_;
    std::unique_ptr<FriendList> friendList_;
    std::shared_ptr<Party> party_;
    bool resigned_;
    bool queueing_{ false };
    Party* _LuaGetParty();
    void LoadFriendList();

    void OnPingObject(uint32_t targetId, AB::GameProtocol::ObjectCallType type, int skillIndex);
    void OnInventoryFull();
    void OnHandleCommand(AB::GameProtocol::CommandTypes type,
        const std::string& arguments, Net::NetworkMessage& message);
    void HandleServerIdCommand(const std::string&, Net::NetworkMessage&);
    void HandleWhisperCommand(const std::string& arguments, Net::NetworkMessage& message);
    void HandleChatGuildCommand(const std::string& arguments, Net::NetworkMessage&);
    void HandleChatTradeCommand(const std::string& arguments, Net::NetworkMessage&);
    void HandleResignCommand(const std::string& arguments, Net::NetworkMessage& message);
    void HandleStuckCommand(const std::string& arguments, Net::NetworkMessage& message);
    void HandleAgeCommand(const std::string& arguments, Net::NetworkMessage& message);
    void HandleHpCommand(const std::string& arguments, Net::NetworkMessage& message);
    void HandleXpCommand(const std::string& arguments, Net::NetworkMessage& message);
    void HandlePosCommand(const std::string& arguments, Net::NetworkMessage& message);
    void HandleRollCommand(const std::string& arguments, Net::NetworkMessage& message);
    void HandleSitCommand(const std::string&, Net::NetworkMessage&);
    void HandleStandCommand(const std::string&, Net::NetworkMessage&);
    void HandleCryCommand(const std::string&, Net::NetworkMessage&);
    void HandleTauntCommand(const std::string&, Net::NetworkMessage&);
    void HandlePonderCommand(const std::string&, Net::NetworkMessage&);
    void HandleWaveCommand(const std::string&, Net::NetworkMessage&);
    void HandleLaughCommand(const std::string&, Net::NetworkMessage&);
    void HandleDeathsCommand(const std::string&, Net::NetworkMessage&);
    void HandleDieCommand(const std::string&, Net::NetworkMessage&);
    void HandleInstancesCommand(const std::string&, Net::NetworkMessage&);
    void HandleGeneralChatCommand(const std::string& arguments, Net::NetworkMessage&);
    void HandlePartyChatCommand(const std::string& arguments, Net::NetworkMessage&);
    void HandleGodModeCommand(const std::string&, Net::NetworkMessage&);
    void HandleGMInfoCommand(const std::string&, Net::NetworkMessage&);
    void HandleEnterMapCommand(const std::string&, Net::NetworkMessage&);
    void HandleEnterInstanceCommand(const std::string&, Net::NetworkMessage&);
    void HandleGotoPlayerCommand(const std::string&, Net::NetworkMessage&);
    // Send command unknown message to the client
    void HandleUnknownCommand();
    void SendFriendInfo(AB::GameProtocol::GameProtocolCodes code, const AB::Entities::Friend& f);
public:
    static void RegisterLua(kaguya::State& state);

    explicit Player(std::shared_ptr<Net::ProtocolGame> client);
    ~Player() override;
    // non-copyable
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;
    inline std::shared_ptr<Player> GetThis()
    {
        return std::static_pointer_cast<Player>(shared_from_this());
    }

    /// We are entering a game
    void SetGame(std::shared_ptr<Game> game) override;
    const std::string& GetName() const override { return data_.name; }
    AB::Entities::CharacterSex GetSex() const override
    {
        return data_.sex;
    }
    uint32_t GetItemIndex() const override
    {
        return data_.modelIndex;
    }
    uint32_t GetGroupId() const override
    {
        return party_->id_;
    }
    size_t GetGroupPos() override;

    bool CanAttack() const override;
    bool CanUseSkill() const override;
    uint32_t GetLevel() const override { return data_.level; }
    void SetLevel(uint32_t value) override { data_.level = static_cast<uint8_t>(value); }

    void AddXp(int value) override;
    uint32_t GetXp() const override { return data_.xp; }
    void AddSkillPoint() override;
    uint32_t GetSkillPoints() const override { return data_.skillPoints; }
    void AdvanceLevel() override;
    AB::GameProtocol::GameObjectType GetType() const override
    {
        return AB::GameProtocol::ObjectTypePlayer;
    }
    uint32_t GetInactiveTime() const
    {
        if (lastPing_ == 0)
            return 0;
        return Utils::TimeElapsed(lastPing_);
    }

    void Initialize() override;
    void Logout();
    void TriggerDialog(uint32_t dialogIndex);
    void ChangeMap(const std::string& mapUuid);
    void ChangeInstance(const std::string& mapUuid, const std::string& instanceUuid);
    void ChangeServerInstance(const std::string& serverUuid, const std::string& mapUuid, const std::string& instanceUuid);
    void UpdateMailBox();
    void NotifyNewMail();

    void WriteToOutput(const Net::NetworkMessage& message);
    bool IsResigned() const { return resigned_; }

    void SetParty(std::shared_ptr<Party> party);
    std::shared_ptr<Party> GetParty() const
    {
        return party_;
    }
    FriendList* GetFriendList()
    {
        if (friendList_)
            return friendList_.get();
        return nullptr;
    }

    void Update(uint32_t timeElapsed, Net::NetworkMessage& message) override;

    const std::string& GetPlayerUuid() const override { return data_.uuid; }
    const std::string& GetAccountUuid() const override { return account_.uuid; }

    bool AddToInventory(uint32_t itemId) override;
    void EquipInventoryItem(uint16_t pos);
    /// Leave current party
    void PartyLeave();

    /// This player ignores player
    bool IsIgnored(const Player& player);
    bool IsFriend(const Player& player);

    //{ Client requests.
    // These functions should not have references as arguments, because
    // they are executed by the dispatcher and not directly.
    void CRQChangeMap(const std::string mapUuid);
    void CRQLogout();
    void CRQPing(int64_t clientTick);
    void CRQGetMailHeaders();
    void CRQSendMail(const std::string recipient, const std::string subject, const std::string body);
    void CRQGetMail(const std::string mailUuid);
    void CRQDeleteMail(const std::string mailUuid);
    void CRQQueueForMatch();
    void CRQUnqueueForMatch();
    void CRQAddFriend(const std::string playerName, AB::Entities::FriendRelation relation);
    void CRQRemoveFriend(const std::string accountUuid);
    void CRQGetFriendByAccount(const std::string accountUuid);
    void CRQGetFriendByName(const std::string name);
    void CRQGetGuildMember(const std::string accountUuid);
    void CRQGetPlayerInfo(const std::string accountUuid);
    void CRQGetGuildInfo();
    /// Client requested the friend list
    void CRQGetFriendList();
    /// Client requested guild members
    void CRQGetGuildMembers();
    /// Client requests the inventory
    void CRQGetInventory();
    void CRQDropInventoryItem(uint16_t pos);
    void CRQDestroyInventoryItem(uint16_t pos);
    /// Move an inventory item to the chest
    void CRQStoreInChest(uint16_t pos);
    void CRQGetChest();
    void CRQDestroyChestItem(uint16_t pos);
    void CRQPartyInvitePlayer(uint32_t playerId);
    void CRQPartyKickPlayer(uint32_t playerId);
    void CRQPartyLeave();
    void CRQPartyAccept(uint32_t inviterId);
    /// We reject the invite of inviterId
    void CRQPartyRejectInvite(uint32_t inviterId);
    void CRQPartyGetMembers(uint32_t partyId);
    //}

    AB::Entities::Character data_;
    AB::Entities::Account account_;
    time_t loginTime_;
    time_t logoutTime_;
    int64_t lastPing_{ 0 };
    std::unique_ptr<Components::QuestComp> questComp_;
};

}

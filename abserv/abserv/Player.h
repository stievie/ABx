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

#include "Actor.h"
#include "Effect.h"
#include "Game.h"
#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/FriendList.h>
#include <AB/Entities/ItemPrice.h>
#include <eastl.hpp>
#include <set>
#include <sa/time.h>

namespace Net {
class ProtocolGame;
}

namespace Game {

class PlayerManager;
class MailBox;
class FriendList;
class Party;

namespace Components {
class QuestComp;
class TradeComp;
class InteractionComp;
}

class Player final : public Actor
{
    friend class PlayerManager;
private:
    // The Player and ConnectionManager owns the client. The client has a weak ref of the player.
    std::shared_ptr<Net::ProtocolGame> client_;
    ea::unique_ptr<MailBox> mailBox_;
    ea::unique_ptr<FriendList> friendList_;
    ea::shared_ptr<Party> party_;
    bool resigned_{ false  };
    bool queueing_{ false };
    ea::map<std::string, AB::Entities::ItemPrice> calculatedItemPrices_;
    Party* _LuaGetParty();
    void LoadFriendList();
    MailBox& GetMailBox();

    void OnPingObject(uint32_t targetId, AB::GameProtocol::ObjectCallType type, int skillIndex);
    void OnInventoryFull();
    void OnChestFull();
    void OnHandleCommand(AB::GameProtocol::CommandType type,
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
    void HandleCheckStepOnCommand(const std::string&, Net::NetworkMessage&);
    void HandleEnterMapCommand(const std::string&, Net::NetworkMessage&);
    void HandleEnterInstanceCommand(const std::string&, Net::NetworkMessage&);
    void HandleGotoPlayerCommand(const std::string&, Net::NetworkMessage&);
    // Send command unknown message to the client
    void HandleUnknownCommand();
public:
    static void RegisterLua(kaguya::State& state);
    static void PlayerError(AB::GameProtocol::PlayerErrorValue, Net::NetworkMessage& message);

    explicit Player(std::shared_ptr<Net::ProtocolGame> client);
    ~Player() override;

    void SendError(AB::GameProtocol::PlayerErrorValue);
    /// We are entering a game
    void SetGame(ea::shared_ptr<Game> game) override;
    const std::string& GetName() const override { return data_.name; }
    AB::Entities::CharacterSex GetSex() const override
    {
        return data_.sex;
    }
    uint32_t GetItemIndex() const override
    {
        return data_.modelIndex;
    }
    size_t GetGroupPos() override;

    bool CanAttack() const override;
    bool CanUseSkill() const override;
    uint32_t GetLevel() const override { return data_.level; }
    void SetLevel(uint32_t value) override;

    void AddXp(int value) override;
    uint32_t GetXp() const override { return data_.xp; }
    void AddSkillPoint() override;
    uint32_t GetSkillPoints() const override { return data_.skillPoints; }
    void AdvanceLevel() override;
    AB::GameProtocol::GameObjectType GetType() const override
    {
        return AB::GameProtocol::GameObjectType::Player;
    }
    uint32_t GetInactiveTime() const
    {
        if (lastPing_ == 0)
            return 0;
        return sa::time::time_elapsed(lastPing_);
    }
    bool IsQueueing() const { return queueing_; }

    void Initialize() override;
    void Logout(bool leavePary);
    void TriggerDialog(uint32_t triggererId, uint32_t dialogIndex);
    void TriggerQuestSelectionDialog(uint32_t triggererId, const ea::set<uint32_t>& quests);
    void TriggerQuestDialog(uint32_t triggererId, uint32_t index);
    void TriggerTradeDialog(uint32_t targetId);
    void ChangeMap(const std::string& mapUuid);
    void ChangeInstance(const std::string& mapUuid, const std::string& instanceUuid);
    void ChangeServerInstance(const std::string& serverUuid, const std::string& mapUuid, const std::string& instanceUuid);
    void UpdateMailBox();
    void NotifyNewMail();
    void SendPlayerInfo(const AB::Entities::Character& ch, uint32_t fields);
    bool AddQuest(uint32_t index);
    bool GetQuestReward(uint32_t index);
    bool SatisfyQuestRequirements(uint32_t index) const;
    void PingPosition(const Math::Vector3& worldPos);

    void WriteToOutput(const Net::NetworkMessage& message);
    bool IsResigned() const { return resigned_; }

    void SetParty(ea::shared_ptr<Party> party);
    ea::shared_ptr<Party> GetParty() const
    {
        return party_;
    }
    FriendList& GetFriendList()
    {
        ASSERT(friendList_);
        return *friendList_;
    }
    const FriendList& GetFriendList() const
    {
        ASSERT(friendList_);
        return *friendList_;
    }

    void Update(uint32_t timeElapsed, Net::NetworkMessage& message) override;

    const std::string& GetPlayerUuid() const override { return data_.uuid; }
    const std::string& GetAccountUuid() const override { return account_.uuid; }

    bool AddToInventory(uint32_t itemId) override;
    bool AddMoney(uint32_t count);
    bool RemoveMoney(uint32_t count);
    void EquipInventoryItem(uint16_t pos);
    /// Leave current party
    void PartyLeave();

    /// This player ignores player
    bool IsIgnored(const Player& player) const;
    bool IsIgnored(const std::string& name) const;
    bool IsFriend(const Player& player) const;
    bool IsOnline() const;

    //{ Client requests.
    // These functions must not have references as arguments, because
    // they are executed by the dispatcher and not directly. All arguments
    // must be passed by value.
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
    void CRQChangeFriendNick(const std::string accountUuid, const std::string newName);
    /// We request info about accountUuid
    void CRQGetPlayerInfoByAccount(const std::string accountUuid, uint32_t fields);
    /// We request inform about player with name
    void CRQGetPlayerInfoByName(const std::string name, uint32_t fields);
    void CRQGetGuildInfo();
    /// Client requested the friend list
    void CRQGetFriendList();
    /// Client requested guild members
    void CRQGetGuildMembers();
    /// Client requests the inventory
    void CRQGetInventory();
    void CRQDropInventoryItem(uint16_t pos, uint32_t count);
    void CRQDestroyInventoryItem(uint16_t pos);
    void CRQSetItemPos(AB::Entities::StoragePlace currentPlace, uint16_t currentPos,
        AB::Entities::StoragePlace newPlace, uint16_t newPos,
        uint32_t count);
    void CRQGetChest();
    void CRQDestroyChestItem(uint16_t pos);
    void CRQDepositMoney(uint32_t amount);
    void CRQWithdrawMoney(uint32_t amount);
    void CRQPartyInvitePlayer(uint32_t playerId);
    void CRQPartyKickPlayer(uint32_t playerId);
    void CRQPartyLeave();
    void CRQPartyAccept(uint32_t inviterId);
    /// We reject the invite of inviterId
    void CRQPartyRejectInvite(uint32_t inviterId);
    void CRQPartyGetMembers(uint32_t partyId);
    void CRQSetOnlineStatus(AB::Entities::OnlineStatus status);
    void CRQDeleteQuest(uint32_t index);
    void CRQHasQuests(uint32_t npcId);
    void CRQSetSecondaryProfession(uint32_t profIndex);
    void CRQSetAttributeValue(uint32_t attribIndex, uint8_t value);
    void CRQEquipSkill(uint32_t skillIndex, uint8_t pos);
    void CRQLoadSkillTemplate(std::string templ);
    void CRQTradeRequest(uint32_t targetId);
    void CRQTradeCancel();
    void CRQTradeOffer(uint32_t money, std::vector<std::pair<uint16_t, uint32_t>> items);
    void CRQTradeAccept();
    void CRQSellItem(uint32_t npcId, uint16_t pos, uint32_t count);
    void CRQBuyItem(uint32_t npcId, uint32_t id, uint32_t count);
    void CRQGetMerchantItems(uint32_t npcId, AB::Entities::ItemType itemType, std::string searchName, uint8_t page);
    void CRQGetItemPrice(std::vector<uint16_t> items);
    void CRQGetCraftsmanItems(uint32_t npcId, AB::Entities::ItemType itemType, std::string searchName, uint8_t page);
    void CRQCraftItem(uint32_t npcId, uint32_t index, uint32_t count, uint32_t attributeIndex);
    void CRQSalvageItem(uint16_t kitPos, uint16_t pos);
    //}

    AB::Entities::Character data_;
    AB::Entities::Account account_;
    Utils::VariantMap deathStats_;
    time_t loginTime_{ 0 };
    time_t logoutTime_{ 0 };
    int64_t lastPing_{ 0 };
    ea::unique_ptr<Components::QuestComp> questComp_;
    ea::unique_ptr<Components::TradeComp> tradeComp_;
    ea::unique_ptr<Components::InteractionComp> interactionComp_;
};

template <>
inline bool Is<Player>(const GameObject& obj)
{
    return obj.GetType() == AB::GameProtocol::GameObjectType::Player;
}

}

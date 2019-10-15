#pragma once

#include "Protocol.h"
#include "PropStream.h"
#include "Structs.h"
#include <AB/ProtocolCodes.h>
#include <AB/Entities/FriendList.h>
#include "Receiver.h"

namespace Client {

class ProtocolGame : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = true };
    enum { ProtocolIdentifier = 0 }; // Not required as we send first
    enum { UseChecksum = true };
private:
    Receiver& receiver_;
    std::string accountUuid_;
    std::string authToken_;
    std::string charUuid_;
    std::string mapUuid_;
    std::string instanceUuid_;
    int64_t pingTick_;
    int64_t updateTick_;
    int64_t clockDiff_;
    int lastPing_;
    bool firstRevc_;
    DH_KEY serverKey_;
    bool loggingOut_;

    void LogMessage(const std::string& message);
    void SendLoginPacket();
protected:
    void OnConnect() override;
    void OnReceive(InputMessage& message) override;
    void OnError(ConnectionError connectionError, const asio::error_code& err) override;

    void ParseMessage(InputMessage& message);
    void ParseKeyExchange(InputMessage& message);
    void ParseServerJoined(InputMessage& message);
    void ParseServerLeft(InputMessage& message);
    void ParseError(InputMessage& message);
    void ParseGameStart(InputMessage& message);
    void ParseEnterWorld(InputMessage& message);
    void ParseChangeInstance(InputMessage& message);
    void ParseMailHeaders(InputMessage& message);
    void ParseMailComplete(InputMessage& message);
    void ParseInventoryContent(InputMessage& message);
    void ParseInventoryItemUpdate(InputMessage& message);
    void ParseInventoryItemDelete(InputMessage& message);
    void ParseChestContent(InputMessage& message);
    void ParseChestItemUpdate(InputMessage& message);
    void ParseChestItemDelete(InputMessage& message);
    void ParsePong(InputMessage& message);
    void ParseGameError(InputMessage& message);
    void ParsePlayerAutoRun(InputMessage& message);
    void ParseUpdate(InputMessage& message);
    void ParseObjectSpawn(bool existing, InputMessage& message);
    void ParseLeaveObject(InputMessage& message);
    void ParseObjectPosUpdate(InputMessage& message);
    void ParseObjectRotUpdate(InputMessage& message);
    void ParseObjectStateChange(InputMessage& message);
    void ParseObjectSpeedChange(InputMessage& message);
    void ParseObjectSelected(InputMessage& message);
    void ParseObjectSkillFailure(InputMessage& message);
    void ParseObjectUseSkill(InputMessage& message);
    void ParseObjectEndUseSkill(InputMessage& message);
    void ParseObjectAttackFailure(InputMessage& message);
    void ParseObjectPingTarget(InputMessage& message);
    void ParseObjectEffectAdded(InputMessage& message);
    void ParseObjectEffectRemoved(InputMessage& message);
    void ParseObjectDamaged(InputMessage& message);
    void ParseObjectHealed(InputMessage& message);
    void ParseObjectProgress(InputMessage& message);
    void ParseObjectDroppedItem(InputMessage& message);
    void ParseObjectSetPosition(InputMessage& message);
    void ParseServerMessage(InputMessage& message);
    void ParseChatMessage(InputMessage& message);
    void ParsePartyPlayerInvited(InputMessage& message);
    void ParsePartyPlayerRemoved(InputMessage& message);
    void ParsePartyPlayerAdded(InputMessage& message);
    void ParsePartyInviteRemoved(InputMessage& message);
    void ParsePartyResigned(InputMessage& message);
    void ParsePartyDefeated(InputMessage& message);
    void ParsePartyInfoMembers(InputMessage& message);
    void ParseResourceChanged(InputMessage& message);
    void ParseDialogTrigger(InputMessage& message);
    void ParseFriendList(InputMessage& message);
    void ParseFriendAdded(InputMessage& message);
    void ParseFriendRemoved(InputMessage& message);
    void ParseGuildInfo(InputMessage& message);
    void ParseGuildMemberList(InputMessage& message);
    void ParsePlayerInfo(InputMessage& message);
public:
    ProtocolGame(Receiver& receiver, Crypto::DHKeys& keys, asio::io_service& ioService);
    ~ProtocolGame() override = default;

    void Login(const std::string& accountUuid, const std::string& authToken,
        const std::string& charUuid, const std::string& mapUuid, const std::string& instanceUuid,
        const std::string& host, uint16_t port);
    void Logout();
    /// Triggers OnPong()
    void Ping();
    void ChangeMap(const std::string& mapUuid);
    void GetMailHeaders();
    void GetInventory();
    void InventoryStoreItem(uint16_t pos);
    void InventoryDestroyItem(uint16_t pos);
    void InventoryDropItem(uint16_t pos);
    void GetChest();
    void ChestDestroyItem(uint16_t pos);
    void GetMail(const std::string& mailUuid);
    void DeleteMail(const std::string& mailUuid);
    void SendMail(const std::string& recipient, const std::string& subject, const std::string& body);
    void GetPlayerInfoByName(const std::string& name, uint32_t fields);
    void GetPlayerInfoByAccount(const std::string& accountUuid, uint32_t fields);

    void Move(uint8_t direction);
    void Turn(uint8_t direction);
    void SetDirection(float rad);
    void ClickObject(uint32_t sourceId, uint32_t targetId);
    void SelectObject(uint32_t sourceId, uint32_t targetId);
    void Command(AB::GameProtocol::CommandTypes type, const std::string& data);
    void GotoPos(const Vec3& pos);
    void Follow(uint32_t targetId, bool ping);
    void UseSkill(uint32_t index, bool ping);
    void Attack(bool ping);
    void Cancel();
    void SetPlayerState(AB::GameProtocol::CreatureState newState);
    void PartyInvitePlayer(uint32_t targetId);
    void PartyKickPlayer(uint32_t targetId);
    void PartyLeave();
    void PartyAcceptInvite(uint32_t inviterId);
    void PartyRejectInvite(uint32_t inviterId);
    void PartyGetMembers(uint32_t partyId);
    void QueueMatch();
    void UnqueueMatch();
    void AddFriend(const std::string& name, AB::Entities::FriendRelation relation);
    void RemoveFriend(const std::string& accountUuid);
    void UpdateFriendList();
    void SetOnlineStatus(RelatedAccount::Status status);

    int64_t GetUpdateTick() const
    {
        return updateTick_;
    }
    int64_t GetClockDiff() const
    {
        return clockDiff_;
    }
};

}

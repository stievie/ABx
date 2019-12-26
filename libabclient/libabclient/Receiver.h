#pragma once

#include <system_error>
#include <stdint.h>
#include <string>
#include "PropStream.h"
#include "Structs.h"
#include <AB/ProtocolCodes.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/Game.h>
#include <AB/Entities/MailList.h>
#include <AB/Entities/Mail.h>
#include <AB/Entities/Service.h>
#include <AB/Entities/Item.h>
#include <AB/Entities/ConcreteItem.h>
#include <AB/Entities/GuildMembers.h>
#include <AB/Entities/Guild.h>
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>
#include "Errors.h"
#include <set>

namespace Client {

struct RelatedAccount
{
    enum Status
    {
        OnlineStatusOffline = 0,
        OnlineStatusAway,
        OnlineStatusDoNotDisturb,
        OnlineStatusOnline,
        OnlineStatusInvisible              // Like offline for other users
    };
    enum Relation
    {
        FriendRelationUnknown = 0,
        FriendRelationFriend = 1,
        FriendRelationIgnore = 2
    };
    enum GuildRole
    {
        GuildRoleUnknown = 0,
        GuildRoleGuest,
        GuildRoleInvited,
        GuildRoleMember,
        GuildRoleOfficer,
        GuildRoleLeader
    };
    uint32_t fields{ 0 };
    std::string accountUuid;
    // Friend nick name
    std::string nickName;
    std::string currentName;
    std::string currentMap;
    Status status;
    // Friend
    Relation relation{ FriendRelationUnknown };
    // Guild member
    std::string guildUuid;
    GuildRole guildRole{ GuildRoleUnknown };
    std::string guildInviteName;
    int64_t invited = 0;
    int64_t joined = 0;
    int64_t expires = 0;
};

inline bool IsOffline(RelatedAccount::Status status)
{
    return (status == RelatedAccount::OnlineStatusOffline) || (status == RelatedAccount::OnlineStatusInvisible);
}

class Receiver
{
public:
    virtual void OnLog(const std::string& message) = 0;
    virtual void OnNetworkError(ConnectionError connectionError, const std::error_code& err) = 0;
    virtual void OnProtocolError(uint8_t err) = 0;
    virtual void OnPong(int lastPing) = 0;

    virtual void OnLoggedIn(const std::string& accountUuid, const std::string& authToken) = 0;
    virtual void OnGetCharlist(const AB::Entities::CharList& chars) = 0;
    virtual void OnGetOutposts(const std::vector<AB::Entities::Game>& games) = 0;
    virtual void OnGetServices(const std::vector<AB::Entities::Service>& services) = 0;
    virtual void OnAccountCreated() = 0;
    virtual void OnPlayerCreated(const std::string& uuid, const std::string& mapUuid) = 0;

    virtual void OnResourceChanged(int64_t updateTick, uint32_t id,
        AB::GameProtocol::ResourceType resType, int16_t value) = 0;
    virtual void OnDialogTrigger(int64_t updateTick, uint32_t dialogId) = 0;
    virtual void OnPlayerInfo(int64_t updateTick, const RelatedAccount& player) = 0;
    virtual void OnFriendList(int64_t updateTick, const std::vector<std::string>&) = 0;
    virtual void OnFriendAdded(int64_t updateTick, const std::string&, RelatedAccount::Relation relation) = 0;
    virtual void OnFriendRemoved(int64_t updateTick, const std::string&, RelatedAccount::Relation relation) = 0;
    virtual void OnGuildMemberList(int64_t updateTick, const std::vector<std::string>&) = 0;
    virtual void OnGuildInfo(int64_t updateTick, const AB::Entities::Guild&) = 0;
    virtual void OnQuestSelectionDialogTrigger(int64_t updateTick, const std::set<uint32_t>&) = 0;
    virtual void OnQuestDialogTrigger(int64_t updateTick, uint32_t) = 0;
    virtual void OnNpcHasQuest(int64_t updateTick, uint32_t npcId, bool hasQuest) = 0;
    virtual void OnQuestDeleted(int64_t updateTick, uint32_t index, bool deleted) = 0;
    virtual void OnQuestRewarded(int64_t updateTick, uint32_t index, bool rewarded) = 0;

    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ServerJoined& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ServerLeft& packet) = 0;

    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ChangeInstance& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::EnterWorld& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::PlayerAutorun& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSpawn& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSpawnExisting& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::MailHeaders& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::MailComplete& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectDespawn& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectPosUpdate& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSpeedChanged& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::InventoryContent& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::InventoryItemUpdate& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::InventoryItemDelete& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ChestContent& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ChestItemUpdate& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ChestItemDelete& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectRotationUpdate& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectTargetSelected& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectStateChanged& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::GameError& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSkillFailure& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectUseSkill& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSkillSuccess& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectAttackFailure& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectPingTarget& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectEffectAdded& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectEffectRemoved& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectDamaged& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectHealed& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectProgress& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectDroppedItem& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSetPosition& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ServerMessage& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::ChatMessage& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::PartyPlayerInvited& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::PartyPlayerRemoved& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::PartyPlayerAdded& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::PartyInviteRemoved& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::PartyResigned& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::PartyDefeated& packet) = 0;
    virtual void OnPacket(int64_t updateTick, const AB::Packets::Server::PartyMembersInfo& packet) = 0;

};

}

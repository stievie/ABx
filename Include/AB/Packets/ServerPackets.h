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

#include "stdint.h"
#include <string>
#include <AB/ProtocolCodes.h>
#include <array>
#include <vector>

namespace AB {
namespace Packets {

namespace Server {
// Packets sent from the server to the client

namespace Internal {
inline constexpr size_t ITEM_UPGRADE_COUNT = 3;
inline constexpr uint8_t ITEM_UPGRADE_1 = 1;
inline constexpr uint8_t ITEM_UPGRADE_2 = 1 << 1;
inline constexpr uint8_t ITEM_UPGRADE_3 = 1 << 2;

struct Item
{
    uint32_t index{ 0 };
    uint16_t type{ 0 };
    uint32_t count{ 0 };
    uint16_t value{ 0 };
    std::string stats;
    uint8_t place{ 0 };
    uint16_t pos{ 0 };
    uint32_t flags{ 0 };
};
struct UpgradeableItem : public Item
{
    uint8_t upgrades{ 0 };
    std::array<Item, ITEM_UPGRADE_COUNT> mods;
};
struct MerchantItem : public Item
{
    uint32_t id{ 0 };
    uint32_t sellPrice{ 0 };
    uint32_t buyPrice{ 0 };
};
}  // namespace Internal

struct KeyExchange
{
    unsigned char key[16];
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        for (size_t i = 0; i < 16; ++i)
        {
            ar.value(key[i]);
        }
    }
};

struct ProtocolError
{
    uint8_t code;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(code);
    }
};

struct PlayerError
{
    uint8_t code;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(code);
    }
};

struct ServerJoined
{
    uint8_t type;
    std::string uuid;
    std::string host;
    uint16_t port;
    std::string location;
    std::string name;
    std::string machine;

    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(type);
        ar.value(uuid);
        ar.value(host);
        ar.value(port);
        ar.value(location);
        ar.value(name);
        ar.value(machine);
    }
};

struct ServerLeft : ServerJoined { };

struct GameStart
{
    int64_t tick;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(tick);
    }
};

struct GamePong
{
    // Difference in ms. Negative = server behind, positive = client behind.
    int32_t clockDiff;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(clockDiff);
    }
};

struct GameUpdate
{
    int64_t tick;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(tick);
    }
};

struct ServerMessage
{
    uint8_t type;
    std::string sender;
    std::string data;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(type);
        ar.value(sender);
        ar.value(data);
    }
};

struct ChatMessage
{
    uint8_t type;
    uint32_t senderId;
    std::string sender;
    std::string data;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(type);
        ar.value(senderId);
        ar.value(sender);
        ar.value(data);
    }
};

struct ChangeInstance
{
    std::string serverUuid;
    std::string mapUuid;
    std::string instanceUuid;
    std::string charUuid;

    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(serverUuid);
        ar.value(mapUuid);
        ar.value(instanceUuid);
        ar.value(charUuid);
    }
};

struct EnterWorld
{
    std::string serverUuid;
    std::string mapUuid;
    std::string instanceUuid;
    uint32_t playerId;
    uint8_t gameType;
    uint8_t partySize;

    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(serverUuid);
        ar.value(mapUuid);
        ar.value(instanceUuid);
        ar.value(playerId);
        ar.value(gameType);
        ar.value(partySize);
    }
};

struct PlayerAutorun
{
    bool autorun;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(autorun);
    }
};

struct MailHeaders
{
    struct Header
    {
        std::string uuid;
        std::string fromName;
        std::string subject;
        int64_t created;
        bool read;
    };
    uint16_t count;
    std::vector<Header> headers;

    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(count);
        headers.resize(count);
        for (uint16_t i = 0; i < count; ++i)
        {
            auto& header = headers[i];
            ar.value(header.uuid);
            ar.value(header.fromName);
            ar.value(header.subject);
            ar.value(header.created);
            ar.value(header.read);
        }
    }
};

struct MailComplete
{
    std::string fromAccountUuid;
    std::string fromName;
    std::string toName;
    std::string subject;
    std::string body;
    int64_t created;
    bool read;

    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(fromAccountUuid);
        ar.value(fromName);
        ar.value(toName);
        ar.value(subject);
        ar.value(body);
        ar.value(created);
        ar.value(read);
    }
};

struct FriendList
{
    uint16_t count;
    std::vector<std::string> friends;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(count);
        friends.resize(count);
        for (uint16_t i = 0; i < count; ++i)
        {
            auto& item = friends[i];
            ar.value(item);
        }
    }
};

struct PlayerInfo
{
    enum Status : uint8_t
    {
        OnlineStatusOffline = 0,
        OnlineStatusAway,
        OnlineStatusDoNotDisturb,
        OnlineStatusOnline,
        OnlineStatusInvisible              // Like offline for other users
    };
    enum Relation : uint8_t
    {
        FriendRelationUnknown = 0,
        FriendRelationFriend = 1,
        FriendRelationIgnore = 2
    };
    enum GuildRole : uint8_t
    {
        GuildRoleUnknown = 0,
        GuildRoleGuest,
        GuildRoleInvited,
        GuildRoleMember,
        GuildRoleOfficer,
        GuildRoleLeader
    };

    uint32_t fields;
    std::string accountUuid;
    // Friend nick name
    std::string nickName;
    std::string currentName;
    std::string currentMap;
    Status status;
    Relation relation { FriendRelationUnknown };
    std::string guildUuid;
    GuildRole guildRole { GuildRoleUnknown };
    std::string guildInviteName;
    int64_t invited { 0 };
    int64_t joined { 0 };
    int64_t expires { 0 };

    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(fields);
        ar.value(accountUuid);
        if (fields & AB::GameProtocol::PlayerInfoFieldName)
            ar.value(nickName);
        if (fields & AB::GameProtocol::PlayerInfoFieldRelation)
            ar.value(relation);
        if (fields & AB::GameProtocol::PlayerInfoFieldOnlineStatus)
            ar.value(status);
        if (fields & AB::GameProtocol::PlayerInfoFieldCurrentName)
            ar.value(currentName);
        if (fields & AB::GameProtocol::PlayerInfoFieldCurrentMap)
            ar.value(currentMap);
        if (fields & AB::GameProtocol::PlayerInfoFieldGuildGuid)
            ar.value(guildUuid);
        if (fields & AB::GameProtocol::PlayerInfoFieldGuildRole)
            ar.value(guildRole);
        if (fields & AB::GameProtocol::PlayerInfoFieldGuildInviteName)
            ar.value(guildInviteName);
        if (fields & AB::GameProtocol::PlayerInfoFieldGuildInvited)
            ar.value(invited);
        if (fields & AB::GameProtocol::PlayerInfoFieldGuildJoined)
            ar.value(joined);
        if (fields & AB::GameProtocol::PlayerInfoFieldGuildExpires)
            ar.value(expires);
    }
};

struct FriendAdded
{
    std::string accountUuid;
    PlayerInfo::Relation relation;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(accountUuid);
        ar.value(relation);
    }
};

struct FriendRemoved
{
    std::string accountUuid;
    PlayerInfo::Relation relation;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(accountUuid);
        ar.value(relation);
    }
};

struct FriendRenamed
{
    std::string accountUuid;
    PlayerInfo::Relation relation;
    std::string newName;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(accountUuid);
        ar.value(relation);
        ar.value(newName);
    }
};

struct GuildInfo
{
    std::string uuid;
    std::string name;
    std::string tag;
    int64_t creation;
    std::string creatorAccountUuid;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(uuid);
        ar.value(name);
        ar.value(tag);
        ar.value(creation);
        ar.value(creatorAccountUuid);
    }
};

struct GuildMemberList
{
    uint16_t count;
    std::vector<std::string> members;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(count);
        members.resize(count);
        for (uint16_t i = 0; i < count; ++i)
        {
            auto& item = members[i];
            ar.value(item);
        }
    }
};

struct ObjectSpawn
{
    uint32_t id;
    uint8_t type;
    uint32_t validFields;
    std::array<float, 3> pos;
    float rot { 0.0f };
    std::array<float, 3> scale;
    bool undestroyable { false };
    bool selectable { false };
    uint8_t state { 0 };
    float speed { 0.0f };
    uint32_t groupId { 0 };
    uint8_t groupPos { 0 };
    uint32_t groupMask{ 0 };
    std::string data;

    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        using namespace AB::GameProtocol;
        ar.value(id);
        ar.value(type);
        ar.value(validFields);
        if (validFields & ObjectSpawnFieldPos)
        {
            ar.value(pos[0]);
            ar.value(pos[1]);
            ar.value(pos[2]);
        }
        if (validFields & ObjectSpawnFieldRot)
            ar.value(rot);
        if (validFields & ObjectSpawnFieldScale)
        {
            ar.value(scale[0]);
            ar.value(scale[1]);
            ar.value(scale[2]);
        }
        if (validFields & ObjectSpawnFieldUndestroyable)
            ar.value(undestroyable);
        if (validFields & ObjectSpawnFieldSelectable)
            ar.value(selectable);
        if (validFields & ObjectSpawnFieldState)
            ar.value(state);
        if (validFields & ObjectSpawnFieldSpeed)
            ar.value(speed);
        if (validFields & ObjectSpawnFieldGroupId)
            ar.value(groupId);
        if (validFields & ObjectSpawnFieldGroupPos)
            ar.value(groupPos);
        if (validFields & ObjectSpawnFieldGroupMask)
            ar.value(groupMask);

        ar.value(data);
    }
};

struct ObjectSpawnExisting : ObjectSpawn { };

struct ObjectDespawn
{
    uint32_t id;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
    }
};

struct ObjectGroupMaskChanged
{
    uint32_t id;
    uint32_t groupMask;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(groupMask);
    }
};

struct ObjectPositionUpdate
{
    uint32_t id;
    std::array<float, 3> pos;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(pos[0]);
        ar.value(pos[1]);
        ar.value(pos[2]);
    }
};

struct ObjectForcePosition
{
    uint32_t id;
    std::array<float, 3> pos;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(pos[0]);
        ar.value(pos[1]);
        ar.value(pos[2]);
    }
};

struct ObjectRotationUpdate
{
    uint32_t id;
    float yRot;
    bool manual;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(yRot);
        ar.value(manual);
    }
};

struct ObjectSpeedChanged
{
    uint32_t id;
    float speed;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(speed);
    }
};

struct ObjectStateChanged
{
    uint32_t id;
    uint8_t state;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(state);
    }
};

struct ObjectTargetSelected
{
    uint32_t id;
    uint32_t targetId;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(targetId);
    }
};

struct InventoryContent
{
    uint32_t maxMoney;
    uint32_t maxItems;
    uint16_t count;
    std::vector<Internal::Item> items;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(maxMoney);
        ar.value(maxItems);
        ar.value(count);
        items.resize(count);
        for (uint16_t i = 0; i < count; ++i)
        {
            auto& item = items[i];
            ar.value(item.type);
            ar.value(item.index);
            ar.value(item.place);
            ar.value(item.pos);
            ar.value(item.count);
            ar.value(item.value);
            ar.value(item.stats);
            ar.value(item.flags);
        }
    }
};

struct ChestContent : public InventoryContent { };

struct MerchantItems
{
    uint8_t page;
    uint8_t pageCount;
    uint16_t typesCount;
    std::vector<uint16_t> types;
    uint16_t count;
    std::vector<Internal::MerchantItem> items;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(page);
        ar.value(pageCount);
        ar.value(typesCount);
        types.resize(typesCount);
        for (uint16_t i = 0; i < typesCount; ++i)
        {
            auto& item = types[i];
            ar.value(item);
        }
        ar.value(count);
        items.resize(count);
        for (uint16_t i = 0; i < count; ++i)
        {
            auto& item = items[i];
            ar.value(item.id);
            ar.value(item.type);
            ar.value(item.index);
            ar.value(item.count);
            ar.value(item.value);
            ar.value(item.stats);
            ar.value(item.flags);
            ar.value(item.sellPrice);
            ar.value(item.buyPrice);
        }
    }
};

struct CraftsmanItems
{
    uint8_t page;
    uint8_t pageCount;
    uint16_t typesCount;
    std::vector<uint16_t> types;
    uint16_t count;
    std::vector<Internal::Item> items;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(page);
        ar.value(pageCount);
        ar.value(typesCount);
        types.resize(typesCount);
        for (uint16_t i = 0; i < typesCount; ++i)
        {
            auto& item = types[i];
            ar.value(item);
        }
        ar.value(count);
        items.resize(count);
        for (uint16_t i = 0; i < count; ++i)
        {
            auto& item = items[i];
            ar.value(item.type);
            ar.value(item.index);
            ar.value(item.count);
            ar.value(item.value);
            ar.value(item.stats);
            ar.value(item.flags);
        }
    }
};

struct ItemPrice
{
    uint8_t count;
    struct Price
    {
        uint16_t pos;
        uint32_t price;
    };
    std::vector<Price> items;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(count);
        items.resize(count);
        for (uint8_t i = 0; i < count; ++i)
        {
            auto& item = items[i];
            ar.value(item.pos);
            ar.value(item.price);
        }
    }
};

struct InventoryItemUpdate
{
    Internal::Item item;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(item.type);
        ar.value(item.index);
        ar.value(item.place);
        ar.value(item.pos);
        ar.value(item.count);
        ar.value(item.value);
        ar.value(item.stats);
        ar.value(item.flags);
    }
};

struct ChestItemUpdate : public InventoryItemUpdate { };

struct InventoryItemDelete
{
    uint16_t pos;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(pos);
    }
};

struct ChestItemDelete : InventoryItemDelete { };

struct QuestSelectionDialogTrigger
{
    // Who triggered the Dialog
    uint32_t triggererId;
    uint8_t count;
    std::vector<uint32_t> quests;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(triggererId);
        ar.value(count);
        quests.resize(count);
        for (uint8_t i = 0; i < count; ++i)
        {
            auto& item = quests[i];
            ar.value(item);
        }
    }
};

struct QuestDialogTrigger
{
    // Who triggered the Dialog
    uint32_t triggererId;
    uint32_t questIndex;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(triggererId);
        ar.value(questIndex);
    }
};

struct NpcHasQuest
{
    uint32_t npcId;
    bool hasQuest;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(npcId);
        ar.value(hasQuest);
    }
};

struct QuestDeleted
{
    uint32_t questIndex;
    bool deleted;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(questIndex);
        ar.value(deleted);
    }
};

struct QuestRewarded
{
    uint32_t questIndex;
    bool rewarded;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(questIndex);
        ar.value(rewarded);
    }
};

struct DialogTrigger
{
    // Who triggered the Dialog
    uint32_t triggererId;
    uint32_t dialogId;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(triggererId);
        ar.value(dialogId);
    }
};

struct TradeDialogTrigger
{
    uint32_t sourceId;
    uint32_t targetId;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(sourceId);
        ar.value(targetId);
    }
};

struct TradeCancel
{
    template<typename _Ar>
    void Serialize(_Ar&)
    { }
};

struct TradeOffer
{
    uint32_t money{ 0 };
    uint8_t itemCount{ 0 };
    std::vector<Internal::UpgradeableItem> items;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(money);
        ar.value(itemCount);
        items.resize(itemCount);
        for (uint8_t i = 0; i < itemCount; ++i)
        {
            auto& item = items[i];
            ar.value(item.index);
            ar.value(item.type);
            ar.value(item.count);
            ar.value(item.value);
            ar.value(item.stats);
            ar.value(item.flags);
            ar.value(item.upgrades);
            for (size_t mi = 0; mi < Internal::ITEM_UPGRADE_COUNT; ++mi)
            {
                if ((item.upgrades & mi) == mi)
                {
                    auto& mod = item.mods[mi];
                    ar.value(mod.index);
                    ar.value(mod.type);
                    ar.value(mod.count);
                    ar.value(mod.value);
                    ar.value(mod.stats);
                    ar.value(mod.flags);
                }
            }
        }
    }
};

struct TradeAccepted
{
    template<typename _Ar>
    void Serialize(_Ar&)
    { }
};

struct ObjectSkillFailure
{
    uint32_t id;
    int8_t skillIndex;
    uint8_t errorCode;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(skillIndex);
        ar.value(errorCode);
    }
};

struct ObjectUseSkill
{
    uint32_t id;
    int8_t skillIndex;
    uint16_t energy;
    uint16_t adrenaline;
    uint16_t activation;
    uint16_t overcast;
    uint16_t hp;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(skillIndex);
        ar.value(energy);
        ar.value(adrenaline);
        ar.value(activation);
        ar.value(overcast);
        ar.value(hp);
    }
};

struct ObjectSkillSuccess
{
    uint32_t id;
    int8_t skillIndex;
    uint32_t recharge;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(skillIndex);
        ar.value(recharge);
    }
};

struct ObjectAttackFailure
{
    uint32_t id;
    uint8_t errorCode;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(errorCode);
    }
};

struct ObjectSetAttackSpeed
{
    uint32_t id;
    // float factor * 100
    uint8_t factor;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(factor);
    }
};

struct ObjectPingTarget
{
    uint32_t id;
    uint32_t targetId;
    uint8_t pingType;
    int8_t skillIndex;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(targetId);
        ar.value(pingType);
        ar.value(skillIndex);
    }
};

struct ObjectEffectAdded
{
    uint32_t id;
    uint32_t effectIndex;
    uint32_t ticks;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(effectIndex);
        ar.value(ticks);
    }
};

struct ObjectEffectRemoved
{
    uint32_t id;
    uint32_t effectIndex;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(effectIndex);
    }
};

struct ObjectDamaged
{
    uint32_t id;
    uint32_t sourceId;
    uint16_t index;
    uint8_t damageType;
    uint16_t damageValue;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(sourceId);
        ar.value(index);
        ar.value(damageType);
        ar.value(damageValue);
    }
};

struct ObjectHealed
{
    uint32_t id;
    uint32_t sourceId;
    uint16_t index;
    int16_t healValue;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(sourceId);
        ar.value(index);
        ar.value(healValue);
    }
};

struct ObjectProgress
{
    uint32_t id;
    uint8_t type;
    int16_t value;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(type);
        ar.value(value);
    }
};

struct ObjectDroppedItem
{
    uint32_t id;
    uint32_t targetId;
    uint32_t itemId;
    uint32_t itemIndex;
    uint32_t count;
    uint16_t value;
    std::string stats;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(targetId);
        ar.value(itemId);
        ar.value(itemIndex);
        ar.value(count);
        ar.value(value);
        ar.value(stats);
    }
};

struct DropTargetChanged
{
    uint32_t id;
    uint32_t newTargetId;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(newTargetId);
    }
};

struct ObjectResourceChanged
{
    uint32_t id;
    uint8_t type;
    int16_t value;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(type);
        ar.value(value);
    }
};

struct PartyPlayerInvited
{
    uint32_t inviterId;
    uint32_t inviteeId;
    uint32_t partyId;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(inviterId);
        ar.value(inviteeId);
        ar.value(partyId);
    }
};

struct PartyPlayerRemoved
{
    uint32_t leaderId;
    uint32_t targetId;
    uint32_t partyId;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(leaderId);
        ar.value(targetId);
        ar.value(partyId);
    }
};

struct PartyInviteRemoved : PartyPlayerRemoved { };

struct PartyPlayerAdded
{
    uint32_t acceptorId;
    uint32_t leaderId;
    uint32_t partyId;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(acceptorId);
        ar.value(leaderId);
        ar.value(partyId);
    }
};

struct PartyResigned
{
    uint32_t partyId;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(partyId);
    }
};

struct PartyDefeated
{
    uint32_t partyId;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(partyId);
    }
};

struct PartyMembersInfo
{
    uint32_t partyId;
    uint8_t count;
    std::vector<uint32_t> members;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(partyId);
        ar.value(count);
        members.resize(count);
        for (uint8_t i = 0; i < count; ++i)
        {
            auto& member = members[i];
            ar.value(member);
        }
    }
};

struct SetObjectAttributeValue
{
    uint32_t objectId;
    uint32_t attribIndex;
    int8_t value;
    uint8_t remaining;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(objectId);
        ar.value(attribIndex);
        ar.value(value);
        ar.value(remaining);
    }
};

struct ObjectSetSkill
{
    uint32_t objectId;
    uint32_t skillIndex;
    uint8_t pos;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(objectId);
        ar.value(skillIndex);
        ar.value(pos);
    }
};

struct SkillTemplateLoaded
{
    uint32_t objectId;
    std::string templ;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(objectId);
        ar.value(templ);
    }
};

struct ObjectSecProfessionChanged
{
    uint32_t objectId;
    uint32_t profIndex;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(objectId);
        ar.value(profIndex);
    }
};

}

}
}

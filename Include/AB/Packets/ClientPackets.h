#pragma once

#include "stdint.h"
#include <string>
#include <array>
#include <abcrypto.hpp>

namespace AB {
namespace Packets {
namespace Client {

struct GameLogin
{
    uint16_t clientOs;
    uint16_t protocolVersion;
    uint8_t key[DH_KEY_LENGTH];
    std::string accountUuid;
    std::string authToken;
    std::string charUuid;
    std::string mapUuid;
    std::string instanceUuid;

    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(clientOs);
        ar.value(protocolVersion);
        for (unsigned i = 0; i < DH_KEY_LENGTH; ++i)
            ar.value(key[i]);
        ar.value(accountUuid);
        ar.value(authToken);
        ar.value(charUuid);
        ar.value(mapUuid);
        ar.value(instanceUuid);
    }
};

struct Logout
{
    template<typename _Ar>
    void Serialize(_Ar&)
    { }
};

struct Ping
{
    int64_t tick;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(tick);
    }
};

struct ChangeMap
{
    std::string mapUuid;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(mapUuid);
    }
};

struct GetMailHeaders
{
    template<typename _Ar>
    void Serialize(_Ar&)
    { }
};

struct GetInventory
{
    template<typename _Ar>
    void Serialize(_Ar&)
    { }
};

struct InventoryStoreItem
{
    uint16_t pos;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(pos);
    }
};

struct InventoryDestroyItem
{
    uint16_t pos;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(pos);
    }
};

struct InventoryDropItem
{
    uint16_t pos;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(pos);
    }
};

struct GetChest
{
    template<typename _Ar>
    void Serialize(_Ar&)
    { }
};

struct ChestDestroyItem
{
    uint16_t pos;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(pos);
    }
};

struct GetMail
{
    std::string mailUuid;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(mailUuid);
    }
};

struct DeleteMail
{
    std::string mailUuid;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(mailUuid);
    }
};

struct SendMail
{
    std::string recipient;
    std::string subject;
    std::string body;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(recipient);
        ar.value(subject);
        ar.value(body);
    }
};

struct GetPlayerInfoByName
{
    std::string name;
    uint32_t fields;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(name);
        ar.value(fields);
    }
};

struct GetPlayerInfoByAccount
{
    std::string accountUuid;
    uint32_t fields;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(accountUuid);
        ar.value(fields);
    }
};

struct Move
{
    uint8_t direction;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(direction);
    }
};

struct Turn
{
    uint8_t direction;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(direction);
    }
};

struct SetDirection
{
    float rad;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(rad);
    }
};

struct ClickObject
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

struct SelectObject
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

struct Command
{
    uint8_t type;
    std::string data;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(type);
        ar.value(data);
    }
};

struct GotoPos
{
    std::array<float, 3> pos;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(pos[0]);
        ar.value(pos[1]);
        ar.value(pos[2]);
    }
};

struct Follow
{
    uint32_t targetId;
    bool ping;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(targetId);
        ar.value(ping);
    }
};

struct UseSkill
{
    uint8_t index;
    bool ping;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(index);
        ar.value(ping);
    }
};

struct Attack
{
    bool ping;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(ping);
    }
};

struct Cancel
{
    template<typename _Ar>
    void Serialize(_Ar&)
    { }
};

struct SetPlayerState
{
    uint8_t newState;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(newState);
    }
};

struct PartyInvitePlayer
{
    uint32_t targetId;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(targetId);
    }
};

struct PartyKickPlayer
{
    uint32_t targetId;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(targetId);
    }
};

struct PartyLeave
{
    template<typename _Ar>
    void Serialize(_Ar&)
    { }
};

struct PartyAcceptInvite
{
    uint32_t inviterId;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(inviterId);
    }
};

struct PartyRejectInvite
{
    uint32_t inviterId;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(inviterId);
    }
};

struct PartyGetMembers
{
    uint32_t partyId;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(partyId);
    }
};

struct QueueMatch
{
    template<typename _Ar>
    void Serialize(_Ar&)
    { }
};

struct UnqueueMatch
{
    template<typename _Ar>
    void Serialize(_Ar&)
    { }
};

struct AddFriend
{
    std::string name;
    uint8_t relation;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(name);
        ar.value(relation);
    }
};

struct RemoveFriend
{
    std::string accountUuid;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(accountUuid);
    }
};

struct RenameFriend
{
    std::string accountUuid;
    std::string newName;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(accountUuid);
        ar.value(newName);
    }
};

struct UpdateFriendList
{
    template<typename _Ar>
    void Serialize(_Ar&)
    { }
};

struct SetOnlineStatus
{
    uint8_t newStatus;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(newStatus);
    }
};

struct SetSecondaryProfession
{
    uint32_t profIndex;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(profIndex);
    }
};

struct SetAttributeValue
{
    uint32_t attribIndex;
    uint8_t value;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(attribIndex);
        ar.value(value);
    }
};

struct EquipSkill
{
    uint32_t skillIndex;
    uint8_t pos;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(skillIndex);
        ar.value(pos);
    }
};

struct GuildInfo
{
    template<typename _Ar>
    void Serialize(_Ar&)
    { }
};

struct GuildMembers
{
    template<typename _Ar>
    void Serialize(_Ar&)
    { }
};

}
}
}

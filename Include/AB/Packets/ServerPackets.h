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

struct ProtocolError
{
    uint8_t code;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(code);
    }
};

struct GameError
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

struct Pong
{
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

struct ObjectPosUpdate
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

struct ObjectSetPosition : ObjectPosUpdate {};

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
    struct Item
    {
        uint16_t type;
        uint32_t index;
        uint8_t place;
        uint16_t pos;
        uint32_t count;
        uint16_t value;
    };

    uint16_t count;
    std::vector<Item> items;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
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
        }
    }
};

struct ChestContent : InventoryContent { };

struct InventoryItemUpdate
{
    uint16_t type;
    uint32_t index;
    uint8_t place;
    uint16_t pos;
    uint32_t count;
    uint16_t value;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(type);
        ar.value(index);
        ar.value(place);
        ar.value(pos);
        ar.value(count);
        ar.value(value);
    }
};

struct ChestItemUpdate : InventoryItemUpdate { };

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

struct QuestRewarded
{
    uint32_t index;
    bool rewarded;

    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(index);
        ar.value(rewarded);
    }
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
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(targetId);
        ar.value(itemId);
        ar.value(itemIndex);
        ar.value(count);
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

}

}
}

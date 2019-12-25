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

/// Dummy packet for unknown messages
struct UnknownPacket
{
    template<typename _Ar>
    void Serialize(_Ar& ar)
    { }
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

}

}
}

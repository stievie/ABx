#pragma once

#include <system_error>
#include <stdint.h>
#include <string>
#include <AB/AccountData.h>
#include <AB/CharacterData.h>
#include <AB/GameData.h>
#include "PropStream.h"
#include "Structs.h"
#include <AB/ProtocolCodes.h>

namespace Client {

class Receiver
{
public:
    virtual void OnGetCharlist(const AB::Data::CharacterList& chars) = 0;
    virtual void OnGetGamelist(const AB::Data::GameList& games) = 0;
    virtual void OnEnterWorld(const std::string& mapName, uint32_t playerId) = 0;
    virtual void OnNetworkError(const std::error_code& err) = 0;
    virtual void OnProtocolError(uint8_t err) = 0;
    virtual void OnSpawnObject(uint32_t id, const Vec3& pos, const Vec3& scale, float rot,
        PropReadStream& data, bool existing) = 0;
    virtual void OnDespawnObject(uint32_t id) = 0;
    virtual void OnObjectPos(uint32_t id, const Vec3& pos) = 0;
    virtual void OnObjectRot(uint32_t id, float rot, bool manual) = 0;
    virtual void OnObjectStateChange(uint32_t id, AB::GameProtocol::CreatureState state) = 0;
    virtual void OnAccountCreated() = 0;
    virtual void OnPlayerCreated(const std::string& name, const std::string& map) = 0;
    virtual void OnObjectSelected(uint32_t sourceId, uint32_t targetId) = 0;
    virtual void OnServerMessage(AB::GameProtocol::ServerMessageType type,
        const std::string& senderName, const std::string& message) = 0;
    virtual void OnChatMessage(AB::GameProtocol::ChatMessageChannel channel,
        uint32_t senderId, const std::string& senderName, const std::string& message) = 0;
};

}

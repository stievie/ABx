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

namespace Client {

struct ObjectSpawn
{
    Vec3 pos;
    Vec3 scale;
    float rot;
    bool undestroyable;
    AB::GameProtocol::CreatureState state;
    float speed;
    uint32_t groupId;
};

class Receiver
{
public:
    virtual void OnNetworkError(const std::error_code& err) = 0;
    virtual void OnProtocolError(uint8_t err) = 0;
    virtual void OnPong(int lastPing) = 0;
    virtual void OnServerJoined(const AB::Entities::Service& service) = 0;
    virtual void OnServerLeft(const AB::Entities::Service& service) = 0;

    virtual void OnLoggedIn(const std::string& accountUuid) = 0;
    virtual void OnGetCharlist(const AB::Entities::CharList& chars) = 0;
    virtual void OnGetOutposts(const std::vector<AB::Entities::Game>& games) = 0;
    virtual void OnGetServices(const std::vector<AB::Entities::Service>& services) = 0;
    virtual void OnAccountCreated() = 0;
    virtual void OnPlayerCreated(const std::string& uuid, const std::string& mapUuid) = 0;

    virtual void OnGetMailHeaders(int64_t updateTick, const std::vector<AB::Entities::MailHeader>& headers) = 0;
    virtual void OnGetMail(int64_t updateTick, const AB::Entities::Mail& mail) = 0;
    virtual void OnChangeInstance(int64_t updateTick, const std::string& serverId,
        const std::string& mapUuid, const std::string& instanceUuid, const std::string& charUuid) = 0;
    virtual void OnEnterWorld(int64_t updateTick, const std::string& serverId,
        const std::string& mapUuid, const std::string& instanceUuid, uint32_t playerId) = 0;
    virtual void OnSpawnObject(int64_t updateTick, uint32_t id, const ObjectSpawn& objectSpawn,
        PropReadStream& data, bool existing) = 0;
    virtual void OnDespawnObject(int64_t updateTick, uint32_t id) = 0;
    virtual void OnObjectPos(int64_t updateTick, uint32_t id, const Vec3& pos) = 0;
    virtual void OnObjectRot(int64_t updateTick, uint32_t id, float rot, bool manual) = 0;
    virtual void OnObjectStateChange(int64_t updateTick, uint32_t id, AB::GameProtocol::CreatureState state) = 0;
    virtual void OnObjectSpeedChange(int64_t updateTick, uint32_t id, float speedFactor) = 0;
    virtual void OnObjectSelected(int64_t updateTick, uint32_t sourceId, uint32_t targetId) = 0;
    virtual void OnObjectSkillFailure(int64_t updateTick, uint32_t id, int skillIndex, AB::GameProtocol::SkillError error) = 0;
    virtual void OnObjectUseSkill(int64_t updateTick, uint32_t id, int skillIndex, uint16_t energy, uint16_t adrenaline,
        uint16_t activation, uint16_t overcast) = 0;
    virtual void OnObjectEffectAdded(int64_t updateTick, uint32_t id, uint32_t effectIndex, uint32_t ticks) = 0;
    virtual void OnObjectEffectRemoved(int64_t updateTick, uint32_t id, uint32_t effectIndex) = 0;
    virtual void OnObjectEndUseSkill(int64_t updateTick, uint32_t id, int skillIndex, uint16_t recharge) = 0;
    virtual void OnResourceChanged(int64_t updateTick, uint32_t id,
        AB::GameProtocol::ResourceType resType, int16_t value) = 0;
    virtual void OnServerMessage(int64_t updateTick, AB::GameProtocol::ServerMessageType type,
        const std::string& senderName, const std::string& message) = 0;
    virtual void OnChatMessage(int64_t updateTick, AB::GameProtocol::ChatMessageChannel channel,
        uint32_t senderId, const std::string& senderName, const std::string& message) = 0;
    /// The player was invited into our party
    virtual void OnPartyInvited(int64_t updateTick, uint32_t sourceId, uint32_t targetId) = 0;
    /// Player was removed from our party
    virtual void OnPartyRemoved(int64_t updateTick, uint32_t sourceId, uint32_t targetId) = 0;
    /// Player was added to our party, i.e. the player accepted the invite
    virtual void OnPartyAdded(int64_t updateTick, uint32_t sourceId, uint32_t targetId) = 0;
    /// The invite to our party was removed
    virtual void OnPartyInviteRemoved(int64_t updateTick, uint32_t sourceId, uint32_t targetId) = 0;
};

}

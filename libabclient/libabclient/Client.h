#pragma once

#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>
#include "Receiver.h"
#include "PropStream.h"
#include "Structs.h"
#include <AB/ProtocolCodes.h>
#include "Structs.h"
#include <memory>
#include <AB/DHKeys.hpp>

namespace Client {

class ProtocolLogin;
class ProtocolGame;
class HttpsClient;

class Client : public Receiver
{
public:
    enum class ClientState
    {
        Disconnected,
        CreateAccount,
        CreateChar,
        SelectChar,
        World
    };
private:
    std::shared_ptr<ProtocolLogin> protoLogin_;
    std::shared_ptr<ProtocolGame> protoGame_;
    std::string accountName_;
    std::string mapUuid_;
    std::vector<int> pings_;
    int lastRun_;
    int lastPing_;
    bool gotPong_;
    // Our (client) key pair
    Crypto::DHKeys dhKeys_;
    std::shared_ptr<ProtocolLogin> GetProtoLogin();
public:
    Client();
    ~Client();
    void Poll();
    void Run();
    /// Login to login server
    void Login(const std::string& name, const std::string& pass);
    void Logout();
    void GetOutposts();
    void GetServers();
    void CreateAccount(const std::string& name, const std::string& pass,
        const std::string& email, const std::string& accKey);
    void CreatePlayer(const std::string& charName, const std::string& profUuid,
        uint32_t modelIndex,
        AB::Entities::CharacterSex sex, bool isPvp);

    /// Connect to game server -> authenticate -> enter game
    void EnterWorld(const std::string& charUuid, const std::string& mapUuid,
        const std::string& host = "", uint16_t port = 0, const std::string& instanceId = "");
    void Update(int timeElapsed);

    bool HttpRequest(const std::string& path, std::ostream& out);
    bool HttpDownload(const std::string& path, const std::string& outFile);

    uint32_t GetIp() const;

    // Receiver
    void OnNetworkError(const std::error_code& err) override;
    void OnProtocolError(uint8_t err) override;
    void OnPong(int lastPing) override;
    void OnServerJoined(const AB::Entities::Service& service) override;
    void OnServerLeft(const AB::Entities::Service& service) override;

    void OnLoggedIn(const std::string& accountUuid) override;
    void OnGetCharlist(const AB::Entities::CharList& chars) override;
    void OnGetOutposts(const std::vector<AB::Entities::Game>& games) override;
    void OnGetServices(const std::vector<AB::Entities::Service>& services) override;
    void OnAccountCreated() override;
    void OnPlayerCreated(const std::string& uuid, const std::string& mapUuid) override;

    void OnGetMailHeaders(int64_t updateTick, const std::vector<AB::Entities::MailHeader>& headers) override;
    void OnGetMail(int64_t updateTick, const AB::Entities::Mail& mail) override;
    void OnEnterWorld(int64_t updateTick, const std::string& serverId,
        const std::string& mapUuid, const std::string& instanceUuid, uint32_t playerId,
        AB::Entities::GameType type, uint8_t partySize) override;
    void OnChangeInstance(int64_t updateTick, const std::string& serverId,
        const std::string& mapUuid, const std::string& instanceUuid, const std::string& charUuid)override;
    void OnSpawnObject(int64_t updateTick, uint32_t id, const ObjectSpawn& objectSpawn,
        PropReadStream& data, bool existing) override;
    void OnDespawnObject(int64_t updateTick, uint32_t id) override;
    void OnObjectPos(int64_t updateTick, uint32_t id, const Vec3& pos) override;
    void OnObjectRot(int64_t updateTick, uint32_t id, float rot, bool manual) override;
    void OnObjectStateChange(int64_t updateTick, uint32_t id, AB::GameProtocol::CreatureState state) override;
    void OnObjectSpeedChange(int64_t updateTick, uint32_t id, float speedFactor) override;
    void OnObjectSelected(int64_t updateTick, uint32_t sourceId, uint32_t targetId) override;
    void OnObjectSkillFailure(int64_t updateTick, uint32_t id, int skillIndex, AB::GameProtocol::SkillError error) override;
    void OnObjectUseSkill(int64_t updateTick, uint32_t id, int skillIndex, uint16_t energy, uint16_t adrenaline,
        uint16_t activation, uint16_t overcast, uint16_t hp) override;
    void OnObjectEndUseSkill(int64_t updateTick, uint32_t id, int skillIndex, uint16_t recharge) override;
    void OnObjectEffectAdded(int64_t updateTick, uint32_t id, uint32_t effectIndex, uint32_t ticks) override;
    void OnObjectEffectRemoved(int64_t updateTick, uint32_t id, uint32_t effectIndex) override;
    void OnObjectDamaged(int64_t updateTick, uint32_t id, uint8_t damageType, int16_t value, uint32_t skillIndex) override;
    void OnResourceChanged(int64_t updateTick, uint32_t id,
        AB::GameProtocol::ResourceType resType, int16_t value) override;
    void OnServerMessage(int64_t updateTick, AB::GameProtocol::ServerMessageType type,
        const std::string& senderName, const std::string& message) override;
    void OnChatMessage(int64_t updateTick, AB::GameProtocol::ChatMessageChannel channel,
        uint32_t senderId, const std::string& senderName, const std::string& message) override;
    /// The player was invited into our party
    void OnPartyInvited(int64_t updateTick, uint32_t sourceId, uint32_t targetId, uint32_t partyId) override;
    /// Player was removed from our party
    void OnPartyRemoved(int64_t updateTick, uint32_t sourceId, uint32_t targetId, uint32_t partyId) override;
    /// Player was added to our party, i.e. the player accepted the invite
    void OnPartyAdded(int64_t updateTick, uint32_t acceptorId, uint32_t leaderId, uint32_t partyId) override;
    /// The invite to our party was removed
    void OnPartyInviteRemoved(int64_t updateTick, uint32_t sourceId, uint32_t targetId, uint32_t partyId) override;
    void OnPartyInfoMembers(uint32_t partyId, const std::vector<uint32_t>& members) override;

    std::string accountUuid_;
    std::string password_;
    std::string loginHost_;
    uint16_t loginPort_;
    std::string fileHost_;
    uint16_t filePort_;
    std::string gameHost_;
    uint16_t gamePort_;
    ClientState state_;
    Receiver* receiver_;
    HttpsClient* httpClient_;
    const std::string& GetMapUuid() const
    {
        return mapUuid_;
    }
    int GetAvgPing() const
    {
        if (pings_.empty())
            return 0;

        float pings = 0.0f;
        for (int p : pings_)
            pings += static_cast<float>(p);
        return static_cast<int>(pings / pings_.size());
    }
    int GetLastPing() const
    {
        if (!pings_.empty())
            return pings_.back();
        return 0;
    }
    int64_t GetClockDiff() const;

    /// Causes the server to change the map for the whole party
    void ChangeMap(const std::string& mapUuid);
    void GetMailHeaders();
    void GetMail(const std::string& mailUuid);
    void DeleteMail(const std::string& mailUuid);
    void SendMail(const std::string& recipient, const std::string& subject, const std::string& body);
    void Move(uint8_t direction);
    void Turn(uint8_t direction);
    void SetDirection(float rad);
    void ClickObject(uint32_t sourceId, uint32_t targetId);
    void SelectObject(uint32_t sourceId, uint32_t targetId);
    void FollowObject(uint32_t targetId);
    void Command(AB::GameProtocol::CommandTypes type, const std::string& data);
    void GotoPos(const Vec3& pos);
    void PartyInvitePlayer(uint32_t targetId);
    /// Kick player from party or remove invitation
    void PartyKickPlayer(uint32_t targetId);
    void PartyAcceptInvite(uint32_t inviterId);
    void PartyRejectInvite(uint32_t inviterId);
    void PartyGetMembers(uint32_t partyId);
    void PartyLeave();
    void UseSkill(uint32_t index);
    void Cancel();
    void SetPlayerState(AB::GameProtocol::CreatureState newState);
};

}

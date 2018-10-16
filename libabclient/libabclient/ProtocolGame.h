#pragma once

#include "Protocol.h"
#include "PropStream.h"
#include "Structs.h"
#include <AB/ProtocolCodes.h>
#include "Receiver.h"

namespace Client {

class ProtocolGame : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = true };
    enum { ProtocolIdentifier = 0 }; // Not required as we send first
    enum { UseChecksum = true };
    typedef std::function<void(const std::string& mapName, uint32_t)> EnterWorldCallback;
    typedef std::function<void(int)> PingCallback;
    typedef std::function<void(uint32_t id, const Vec3& pos, const Vec3& scale, float rot,
        PropReadStream& data, bool existing)> SpawnCallback;
    typedef std::function<void(uint32_t id)> DespawnCallback;
    typedef std::function<void(uint32_t id, const Vec3& pos)> ObjectPosCallback;
private:
    std::string accountUuid_;
    std::string accountPass_;
    std::string charUuid_;
    std::string mapUuid_;
    std::string instanceUuid_;
    int64_t pingTick_;
    int64_t updateTick_;
    int64_t clockDiff_;
    int lastPing_;
    bool firstRevc_;

    void SendLoginPacket();
protected:
    void OnConnect() override;
    void OnReceive(const std::shared_ptr<InputMessage>& message) override;
    void OnError(const asio::error_code& err) override;

    void ParseMessage(const std::shared_ptr<InputMessage>& message);
    void ParseServerJoined(const std::shared_ptr<InputMessage>& message);
    void ParseServerLeft(const std::shared_ptr<InputMessage>& message);
    void ParseError(const std::shared_ptr<InputMessage>& message);
    void ParseEnterWorld(const std::shared_ptr<InputMessage>& message);
    void ParseChangeInstance(const std::shared_ptr<InputMessage>& message);
    void ParseMailHeaders(const std::shared_ptr<InputMessage>& message);
    void ParseMailComplete(const std::shared_ptr<InputMessage>& message);
    void ParsePong(const std::shared_ptr<InputMessage>& message);
    void ParseUpdate(const std::shared_ptr<InputMessage>& message);
    void ParseSpawnObject(bool existing, const std::shared_ptr<InputMessage>& message);
    void ParseLeaveObject(const std::shared_ptr<InputMessage>& message);
    void ParseObjectPosUpdate(const std::shared_ptr<InputMessage>& message);
    void ParseObjectRotUpdate(const std::shared_ptr<InputMessage>& message);
    void ParseObjectStateChange(const std::shared_ptr<InputMessage>& message);
    void ParseObjectSpeedChange(const std::shared_ptr<InputMessage>& message);
    void ParseObjectSelected(const std::shared_ptr<InputMessage>& message);
    void ParseServerMessage(const std::shared_ptr<InputMessage>& message);
    void ParseChatMessage(const std::shared_ptr<InputMessage>& message);
    void ParsePartyPlayerInvited(const std::shared_ptr<InputMessage>& message);
    void ParsePartyPlayerRemoved(const std::shared_ptr<InputMessage>& message);
    void ParsePartyPlayerAdded(const std::shared_ptr<InputMessage>& message);
    void ParsePartyInviteRemoved(const std::shared_ptr<InputMessage>& message);
public:
    ProtocolGame();
    ~ProtocolGame() = default;

    Receiver* receiver_;

    void Login(const std::string& accountUuid, const std::string& accountPass,
        const std::string& charUuid, const std::string& mapUuid, const std::string& instanceUuid,
        const std::string& host, uint16_t port);
    void Logout();
    /// Triggers OnPong()
    void Ping();
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
    void Command(AB::GameProtocol::CommandTypes type, const std::string& data);
    void GotoPos(const Vec3& pos);
    void Follow(uint32_t targetId);
    void UseSkill(uint32_t index);
    void Cancel();
    void SetPlayerState(AB::GameProtocol::CreatureState newState);
    void PartyInvitePlayer(uint32_t targetId);
    void PartyKickPlayer(uint32_t targetId);
    void PartyLeave();
    void PartyAccept(uint32_t inviterId);

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

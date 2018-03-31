#pragma once

#include <AB/AccountData.h>
#include "Receiver.h"
#include "PropStream.h"
#include "Structs.h"
#include <AB/ProtocolCodes.h>
#include "Structs.h"

namespace Client {

class ProtocolLogin;
class ProtocolGame;

class Client : public Receiver
{
public:
    enum ClientState
    {
        StateDisconnected,
        StateCreateAccount,
        StateCreateChar,
        StateSelectChar,
        StateWorld
    };
private:
    std::shared_ptr<ProtocolLogin> protoLogin_;
    std::shared_ptr<ProtocolGame> protoGame_;
    std::string accountName_;
    std::string password_;
    std::string mapName_;
    std::vector<int> pings_;
    int lastRun_;
    int lastPing_;
    bool gotPong_;
    void OnPong(int ping);
    std::shared_ptr<ProtocolLogin> GetProtoLogin();
public:
    Client();
    ~Client();
    /// Login to login server
    void Login(const std::string& name, const std::string& pass);
    void Logout();
    void GetGameList();
    void CreateAccount(const std::string& name, const std::string& pass,
        const std::string& email, const std::string& accKey);
    void CreatePlayer(const std::string& account, const std::string& password,
        const std::string& charName, const std::string& prof, PlayerSex sex, bool isPvp);

    /// Connect to game server -> authenticate -> enter game
    void EnterWorld(const std::string& charName, const std::string& map);
    void Update(int timeElapsed);

    uint32_t GetIp() const;

    // Receiver
    void OnNetworkError(const std::error_code& err) override;
    void OnProtocolError(uint8_t err) override;

    void OnGetCharlist(const AB::Data::CharacterList& chars) override;
    void OnGetGamelist(const AB::Data::GameList& games) override;
    void OnAccountCreated() override;
    void OnPlayerCreated(const std::string& name, const std::string& map) override;

    void OnEnterWorld(int64_t updateTick, const std::string& mapName, uint32_t playerId) override;
    void OnSpawnObject(int64_t updateTick, uint32_t id, const Vec3& pos, const Vec3& scale, float rot,
        PropReadStream& data, bool existing) override;
    void OnDespawnObject(int64_t updateTick, uint32_t id) override;
    void OnObjectPos(int64_t updateTick, uint32_t id, const Vec3& pos) override;
    void OnObjectRot(int64_t updateTick, uint32_t id, float rot, bool manual) override;
    void OnObjectStateChange(int64_t updateTick, uint32_t id, AB::GameProtocol::CreatureState state) override;
    void OnObjectSelected(int64_t updateTick, uint32_t sourceId, uint32_t targetId) override;
    void OnServerMessage(int64_t updateTick, AB::GameProtocol::ServerMessageType type,
        const std::string& senderName, const std::string& message) override;
    void OnChatMessage(int64_t updateTick, AB::GameProtocol::ChatMessageChannel channel,
        uint32_t senderId, const std::string& senderName, const std::string& message) override;

    std::string loginHost_;
    uint16_t loginPort_;
    std::string gameHost_;
    uint16_t gamePort_;
    ClientState state_;
    Receiver* receiver_;
    const std::string& GetMapName() const
    {
        return mapName_;
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

    void Move(uint8_t direction);
    void Turn(uint8_t direction);
    void SetDirection(float rad);
    void SelectObject(uint32_t sourceId, uint32_t targetId);
    void FollowObject(uint32_t targetId);
    void Command(AB::GameProtocol::CommandTypes type, const std::string& data);
    void GotoPos(const Vec3& pos);
};

}

#pragma once

#include "Account.h"
#include "Receiver.h"
#include "DHKeys.h"
#include "PropStream.h"
#include "Structs.h"
#include <AB/ProtocolCodes.h>

namespace Client {

class ProtocolLogin;
class ProtocolGame;

class Client : public Receiver
{
public:
    enum ClientState
    {
        StateDisconnected,
        StateCreateChar,
        StateSelecChar,
        StateWorld
    };
private:
    std::shared_ptr<ProtocolLogin> protoLogin_;
    std::shared_ptr<ProtocolGame> protoGame_;
    std::string accountName_;
    std::string password_;
    std::string mapName_;
    std::vector<int> pings_;
    DH_KEY serverPublicKey_;
    DH_KEY sharedKey_;
    void OnPong(int ping);
public:
    Client();
    ~Client();
    /// Login to login server
    void Login(const std::string& name, const std::string& pass);
    void Logout();
    /// Connect to game server -> authenticate -> enter game
    void EnterWorld(const std::string& charName, const std::string& map);
    void Update(int timeElapsed);

    // Receiver
    void OnGetCharlist(const CharList& chars) override;
    void OnEnterWorld(const std::string& mapName, uint32_t playerId) override;
    void OnNetworkError(const std::error_code& err) override;
    void OnProtocolError(uint8_t err) override;
    void OnSpawnObject(uint32_t id, const Vec3& pos, const Vec3& scale, float rot,
        PropReadStream& data, bool existing) override;
    void OnDespawnObject(uint32_t id) override;
    void OnObjectPos(uint32_t id, const Vec3& pos) override;
    void OnObjectRot(uint32_t id, float rot) override;

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

    void Move(uint8_t direction);
    void Turn(uint8_t direction);
};

}

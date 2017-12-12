#pragma once

#include "Protocol.h"
#include <string>
#include <stdint.h>
#include "PropStream.h"
#include "Structs.h"

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
    typedef std::function<void(uint32_t id)> DespawnCallback;;
private:
    std::string accountName_;
    std::string accountPass_;
    std::string charName_;
    std::string map_;
    int64_t pingTick_;
    int lastPing_;
    bool firstRevc_;
    EnterWorldCallback enterWorldCallback_;
    PingCallback pingCallback_;
    SpawnCallback spawnCallback_;
    DespawnCallback despawnCallback_;

    void SendLoginPacket();
protected:
    void OnConnect() override;
    void OnReceive(const std::shared_ptr<InputMessage>& message) override;
    void OnError(const asio::error_code& err) override;

    void ParseMessage(const std::shared_ptr<InputMessage>& message);
    void ParseError(const std::shared_ptr<InputMessage>& message);
    void ParseEnterWorld(const std::shared_ptr<InputMessage>& message);
    void ParsePong(const std::shared_ptr<InputMessage>& message);
    void ParseUpdate(const std::shared_ptr<InputMessage>& message);
    void ParseSpawnObject(bool existing, const std::shared_ptr<InputMessage>& message);
    void ParseLeaveObject(const std::shared_ptr<InputMessage>& message);
public:
    ProtocolGame();
    ~ProtocolGame();

    void Login(const std::string& accountName, const std::string& accountPass,
        const std::string& charName, const std::string& map, const std::string& host, uint16_t port,
        const EnterWorldCallback& callback);
    void Logout();
    void Ping(const PingCallback& callback);
    void SetSpawnCallback(const SpawnCallback& callback) { spawnCallback_ = callback; }
    void SetDespawnCallback(const DespawnCallback& callback) { despawnCallback_ = callback; }
};

}

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
    std::string accountName_;
    std::string accountPass_;
    std::string charName_;
    std::string map_;
    int64_t pingTick_;
    int lastPing_;
    bool firstRevc_;
    PingCallback pingCallback_;

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
    void ParseObjectPosUpdate(const std::shared_ptr<InputMessage>& message);
    void ParseObjectRotUpdate(const std::shared_ptr<InputMessage>& message);
    void ParseObjectStateChange(const std::shared_ptr<InputMessage>& message);
public:
    ProtocolGame();
    ~ProtocolGame();

    Receiver* receiver_;

    void Login(const std::string& accountName, const std::string& accountPass,
        const std::string& charName, const std::string& map, const std::string& host, uint16_t port);
    void Logout();
    void Ping(const PingCallback& callback);
    void Move(uint8_t direction);
    void Turn(uint8_t direction);
    void SetDirection(float rad);

};

}

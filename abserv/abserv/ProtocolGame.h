#pragma once

#include <memory>
#include "Protocol.h"
#include "Connection.h"

namespace Game {
class Player;
}

namespace Net {

class ProtocolGame final : public Protocol
{
public:
    // static protocol information
    enum { ServerSendsFirst = true };
    enum { ProtocolIdentifier = 0 }; // Not required as we send first
    enum { UseChecksum = true };
    static const char* ProtocolName() { return "Game Protocol"; };
    friend class Game::Player;
private:
    std::shared_ptr<Game::Player> player_;
public:
    explicit ProtocolGame(std::shared_ptr<Connection> connection) :
        Protocol(connection)
    {}

    void Login(const std::string& name, uint32_t accountId);
    void Logout();
private:
    std::shared_ptr<ProtocolGame> GetThis()
    {
        return std::static_pointer_cast<ProtocolGame>(shared_from_this());
    }
    void ParsePacket(NetworkMessage& message) final;
    void OnRecvFirstMessage(NetworkMessage& msg) final;
    void OnConnect() final;

    void DisconnectClient(const std::string& message);
    void Connect(uint32_t playerId);

    bool acceptPackets_ = false;
};

}

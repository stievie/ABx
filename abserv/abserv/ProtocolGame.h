#pragma once

#include <memory>
#include "Protocol.h"
#include "Connection.h"
#include "Dispatcher.h"

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
    uint32_t challengeTimestamp_ = 0;
    uint8_t challengeRandom_ = 0;
public:
    explicit ProtocolGame(std::shared_ptr<Connection> connection) :
        Protocol(connection)
    {
        checksumEnabled_ = ProtocolGame::UseChecksum;
    }

    void Login(const std::string& name, uint32_t accountId, const std::string& map);
    void Logout();
    void EnterGame(const std::string& mapName);
    void WriteToOutput(const NetworkMessage& message);
private:
    // Helpers so we don't need to bind every time
    template <typename Callable, typename... Args>
    void AddGameTask(Callable function, Args&&... args)
    {
        Asynch::Dispatcher::Instance.Add(
            Asynch::CreateTask(std::bind(function, player_->GetGame(), std::forward<Args>(args)...))
        );
    }

    template <typename Callable, typename... Args>
    void AddGameTaskTimed(uint32_t delay, Callable function, Args&&... args)
    {
        Asynch::Dispatcher::Instance.Add(
            Asynch::CreateTask(delay, std::bind(function, player_->GetGame(), std::forward<Args>(args)...))
        );
    }

    std::shared_ptr<ProtocolGame> GetThis()
    {
        return std::static_pointer_cast<ProtocolGame>(shared_from_this());
    }
    void ParsePacket(NetworkMessage& message) final;
    void OnRecvFirstMessage(NetworkMessage& msg) final;
    void OnConnect() final;

    void DisconnectClient(uint8_t error);
    void Connect(uint32_t playerId);

    bool acceptPackets_ = false;
};

}

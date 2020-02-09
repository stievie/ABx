#pragma once

#include "IpcServer.h"
#include <memory>
#include <sa/Noncopyable.h>
#include <vector>

namespace Game {
class Game;
}

namespace AI {

class DebugServer
{
    NON_COPYABLE(DebugServer)
private:
    std::unique_ptr<IPC::Server> server_;
    bool active_{ false };
    std::vector<std::weak_ptr<Game::Game>> games_;
    void BroadcastGame(const Game::Game& game);
    void BroadcastGameAdded(const Game::Game& game);
    void BroadcastGameRemoved(const Game::Game& game);
public:
    explicit DebugServer(asio::io_service& ioService, uint32_t ip, uint16_t port);
    DebugServer() = default;
    void AddGame(std::shared_ptr<Game::Game> game);
    void RemoveGame(std::shared_ptr<Game::Game> game);
    void Update();
};

}

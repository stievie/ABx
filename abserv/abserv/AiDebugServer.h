/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "IpcServer.h"
#include <memory>
#include <sa/Noncopyable.h>
#include <vector>
#include <AB/IPC/AI/ClientMessages.h>
#include <map>
#include <set>

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
    std::map<uint32_t, uint32_t> selectedGames_;
    void BroadcastGame(const Game::Game& game);
    void BroadcastGameAdded(const Game::Game& game);
    void BroadcastGameRemoved(uint32_t id);
    void HandleGetGames(IPC::ServerConnection& client, const GetGames&);
    void HandleSelectGame(IPC::ServerConnection& client, const SelectGame&);
    std::set<uint32_t> GetSubscribedClients(uint32_t gameId);
public:
    explicit DebugServer(asio::io_service& ioService, uint32_t ip, uint16_t port);
    DebugServer() = default;
    void AddGame(std::shared_ptr<Game::Game> game);
    void RemoveGame(uint32_t id);
    void Update();
    bool IsActive() const { return active_; }
};

}

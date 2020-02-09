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

#include "stdafx.h"
#include "AiDebugServer.h"
#include "Game.h"
#include <AB/IPC/AI/ServerMessages.h>
#include "Npc.h"
#include <functional>

namespace AI {

DebugServer::DebugServer(asio::io_service& ioService, uint32_t ip, uint16_t port) :
    server_(std::make_unique<IPC::Server>(ioService, asio::ip::tcp::endpoint(asio::ip::address(asio::ip::address_v4(ip)), port))),
    active_{ true }
{
    server_->handlers_.Add<GetGames>(std::bind(&DebugServer::HandleGetGames, this, std::placeholders::_1));
}

void DebugServer::HandleGetGames(const GetGames&)
{

}


void DebugServer::AddGame(std::shared_ptr<Game::Game> game)
{
    if (!active_)
        return;

    const auto it = std::find_if(games_.begin(), games_.end(), [&](const std::weak_ptr<Game::Game>& current)
    {
        if (auto c = current.lock())
            return c->id_ == game->id_;
        return false;
    });
    if (it != games_.end())
        return;
    const Game::Game& g = *game;
    games_.push_back(game);
    BroadcastGameAdded(g);
}

void DebugServer::RemoveGame(std::shared_ptr<Game::Game> game)
{
    if (!active_)
        return;
    auto it = std::find_if(games_.begin(), games_.end(), [&](const std::weak_ptr<Game::Game>& current)
    {
        if (auto c = current.lock())
            return c->id_ == game->id_;
        return false;
    });
    if (it != games_.end())
    {
        auto sg = (*it).lock();
        games_.erase(it);
        if (sg)
            BroadcastGameRemoved(*sg);
    }
}

void DebugServer::Update()
{
    if (!active_)
        return;
    if (games_.size() == 0)
        return;

    for (auto weakGame : games_)
    {
        if (auto sGame = weakGame.lock())
            BroadcastGame(*sGame);
    }
}

void DebugServer::BroadcastGame(const Game::Game& game)
{
    game.VisitObjects<Game::Npc>([this, &game](const Game::Npc& current)
    {
        NpcUpdate msg;
        msg.id = current.id_;
        msg.gameId = game.id_;
        msg.name = current.GetName();
        server_->Send(msg);
        return Iteration::Continue;
    });
}

void DebugServer::BroadcastGameAdded(const Game::Game& game)
{
    GameAdd msg;
    msg.id = game.id_;
    msg.name = game.GetName();
    server_->Send(msg);
}

void DebugServer::BroadcastGameRemoved(const Game::Game& game)
{
    GameRemove msg;
    msg.id = game.id_;
    server_->Send(msg);
}

}

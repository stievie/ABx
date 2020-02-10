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
#include "Player.h"
#include <functional>
#include "ServerConnection.h"

namespace AI {

DebugServer::DebugServer(asio::io_service& ioService, uint32_t ip, uint16_t port) :
    server_(std::make_unique<IPC::Server>(ioService, asio::ip::tcp::endpoint(asio::ip::address(asio::ip::address_v4(ip)), port))),
    active_{ true }
{
    server_->handlers_.Add<GetGames>(std::bind(&DebugServer::HandleGetGames, this, std::placeholders::_1, std::placeholders::_2));
    server_->handlers_.Add<SelectGame>(std::bind(&DebugServer::HandleSelectGame, this, std::placeholders::_1, std::placeholders::_2));
    server_->onClientDisconnect = [this](IPC::ServerConnection& client)
    {
        selectedGames_.erase(client.GetId());
    };
}

void DebugServer::HandleGetGames(IPC::ServerConnection& client, const GetGames&)
{
    for (auto weakGame : games_)
    {
        if (auto sGame = weakGame.lock())
        {
            GameAdd msg;
            msg.id = sGame->id_;
            msg.name = sGame->GetName();
            server_->SendTo(client, msg);
        }
    }
}

void DebugServer::HandleSelectGame(IPC::ServerConnection& client, const SelectGame& msg)
{
    auto it = std::find_if(games_.begin(), games_.end(), [&](const std::weak_ptr<Game::Game>& current)
    {
        if (auto c = current.lock())
            return c->id_ == msg.gameId;
        return false;
    });
    if (it != games_.end())
        return;
    auto game = (*it).lock();
    if (!game)
        return;

    selectedGames_.emplace(client.GetId(), msg.gameId);
    game->VisitObjects<Game::Actor>([this, &game](const Game::Actor& current)
    {
        ObjectUpdate msg;
        msg.id = current.id_;
        msg.gameId = game->id_;
        if (Game::Is<Game::Npc>(current))
            msg.objectType = ObjectUpdate::ObjectType::Npc;
        else if (Game::Is<Game::Player>(current))
            msg.objectType = ObjectUpdate::ObjectType::Player;
        msg.name = current.GetName();
        server_->Send(msg);
        return Iteration::Continue;
    });
}

std::set<uint32_t> DebugServer::GetSubscribedClients(uint32_t gameId)
{
    std::set<uint32_t> result;
    for (const auto& i : selectedGames_)
    {
        if (i.second == gameId)
            result.emplace(i.first);
    }
    return result;
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
    auto it = std::find_if(games_.begin(), games_.end(), [id = game->id_](const std::weak_ptr<Game::Game>& current) -> bool
    {
        if (auto c = current.lock())
            return c->id_ == id;
        return false;
    });
    if (it != games_.end())
    {
        games_.erase(it);
        BroadcastGameRemoved(*game);
    }

    // Unsubscribe all from this game
    auto i = selectedGames_.begin();
    while ((i = std::find_if(i, selectedGames_.end(), [id = game->id_](const auto& current) -> bool
    {
        return current.second == id;
    })) != selectedGames_.end())
        selectedGames_.erase(i++);
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
    auto clients = GetSubscribedClients(game.id_);
    if (clients.size() == 0)
        return;

    game.VisitObjects<Game::Actor>([this, &clients](const Game::Actor& current)
    {
        ObjectUpdate msg;
        msg.id = current.id_;
        for (const auto& i : clients)
            server_->SendTo(i, msg);
        return Iteration::Continue;
    });
}

void DebugServer::BroadcastGameAdded(const Game::Game& game)
{
    GameAdd msg;
    msg.id = game.id_;
    msg.name = game.GetName();
    msg.mapUuid = game.data_.uuid;
    msg.instanceUuid = game.instanceData_.uuid;
    server_->Send(msg);
}

void DebugServer::BroadcastGameRemoved(const Game::Game& game)
{
    GameRemove msg;
    msg.id = game.id_;
    server_->Send(msg);
}

}

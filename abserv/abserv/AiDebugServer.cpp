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
#include "Npc.h"
#include "Player.h"
#include "ServerConnection.h"
#include <functional>

namespace AI {

DebugServer::DebugServer(asio::io_service& ioService, uint32_t ip, uint16_t port) :
    server_(std::make_unique<IPC::Server>(ioService, asio::ip::tcp::endpoint(asio::ip::address(asio::ip::address_v4(ip)), port))),
    active_{ true }
{
    server_->handlers_.Add<GetGames>(std::bind(&DebugServer::HandleGetGames, this, std::placeholders::_1, std::placeholders::_2));
    server_->handlers_.Add<SelectGame>(std::bind(&DebugServer::HandleSelectGame, this, std::placeholders::_1, std::placeholders::_2));
    server_->onClientDisconnect = [this](IPC::ServerConnection& client)
    {
        std::lock_guard<std::mutex> loock(lock_);
        selectedGames_.erase(client.GetId());
    };
}

void DebugServer::HandleGetGames(IPC::ServerConnection& client, const GetGames&)
{
    for (auto weakGame : games_)
    {
        if (auto sGame = weakGame.lock())
        {
            GameAdd msg {
                sGame->id_,
                sGame->GetName(),
                sGame->data_.uuid,
                sGame->instanceData_.uuid
            };
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
    if (it == games_.end())
        return;
    auto game = (*it).lock();
    if (!game)
        return;

    std::lock_guard<std::mutex> loock(lock_);
    selectedGames_.emplace(client.GetId(), msg.gameId);
    GameSelected gmsg{ game->id_ };
    server_->SendTo(client, gmsg);
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

void DebugServer::RemoveGame(uint32_t id)
{
    if (!active_)
        return;

    auto it = std::find_if(games_.begin(), games_.end(), [&id](const std::weak_ptr<Game::Game>& current) -> bool
    {
        if (auto c = current.lock())
            return c->id_ == id;
        return false;
    });
    if (it != games_.end())
    {
        games_.erase(it);
        BroadcastGameRemoved(id);
    }

    // Unsubscribe all from this game
    auto i = selectedGames_.begin();
    while ((i = std::find_if(i, selectedGames_.end(), [&id](const auto& current) -> bool
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
    {
//        LOG_DEBUG << "No clients subscribed to " << game.id_ << std::endl;
        return;
    }

    // First tell the client how many objects we have
    GameUpdate msg;
    msg.id = game.id_;
    msg.count = 0;
    game.VisitObjects<Game::Actor>([this, &msg](const Game::Actor& current)
    {
        if (!Game::Is<Game::Npc>(current) && !Game::Is<Game::Player>(current))
            return Iteration::Continue;

        ++msg.count;
        msg.objects.push_back(current.id_);
        return Iteration::Continue;
    });
    for (const auto& i : clients)
        server_->SendTo(i, msg);

    // Then send the details
    game.VisitObjects<Game::Actor>([this, &clients](const Game::Actor& current)
    {
        GameObject msg;
        if (Game::Is<Game::Npc>(current))
            msg.objectType = GameObject::ObjectType::Npc;
        else if (Game::Is<Game::Player>(current))
            msg.objectType = GameObject::ObjectType::Player;
        else
            return Iteration::Continue;

        msg.id = current.id_;
        msg.gameId = current.GetGame()->id_;
        msg.name = current.GetName();
        msg.position = current.GetPosition();
        for (const auto& i : clients)
            server_->SendTo(i, msg);
        return Iteration::Continue;
    });
}

void DebugServer::BroadcastGameAdded(const Game::Game& game)
{
    GameAdd msg {
        game.id_,
        game.GetName(),
        game.data_.uuid,
        game.instanceData_.uuid
    };
    server_->Send(msg);
}

void DebugServer::BroadcastGameRemoved(uint32_t id)
{
    GameRemove msg { id };
    server_->Send(msg);
}

}

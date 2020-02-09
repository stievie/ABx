#include "stdafx.h"
#include "AiDebugServer.h"
#include "Game.h"
#include <AB/IPC/AI/ServerMessages.h>
#include "Npc.h"

namespace AI {

DebugServer::DebugServer(asio::io_service& ioService, uint32_t ip, uint16_t port) :
    server_(std::make_unique<IPC::Server>(ioService, asio::ip::tcp::endpoint(asio::ip::address(asio::ip::address_v4(ip)), port))),
    active_{ true }
{ }

void DebugServer::AddGame(std::shared_ptr<Game::Game> game)
{
    if (!active_)
        return;

    const auto it = std::find_if(games_.begin(), games_.end(), [&](const std::weak_ptr<Game::Game>& current) {
        if (auto c = current.lock())
            return c->id_ == game->id_;
        return false;
    });
    if (it != games_.end())
        return;
    BroadcastGameAdded(*game);
    games_.push_back(game);
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
        if (auto sg = (*it).lock())
            BroadcastGameRemoved(*sg);
        games_.erase(it);
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
    game.VisitObjects<Game::Npc>([this](const Game::Npc& current)
    {
        NpcUpdate msg;
        msg.id = current.id_;
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

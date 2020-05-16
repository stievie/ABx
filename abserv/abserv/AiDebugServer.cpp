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
#include "AiAgent.h"
#include "Game.h"
#include "Npc.h"
#include "Player.h"
#include <abai/BevaviorCache.h>
#include <abai/Root.h>
#include <abai/Condition.h>
#include <abipc/ServerConnection.h>
#include <abscommon/Logger.h>
#include <abscommon/StringUtils.h>
#include <abshared/Mechanic.h>
#include <functional>

namespace AI
{

DebugServer::DebugServer(asio::io_service& ioService, uint32_t ip, uint16_t port) :
    server_(ea::make_unique<IPC::Server>(ioService, asio::ip::tcp::endpoint(asio::ip::address(asio::ip::address_v4(ip)), port))),
    active_{ true }
{
    server_->handlers_.Add<GetTrees>(std::bind(&DebugServer::HandleGetTrees, this, std::placeholders::_1, std::placeholders::_2));
    server_->handlers_.Add<GetGames>(std::bind(&DebugServer::HandleGetGames, this, std::placeholders::_1, std::placeholders::_2));
    server_->handlers_.Add<SelectGame>(std::bind(&DebugServer::HandleSelectGame, this, std::placeholders::_1, std::placeholders::_2));
    server_->onClientConnect = [](IPC::ServerConnection& client)
    {
        LOG_INFO << "Debug server: Client " << client.GetId() << " connected from " << Utils::ConvertIPToString(client.GetIP()) << ":" << client.GetPort() << std::endl;
    };
    server_->onClientDisconnect = [this](IPC::ServerConnection& client)
    {
        std::scoped_lock lock(lock_);
        LOG_INFO << "Debug server: Client " << client.GetId() << " disconnected" << std::endl;
        selectedGames_.erase(client.GetId());
    };
}

void DebugServer::HandleGetGames(IPC::ServerConnection& client, const GetGames&)
{
    for (const auto& weakGame : games_)
    {
        if (auto sGame = weakGame.lock())
        {
            GameAdd msg{
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
    {
        std::scoped_lock lock(lock_);
        selectedGames_.erase(client.GetId());
    }
    auto it = ea::find_if(games_.begin(), games_.end(), [&](const ea::weak_ptr<Game::Game>& current)
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

    std::scoped_lock lock(lock_);
    selectedGames_.emplace(client.GetId(), msg.gameId);
    GameSelected gmsg{ game->id_ };
    server_->SendTo(client, gmsg);
}

void DebugServer::HandleGetTrees(IPC::ServerConnection& client, const GetTrees&)
{
    auto* cache = GetSubsystem<AI::BevaviorCache>();
    cache->VisitBehaviors([&](const std::string& name, const Root& root)
    {
        BehaviorTree tree;
        tree.id = root.GetId();
        tree.name = name;

        ForEachChildNode(root, [&](const Node& parent, const Node& child)
        {
            BehaviorTree::Node nd;
            nd.parentId = parent.GetId();
            nd.id = child.GetId();
            nd.name = child.GetClassName();
            if (auto* cond = child.GetCondition())
                nd.condition = cond->GetFriendlyName();
            tree.nodes.push_back(std::move(nd));
            return Iteration::Continue;
        });
        tree.nodeCount = tree.nodes.size();
        server_->SendTo(client, tree);
        return Iteration::Continue;
    });
}

ea::set<uint32_t> DebugServer::GetSubscribedClients(uint32_t gameId)
{
    ea::set<uint32_t> result;
    for (const auto& i : selectedGames_)
    {
        if (i.second == gameId)
            result.emplace(i.first);
    }
    return result;
}

void DebugServer::AddGame(ea::shared_ptr<Game::Game> game)
{
    if (!active_)
        return;

    const auto it = ea::find_if(games_.begin(), games_.end(), [&](const ea::weak_ptr<Game::Game>& current)
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

    auto it = ea::find_if(games_.begin(), games_.end(), [&id](const ea::weak_ptr<Game::Game>& current) -> bool
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
    while ((i = ea::find_if(i, selectedGames_.end(), [&id](const auto& current) -> bool
    {
        return current.second == id;
    })) != selectedGames_.end())
        selectedGames_.erase(i++);
}

void DebugServer::Update()
{
    if (!active_)
        return;

    if (games_.size() == 0 || selectedGames_.size() == 0)
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

    // First tell the client how many objects we have
    GameUpdate msg;
    msg.id = game.id_;
    msg.tick = game.GetUpdateTick();
    msg.count = 0;
    game.VisitObjects<Game::Actor>([&msg](const Game::Actor& current)
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
        msg.objectState = static_cast<uint8_t>(current.stateComp_.GetState());
        msg.name = current.GetName();
        msg.classLevel = current.GetClassLevel();
        msg.position = current.GetPosition();
        msg.health = current.resourceComp_->GetHealth();
        msg.maxHealth = current.resourceComp_->GetMaxHealth();
        msg.energy = current.resourceComp_->GetEnergy();
        msg.maxEnergy = current.resourceComp_->GetMaxEnergy();
        msg.morale = current.resourceComp_->GetMorale();
        if (Game::Is<Game::Npc>(current) && Game::To<Game::Npc>(current).aiComp_)
        {
            const AiAgent& agent = Game::To<Game::Npc>(current).aiComp_->GetAgent();
            msg.currentNodeStatus = static_cast<int>(agent.GetCurrentStatus());
            if (auto b = agent.GetBehavior())
            {
                // The ID of the root node
                msg.behaviorId = b->GetId();
            }

            if (agent.selectedSkill_ > -1 && agent.selectedSkill_ < Game::PLAYER_MAX_SKILLS)
            {
                msg.selectedSkillIndex = agent.selectedSkill_;
                auto skill = current.skills_->GetSkill(agent.selectedSkill_);
                if (skill)
                    msg.selectedSkillName = skill->GetName();
                else
                    msg.selectedSkillName = "None";
            }
            else
            {
                msg.selectedSkillIndex = -1;
                msg.selectedSkillName = "None";
            }
            if (auto ca = agent.context_.currentAction_.lock())
            {
                msg.currActionId = ca->GetId();
                msg.currAction = ca->GetClassName();
            }
            else
                msg.currAction = "None";
            msg.selectedAgentsCount = agent.filteredAgents_.size();
            msg.selectedAgents = agent.filteredAgents_;
            agent.context_.VisitTypes<AI::node_status_type>([&](const AI::Id& id, const AI::Node::Status& value)
            {
                auto st = std::make_pair(static_cast<uint32_t>(id), static_cast<int>(value));
                msg.nodeStatus.push_back(std::move(st));
                return Iteration::Continue;
            });
            msg.nodeStatusCount = msg.nodeStatus.size();
        }
        else
            msg.currAction = "No AI";

        for (const auto& i : clients)
            server_->SendTo(i, msg);
        return Iteration::Continue;
    });
}

void DebugServer::BroadcastGameAdded(const Game::Game& game)
{
    GameAdd msg{
        game.id_,
        game.GetName(),
        game.data_.uuid,
        game.instanceData_.uuid
    };
    server_->Send(msg);
}

void DebugServer::BroadcastGameRemoved(uint32_t id)
{
    GameRemove msg{ id };
    server_->Send(msg);
}

}

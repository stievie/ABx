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
#include "GameManager.h"
#include "Player.h"
#include "Npc.h"
#include "IOGame.h"
#include "Group.h"
#include "AiDebugServer.h"

namespace Game {

void GameManager::Start()
{
    // Main Thread
    std::scoped_lock lock(lock_);
    state_ = State::ManagerStateRunning;
}

void GameManager::Stop()
{
    // Main Thread
    if (state_ == State::ManagerStateRunning)
    {
        {
            std::scoped_lock lock(lock_);
            state_ = State::ManagerStateTerminated;
        }
        for (const auto& g : games_)
        {
            g.second->SetState(Game::ExecutionState::Terminated);
        }
    }
}

std::shared_ptr<Game> GameManager::CreateGame(const std::string& mapUuid)
{
    assert(state_ == State::ManagerStateRunning);
#ifdef DEBUG_GAME
    LOG_DEBUG << "Creating game " << mapUuid << std::endl;
#endif
    std::shared_ptr<Game> game = std::make_shared<Game>();
    {
        std::scoped_lock lock(lock_);
        game->id_ = GetNewGameId();
        games_[game->id_] = game;
        maps_[mapUuid].push_back(game.get());
    }
    game->instanceData_.number = static_cast<uint16_t>(maps_[mapUuid].size());
    game->Load(mapUuid);
    {
        std::scoped_lock lock(lock_);
        GetSubsystem<AI::DebugServer>()->AddGame(game);
    }
    return game;
}

void GameManager::DeleteGameTask(uint32_t gameId)
{
#ifdef DEBUG_GAME
    LOG_DEBUG << "Deleting game " << gameId << std::endl;
#endif
    // Dispatcher Thread
    auto it = games_.find(gameId);
    if (it != games_.end())
    {
        // games_.size() may be called from another thread so lock it
        std::scoped_lock lock(lock_);
        GetSubsystem<AI::DebugServer>()->RemoveGame(gameId);
        maps_.erase((*it).second->data_.uuid);
        games_.erase(it);
    }
}

std::shared_ptr<Game> GameManager::GetOrCreateInstance(const std::string& mapUuid, const std::string& instanceUuid)
{
    auto result = GetInstance(instanceUuid);
    if (result)
        return result;
    result = CreateGame(mapUuid);
    if (result)
    {
        result->instanceData_.uuid = instanceUuid;
    }
    return result;
}

bool GameManager::InstanceExists(const std::string& uuid)
{
    auto it = ea::find_if(games_.begin(), games_.end(), [&](auto const& current)
    {
        return Utils::Uuid::IsEqual(current.second->instanceData_.uuid, uuid);
    });
    return it != games_.end();
}

std::shared_ptr<Game> GameManager::GetInstance(const std::string& instanceUuid)
{
    auto it = ea::find_if(games_.begin(), games_.end(), [&](auto const& current)
    {
        return Utils::Uuid::IsEqual(current.second->instanceData_.uuid, instanceUuid);
    });
    if (it != games_.end())
        return (*it).second;

    return std::shared_ptr<Game>();
}

std::shared_ptr<Game> GameManager::GetGame(const std::string& mapUuid, bool canCreate /* = false */)
{
    AB::Entities::GameType gType = GetGameType(mapUuid);
    if (gType >= AB::Entities::GameTypePvPCombat && canCreate)
        // These games are exclusive
        return CreateGame(mapUuid);

    const auto it = maps_.find(mapUuid);
    if (it == maps_.end())
    {
        // No instance of this map exists
        if (!canCreate)
            return std::shared_ptr<Game>();
        return CreateGame(mapUuid);
    }
    // There are already some games with this map
    for (const auto& g : it->second)
    {
        if (g->GetPlayerCount() < GAME_MAX_PLAYER &&
            (g->GetState() == Game::ExecutionState::Running || g->GetState() == Game::ExecutionState::Startup))
        {
            const auto git = games_.find(g->id_);
            return (*git).second;
        }
    }
    if (canCreate)
        return CreateGame(mapUuid);
    LOG_WARNING << "No game with UUID " << mapUuid << std::endl;
    return std::shared_ptr<Game>();
}

std::shared_ptr<Game> GameManager::Get(uint32_t gameId)
{
    auto it = games_.find(gameId);
    if (it != games_.end())
    {
        return (*it).second;
    }
    return std::shared_ptr<Game>();
}

bool GameManager::AddPlayer(const std::string& mapUuid, std::shared_ptr<Player> player)
{
    std::shared_ptr<Game> game = GetGame(mapUuid, true);
    if (!game)
    {
        LOG_ERROR << "Unable to get game, Map UUID: " << mapUuid << std::endl;
        return false;
    }

    // No need to wait until assets loaded
    game->PlayerJoin(player->id_);
    return true;
}

void GameManager::CleanGames()
{
    if (games_.size() == 0)
    {
        // If no games we can reset IDs
        gameIds_.Reset();
        GameObject::objectIds_.Reset();
        Group::groupIds_.Reset();
        return;
    }

    for (const auto& g : games_)
    {
        if (g.second->IsInactive())
            g.second->SetState(Game::ExecutionState::Terminated);
    }
}

AB::Entities::GameType GameManager::GetGameType(const std::string& mapUuid)
{
    return IO::IOGame::GetGameType(mapUuid);
}

}

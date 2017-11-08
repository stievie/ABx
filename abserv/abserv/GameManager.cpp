#include "stdafx.h"
#include "GameManager.h"
#include "Utils.h"
#include <algorithm>
#include "Scheduler.h"
#include "Dispatcher.h"

#include "DebugNew.h"

namespace Game {

GameManager GameManager::Instance;

void GameManager::Start(Net::ServiceManager* serviceManager)
{
    // Main Thread
    serviceManager_ = serviceManager;
    std::lock_guard<std::recursive_mutex> lockClass(lock_);
    state_ = State::Running;
}

void GameManager::Stop()
{
    // Main Thread
    if (state_ == State::Running)
    {
        {
            std::lock_guard<std::recursive_mutex> lockClass(lock_);
            state_ = State::Terminated;
        }
        for (const auto& g : games_)
        {
            g.second->SetState(Game::GameStateTerminated);
        }
    }
}

std::shared_ptr<Game> GameManager::CreateGame(const std::string& mapName)
{
    assert(state_ == State::Running);

    std::shared_ptr<Game> game = std::make_shared<Game>();
    {
        std::lock_guard<std::recursive_mutex> lockClass(lock_);
        game->id_ = GetNewGameId();
        games_[game->id_] = game;
        maps_[mapName].push_back(game.get());
    }
    game->SetState(Game::GameStateStartup);
    Asynch::Dispatcher::Instance.Add(
        Asynch::CreateTask(std::bind(&Game::Load, game, mapName))
    );
    return game;
}

void GameManager::DeleteGameTask(uint32_t gameId)
{
    // Dispatcher Thread
    auto it = games_.find(gameId);
    if (it != games_.end())
        games_.erase(it);
}

std::shared_ptr<Game> GameManager::GetGame(const std::string& mapName, bool canCreate /* = false */)
{
    const auto it = maps_.find(mapName);
    if (it == maps_.end())
    {
        if (!canCreate)
            return std::shared_ptr<Game>();
        return CreateGame(mapName);
    }
    for (const auto& g : it->second)
    {
        if (g->GetPlayerCount() < GAME_MAX_PLAYER || !canCreate)
        {
            const auto git = games_.find(g->id_);
            return (*git).second;
        }
    }
    if (canCreate)
        return CreateGame(mapName);
    return std::shared_ptr<Game>();
}

void GameManager::AddPlayer(const std::string& mapName, std::shared_ptr<Player> player)
{
    std::lock_guard<std::recursive_mutex> lockClass(lock_);
    std::shared_ptr<Game> game = GetGame(mapName, true);


}

}

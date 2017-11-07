#include "stdafx.h"
#include "GameManager.h"
#include "Utils.h"
#include <algorithm>
#include "Scheduler.h"

#include "DebugNew.h"

namespace Game {

GameManager GameManager::Instance;

void GameManager::UpdateThread()
{
    int64_t prevTime = Utils::AbTick();
    while (state_ != Terminated)
    {
        int64_t start = Utils::AbTick();
        uint32_t delta = static_cast<uint32_t>(start - prevTime);
        prevTime = start;
        for (const auto& g : games_)
        {
            if (g.second->GetPlayerCount() > 0)
                g.second->Update(delta);
            else
            {
                // No players -> delete the game
                Asynch::Scheduler::Instance.Add(
                    Asynch::CreateScheduledTask(500, std::bind(&GameManager::DeleteGameTask, this, g.first))
                );
            }
        }
        int64_t end = Utils::AbTick();
        uint32_t duration = static_cast<uint32_t>(end - start);
        int32_t sleepTime = NETWORK_TICK - duration;
        if (sleepTime < 0)
            sleepTime = 5;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
    }
}

void GameManager::Start(Net::ServiceManager* serviceManager)
{
    serviceManager_ = serviceManager;
    state_ = State::Running;
    thread_ = std::thread(&GameManager::UpdateThread, this);
}

void GameManager::Stop()
{
    if (state_ == State::Running)
    {
        lock_.lock();
        state_ = Terminated;
        lock_.unlock();
        thread_.join();
    }
}

std::shared_ptr<Game> GameManager::CreateGame(const std::string& mapName)
{
    std::shared_ptr<Game> game = std::make_shared<Game>();
    std::lock_guard<std::recursive_mutex> lockClass(lock_);
    game->id_ = GetNewGameId();
    games_[game->id_] = game;
    maps_[mapName].push_back(game.get());
    return game;
}

void GameManager::DeleteGameTask(uint32_t gameId)
{
    std::lock_guard<std::recursive_mutex> lockClass(lock_);
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

#include "stdafx.h"
#include "Game.h"
#include "PlayerManager.h"
#include "Player.h"
#include <algorithm>
#include "Scheduler.h"
#include "Dispatcher.h"
#include "GameManager.h"

#include "DebugNew.h"

namespace Game {

Game::Game() :
    state_(GameStateTerminated)
{
    startTime_ = Utils::AbTick();
}

void Game::Start()
{
    if (state_ == GameStateStartup)
    {
        SetState(GameStateRunning);
        Asynch::Dispatcher::Instance.Add(
            Asynch::CreateTask(std::bind(&Game::Update, shared_from_this()))
        );
    }
}

void Game::Stop()
{
    SetState(GameStateTerminated);
}

void Game::Update()
{
    // Dispatcher Thread
    static int64_t prevTime = Utils::AbTick();
    int64_t start = Utils::AbTick();
    uint32_t delta = static_cast<uint32_t>(start - prevTime);
    prevTime = start;

    for (const auto& o : objects_)
    {
        o->Update(delta);
    }

    switch (state_)
    {
    case GameStateRunning:
    {
        // Reschedule next update
        int64_t end = Utils::AbTick();
        uint32_t duration = static_cast<uint32_t>(end - start);
        // At least 5ms
        int32_t sleepTime = std::max<int32_t>(5, NETWORK_TICK - duration);
        Asynch::Scheduler::Instance.Add(
            Asynch::CreateScheduledTask(sleepTime, std::bind(&Game::Update, shared_from_this()))
        );
        break;
    }
    case GameStateTerminated:
        // Delete this game
        Asynch::Scheduler::Instance.Add(
            Asynch::CreateScheduledTask(500, std::bind(&GameManager::DeleteGameTask, &GameManager::Instance, id_))
        );
        break;
    }
}

Player* Game::GetPlayerById(uint32_t playerId)
{
    auto it = players_.find(playerId);
    if (it == players_.end())
        return nullptr;
    return (*it).second;
}

void Game::SetState(GameState state)
{
    if (state_ != state)
    {
        std::lock_guard<std::recursive_mutex> lockClass(lock_);
        state_ = state;
    }
}

void Game::InternalLoad()
{
    // Game::Load() Thread

    // TODO: Load Data, Assets etc

    if (state_ == GameStateStartup)
        Start();
}

void Game::Load(const std::string& mapName)
{
    // Dispatcher Thread
    data_.mapName = mapName;
    std::thread(&Game::InternalLoad, shared_from_this()).detach();
}

void Game::PlayerJoin(uint32_t playerId)
{
    std::shared_ptr<Player> player = PlayerManager::Instance.GetPlayerById(playerId);
    if (player)
    {
        std::lock_guard<std::recursive_mutex> lockClass(lock_);
        players_[player->id_] = player.get();
        objects_.push_back(player);
        player->SetGame(shared_from_this());
    }
}

void Game::PlayerLeave(uint32_t playerId)
{
    std::shared_ptr<Player> player = PlayerManager::Instance.GetPlayerById(playerId);
    if (player)
    {
        std::lock_guard<std::recursive_mutex> lockClass(lock_);
        player->SetGame(std::shared_ptr<Game>());
        auto it = players_.find(playerId);
        if (it != players_.end())
            players_.erase(it);
        auto ito = std::find_if(objects_.begin(), objects_.end(), [&](std::shared_ptr<GameObject> const& o) -> bool
        {
            return o->id_ == player->id_;
        });
        if (ito != objects_.end())
            objects_.erase(ito);
    }
}

void Game::PlayerMove(uint32_t playerId, MoveDirection direction)
{
    Player* player = GetPlayerById(playerId);
    if (!player)
        return;

}

}

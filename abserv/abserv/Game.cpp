#include "stdafx.h"
#include "Game.h"
#include "PlayerManager.h"
#include "Player.h"
#include <algorithm>
#include "Scheduler.h"
#include "Dispatcher.h"
#include "GameManager.h"
#include "Effect.h"
#include "IOGame.h"

#include "DebugNew.h"

namespace Game {

Game::Game() :
    state_(GameStateTerminated),
    lastUpdate_(0)
{
    InitializeLua();
    startTime_ = Utils::AbTick();
}

void Game::Start()
{
    if (state_ == GameStateStartup)
    {
#ifdef DEBUG_GAME
        LOG_DEBUG << "Starting game " << id_ << ", " << data_.mapName << std::endl;
#endif // DEBUG_GAME
        SetState(GameStateRunning);
        Asynch::Dispatcher::Instance.Add(
            Asynch::CreateTask(std::bind(&Game::Update, shared_from_this()))
        );
    }
}

void Game::Stop()
{
#ifdef DEBUG_GAME
    LOG_DEBUG << "Stopping game " << id_ << ", " << data_.mapName << std::endl;
#endif // DEBUG_GAME
    SetState(GameStateTerminated);
}

void Game::Update()
{
    // Dispatcher Thread
    static int64_t prevTime = Utils::AbTick();
    lastUpdate_ = Utils::AbTick();
    uint32_t delta = static_cast<uint32_t>(lastUpdate_ - prevTime);
    prevTime = lastUpdate_;

    for (const auto& o : objects_)
    {
        o->Update(delta);
    }

    switch (state_)
    {
    case GameStateRunning:
    case GameStateShutdown:
    {
        if (state_ == GameStateShutdown && GetPlayerCount() == 0)
        {
            // If all players left the game, delete it
            SetState(GameStateTerminated);
        }

        // Schedule next update
        int64_t end = Utils::AbTick();
        uint32_t duration = static_cast<uint32_t>(end - lastUpdate_);
        // At least 5ms
        int32_t sleepTime = std::max<int32_t>(5, NETWORK_TICK - duration);
        Asynch::Scheduler::Instance.Add(
            Asynch::CreateScheduledTask(sleepTime, std::bind(&Game::Update, this))
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

void Game::RegisterLua(kaguya::State& state)
{
    state["Game"].setClass(kaguya::UserdataMetatable<Game>()
        /*        .addFunction("GetName", &Skill::GetName)
        */
    );
}

void Game::InitializeLua()
{
    // Register all used classes
    Game::RegisterLua(luaState_);
    Effect::RegisterLua(luaState_);
    Player::RegisterLua(luaState_);
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
        // Loading done -> start it
        Start();
}

void Game::Load(const std::string& mapName)
{
    // Dispatcher Thread
    data_.mapName = mapName;
    if (!DB::IOGame::LoadGameByName(this, mapName))
    {
        LOG_ERROR << "Error loading game with name " << mapName << std::endl;
        return;
    }
    // Load Assets
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

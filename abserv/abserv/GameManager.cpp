#include "stdafx.h"
#include "GameManager.h"
#include "Utils.h"
#include "Scheduler.h"
#include "Dispatcher.h"
#include "Player.h"
#include "Npc.h"

#include "DebugNew.h"

namespace Game {

GameManager GameManager::Instance;

void GameManager::Start(Net::ServiceManager* serviceManager)
{
    // Main Thread
    serviceManager_ = serviceManager;
    std::lock_guard<std::recursive_mutex> lockClass(lock_);
    state_ = State::ManagerStateRunning;
}

void GameManager::Stop()
{
    // Main Thread
    if (state_ == State::ManagerStateRunning)
    {
        {
            std::lock_guard<std::recursive_mutex> lockClass(lock_);
            state_ = State::ManagerStateTerminated;
        }
        for (const auto& g : games_)
        {
            g.second->SetState(Game::ExecutionStateTerminated);
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
        std::lock_guard<std::recursive_mutex> lockClass(lock_);
        game->id_ = GetNewGameId();
        games_[game->id_] = game;
        maps_[mapUuid].push_back(game.get());
    }
    game->SetState(Game::ExecutionStateStartup);
    game->Load(mapUuid);
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
        maps_.erase((*it).second->data_.uuid);
        games_.erase(it);
    }
}

std::shared_ptr<Game> GameManager::GetGame(const std::string& mapUuid, bool canCreate /* = false */)
{
    const auto it = maps_.find(mapUuid);
    if (it == maps_.end())
    {
        // Map does not exist
        if (!canCreate)
            return std::shared_ptr<Game>();
        return CreateGame(mapUuid);
    }
    // There are already some games with this map
    for (const auto& g : it->second)
    {
        if (g->GetPlayerCount() < GAME_MAX_PLAYER &&
            (g->GetState() == Game::ExecutionStateRunning || g->GetState() == Game::ExecutionStateStartup))
        {
            const auto git = games_.find(g->id_);
            return (*git).second;
        }
    }
    if (canCreate)
        return CreateGame(mapUuid);
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
    std::shared_ptr<Game> game;
    {
        std::lock_guard<std::recursive_mutex> lockClass(lock_);
        game = GetGame(mapUuid, true);
    }

    if (!game)
        return false;

    // No need to wait until assets loaded
    game->PlayerJoin(player->id_);
    return true;
}

void GameManager::CleanGames()
{
    for (const auto& g : games_)
    {
        if (g.second->GetPlayerCount() == 0)
            g.second->SetState(Game::ExecutionStateTerminated);
    }
}

void GameManager::LuaErrorHandler(int errCode, const char* message)
{
    LOG_ERROR << "Lua Error (" << errCode << "): " << message << std::endl;
}

void GameManager::RegisterLuaAll(kaguya::State& state)
{
    state.setErrorHandler(LuaErrorHandler);
#ifdef DEBUG_GAME
    if (!state.gc().isrunning())
    {
        LOG_ERROR << "Lua GC not running" << std::endl;
    }
#endif
    // Register all used classes
    GameObject::RegisterLua(state);
    Creature::RegisterLua(state);
    Game::RegisterLua(state);
    Effect::RegisterLua(state);
    Skill::RegisterLua(state);
    Player::RegisterLua(state);
    Npc::RegisterLua(state);
}

}

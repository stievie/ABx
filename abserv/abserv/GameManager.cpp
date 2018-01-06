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
    {
        std::lock_guard<std::recursive_mutex> lockClass(lock_);
        state_ = State::ManagerStateRunning;
    }
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
            g.second->SetState(Game::GameStateTerminated);
        }
    }
}

std::shared_ptr<Game> GameManager::CreateGame(const std::string& mapName)
{
    assert(state_ == State::ManagerStateRunning);
#ifdef DEBUG_GAME
    LOG_DEBUG << "Creating game " << mapName << std::endl;
#endif
    std::shared_ptr<Game> game = std::make_shared<Game>();
    {
        std::lock_guard<std::recursive_mutex> lockClass(lock_);
        game->id_ = GetNewGameId();
        games_[game->id_] = game;
        maps_[mapName].push_back(game.get());
    }
    game->SetState(Game::GameStateStartup);
    game->Load(mapName);
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
        maps_.erase((*it).second->GetName());
        games_.erase(it);
    }
}

std::shared_ptr<Game> GameManager::GetGame(const std::string& mapName, bool canCreate /* = false */)
{
    const auto it = maps_.find(mapName);
    if (it == maps_.end())
    {
        // Map does not exist
        if (!canCreate)
            return std::shared_ptr<Game>();
        return CreateGame(mapName);
    }
    // There are already some games with this map
    for (const auto& g : it->second)
    {
        if (g->GetPlayerCount() < GAME_MAX_PLAYER &&
            (g->GetState() == Game::GameStateRunning || g->GetState() == Game::GameStateStartup))
        {
            const auto git = games_.find(g->id_);
            return (*git).second;
        }
    }
    if (canCreate)
        return CreateGame(mapName);
    return std::shared_ptr<Game>();
}

bool GameManager::AddPlayer(const std::string& mapName, std::shared_ptr<Player> player)
{
    std::shared_ptr<Game> game;
    {
        std::lock_guard<std::recursive_mutex> lockClass(lock_);
        game = GetGame(mapName, true);
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
            g.second->SetState(Game::GameStateTerminated);
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

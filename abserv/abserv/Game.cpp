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
#include "ConfigManager.h"
#include "Logger.h"
#include "Skill.h"
#include "Npc.h"
#include "DataProvider.h"
#include "OutputMessage.h"
#include <AB/ProtocolCodes.h>
#include "ProtocolGame.h"

#include "DebugNew.h"

namespace Game {

Game::Game() :
    state_(GameStateTerminated),
    lastUpdate_(0),
    navMesh_(nullptr)
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
        lastUpdate_ = 0;
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
    if (lastUpdate_ == 0)
        luaState_["onStart"](this);

    // Dispatcher Thread
    static int64_t prevTime = Utils::AbTick();
    lastUpdate_ = Utils::AbTick();
    uint32_t delta = static_cast<uint32_t>(lastUpdate_ - prevTime);
    prevTime = lastUpdate_;

    // First Update all objects
    for (const auto& o : objects_)
    {
        o->Update(delta);
    }

    // Then call Lua Update function
    luaState_["onUpdate"](this, delta);

    // Send game status to players
    SendStatus();

    switch (state_)
    {
    case GameStateRunning:
    case GameStateShutdown:
    {
        if (state_ == GameStateShutdown && GetPlayerCount() == 0)
        {
            // If all players left the game, delete it
            SetState(GameStateTerminated);
            luaState_["onStop"](this);
        }

        // Schedule next update
        const int64_t end = Utils::AbTick();
        const uint32_t duration = static_cast<uint32_t>(end - lastUpdate_);
        // At least SCHEDULER_MINTICKS
        const int32_t sleepTime = std::max<int32_t>(SCHEDULER_MINTICKS, NETWORK_TICK - duration);
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

void Game::SendStatus()
{
    int64_t tick = Utils::AbTick();
    for (const auto& p : players_)
    {
        // If it didn't send a ping the last 2 sec it my be disconnected

        if (p.second->lastPing_ + 2000 < tick)
            continue;

        std::shared_ptr<Net::OutputMessage> output = Net::OutputMessagePool::Instance()->GetOutputMessage();
        output->AddByte(AB::GameProtocol::GameUpdate);
        p.second->client_->Send(output);
    }
}

void Game::Ping(uint32_t playerId)
{
    std::shared_ptr<Player> player = PlayerManager::Instance.GetPlayerById(playerId);
    if (!player)
        return;

    player->lastPing_ = Utils::AbTick();
    std::shared_ptr<Net::OutputMessage> output = Net::OutputMessagePool::Instance()->GetOutputMessage();
    output->AddByte(AB::GameProtocol::GamePong);
    player->client_->Send(output);
}

void Game::RegisterLua(kaguya::State& state)
{
    state["Game"].setClass(kaguya::UserdataMetatable<Game>()
        .addFunction("GetName", &Game::GetName)
        // Get any game object by ID
        .addFunction("GetObject", &Game::GetObjectById)
        // Get player of game by ID or name
        .addOverloadedFunctions("GetPlayer", &Game::GetPlayerById, &Game::GetPlayerByName)

        .addFunction("AddNpc", &Game::AddNpc)
    );
}

void Game::InitializeLua()
{
    GameManager::RegisterLuaAll(luaState_);
}

Player* Game::GetPlayerById(uint32_t playerId)
{
    auto it = players_.find(playerId);
    if (it == players_.end())
        return nullptr;
    return (*it).second;
}

Player* Game::GetPlayerByName(const std::string& name)
{
    uint32_t playerId = PlayerManager::Instance.GetPlayerId(name);
    if (playerId != 0)
        return GetPlayerById(playerId);
    return nullptr;
}

GameObject* Game::GetObjectById(uint32_t objectId)
{
    auto it = std::find_if(objects_.begin(), objects_.end(), [&](std::shared_ptr<GameObject> const& o) -> bool
    {
        return o->id_ == objectId;
    });
    if (it != objects_.end())
        return (*it).get();
    return nullptr;
}

std::shared_ptr<Npc> Game::AddNpc(const std::string& script)
{
    std::shared_ptr<Npc> result = std::make_shared<Npc>();
    if (!result->LoadScript(IO::DataProvider::Instance.GetDataFile(script)))
    {
        return std::shared_ptr<Npc>();
    }
    objects_.push_back(result);
    result->SetGame(shared_from_this());
    luaState_["onAddObject"](this, result);
    return result;
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
    navMesh_ = IO::DataProvider::Instance.GetAsset<NavigationMesh>(data_.navMeshFile);

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
    std::string luaFile = IO::DataProvider::Instance.GetDataFile(data_.scriptFile);
    // Execute initialization code if any
    if (!luaState_.dofile(luaFile.c_str()))
    {
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
        {
            std::lock_guard<std::recursive_mutex> lockClass(lock_);
            players_[player->id_] = player.get();
            objects_.push_back(player);
            player->SetGame(shared_from_this());
        }
        luaState_["onAddObject"](this, player);
        luaState_["onPlayerJoin"](this, player.get());
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
        {
            luaState_["onPlayerLeave"](this, (*it).second);
            players_.erase(it);
        }
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

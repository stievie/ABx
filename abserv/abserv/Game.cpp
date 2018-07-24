#include "stdafx.h"
#include "Game.h"
#include "PlayerManager.h"
#include "Player.h"
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
#include "PropStream.h"
#include "Random.h"
#include "IOMap.h"

#include "DebugNew.h"

namespace Game {

Game::Game() :
    state_(ExecutionState::Terminated),
    lastUpdate_(0),
    startTime_(0)
{
    InitializeLua();
    // Create gameStatus_ here, because we may already write it.
    ResetStatus();
    const uuids::uuid guid = uuids::uuid_system_generator{}();
    instanceData_.uuid = guid.to_string();
}

Game::~Game()
{
    DeleteEntity(instanceData_);
    players_.clear();
    objects_.clear();
    Chat::Instance.Remove(ChannelMap, id_);
}

void Game::Start()
{
    if (state_ == ExecutionState::Startup)
    {
        startTime_ = Utils::AbTick();
        instanceData_.startTime = startTime_;
        instanceData_.serverUuid = Application::Instance->GetServerId();
        CreateEntity(instanceData_);
        LOG_INFO << "Starting game " << id_ << ", " << map_->data_.name << std::endl;

        lastUpdate_ = 0;
        SetState(ExecutionState::Running);

        // Now that we are running we can spawn the queued players
        if (!queuedPlayers_.empty())
        {
            for (const auto& p : queuedPlayers_)
            {
                Asynch::Scheduler::Instance.Add(
                    Asynch::CreateScheduledTask(std::bind(&Game::QueueSpawnObject, shared_from_this(), p))
                );
            }
            queuedPlayers_.clear();
        }

        // Initial game update
        Asynch::Dispatcher::Instance.Add(
            Asynch::CreateTask(std::bind(&Game::Update, shared_from_this()))
        );
    }
}

void Game::Update()
{
    if (lastUpdate_ == 0)
    {
        luaState_["onStart"](this);
        // Add start tick at the beginning
        gameStatus_->AddByte(AB::GameProtocol::GameStart);
        gameStatus_->Add<int64_t>(startTime_);
    }

    // Dispatcher Thread
    int64_t tick = Utils::AbTick();
    if (lastUpdate_ == 0)
        lastUpdate_ = tick - NETWORK_TICK;
    uint32_t delta = static_cast<uint32_t>(tick - lastUpdate_);
    lastUpdate_ = tick;

    // Add timestamp
    gameStatus_->AddByte(AB::GameProtocol::GameUpdate);
    gameStatus_->Add<int64_t>(tick);

    // First Update all objects
    for (const auto& o : objects_)
    {
        o->Update(delta, *gameStatus_.get());
    }

    // Update Octree stuff
    map_->Update(delta);

    // Then call Lua Update function
    luaState_["onUpdate"](this, delta);

    // Send game status to players
    SendStatus();

    switch (state_)
    {
    case ExecutionState::Running:
    case ExecutionState::Shutdown:
    {
        if (state_ == ExecutionState::Shutdown && GetPlayerCount() == 0)
        {
            // If all players left the game, delete it. Actually just mark as
            // terminated, it'll be deleted in the next update.
            SetState(ExecutionState::Terminated);
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
    case ExecutionState::Terminated:
        // Delete this game
        LOG_INFO << "Stopping game " << id_ << ", " << map_->data_.name << std::endl;
        Asynch::Scheduler::Instance.Add(
            Asynch::CreateScheduledTask(500, std::bind(&GameManager::DeleteGameTask, &GameManager::Instance, id_))
        );
        break;
    }

}

void Game::SendStatus()
{
    // Must not be empty. Update adds at least the time stamp.
    assert(gameStatus_->GetSize() != 0);

    for (const auto& p : players_)
    {
        // Write to buffered, auto-sent output message
        p.second->client_->WriteToOutput(*gameStatus_.get());
    }
    ResetStatus();
}

void Game::ResetStatus()
{
    gameStatus_ = std::make_unique<Net::NetworkMessage>();
}

void Game::RegisterLua(kaguya::State& state)
{
    state["Game"].setClass(kaguya::UserdataMetatable<Game>()
        .addFunction("GetName", &Game::GetName)
        // Get any game object by ID
        .addFunction("GetObject", &Game::GetObjectById)
        // Get player of game by ID or name
        .addFunction("GetPlayer", &Game::GetPlayerById)

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
    uint32_t playerId = PlayerManager::Instance.GetPlayerIdByName(name);
    if (playerId != 0)
        return GetPlayerById(playerId);
    return nullptr;
}

std::shared_ptr<GameObject> Game::GetObjectById(uint32_t objectId)
{
    auto it = std::find_if(objects_.begin(), objects_.end(), [&](std::shared_ptr<GameObject> const& o) -> bool
    {
        return o->id_ == objectId;
    });
    if (it != objects_.end())
        return (*it);
    return std::shared_ptr<GameObject>();
}

void Game::AddObject(std::shared_ptr<GameObject> object)
{
    AddObjectInternal(object);
    luaState_["onAddObject"](this, object);
}

void Game::AddObjectInternal(std::shared_ptr<GameObject> object)
{
    objects_.push_back(object);
    object->SetGame(shared_from_this());
}

void Game::RemoveObject(std::shared_ptr<GameObject> object)
{
    luaState_["onRemoveObject"](this, object);
    object->SetGame(std::shared_ptr<Game>());
    auto ito = std::find_if(objects_.begin(), objects_.end(), [&](std::shared_ptr<GameObject> const& o) -> bool
    {
        return o->id_ == object->id_;
    });
    if (ito != objects_.end())
    {
        objects_.erase(ito);
    }
}

std::shared_ptr<Npc> Game::AddNpc(const std::string& script)
{
    std::shared_ptr<Npc> result = std::make_shared<Npc>();
    if (!result->LoadScript(IO::DataProvider::Instance.GetDataFile(script)))
    {
        return std::shared_ptr<Npc>();
    }
    AddObject(result);
    return result;
}

void Game::SetState(ExecutionState state)
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

    if (!IO::IOMap::Load(*map_.get()))
    {
        LOG_ERROR << "Error loading map with name " << map_->data_.name << std::endl;
        return;
    }

    if (state_ == ExecutionState::Startup)
        // Loading done -> start it
        Start();
}

void Game::Load(const std::string& mapUuid)
{
    // Dispatcher Thread
    map_ = std::make_shared<Map>(shared_from_this());
    if (!IO::IOGame::LoadGameByUuid(this, mapUuid))
    {
        LOG_ERROR << "Error loading game " << mapUuid << std::endl;
        return;
    }
    map_->data_.name = data_.name;
    map_->data_.directory = data_.directory;

    // Must be executed here because the player doesn't wait to fully load the game to join
    std::string luaFile = IO::DataProvider::Instance.GetDataFile(data_.script);
    // Execute initialization code if any
    if (!luaState_.dofile(luaFile.c_str()))
        return;

    // Load Assets
    std::thread(&Game::InternalLoad, shared_from_this()).detach();
}

void Game::QueueSpawnObject(std::shared_ptr<GameObject> object)
{
    gameStatus_->AddByte(AB::GameProtocol::GameSpawnObject);
    gameStatus_->Add<uint32_t>(object->id_);
    if (object->GetType() == AB::GameProtocol::ObjectTypePlayer)
    {
        // Spawn points are loaded now
        const SpawnPoint& p = map_->GetFreeSpawnPoint();
        object->transformation_.position_ = p.position;
        object->transformation_.rotation_ = p.rotation.EulerAngles().y_;
    }
    gameStatus_->Add<float>(object->transformation_.position_.x_);
    gameStatus_->Add<float>(object->transformation_.position_.y_);
    gameStatus_->Add<float>(object->transformation_.position_.z_);
    gameStatus_->Add<float>(object->transformation_.rotation_);
    gameStatus_->Add<float>(object->transformation_.scale_.x_);
    gameStatus_->Add<float>(object->transformation_.scale_.y_);
    gameStatus_->Add<float>(object->transformation_.scale_.z_);

    IO::PropWriteStream data;
    size_t dataSize;
    object->Serialize(data);
    const char* cData = data.GetStream(dataSize);
    gameStatus_->AddString(std::string(cData, dataSize));
}

void Game::QueueLeaveObject(uint32_t objectId)
{
    gameStatus_->AddByte(AB::GameProtocol::GameLeaveObject);
    gameStatus_->Add<uint32_t>(objectId);
}

void Game::SendSpawnAll(uint32_t playerId)
{
    std::shared_ptr<Player> player = PlayerManager::Instance.GetPlayerById(playerId);
    if (!player)
        return;

    // Send all already existing objects to the player, excluding the player itself.
    // This is sent to all players.
    // Only called when the player enters a game. All spawns during the game are sent
    // when they happen.
    Net::NetworkMessage msg;
    for (const auto& o : objects_)
    {
        if (o.get() == player.get())
            // Don't send spawn of our self
            continue;

        msg.AddByte(AB::GameProtocol::GameSpawnObjectExisting);
        msg.Add<uint32_t>(o->id_);
        msg.Add<float>(o->transformation_.position_.x_);
        msg.Add<float>(o->transformation_.position_.y_);
        msg.Add<float>(o->transformation_.position_.z_);
        msg.Add<float>(o->transformation_.rotation_);
        msg.Add<float>(o->transformation_.scale_.x_);
        msg.Add<float>(o->transformation_.scale_.y_);
        msg.Add<float>(o->transformation_.scale_.z_);
        IO::PropWriteStream data;
        size_t dataSize;
        o->Serialize(data);
        const char* cData = data.GetStream(dataSize);
        msg.AddString(std::string(cData, dataSize));
    }
    if (msg.GetSize() != 0)
        player->client_->WriteToOutput(msg);
}

void Game::PlayerJoin(uint32_t playerId)
{
    std::shared_ptr<Player> player = PlayerManager::Instance.GetPlayerById(playerId);
    if (player)
    {
        {
            std::lock_guard<std::recursive_mutex> lockClass(lock_);
            players_[player->id_] = player.get();
            player->data_.instanceUuid = instanceData_.uuid;
            AddObject(player);
        }
        UpdateEntity(player->data_);

        luaState_["onPlayerJoin"](this, player.get());
        Asynch::Scheduler::Instance.Add(
            Asynch::CreateScheduledTask(std::bind(&Game::SendSpawnAll, shared_from_this(), playerId))
        );

        if (GetState() == ExecutionState::Running)
        {
            // In worst case (i.e. the game data is still loading): will be sent as
            // soon as the game runs and entered the Update loop.
            Asynch::Scheduler::Instance.Add(
                Asynch::CreateScheduledTask(std::bind(&Game::QueueSpawnObject, shared_from_this(), player))
            );
        }
        else
            queuedPlayers_.push_back(player);
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
        RemoveObject(player);
        player->data_.instanceUuid = "";
        UpdateEntity(player->data_);

        Asynch::Scheduler::Instance.Add(
            Asynch::CreateScheduledTask(std::bind(&Game::QueueLeaveObject, shared_from_this(), playerId))
        );
    }
}

}

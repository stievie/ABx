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
#include "Profiler.h"
#include "ScriptManager.h"
#include "ThreadPool.h"
#include "Subsystems.h"
#include "AreaOfEffect.h"
#include "ItemFactory.h"
#include "ItemDrop.h"

#include "DebugNew.h"

namespace Game {

Game::Game() :
    state_(ExecutionState::Terminated),
    lastUpdate_(0),
    noplayerTime_(0),
    id_(0),
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
    GetSubsystem<Chat>()->Remove(ChatType::Map, id_);
}

GameObject* Game::_LuaGetObjectById(uint32_t objectId)
{
    auto o = GetObjectById(objectId);
    if (o)
        return o.get();
    return nullptr;
}

Npc* Game::_LuaAddNpc(const std::string& script)
{
    auto o = AddNpc(script);
    if (o)
        return o.get();
    return nullptr;
}

AreaOfEffect* Game::_LuaAddAreaOfEffect(const std::string& script, Actor* source, uint32_t index, float x, float y, float z)
{
    auto o = AddAreaOfEffect(script, source ? source->GetThis<Actor>() : std::shared_ptr<Actor>(), index, Math::Vector3(x, y, z));
    if (o)
        return o.get();
    return nullptr;
}

ItemDrop* Game::_LuaAddItemDrop(Actor* dropper)
{
    auto o = AddRandomItemDrop(dropper);
    if (o)
        return o.get();
    return nullptr;
}

void Game::BroadcastPlayerLoggedIn(std::shared_ptr<Player> player)
{
    auto client = GetSubsystem<Net::MessageClient>();
    Net::MessageMsg msg;
    msg.type_ = Net::MessageType::PlayerLoggedIn;

    IO::PropWriteStream stream;
    stream.WriteString(player->account_.uuid);   // Account
    stream.WriteString(player->data_.uuid);      // Character
    msg.SetPropStream(stream);
    client->Write(msg);
}

void Game::BroadcastPlayerLoggedOut(std::shared_ptr<Player> player)
{
    auto client = GetSubsystem<Net::MessageClient>();
    Net::MessageMsg msg;
    msg.type_ = Net::MessageType::PlayerLoggedOut;

    IO::PropWriteStream stream;
    stream.WriteString(player->account_.uuid);   // Account
    stream.WriteString(player->data_.uuid);      // Character
    msg.SetPropStream(stream);
    client->Write(msg);
}

void Game::RegisterLua(kaguya::State& state)
{
    state["Game"].setClass(kaguya::UserdataMetatable<Game>()
        .addFunction("GetName", &Game::GetName)
        .addFunction("GetType", &Game::_LuaGetType)
        .addFunction("GetDefaultLevel", &Game::GetDefaultLevel)
        // Get any game object by ID
        .addFunction("GetObject", &Game::_LuaGetObjectById)
        // Get player of game by ID or name
        .addFunction("GetPlayer", &Game::GetPlayerById)
        .addFunction("GetParties", &Game::GetParties)

        .addFunction("GetTerrainHeight", &Game::_LuaGetTerrainHeight)
        .addFunction("GetStartTime", &Game::_LuaGetStartTime)
        .addFunction("GetInstanceTime", &Game::GetInstanceTime)

        .addFunction("AddNpc", &Game::_LuaAddNpc)
        .addFunction("AddAreaOfEffect", &Game::_LuaAddAreaOfEffect)
        .addFunction("AddRandomItemDrop", &Game::_LuaAddItemDrop)
    );
}

void Game::InitializeLua()
{
    ScriptManager::RegisterLuaAll(luaState_);
    luaState_["self"] = this;
}

void Game::Start()
{
    if (state_ == ExecutionState::Startup)
    {
        auto config = GetSubsystem<ConfigManager>();
        startTime_ = Utils::Tick();
        instanceData_.startTime = startTime_;
        instanceData_.serverUuid = Application::Instance->GetServerId();
        instanceData_.gameUuid = data_.uuid;
        CreateEntity(instanceData_);
        LOG_INFO << "Starting game " << id_ << ", " << map_->data_.name << std::endl;

        if ((*config)[ConfigManager::Key::RecordGames])
        {
            writeStream_ = std::make_unique<IO::GameWriteStream>();
            writeStream_->Open((*config)[ConfigManager::Key::RecordingsDir], this);
        }

        lastUpdate_ = 0;
        SetState(ExecutionState::Running);

        // Now that we are running we can spawn the queued players
        if (!queuedObjects_.empty())
        {
            auto it = queuedObjects_.begin();
            while (it != queuedObjects_.end())
            {
                QueueSpawnObject((*it));
                it = queuedObjects_.erase(it);
            }
        }

        // Initial game update
        GetSubsystem<Asynch::Dispatcher>()->Add(
            Asynch::CreateTask(std::bind(&Game::Update, shared_from_this()))
        );
    }
#ifdef DEBUG_GAME
    else
        LOG_DEBUG << "Game state is not Startup it is " << static_cast<int>(state_.load()) << std::endl;
#endif // DEBUG_GAME

}

void Game::Update()
{
    // Dispatcher Thread
    if (state_ != ExecutionState::Terminated)
    {
        if (lastUpdate_ == 0)
        {
            noplayerTime_ = 0;
            ScriptManager::CallFunction(luaState_, "onStart");
            // Add start tick at the beginning
            gameStatus_->AddByte(AB::GameProtocol::GameStart);
            gameStatus_->Add<int64_t>(startTime_);
        }

        int64_t tick = Utils::Tick();
        if (lastUpdate_ == 0)
            lastUpdate_ = tick - NETWORK_TICK;
        uint32_t delta = static_cast<uint32_t>(tick - lastUpdate_);
        lastUpdate_ = tick;

        // Add timestamp
        gameStatus_->AddByte(AB::GameProtocol::GameUpdate);
        gameStatus_->Add<int64_t>(tick);

        map_->UpdateAi(delta);

        // First Update all objects
        for (const auto& o : objects_)
        {
            if (!o.second)
                return;
            o.second->Update(delta, *gameStatus_.get());
        }

        // Update Octree stuff
        map_->UpdateOctree(delta);

        // Then call Lua Update function
        ScriptManager::CallFunction(luaState_, "onUpdate", delta);

        // Send game status to players
        SendStatus();

        if (GetPlayerCount() == 0)
            noplayerTime_ += delta;
        else
            noplayerTime_ = 0;
    }

    switch (state_)
    {
    case ExecutionState::Running:
    case ExecutionState::Shutdown:
    {
        if (state_ == ExecutionState::Shutdown && IsInactive())
        {
            // If all players left the game, delete it. Actually just mark as
            // terminated, it'll be deleted in the next update.
            // Keep empty games for 10 seconds
            LOG_INFO << "Shutting down game " << id_ << ", " << map_->data_.name << " no players for " << noplayerTime_ << std::endl;
            SetState(ExecutionState::Terminated);
            ScriptManager::CallFunction(luaState_, "onStop");
        }

        // Schedule next update
        const int64_t end = Utils::Tick();
        const uint32_t duration = static_cast<uint32_t>(end - lastUpdate_);
        // At least SCHEDULER_MINTICKS
        const int32_t sleepTime = std::max<int32_t>(SCHEDULER_MINTICKS, NETWORK_TICK - duration);
        GetSubsystem<Asynch::Scheduler>()->Add(
            Asynch::CreateScheduledTask(sleepTime, std::bind(&Game::Update, shared_from_this()))
        );

        break;
    }
    case ExecutionState::Terminated:
        // Delete this game
        LOG_INFO << "Stopping game " << id_ << ", " << map_->data_.name << std::endl;
        GetSubsystem<Asynch::Scheduler>()->Add(
            Asynch::CreateScheduledTask(500, std::bind(&GameManager::DeleteGameTask,
                GetSubsystem<GameManager>(), id_))
        );
        break;
    case ExecutionState::Startup:
        // Do nothing
        break;
    }
    ScriptManager::CollectGarbage(luaState_);
}

void Game::SendStatus()
{
    // Must not be empty. Update adds at least the time stamp.
    assert(gameStatus_->GetSize() != 0);

    for (const auto& p : players_)
    {
        // Write to buffered, auto-sent output message
        p.second->WriteToOutput(*gameStatus_.get());
    }

    if (writeStream_ && writeStream_->IsOpen())
        writeStream_->Write(*gameStatus_.get());

    ResetStatus();
}

void Game::ResetStatus()
{
    gameStatus_ = Net::NetworkMessage::GetNew();
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
    uint32_t playerId = GetSubsystem<PlayerManager>()->GetPlayerIdByName(name);
    if (playerId != 0)
        return GetPlayerById(playerId);
    return nullptr;
}

std::shared_ptr<GameObject> Game::GetObjectById(uint32_t objectId)
{
    auto it = objects_.find(objectId);
    if (it != objects_.end())
        return (*it).second;
    return std::shared_ptr<GameObject>();
}

void Game::AddObject(std::shared_ptr<GameObject> object)
{
    AddObjectInternal(object);
    ScriptManager::CallFunction(luaState_, "onAddObject", object.get());
}

void Game::AddObjectInternal(std::shared_ptr<GameObject> object)
{
    objects_.emplace(object->id_, object);
    object->SetGame(shared_from_this());
}

void Game::InternalRemoveObject(GameObject* object)
{
    ScriptManager::CallFunction(luaState_, "onRemoveObject", object);
    object->SetGame(std::shared_ptr<Game>());
    auto it = objects_.find(object->id_);
    if (it != objects_.end())
    {
        objects_.erase(it);
    }
}

std::shared_ptr<Npc> Game::AddNpc(const std::string& script)
{
    std::shared_ptr<Npc> result = std::make_shared<Npc>();
    result->SetGame(shared_from_this());
    if (!result->LoadScript(script))
    {
        return std::shared_ptr<Npc>();
    }
    map_->AddEntity(result->GetAi(), result->GetGroupId());

    // After all initialization is done, we can call this
    GetSubsystem<Asynch::Scheduler>()->Add(
        Asynch::CreateScheduledTask(std::bind(&Game::QueueSpawnObject,
            shared_from_this(), result))
    );

    return result;
}

std::shared_ptr<AreaOfEffect> Game::AddAreaOfEffect(const std::string& script,
    std::shared_ptr<Actor> source,
    uint32_t index,
    const Math::Vector3& pos)
{
    std::shared_ptr<AreaOfEffect> result = std::make_shared<AreaOfEffect>();
    result->SetGame(shared_from_this());
    result->transformation_.position_ = pos;
    result->SetSource(source);
    result->SetIndex(index);
    if (!result->LoadScript(script))
    {
        return std::shared_ptr<AreaOfEffect>();
    }

    // After all initialization is done, we can call this
    GetSubsystem<Asynch::Scheduler>()->Add(
        Asynch::CreateScheduledTask(std::bind(&Game::QueueSpawnObject,
            shared_from_this(), result))
    );

    return result;
}

std::shared_ptr<ItemDrop> Game::AddRandomItemDrop(Actor* dropper)
{
    if (state_ != ExecutionState::Running || !dropper)
        return std::shared_ptr<ItemDrop>();

    auto factory = GetSubsystem<ItemFactory>();
    auto rng = GetSubsystem<Crypto::Random>();
    const float rnd = rng->GetFloat();
    auto p = Utils::SelectRandomly(players_.begin(), players_.end(), rnd);
    if (p == players_.end())
        return std::shared_ptr<ItemDrop>();

    Player* target = (*p).second;
    if (!target)
        return std::shared_ptr<ItemDrop>();
    auto item = factory->CreateDropItem(instanceData_.uuid, data_.uuid, dropper->GetLevel(), target);
    if (!item)
        return std::shared_ptr<ItemDrop>();

    std::shared_ptr<ItemDrop> result = std::make_shared<ItemDrop>(item);
    result->transformation_.position_ = dropper->transformation_.position_;
    // Random pos around dropper
    result->transformation_.position_.y_ += 0.2f;
    result->transformation_.position_.x_ += rng->Get<float>(-RANGE_TOUCH, RANGE_TOUCH);
    result->transformation_.position_.z_ += rng->Get<float>(-RANGE_TOUCH, RANGE_TOUCH);
    result->SetSource(dropper->GetThis<Actor>());
    result->actorId_ = target->id_;

    SpawnItemDrop(result);
    return result;
}

void Game::SpawnItemDrop(std::shared_ptr<ItemDrop> item)
{
    item->SetGame(shared_from_this());
    // Also adds it to the objects array
    QueueSpawnObject(item);

    gameStatus_->AddByte(AB::GameProtocol::GameObjectDropItem);
    gameStatus_->Add<uint32_t>(item->GetSourceId());
    gameStatus_->Add<uint32_t>(item->actorId_);
    gameStatus_->Add<uint32_t>(item->id_);
    gameStatus_->Add<uint32_t>(item->GetItemIndex());
    const Item* pItem = item->GetItem();
    if (pItem)
    {
        gameStatus_->Add<uint32_t>(pItem->concreteItem_.count);
        gameStatus_->Add<uint16_t>(pItem->concreteItem_.value);
    }
    else
    {
        // Shouldn't get here!
        gameStatus_->Add<uint32_t>(1);
        gameStatus_->Add<uint16_t>(0);
    }
}

std::vector<Party*> Game::GetParties() const
{
    // TODO: This looks not so smart...
    std::vector<Party*> result;
    std::map<uint32_t, Party*> pm;
    for (const auto& player : players_)
    {
        auto p = player.second->GetParty();
        if (p)
            pm[p->id_] = p.get();
    }
    result.reserve(pm.size());
    for (const auto& p : pm)
        result.push_back(p.second);
    return result;
}

void Game::CallLuaEvent(const std::string& name, GameObject* sender, GameObject* data)
{
    if (ScriptManager::IsFunction(luaState_, name))
        luaState_[name](sender, data);
}

void Game::SetState(ExecutionState state)
{
    if (state_ != state)
        state_ = state;
}

void Game::InternalLoad()
{
    // Game::Load() Thread

    if (!IO::IOMap::Load(*map_.get()))
    {
        LOG_ERROR << "Error loading map with name " << map_->data_.name << std::endl;
        return;
    }

    // Loading done -> start it
    Start();
}

void Game::Load(const std::string& mapUuid)
{
    state_ = ExecutionState::Startup;
    AB_PROFILE;
    // Dispatcher Thread
    map_ = std::make_unique<Map>(shared_from_this());
    if (!IO::IOGame::LoadGameByUuid(this, mapUuid))
    {
        LOG_ERROR << "Error loading game " << mapUuid << std::endl;
        return;
    }
    map_->data_.name = data_.name;
    map_->data_.directory = data_.directory;

    // Must be executed here because the player doesn't wait to fully load the game to join
    // Execute initialization code if any
    script_ = GetSubsystem<IO::DataProvider>()->GetAsset<Script>(data_.script);
    if (!script_)
        return;
    if (!script_->Execute(luaState_))
        return;

    auto thPool = GetSubsystem<Asynch::ThreadPool>();
    // Load Game Assets
    thPool->Enqueue(&Game::InternalLoad, shared_from_this());
    // Load item drop chances on this map
    auto factory = GetSubsystem<ItemFactory>();
    thPool->Enqueue(&ItemFactory::LoadDropChances, factory, mapUuid);
}

void Game::QueueSpawnObject(std::shared_ptr<GameObject> object)
{
    AB::GameProtocol::GameObjectType objectType = object->GetType();
    if (objectType < AB::GameProtocol::ObjectTypeSentToPlayer)
        return;
    if (objectType == AB::GameProtocol::ObjectTypePlayer)
    {
        // Spawn points are loaded now
        const SpawnPoint p = map_->GetFreeSpawnPoint("Player");
#ifdef DEBUG_GAME
//        LOG_DEBUG << "Spawn point: " << p.group << "; Pos: " << p.position.ToString() << std::endl;
#endif
        object->transformation_.position_ = p.position;
        object->transformation_.SetYRotation(p.rotation.EulerAngles().y_);
    }

    gameStatus_->AddByte(AB::GameProtocol::GameSpawnObject);
    object->WriteSpawnData(*gameStatus_);
    AddObject(object);
}

void Game::QueueLeaveObject(uint32_t objectId)
{
    gameStatus_->AddByte(AB::GameProtocol::GameLeaveObject);
    gameStatus_->Add<uint32_t>(objectId);
}

void Game::SendSpawnAll(uint32_t playerId)
{
    std::shared_ptr<Player> player = GetSubsystem<PlayerManager>()->GetPlayerById(playerId);
    if (!player)
        return;

    // Send all already existing objects to the player, excluding the player itself.
    // This is sent to all players.
    // Only called when the player enters a game. All spawns during the game are sent
    // when they happen.
    auto msg = Net::NetworkMessage::GetNew();
    const auto write = [&msg, &player](const std::shared_ptr<GameObject>& o)
    {
        if (o->GetType() < AB::GameProtocol::ObjectTypeSentToPlayer)
            // No need to send terrain patch to client
            return;
        if (o.get() == player.get())
            // Don't send spawn of our self
            return;

        msg->AddByte(AB::GameProtocol::GameSpawnObjectExisting);
        o->WriteSpawnData(*msg.get());
    };

    for (const auto& o : objects_)
    {
        write(o.second);
    }
    // Also send queued objects
    for (const auto& o : queuedObjects_)
    {
        write(o);
    }

    if (msg->GetSize() != 0)
        player->WriteToOutput(*msg.get());
}

void Game::PlayerJoin(uint32_t playerId)
{
    std::shared_ptr<Player> player = GetSubsystem<PlayerManager>()->GetPlayerById(playerId);
    if (player)
    {
        {
            std::lock_guard<std::mutex> lockClass(lock_);
            players_[player->id_] = player.get();
            if (data_.type == AB::Entities::GameTypeOutpost)
                player->data_.lastOutpostUuid = data_.uuid;
            player->data_.instanceUuid = instanceData_.uuid;
        }
        UpdateEntity(player->data_);

        ScriptManager::CallFunction(luaState_, "onPlayerJoin", player.get());
        SendSpawnAll(playerId);

        if (GetState() == ExecutionState::Running)
        {
            // In worst case (i.e. the game data is still loading): will be sent as
            // soon as the game runs and entered the Update loop.
            QueueSpawnObject(player);
        }
        else
            queuedObjects_.push_back(player);

        // Notify other servers that a player joined, e.g. for friend list
        GetSubsystem<Asynch::Scheduler>()->Add(
            Asynch::CreateScheduledTask(std::bind(&Game::BroadcastPlayerLoggedIn,
                shared_from_this(),
                player))
        );
    }
}

void Game::RemoveObject(GameObject* object)
{
    GetSubsystem<Asynch::Scheduler>()->Add(
        Asynch::CreateScheduledTask(std::bind(&Game::QueueLeaveObject, shared_from_this(), object->id_))
    );
    InternalRemoveObject(object);
}

void Game::PlayerLeave(uint32_t playerId)
{
    Player* player = GetPlayerById(playerId);
    if (player)
    {
        std::lock_guard<std::mutex> lockClass(lock_);
        player->SetGame(std::shared_ptr<Game>());
        auto it = players_.find(playerId);
        if (it != players_.end())
        {
            ScriptManager::CallFunction(luaState_, "onPlayerLeave", player);
            players_.erase(it);
        }
        player->data_.instanceUuid = "";
        UpdateEntity(player->data_);

        GetSubsystem<Asynch::Scheduler>()->Add(
            Asynch::CreateScheduledTask(std::bind(&Game::QueueLeaveObject, shared_from_this(), playerId))
        );
        // Notify other servers that a player left, e.g. for friend list
        GetSubsystem<Asynch::Scheduler>()->Add(
            Asynch::CreateScheduledTask(std::bind(&Game::BroadcastPlayerLoggedOut,
                shared_from_this(),
                player->GetThis()))
        );
        InternalRemoveObject(player);
    }
}

}

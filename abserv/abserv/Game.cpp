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
#include "Game.h"
#include "AreaOfEffect.h"
#include "ConfigManager.h"
#include "Crowd.h"
#include "DataProvider.h"
#include "Effect.h"
#include "GameManager.h"
#include "IOGame.h"
#include "IOMap.h"
#include "ItemDrop.h"
#include "ItemFactory.h"
#include "Npc.h"
#include "Player.h"
#include "PlayerManager.h"
#include "Projectile.h"
#include "ProtocolGame.h"
#include "Script.h"
#include "ScriptManager.h"
#include "Skill.h"
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>
#include <AB/ProtocolCodes.h>
#include <sa/EAIterator.h>

namespace Game {

Game::Game()
{
    InitializeLua();
    // Create gameStatus_ here, because we may already write it.
    ResetStatus();
    instanceData_.uuid = Utils::Uuid::New();
}

Game::~Game()
{
    instanceData_.running = false;
    instanceData_.stopTime = Utils::Tick();
    UpdateEntity(instanceData_);
    players_.clear();
    objects_.clear();
    GetSubsystem<Chat>()->Remove(ChatType::Map, id_);
}

GameObject* Game::_LuaGetObjectById(uint32_t objectId)
{
    return GetObject<GameObject>(objectId);
}

Npc* Game::_LuaAddNpc(const std::string& script)
{
    auto o = AddNpc(script);
    if (o)
        return o.get();
    return nullptr;
}

AreaOfEffect* Game::_LuaAddAreaOfEffect(const std::string& script,
    Actor* source, uint32_t index,
    const Math::StdVector3& pos)
{
    auto o = AddAreaOfEffect(script,
        source ? source->GetPtr<Actor>() : ea::shared_ptr<Actor>(),
        index, pos);
    if (o)
        return o.get();
    return nullptr;
}

void Game::_LuaAddProjectile(const std::string& itemUuid,
    Actor* source,
    Actor* target)
{
    assert(source);
    assert(target);
    AddProjectile(itemUuid, source->GetPtr<Actor>(), target->GetPtr<Actor>());
}

ItemDrop* Game::_LuaAddItemDrop(Actor* dropper)
{
    auto o = AddRandomItemDrop(dropper);
    if (o)
        return o.get();
    return nullptr;
}

void Game::BroadcastPlayerLoggedIn(ea::shared_ptr<Player> player)
{
    if (player->account_.onlineStatus == AB::Entities::OnlineStatusInvisible ||
        player->account_.onlineStatus == AB::Entities::OnlineStatusOffline)
        return;

    BroadcastPlayerChanged(*player, AB::GameProtocol::PlayerInfoFieldOnlineStatus |
        AB::GameProtocol::PlayerInfoFieldCurrentName | AB::GameProtocol::PlayerInfoFieldCurrentMap);
}

void Game::BroadcastPlayerLoggedOut(ea::shared_ptr<Player> player)
{
    if (player->account_.onlineStatus == AB::Entities::OnlineStatusInvisible ||
        player->account_.onlineStatus == AB::Entities::OnlineStatusOffline)
        return;

    BroadcastPlayerChanged(*player, AB::GameProtocol::PlayerInfoFieldOnlineStatus);
}

void Game::BroadcastPlayerChanged(const Player& player, uint32_t fields)
{
    auto* client = GetSubsystem<Net::MessageClient>();
    Net::MessageMsg msg;
    msg.type_ = Net::MessageType::PlayerChanged;

    sa::PropWriteStream stream;
    stream.Write<uint32_t>(fields);              // What has changed
    stream.WriteString(player.account_.uuid);    // Account
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
        .addFunction("GetParties", &Game::_LuaGetParties)
        .addFunction("GetGroup", &Game::GetGroup)
        .addFunction("AddCrowd", &Game::AddCrowd)

        .addFunction("GetTerrainHeight", &Game::_LuaGetTerrainHeight)
        .addFunction("GetStartTime", &Game::_LuaGetStartTime)
        .addFunction("GetInstanceTime", &Game::GetInstanceTime)

        .addFunction("AddNpc", &Game::_LuaAddNpc)
        .addFunction("AddAreaOfEffect", &Game::_LuaAddAreaOfEffect)
        .addFunction("AddProjectile", &Game::_LuaAddProjectile)
        .addFunction("AddRandomItemDrop", &Game::_LuaAddItemDrop)
    );
}

void Game::InitializeLua()
{
    Lua::RegisterLuaAll(luaState_);
    luaState_["self"] = this;
}

void Game::Start()
{
    if (state_ != ExecutionState::Startup)
    {
#ifdef DEBUG_GAME
        LOG_DEBUG << "Game state is not Startup it is " << static_cast<int>(state_.load()) << std::endl;
#endif // DEBUG_GAME
        return;
    }

    auto* config = GetSubsystem<ConfigManager>();
    startTime_ = Utils::Tick();
    instanceData_.startTime = startTime_;
    instanceData_.serverUuid = Application::Instance->GetServerId();
    instanceData_.gameUuid = data_.uuid;
    instanceData_.name = map_->name_;
    LOG_INFO << "Starting game " << id_ << ", " << map_->name_ << std::endl;

    if ((*config)[ConfigManager::Key::RecordGames])
    {
        writeStream_ = std::make_unique<IO::GameWriteStream>();
        if (writeStream_->Open((*config)[ConfigManager::Key::RecordingsDir], this))
            instanceData_.recording = writeStream_->GetFilename();
    }
    instanceData_.running = true;
    CreateEntity(instanceData_);

    lastUpdate_ = 0;
    SetState(ExecutionState::Running);

    // Now that we are running we can spawn the queued players
    if (!queuedObjects_.empty())
    {
        auto it = queuedObjects_.begin();
        while (it != queuedObjects_.end())
        {
            SendSpawnObject((*it));
            it = queuedObjects_.erase(it);
        }
    }

    // Initial game update
    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(std::bind(&Game::Update, shared_from_this()))
    );
}

void Game::Update()
{
    // Dispatcher Thread
    if (state_ != ExecutionState::Terminated)
    {
        if (lastUpdate_ == 0)
        {
            noplayerTime_ = 0;
            Lua::CallFunction(luaState_, "onStart");
            // Add start tick at the beginning
            gameStatus_->AddByte(AB::GameProtocol::ServerPacketType::GameStart);
            AB::Packets::Server::GameStart packet = { startTime_ };
            AB::Packets::Add(packet, *gameStatus_);
        }

        int64_t tick = Utils::Tick();
        if (lastUpdate_ == 0)
            lastUpdate_ = tick - NETWORK_TICK;
        uint32_t delta = static_cast<uint32_t>(tick - lastUpdate_);
        lastUpdate_ = tick;

        {
            // Add timestamp
            gameStatus_->AddByte(AB::GameProtocol::ServerPacketType::GameUpdate);
            AB::Packets::Server::GameUpdate packet = { tick };
            AB::Packets::Add(packet, *gameStatus_);
        }

        // First Update all objects
        {
            // We need a copy of the objects because the iterator may become
            // invalid if objects are added.
            const auto localObjs = objects_;
            for (const auto& o : localObjs)
            {
                if (!o.second)
                    return;
                o.second->Update(delta, *gameStatus_);
            }
        }

        // Update Octree
        map_->UpdateOctree(delta);

        // Then call Lua Update function
        Lua::CallFunction(luaState_, "onUpdate", delta);

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
            LOG_INFO << "Shutting down game " << id_ << ", " << map_->name_ << " no players for " << noplayerTime_ << std::endl;
            SetState(ExecutionState::Terminated);
            Lua::CallFunction(luaState_, "onStop");
        }

        // Schedule next update
        const int64_t end = Utils::Tick();
        const uint32_t duration = static_cast<uint32_t>(end - lastUpdate_);
        const uint32_t sleepTime = NETWORK_TICK > duration ?
                    NETWORK_TICK - duration :
                    0;
        GetSubsystem<Asynch::Scheduler>()->Add(
            Asynch::CreateScheduledTask(sleepTime, std::bind(&Game::Update, shared_from_this()))
        );

        break;
    }
    case ExecutionState::Terminated:
        // Delete this game
        LOG_INFO << "Stopping game " << id_ << ", " << map_->name_ << std::endl;
        GetSubsystem<Asynch::Scheduler>()->Add(
            Asynch::CreateScheduledTask(500, std::bind(&GameManager::DeleteGameTask,
                GetSubsystem<GameManager>(), id_))
        );
        break;
    case ExecutionState::Startup:
        // Do nothing
        break;
    }
    Lua::CollectGarbage(luaState_);
}

void Game::SendStatus()
{
    // Must not be empty. Update adds at least the time stamp.
    assert(gameStatus_->GetSize() != 0);

    for (const auto& p : players_)
    {
        // Write to buffered, auto-sent output message
        p.second->WriteToOutput(*gameStatus_);
    }

    if (writeStream_ && writeStream_->IsOpen())
        writeStream_->Write(*gameStatus_);

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
    const uint32_t playerId = GetSubsystem<PlayerManager>()->GetPlayerIdByName(name);
    if (playerId != 0)
        return GetPlayerById(playerId);
    return nullptr;
}

void Game::AddObject(ea::shared_ptr<GameObject> object)
{
    AddObjectInternal(object);
    Lua::CallFunction(luaState_, "onAddObject", object.get());
}

void Game::AddObjectInternal(ea::shared_ptr<GameObject> object)
{
    objects_.emplace(object->id_, object);
    object->SetGame(shared_from_this());
}

void Game::InternalRemoveObject(GameObject* object)
{
    const auto it = objects_.find(object->id_);
    if (it == objects_.end())
        return;
    Lua::CallFunction(luaState_, "onRemoveObject", object);
    object->SetGame(ea::shared_ptr<Game>());
    if (it != objects_.end())
        objects_.erase(it);
}

ea::shared_ptr<Npc> Game::AddNpc(const std::string& script)
{
    ea::shared_ptr<Npc> result = ea::make_shared<Npc>();
    result->SetGame(shared_from_this());
    if (!result->LoadScript(script))
        return ea::shared_ptr<Npc>();

    // After all initialization is done, we can call this
    GetSubsystem<Asynch::Scheduler>()->Add(
        Asynch::CreateScheduledTask(std::bind(&Game::SendSpawnObject,
            shared_from_this(), result))
    );

    return result;
}

ea::shared_ptr<AreaOfEffect> Game::AddAreaOfEffect(const std::string& script,
    ea::shared_ptr<Actor> source,
    uint32_t index,
    const Math::Vector3& pos)
{
    ea::shared_ptr<AreaOfEffect> result = ea::make_shared<AreaOfEffect>();
    result->SetGame(shared_from_this());
    result->SetSource(source);
    result->SetIndex(index);
    result->transformation_.position_ = pos;
    if (Math::Equals(pos.y_, 0.0f))
        result->transformation_.position_.y_ = map_->GetTerrainHeight(pos);
    if (!result->LoadScript(script))
        return ea::shared_ptr<AreaOfEffect>();

    // After all initialization is done, we can call this
    GetSubsystem<Asynch::Scheduler>()->Add(
        Asynch::CreateScheduledTask(std::bind(&Game::SendSpawnObject,
            shared_from_this(), result))
    );

    return result;
}

void Game::AddProjectile(const std::string& itemUuid,
    ea::shared_ptr<Actor> source,
    ea::shared_ptr<Actor> target)
{
    ea::shared_ptr<Projectile> result = ea::make_shared<Projectile>(itemUuid);
    result->SetGame(shared_from_this());
    result->SetSource(source);
    result->SetTarget(target);
    // Speed is set by the script
    if (!result->Load())
        return;

    GetSubsystem<Asynch::Scheduler>()->Add(
        Asynch::CreateScheduledTask(std::bind(&Game::SendSpawnObject,
            shared_from_this(), result))
    );
}

ea::shared_ptr<ItemDrop> Game::AddRandomItemDropFor(Actor* dropper, Actor* target)
{
    if (state_ != ExecutionState::Running || !dropper)
        return ea::shared_ptr<ItemDrop>();
    if (AB::Entities::IsOutpost(data_.type))
        // No drops in outposts
        return ea::shared_ptr<ItemDrop>();

    if (target->GetType() != AB::GameProtocol::GameObjectType::Player)
        return ea::shared_ptr<ItemDrop>();

    Player* targetPlayer = To<Player>(target);
    auto* factory = GetSubsystem<ItemFactory>();
    auto* rng = GetSubsystem<Crypto::Random>();
    uint32_t itemId = factory->CreateDropItem(instanceData_.uuid, data_.uuid, dropper->GetLevel(), targetPlayer);
    if (itemId == 0)
        return ea::shared_ptr<ItemDrop>();

    ea::shared_ptr<ItemDrop> result = ea::make_shared<ItemDrop>(itemId);
    result->transformation_.position_ = dropper->transformation_.position_;
    // Random pos around dropper
    result->transformation_.position_.y_ += 0.2f;
    result->transformation_.position_.x_ += rng->Get<float>(-RANGE_TOUCH, RANGE_TOUCH);
    result->transformation_.position_.z_ += rng->Get<float>(-RANGE_TOUCH, RANGE_TOUCH);
    result->SetSource(dropper->GetPtr<Actor>());
    result->actorId_ = target->id_;

    SpawnItemDrop(result);
    return result;
}

ea::shared_ptr<ItemDrop> Game::AddRandomItemDrop(Actor* dropper)
{
    if (state_ != ExecutionState::Running || !dropper)
        return ea::shared_ptr<ItemDrop>();
    if (AB::Entities::IsOutpost(data_.type) || data_.type == AB::Entities::GameTypePvPCombat)
        // No drops in outposts and PvP games
        return ea::shared_ptr<ItemDrop>();

    auto* rng = GetSubsystem<Crypto::Random>();
    const float rnd = rng->GetFloat();
    auto p = sa::ea::SelectRandomly(players_.begin(), players_.end(), rnd);
    if (p == players_.end())
        return ea::shared_ptr<ItemDrop>();

    Player* target = (*p).second;
    if (!target)
        return ea::shared_ptr<ItemDrop>();
    return AddRandomItemDropFor(dropper, target);
}

void Game::SpawnItemDrop(ea::shared_ptr<ItemDrop> item)
{
    item->SetGame(shared_from_this());
    // Also adds it to the objects array
    SendSpawnObject(item);

    gameStatus_->AddByte(AB::GameProtocol::ServerPacketType::GameObjectDropItem);
    const Item* pItem = item->GetItem();
    AB::Packets::Server::ObjectDroppedItem packet = {
        item->GetSourceId(),
        item->actorId_,
        item->id_,
        item->GetItemIndex(),
        static_cast<uint32_t>(pItem ? pItem->concreteItem_.count : 1u),
        static_cast<uint16_t>(pItem ? pItem->concreteItem_.value : 0u)
    };
    AB::Packets::Add(packet, *gameStatus_);
}

std::vector<Party*> Game::_LuaGetParties() const
{
    auto* partyMngr = GetSubsystem<PartyManager>();
    return partyMngr->GetByGame(id_);
}

void Game::CallLuaEvent(const std::string& name, GameObject* sender, GameObject* data)
{
    if (Lua::IsFunction(luaState_, name))
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

    if (!IO::IOMap::Load(*map_))
    {
        LOG_ERROR << "Error loading map with name " << map_->name_ << std::endl;
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
    if (!IO::IOGame::LoadGameByUuid(*this, mapUuid))
    {
        LOG_ERROR << "Error loading game " << mapUuid << std::endl;
        return;
    }
    map_ = std::make_unique<Map>(shared_from_this());
    map_->name_ = data_.name;
    map_->directory_ = data_.directory;

    // Must be executed here because the player doesn't wait to fully load the game to join
    // Execute initialization code if any
    auto script = GetSubsystem<IO::DataProvider>()->GetAsset<Script>(data_.script);
    if (!script)
    {
        LOG_ERROR << "Unable to get script " << data_.script << std::endl;
        return;
    }
    if (!script->Execute(luaState_))
    {
        LOG_ERROR << "Error executing script " << data_.script << std::endl;
        return;
    }

    auto* thPool = GetSubsystem<Asynch::ThreadPool>();
    // Load Game Assets
    thPool->Enqueue(&Game::InternalLoad, shared_from_this());
    // Load item drop chances on this map
    auto* factory = GetSubsystem<ItemFactory>();
    thPool->Enqueue(&ItemFactory::LoadDropChances, factory, mapUuid);
}

void Game::SendSpawnObject(ea::shared_ptr<GameObject> object)
{
    AB::GameProtocol::GameObjectType objectType = object->GetType();
    if (objectType < AB::GameProtocol::GameObjectType::__SentToPlayer)
        return;
    if (objectType == AB::GameProtocol::GameObjectType::Player)
    {
        // Spawn points are loaded now
        const SpawnPoint p = map_->GetFreeSpawnPoint("Player");
#ifdef DEBUG_GAME
//        LOG_DEBUG << "Spawn point: " << p.group << "; Pos: " << p.position << std::endl;
#endif
        object->transformation_.position_ = p.position;
        object->transformation_.SetYRotation(p.rotation.EulerAngles().y_);
    }

    gameStatus_->AddByte(AB::GameProtocol::ServerPacketType::GameSpawnObject);
    object->WriteSpawnData(*gameStatus_);
    AddObject(object);
}

void Game::SendLeaveObject(uint32_t objectId)
{
    gameStatus_->AddByte(AB::GameProtocol::ServerPacketType::GameLeaveObject);
    AB::Packets::Server::ObjectDespawn packet = {
        objectId
    };
    AB::Packets::Add(packet, *gameStatus_);
}

void Game::SendInitStateToPlayer(Player& player)
{
    // Send all already existing objects to the player, excluding the player itself.
    // This is sent to all players when they enter a game.
    // Only called when the player enters a game. All spawns during the game are sent
    // when they happen.
    const auto write = [&player](Net::NetworkMessage& msg, GameObject& o)
    {
        if (o.GetType() < AB::GameProtocol::GameObjectType::__SentToPlayer)
            // No need to send terrain patch to client
            return;
        if (o.id_ == player.id_)
            // Don't send spawn of our self
            return;

        msg.AddByte(AB::GameProtocol::ServerPacketType::GameSpawnObjectExisting);
        o.WriteSpawnData(msg);
    };

    auto msg = Net::NetworkMessage::GetNew();
    for (const auto& o : objects_)
    {
        write(*msg, *o.second);
        // When there are many objects this may exceed the buffer size.
        if (msg->GetSpace() < 512)
        {
            player.WriteToOutput(*msg);
            msg = Net::NetworkMessage::GetNew();
        }
    }

    // Also send queued objects
    for (const auto& o : queuedObjects_)
    {
        write(*msg, *o);
        if (msg->GetSpace() < 512)
        {
            player.WriteToOutput(*msg);
            msg = Net::NetworkMessage::GetNew();
        }
    }

    if (msg->GetSize() != 0)
        player.WriteToOutput(*msg);
}

void Game::PlayerJoin(uint32_t playerId)
{
    ea::shared_ptr<Player> player = GetSubsystem<PlayerManager>()->GetPlayerById(playerId);
    if (!player)
        return;

    {
        std::scoped_lock lock(lock_);
        players_[player->id_] = player.get();
        if (AB::Entities::IsOutpost(data_.type))
            player->data_.lastOutpostUuid = data_.uuid;
        player->data_.instanceUuid = instanceData_.uuid;
    }
    UpdateEntity(player->data_);

    Lua::CallFunction(luaState_, "onPlayerJoin", player.get());
    SendInitStateToPlayer(*player);

    if (GetState() == ExecutionState::Running)
    {
        // In worst case (i.e. the game data is still loading): will be sent as
        // soon as the game runs and entered the Update loop.
        SendSpawnObject(player);
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

void Game::RemoveObject(GameObject* object)
{
    if (!object)
        return;
    auto it = objects_.find(object->id_);
    if (it == objects_.end())
        return;

    GetSubsystem<Asynch::Scheduler>()->Add(
        Asynch::CreateScheduledTask(std::bind(&Game::SendLeaveObject, shared_from_this(), object->id_))
    );
    InternalRemoveObject(object);
}

void Game::PlayerLeave(uint32_t playerId)
{
    Player* player = GetPlayerById(playerId);
    if (!player)
        return;

    std::scoped_lock lock(lock_);
    player->SetGame(ea::shared_ptr<Game>());
    auto it = players_.find(playerId);
    if (it != players_.end())
    {
        Lua::CallFunction(luaState_, "onPlayerLeave", player);
        players_.erase(it);
    }
    player->data_.instanceUuid = "";
    UpdateEntity(player->data_);

    auto* sched = GetSubsystem<Asynch::Scheduler>();
    sched->Add(
        Asynch::CreateScheduledTask(std::bind(&Game::SendLeaveObject, shared_from_this(), playerId))
    );
    // Notify other servers that a player left, e.g. for friend list
    sched->Add(
        Asynch::CreateScheduledTask(std::bind(&Game::BroadcastPlayerLoggedOut,
            shared_from_this(),
            player->GetPtr<Player>()))
    );
    InternalRemoveObject(player);
}

Crowd* Game::AddCrowd()
{
    ea::unique_ptr<Crowd> crowd = ea::make_unique<Crowd>();
    Crowd* result = crowd.get();
    crowds_.emplace(crowd->GetId(), ea::move(crowd));
    return result;
}

Group* Game::GetGroup(uint32_t id)
{
    const auto it = crowds_.find(id);
    if (it != crowds_.end())
        return (*it).second.get();
    auto pMngr = GetSubsystem<PartyManager>();
    auto party = pMngr->Get(id);
    if (party)
        return party.get();
    return nullptr;
}

}

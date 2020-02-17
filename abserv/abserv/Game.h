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

#pragma once

#include "Chat.h"
#include "Config.h"
#include "GameObject.h"
#include "GameStream.h"
#include "Map.h"
#include "NavigationMesh.h"
#include "PartyManager.h"
#include "Script.h"
#include <AB/Entities/Game.h>
#include <AB/Entities/GameInstance.h>
#include <CleanupNs.h>
#include <abscommon/NetworkMessage.h>
#include <atomic>
#include <mutex>
#include <sa/Iteration.h>

namespace Game {

class Player;
class Npc;
class AreaOfEffect;
class ItemDrop;
class Projectile;
class Crowd;

/// The list which owns the objects. We use a std::map because we want to
/// have it in the order of creation (allocation) when Update() is called.
using ObjectList = std::map<uint32_t, std::shared_ptr<GameObject>>;
using PlayersList = std::unordered_map<uint32_t, Player*>;
using CrowdList = std::unordered_map<uint32_t, std::unique_ptr<Crowd>>;

class Game : public std::enable_shared_from_this<Game>
{
public:
    enum class ExecutionState
    {
        Startup,
        Running,
        Shutdown,
        Terminated
    };
private:
    std::mutex lock_;
    std::atomic<ExecutionState> state_{ ExecutionState::Terminated };           // Just changed when starting/stopping a game
    int64_t lastUpdate_{ 0 };
    uint32_t noplayerTime_{ 0 };
    /// The primary owner of the game objects
    ObjectList objects_;
    PlayersList players_;
    CrowdList crowds_;
    kaguya::State luaState_;
    std::shared_ptr<Script> script_;
    /// First player(s) triggering the creation of this game
    std::vector<std::shared_ptr<GameObject>> queuedObjects_;
    void InitializeLua();
    void InternalLoad();
    void Update();
    void SendStatus();
    void ResetStatus();
    /// Changes to the game are written to this message and sent to all players
    std::unique_ptr<Net::NetworkMessage> gameStatus_;
    /// Stream to record games
    std::unique_ptr<IO::GameWriteStream> writeStream_;
    template<typename E>
    bool UpdateEntity(const E& e)
    {
        IO::DataClient* cli = GetSubsystem<IO::DataClient>();
        return cli->Update(e);
    }
    template<typename E>
    bool DeleteEntity(const E& e)
    {
        IO::DataClient* cli = GetSubsystem<IO::DataClient>();
        return cli->Delete(e);
    }
    template<typename E>
    bool CreateEntity(const E& e)
    {
        IO::DataClient* cli = GetSubsystem<IO::DataClient>();
        return cli->Create(e);
    }
    float _LuaGetTerrainHeight(float x, float z) const
    {
        if (map_)
            return map_->terrain_->GetHeight({ x, 0.0f, z });
        return 0.0f;
    }
    int64_t _LuaGetStartTime() const
    {
        return startTime_;
    }
    GameObject* _LuaGetObjectById(uint32_t objectId);
    /// Return all Parties in this Game
    std::vector<Party*> _LuaGetParties() const;
    Npc* _LuaAddNpc(const std::string& script);
    AreaOfEffect* _LuaAddAreaOfEffect(const std::string& script,
        Actor* source,
        uint32_t index,
        const Math::STLVector3& pos);
    void _LuaAddProjectile(const std::string& itemUuid,
        Actor* source,
        Actor* target);
    ItemDrop* _LuaAddItemDrop(Actor* dropper);
    int _LuaGetType() const { return data_.type; }
    void BroadcastPlayerLoggedIn(std::shared_ptr<Player> player);
    void BroadcastPlayerLoggedOut(std::shared_ptr<Player> player);
    void InternalRemoveObject(GameObject* object);
    void SendSpawnObject(std::shared_ptr<GameObject> object);
    void SendLeaveObject(uint32_t objectId);
    /// Send spawn message for all existing objects
    void SendInitStateToPlayer(Player& player);
public:
    static void RegisterLua(kaguya::State& state);

    Game();
    ~Game();

    void Start();

    /// Auto generated ID used by the GameManager
    uint32_t id_{ 0 };
    int64_t startTime_{ 0 };
    AB::Entities::Game data_;
    AB::Entities::GameInstance instanceData_;

    std::unique_ptr<Map> map_;

    uint32_t GetPlayerCount() const { return static_cast<uint32_t>(players_.size()); }
    int64_t GetInstanceTime() const { return Utils::TimeElapsed(startTime_); }
    std::string GetName() const { return map_->data_.name; }
    /// Default level on this map
    uint32_t GetDefaultLevel() const { return static_cast<uint32_t>(data_.defaultLevel); }
    /// Returns only players that are part of this game
    Player* GetPlayerById(uint32_t playerId);
    Player* GetPlayerByName(const std::string& name);
    const ObjectList& GetObjects() const { return objects_; }
    template <typename T>
    T* GetObject(uint32_t id)
    {
        auto it = objects_.find(id);
        if (it == objects_.end())
            return nullptr;
        GameObject* o = (*it).second.get();
        if (Is<T>(o))
            return To<T>(o);
        return nullptr;
    }
    void AddObject(std::shared_ptr<GameObject> object);
    void AddObjectInternal(std::shared_ptr<GameObject> object);
    Group* GetGroup(uint32_t id);
    Crowd* AddCrowd();

    std::shared_ptr<Npc> AddNpc(const std::string& script);
    std::shared_ptr<AreaOfEffect> AddAreaOfEffect(const std::string& script,
        std::shared_ptr<Actor> source,
        uint32_t index,
        const Math::Vector3& pos);
    void AddProjectile(const std::string& itemUuid,
        std::shared_ptr<Actor> source,
        std::shared_ptr<Actor> target);
    std::shared_ptr<ItemDrop> AddRandomItemDrop(Actor* dropper);
    std::shared_ptr<ItemDrop> AddRandomItemDropFor(Actor* dropper, Actor* target);
    void SpawnItemDrop(std::shared_ptr<ItemDrop> item);

    ExecutionState GetState() const { return state_; }
    Net::NetworkMessage& GetGameStatus()
    {
        assert(gameStatus_);
        return *gameStatus_;
    }
    bool IsInactive() const
    {
        if (state_ == ExecutionState::Startup)
            return false;
        return noplayerTime_ > GAME_INACTIVE_TIME;
    }
    void CallLuaEvent(const std::string& name, GameObject* sender, GameObject* data);
    void SetState(ExecutionState state);
    void Load(const std::string& mapUuid);

    void RemoveObject(GameObject* object);
    void BroadcastPlayerChanged(const Player& player, uint32_t fields);

    /// From GameProtocol (Dispatcher Thread)
    void PlayerJoin(uint32_t playerId);
    void PlayerLeave(uint32_t playerId);

    template<typename O = GameObject, typename Callback>
    inline void VisitObjects(const Callback& callback)
    {
        for (auto& object : objects_)
        {
            if (object.second && Is<O>(*object.second))
            {
                if (callback(To<O>(*object.second)) != Iteration::Continue)
                    break;
            }
        }
    }
    template<typename O = GameObject, typename Callback>
    inline void VisitObjects(const Callback& callback) const
    {
        for (auto& object : objects_)
        {
            if (object.second && Is<O>(*object.second))
            {
                if (callback(To<O>(*object.second)) != Iteration::Continue)
                    break;
            }
        }
    }
    template<typename Callback>
    inline void VisitPlayers(const Callback& callback)
    {
        for (auto& player : players_)
        {
            if (callback(player.second) != Iteration::Continue)
                break;
        }
    }
    template <typename Callback>
    void VisitParties(const Callback& callback)
    {
        auto* partyMngr = GetSubsystem<PartyManager>();
        partyMngr->VisitGameParties(id_, callback);
    }
};

}

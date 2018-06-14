#pragma once

#include "Map.h"
#include "GameObject.h"
#include "NavigationMesh.h"
#include "Utils.h"
#include "NetworkMessage.h"
#include "Chat.h"
#include <AB/Entities/Game.h>
#include <AB/Entities/GameInstance.h>

namespace Game {

class Player;
class Npc;

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
    std::recursive_mutex lock_;
    ExecutionState state_;
    /// Game Tick -> Game State
    std::vector<std::shared_ptr<GameObject>> objects_;
    std::map<uint32_t, Player*> players_;
    int64_t lastUpdate_;
    kaguya::State luaState_;
    /// First player(s) triggering the creation of this game
    std::vector<std::shared_ptr<Player>> queuedPlayers_;
    void InitializeLua();
    void InternalLoad();
    void Update();
    void SendStatus();
    void ResetStatus();
    /// Changes to the game are written to this message and sent to all players
    std::shared_ptr<Net::NetworkMessage> gameStatus_;
    template<typename E>
    bool UpdateEntity(const E& e)
    {
        IO::DataClient* cli = Application::Instance->GetDataClient();
        return cli->Update(e);
    }
    template<typename E>
    bool DeleteEntity(const E& e)
    {
        IO::DataClient* cli = Application::Instance->GetDataClient();
        return cli->Delete(e);
    }
    template<typename E>
    bool CreateEntity(const E& e)
    {
        IO::DataClient* cli = Application::Instance->GetDataClient();
        return cli->Create(e);
    }
public:
    static void RegisterLua(kaguya::State& state);

    Game();
    ~Game();

    void Start();

    AB::Entities::Game data_;
    AB::Entities::GameInstance instanceData_;
    /// Auto generated ID used by the GameManager
    uint32_t id_;
    int64_t startTime_;

    std::shared_ptr<Map> map_;

    uint32_t GetPlayerCount() const { return static_cast<uint32_t>(players_.size()); }
    const std::map<uint32_t, Player*>& GetPlayers() const { return players_; }
    int64_t GetInstanceTime() const { return Utils::AbTick() - startTime_; }
    std::string GetName() const { return map_->data_.name; }
    /// Returns only players that are part of this game
    Player* GetPlayerById(uint32_t playerId);
    Player* GetPlayerByName(const std::string& name);
    std::shared_ptr<GameObject> GetObjectById(uint32_t objectId);
    void AddObject(std::shared_ptr<GameObject> object);
    void AddObjectInternal(std::shared_ptr<GameObject> object);
    void RemoveObject(std::shared_ptr<GameObject> object);

    std::shared_ptr<Npc> AddNpc(const std::string& script);

    ExecutionState GetState() const { return state_; }
    const kaguya::State& GetLuaState() const { return luaState_; }
    void SetState(ExecutionState state);
    void Load(const std::string& mapUuid);
    /// Send spawn message for all existing objects
    void SendSpawnAll(uint32_t playerId);
    void QueueSpawnObject(std::shared_ptr<GameObject> object);
    void QueueLeaveObject(uint32_t objectId);

    /// From GameProtocol (Dispatcher Thread)
    void PlayerJoin(uint32_t playerId);
    void PlayerLeave(uint32_t playerId);
};

}

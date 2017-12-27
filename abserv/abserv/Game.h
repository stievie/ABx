#pragma once

#include "Map.h"
#include "GameObject.h"
#include "NavigationMesh.h"
#include "Utils.h"
#include "NetworkMessage.h"
#include "GameState.h"

namespace Game {

class Player;
class Npc;

enum GameType
{
    GameTypeOutpost = 1,
    GameTypePvPCombat,
    GameTypeExploreable,
    GameTypeMission,
};

struct GameData
{
    uint32_t id;
    GameType type;
    std::string mapName;
    std::string mapFile;
    std::string navMeshFile;
    std::string scriptFile;
};

class Game : public std::enable_shared_from_this<Game>
{
public:
    enum GameLoopState
    {
        GameStateStartup,
        GameStateRunning,
        GameStateShutdown,
        GameStateTerminated
    };
private:
    std::recursive_mutex lock_;
    GameLoopState state_;
    /// Game Tick -> Game State
    std::map<int64_t, std::unique_ptr<GameState>> gameStates_;
    std::vector<std::shared_ptr<GameObject>> objects_;
    std::map<uint32_t, Player*> players_;
    int64_t lastUpdate_;
    kaguya::State luaState_;
    void InitializeLua();
    void InternalLoad();
    void Update();
    void SendStatus();
    void ResetStatus();
    /// Changes to the game are written to this message and sent to all players
    std::shared_ptr<Net::NetworkMessage> gameStatus_;
    Math::Vector3 GetSpawnPoint();
public:
    static void RegisterLua(kaguya::State& state);

    Game();
    ~Game()
    {
        players_.clear();
        objects_.clear();
    }

    void Start();
    void Stop();

    GameData data_;
    /// Auto generated ID used by the GameManager
    uint32_t id_;
    int64_t startTime_;

    std::shared_ptr<Map> map_;
    std::shared_ptr<NavigationMesh> navMesh_;

    uint32_t GetPlayerCount() const { return static_cast<uint32_t>(players_.size()); }
    int64_t GetInstanceTime() const { return Utils::AbTick() - startTime_; }
    std::string GetName() const { return data_.mapName; }
    /// Returns only players that are part of this game
    Player* GetPlayerById(uint32_t playerId);
    Player* GetPlayerByName(const std::string& name);
    std::shared_ptr<GameObject> GetObjectById(uint32_t objectId);

    std::shared_ptr<Npc> AddNpc(const std::string& script);

    GameLoopState GetState() const { return state_; }
    const kaguya::State& GetLuaState() const { return luaState_; }
    void SetState(GameLoopState state);
    void Load(const std::string& mapName);
    /// Send spawn message for all existing objects
    void SendSpawnAll(uint32_t playerId);
    void QueueSpawnObject(std::shared_ptr<GameObject> object);
    void QueueLeaveObject(uint32_t objectId);

    /// From GameProtocol (Dispatcher Thread)
    void PlayerJoin(uint32_t playerId);
    void PlayerLeave(uint32_t playerId);

};

}

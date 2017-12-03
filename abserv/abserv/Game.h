#pragma once

#include <stdint.h>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include "Map.h"
#include "GameObject.h"
#include "NavigationMesh.h"
#include "Utils.h"
#include <mutex>
#pragma warning(push)
#pragma warning(disable: 4702 4127 4244)
#include <kaguya/kaguya.hpp>
#pragma warning(pop)

namespace Game {

class Player;
class Npc;

/// Direction relative to current rotation
enum MoveDirection
{
    MoveDirectionNorth = 0,
    MoveDirectionNorthWest = 45,
    MoveDirectionWest = 90,
    MoveDirectionSouthWest = 135,
    MoveDirectionSouth = 180,
    MoveDirectionSouthEast = 225,
    MoveDirectionEast = 270,
    MoveDirectionNorthEast = 315,
};

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
    enum GameState
    {
        GameStateStartup,
        GameStateRunning,
        GameStateShutdown,
        GameStateTerminated
    };
private:
    std::recursive_mutex lock_;
    GameState state_;
    std::vector<std::shared_ptr<GameObject>> objects_;
    std::map<uint32_t, Player*> players_;
    int64_t lastUpdate_;
    kaguya::State luaState_;
    void InitializeLua();
    /// Returns only players that are part of this game
    Player* GetPlayerById(uint32_t playerId);
    Player* GetPlayerByName(const std::string& name);
    GameObject* GetObjectById(uint32_t objectId);
    void InternalLoad();
    void Update();
    void SendStatus();
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

    std::shared_ptr<Npc> AddNpc(const std::string& script);

    GameState GetState() const { return state_; }
    const kaguya::State& GetLuaState() const { return luaState_; }
    void SetState(GameState state);
    void Load(const std::string& mapName);


    /// From GameProtocol (Dispatcher Thread)
    void Ping(uint32_t playerId);
    void PlayerJoin(uint32_t playerId);
    void PlayerLeave(uint32_t playerId);
    void PlayerMove(uint32_t playerId, MoveDirection direction);

};

}

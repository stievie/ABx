#pragma once

#include <stdint.h>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include "Map.h"
#include "GameObject.h"
#include "Utils.h"
#include <mutex>

namespace Game {

class Player;

enum MoveDirection
{
    MoveDirectionNorth,
    MoveDirectionNorthEast,
    MoveDirectionEast,
    MoveDirectionSouthEast,
    MoveDirectionSouth,
    MoveDirectionSouthWest,
    MoveDirectionWest,
    MoveDirectionNorthWest,
};

struct GameData
{
    std::string mapName;
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
    GameData data_;
    /// Returns only players that are part of this game
    Player* GetPlayerById(uint32_t playerId);
    void InternalLoad();
    void Update();
public:
    Game();
    ~Game() {}

    void Start();
    void Stop();

    /// Auto generated ID used by the GameManager
    uint32_t id_;
    int64_t startTime_;

    std::shared_ptr<Map> map_;

    uint32_t GetPlayerCount() const { return static_cast<uint32_t>(players_.size()); }
    int64_t GetInstanceTime() const { return Utils::AbTick() - startTime_; }
    GameState GetState() const { return state_; }
    void SetState(GameState state);
    void Load(const std::string& mapName);

    /// From GameProtocol (Dispatcher Thread)
    void PlayerJoin(uint32_t playerId);
    void PlayerLeave(uint32_t playerId);
    void PlayerMove(uint32_t playerId, MoveDirection direction);

};

}

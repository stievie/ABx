#pragma once

#include <stdint.h>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include "Map.h"
#include "GameObject.h"
#include "Utils.h"

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

class Game : public std::enable_shared_from_this<Game>
{
private:
    std::vector<std::shared_ptr<GameObject>> objets_;
    std::map<uint32_t, Player*> players_;
    /// Returns only players that are part of this game
    Player* GetPlayerById(uint32_t playerId);
public:
    Game();
    ~Game() {}

    void Update(uint32_t timeElapsed);

    /// Auto generated ID used by the GameManager
    uint32_t id_;
    int64_t startTime_;

    std::shared_ptr<Map> map_;

    uint32_t GetPlayerCount() const { return static_cast<uint32_t>(players_.size()); }
    int64_t GetInstanceTime() const { return Utils::AbTick() - startTime_; }

    /// From GameProtocol
    void PlayerJoin(uint32_t playerId);
    void PlayerMove(uint32_t playerId, MoveDirection direction);

};

}

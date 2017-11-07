#pragma once

#include <stdint.h>
#include <memory>
#include <string>
#include <vector>
#include "Map.h"
#include "GameObject.h"

namespace Game {

class Player;

class Game : public std::enable_shared_from_this<Game>
{
private:
    std::vector<std::shared_ptr<GameObject>> objets_;
    std::vector<Player*> players_;
public:
    Game();
    ~Game();

    void Update(uint32_t timeElapsed);

    /// Auto generated ID used by the GameManager
    uint32_t id_;
    int64_t startTime_;

    std::shared_ptr<Map> map_;

    uint32_t GetPlayerCount() const { return static_cast<uint32_t>(players_.size()); }
};

}

#pragma once

#include <stdint.h>
#include <memory>
#include <string>
#include "Map.h"

namespace Game {

/// Database game data
struct GameData
{
    /// DB ID
    uint32_t id;
    std::string mapName;
};

class Game : public std::enable_shared_from_this<Game>
{
public:
    Game();
    ~Game();

    void Update(uint32_t timeElapsed);

    GameData data_;
    /// Auto generated ID
    uint32_t id_;

    std::shared_ptr<Map> map_;
};

}

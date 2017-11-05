#pragma once

#include "Player.h"
#include <string>
#include "Database.h"

namespace DB {

class IOPlayer
{
private:
    static bool LoadPlayer(Game::Player* player, std::shared_ptr<DBResult> result);
public:
    IOPlayer() = delete;

    static bool PreloadPlayer(Game::Player* player, const std::string& name);
    static bool LoadPlayerByName(Game::Player* player, const std::string& name);
    static bool LoadPlayerById(Game::Player* player, uint32_t playerId);

};

}

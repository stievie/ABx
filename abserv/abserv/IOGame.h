#pragma once

#include <AB/Entities/Game.h>
#include "Database.h"
#include "Game.h"

namespace IO {

class IOGame
{
public:
    IOGame() = delete;
    static bool LoadGame(AB::Entities::Game& game);
    static bool LoadGameByName(Game::Game* game, const std::string& name);
    static bool LoadGameByUuid(Game::Game* game, const std::string& uuid);
    static std::string GetLandingGame();
    static AB::Entities::GameType GetGameType(const std::string& mapName);
    static std::vector<AB::Entities::Game> GetGameList();
};

}

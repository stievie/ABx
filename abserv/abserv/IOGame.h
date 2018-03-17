#pragma once

#include <AB/GameData.h>
#include "Database.h"
#include "Game.h"

namespace DB {

class IOGame
{
private:
    static bool LoadGame(Game::Game* game, std::shared_ptr<DBResult> result);
public:
    IOGame() = delete;
    static bool LoadGameByName(Game::Game* game, const std::string& name);
    static bool LoadGameById(Game::Game* game, uint32_t gameId);
    static std::string GetLandingGame();
    static AB::Data::GameType GetGameType(const std::string& mapName);
    static std::vector<AB::Data::GameData> GetGameList(AB::Data::GameType type);
};

}

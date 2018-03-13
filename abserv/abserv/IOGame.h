#pragma once

#include "Game.h"
#include "Database.h"

namespace DB {

struct GameEntity
{
    uint32_t id;
    std::string name;
    Game::GameType type;
};

class IOGame
{
private:
    static bool LoadGame(Game::Game* game, std::shared_ptr<DBResult> result);
public:
    IOGame() = delete;
    static bool LoadGameByName(Game::Game* game, const std::string& name);
    static bool LoadGameById(Game::Game* game, uint32_t gameId);
    static std::string GetLandingGame();
    static Game::GameType GetGameType(const std::string& mapName);
    static std::vector<GameEntity> GetGameList();
};

}

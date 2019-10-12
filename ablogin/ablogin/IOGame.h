#pragma once

#include <AB/Entities/Game.h>
#include <set>

namespace IO {

class IOGame
{
public:
    IOGame() = delete;
    static std::string GetLandingGameUuid();
    static AB::Entities::GameType GetGameType(const std::string& mapName);
    /// Get a list of games of a certain type. If types is empty it returns all games.
    static std::vector<AB::Entities::Game> GetGameList(std::set<AB::Entities::GameType> types);
};

}

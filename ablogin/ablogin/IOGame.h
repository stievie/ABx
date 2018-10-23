#pragma once

#include <AB/Entities/Game.h>

namespace IO {

class IOGame
{
public:
    IOGame() = delete;
    static std::string GetLandingGameUuid();
    static AB::Entities::GameType GetGameType(const std::string& mapName);
    /// Get a list of games of a certain type. If type == AB::Entities::GameType::GameTypeUnknown it returns all games.
    static std::vector<AB::Entities::Game> GetGameList(AB::Entities::GameType type);
};

}

#pragma once

#include <AB/Entities/Game.h>

namespace IO {

class IOGame
{
public:
    IOGame() = delete;
    static std::string GetLandingGameUuid();
    static AB::Entities::GameType GetGameType(const std::string& mapName);
    static std::vector<AB::Entities::Game> GetGameList();
};

}

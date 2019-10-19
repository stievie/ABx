#pragma once

#include <AB/Entities/Game.h>
#include "Game.h"

namespace IO {
namespace IOGame {

bool LoadGame(AB::Entities::Game& game);
bool LoadGameByName(Game::Game& game, const std::string& name);
bool LoadGameByUuid(Game::Game& game, const std::string& uuid);
std::string GetLandingGameUuid();
AB::Entities::GameType GetGameType(const std::string& mapUuid);
std::vector<AB::Entities::Game> GetGameList();
std::string GetGameUuidFromName(const std::string& name);

}
}

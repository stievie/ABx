#pragma once

#include <AB/Entities/Game.h>
#include "Game.h"

namespace IO {

bool IOGame_LoadGame(AB::Entities::Game& game);
bool IOGame_LoadGameByName(Game::Game& game, const std::string& name);
bool IOGame_LoadGameByUuid(Game::Game& game, const std::string& uuid);
std::string IOGame_GetLandingGameUuid();
AB::Entities::GameType IOGame_GetGameType(const std::string& mapUuid);
std::vector<AB::Entities::Game> IOGame_GetGameList();
std::string IOGame_GetGameUuidFromName(const std::string& name);

}

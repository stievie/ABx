#pragma once

#include "Player.h"
#include <AB/Entities/Character.h>

namespace IO {

bool IOPlayer_LoadCharacter(AB::Entities::Character& ch);
bool IOPlayer_LoadPlayerByName(Game::Player& player, const std::string& name);
bool IOPlayer_LoadPlayerByUuid(Game::Player& player, const std::string& uuid);
bool IOPlayer_SavePlayer(Game::Player& player);
size_t IOPlayer_GetInterestedParties(const std::string& accountUuid, std::vector<std::string>& accounts);

}

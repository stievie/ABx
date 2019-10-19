#pragma once

#include "Player.h"
#include <AB/Entities/Character.h>
#include <AB/Entities/FriendList.h>

namespace IO {
namespace IOPlayer {

bool LoadCharacter(AB::Entities::Character& ch);
bool LoadPlayerByName(Game::Player& player, const std::string& name);
bool LoadPlayerByUuid(Game::Player& player, const std::string& uuid);
bool SavePlayer(Game::Player& player);
size_t GetInterestedParties(const std::string& accountUuid, std::vector<std::string>& accounts);
bool GetPlayerInfoByName(const std::string& name, AB::Entities::Character& player);
bool GetPlayerInfoByAccount(const std::string& accountUuid, AB::Entities::Character& player);
/// Check if `uuid` is ignoring `meUuid`
bool IsIgnoringMe(const std::string& meUuid, const std::string& uuid);
bool HasFriendedMe(const std::string& meUuid, const std::string& uuid);
AB::Entities::FriendRelation GetRelationToMe(const std::string& meUuid, const std::string& uuid);

}
}

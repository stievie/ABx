/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "Player.h"
#include <AB/Entities/Character.h>
#include <AB/Entities/FriendList.h>
#include <sa/Assert.h>

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

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

#include "stdafx.h"
#include "PlayerManager.h"
#include "Player.h"
#include <abscommon/StringUtils.h>

namespace Game {

ea::shared_ptr<Player> PlayerManager::GetPlayerByName(const std::string& name)
{
    return GetPlayerById(GetPlayerIdByName(name));
}

ea::shared_ptr<Player> PlayerManager::GetPlayerByUuid(const std::string& uuid)
{
    auto& index = playerIndex_.get<PlayerUuidIndexTag>();
    const auto accountIt = index.find(uuid);
    if (accountIt == index.end())
        return ea::shared_ptr<Player>();
    return GetPlayerById((*accountIt).id);
}

ea::shared_ptr<Player> PlayerManager::GetPlayerById(uint32_t id)
{
    auto it = players_.find(id);
    if (it != players_.end())
        return (*it).second;

    return ea::shared_ptr<Player>();
}

ea::shared_ptr<Player> PlayerManager::GetPlayerByAccountUuid(const std::string& uuid)
{
    auto& index = playerIndex_.get<AccountUuidIndexTag>();
    const auto accountIt = index.find(uuid);
    if (accountIt == index.end())
        return ea::shared_ptr<Player>();
    return GetPlayerById((*accountIt).id);
}

uint32_t PlayerManager::GetPlayerIdByName(const std::string& name)
{
    auto& index = playerIndex_.get<PlayerNameIndexTag>();
    // Player names are case insensitive
    const auto accountIt = index.find(Utils::Utf8ToLower(name));
    if (accountIt == index.end())
        return 0;
    return (*accountIt).id;
}

ea::shared_ptr<Player> PlayerManager::CreatePlayer(std::shared_ptr<Net::ProtocolGame> client)
{
    ea::shared_ptr<Player> result = ea::make_shared<Player>(client);
    players_[result->id_] = result;

    return result;
}

void PlayerManager::UpdatePlayerIndex(const Player& player)
{
    playerIndex_.insert({
        player.id_,
        player.data_.uuid,
        Utils::Utf8ToLower(player.GetName()),
        player.account_.uuid
    });
}

void PlayerManager::RemovePlayer(uint32_t playerId)
{
    auto it = players_.find(playerId);
    if (it != players_.end())
    {
        auto& idIndex = playerIndex_.get<IdIndexTag>();
        auto indexIt = idIndex.find((*it).second->id_);
        if (indexIt != idIndex.end())
            idIndex.erase(indexIt);

        players_.erase(it);

        if (players_.size() == 0)
            idleTime_ = Utils::Tick();
    }
}

void PlayerManager::CleanPlayers()
{
    if (players_.size() == 0)
        return;

    // Logout all inactive players
    auto i = players_.begin();
    while ((i = ea::find_if(i, players_.end(), [](const auto& current) -> bool
    {
        // Disconnect after 10sec
        return current.second->GetInactiveTime() > PLAYER_INACTIVE_TIME_KICK;
    })) != players_.end())
    {
        ea::shared_ptr<Player> p = (*i).second;
        ++i;
        // Calls PlayerManager::RemovePlayer()
        p->PartyLeave();
        p->Logout();
   }
}

void PlayerManager::RefreshAuthTokens()
{
    // No inactive players here
    auto* client = GetSubsystem<IO::DataClient>();
    int64_t tick = Utils::Tick();
    for (const auto& player : players_)
    {
        if (tick - player.second->account_.authTokenExpiry < Auth::AUTH_TOKEN_EXPIRES_IN / 2)
        {
            player.second->account_.authTokenExpiry = tick + Auth::AUTH_TOKEN_EXPIRES_IN;
            client->Update(player.second->account_);
        }
    }
}

void PlayerManager::KickPlayer(uint32_t playerId)
{
    auto it = players_.find(playerId);
    if (it != players_.end())
    {
        ea::shared_ptr<Player> p = (*it).second;
        p->PartyLeave();
        p->Logout();
    }
}

void PlayerManager::KickAllPlayers()
{
    while (!players_.empty())
    {
        auto it = players_.begin();
        ea::shared_ptr<Player> p = (*it).second;
        p->PartyLeave();
        p->Logout();
    }
}

void PlayerManager::BroadcastNetMessage(const Net::NetworkMessage& msg)
{
    for (const auto& p : players_)
    {
        p.second->WriteToOutput(msg);
    }
}

}

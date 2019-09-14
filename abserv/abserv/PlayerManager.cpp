#include "stdafx.h"
#include "PlayerManager.h"
#include "Player.h"
#include "StringUtils.h"
#include "UuidUtils.h"

namespace Game {

std::shared_ptr<Player> PlayerManager::GetPlayerByName(const std::string& name)
{
    return GetPlayerById(GetPlayerIdByName(name));
}

std::shared_ptr<Player> PlayerManager::GetPlayerByUuid(const std::string& uuid)
{
    auto& index = playerIndex_.get<PlayerUuidIndexTag>();
    const auto accountIt = index.find(uuid);
    if (accountIt == index.end())
        return std::shared_ptr<Player>();
    return GetPlayerById((*accountIt).id);
}

std::shared_ptr<Player> PlayerManager::GetPlayerById(uint32_t id)
{
    auto it = players_.find(id);
    if (it != players_.end())
        return (*it).second;

    return std::shared_ptr<Player>();
}

std::shared_ptr<Player> PlayerManager::GetPlayerByAccountUuid(const std::string& uuid)
{
    auto& index = playerIndex_.get<AccountUuidIndexTag>();
    const auto accountIt = index.find(uuid);
    if (accountIt == index.end())
        return std::shared_ptr<Player>();
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

std::shared_ptr<Player> PlayerManager::CreatePlayer(std::shared_ptr<Net::ProtocolGame> client)
{
    std::shared_ptr<Player> result = std::make_shared<Player>(client);
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
    while ((i = std::find_if(i, players_.end(), [](const auto& current) -> bool
    {
        // Disconnect after 10sec
        return current.second->GetInactiveTime() > PLAYER_INACTIVE_TIME_KICK;
    })) != players_.end())
    {
        std::shared_ptr<Player> p = (*i).second;
        ++i;
        // Calls PlayerManager::RemovePlayer()
        p->PartyLeave();
        p->Logout();
   }
}

void PlayerManager::KickPlayer(uint32_t playerId)
{
    auto it = players_.find(playerId);
    if (it != players_.end())
    {
        std::shared_ptr<Player> p = (*it).second;
        p->PartyLeave();
        p->Logout();
    }
}

void PlayerManager::KickAllPlayers()
{
    while (!players_.empty())
    {
        auto it = players_.begin();
        std::shared_ptr<Player> p = (*it).second;
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

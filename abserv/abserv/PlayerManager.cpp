#include "stdafx.h"
#include "PlayerManager.h"
#include "Player.h"

#include "DebugNew.h"

namespace Game {

PlayerManager PlayerManager::Instance;

std::shared_ptr<Player> PlayerManager::GetPlayerByName(const std::string& name)
{
    auto it = std::find_if(players_.begin(), players_.end(), [&](const std::pair<uint32_t, std::shared_ptr<Player>>& current)
    {
        return current.second->data_.name.compare(name) == 0;
    });
    if (it != players_.end())
        return (*it).second;

    return std::shared_ptr<Player>();
}

std::shared_ptr<Player> PlayerManager::GetPlayerByUuid(const std::string& uuid)
{
    auto it = playerUuids_.find(uuid);
    if (it != playerUuids_.end())
        return (*it).second->GetThis();
    return std::shared_ptr<Player>();
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
    auto it = std::find_if(players_.begin(), players_.end(), [&](const std::pair<uint32_t, std::shared_ptr<Player>>& current)
    {
        return current.second->data_.accountUuid.compare(uuid) == 0;
    });
    if (it != players_.end())
        return (*it).second;

    return std::shared_ptr<Player>();
}

uint32_t PlayerManager::GetPlayerIdByName(const std::string& name)
{
    auto it = std::find_if(players_.begin(), players_.end(), [&](const std::pair<uint32_t, std::shared_ptr<Player>>& current)
    {
        return current.second->data_.name.compare(name) == 0;
    });
    if (it != players_.end())
        return (*it).first;
    return 0;
}

std::shared_ptr<Player> PlayerManager::CreatePlayer(const std::string& playerUuid,
    std::shared_ptr<Net::ProtocolGame> client)
{
    std::shared_ptr<Player> result = std::make_shared<Player>(client);

    {
        std::lock_guard<std::recursive_mutex> lockClass(lock_);
        players_[result->id_] = result;
        playerUuids_[playerUuid] = result.get();
    }

    return result;
}

void PlayerManager::RemovePlayer(uint32_t playerId)
{
    auto it = players_.find(playerId);
    if (it != players_.end())
    {
        const std::string& uuid = (*it).second->data_.uuid;
        std::lock_guard<std::recursive_mutex> lockClass(lock_);
        playerUuids_.erase(uuid);
        players_.erase(it);
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
        i++;
//        std::string name = p->GetName();
        // Calls PlayerManager::RemovePlayer()
        p->Logout();
   }
}

void PlayerManager::KickPlayer(uint32_t playerId)
{
    auto it = players_.find(playerId);
    if (it == players_.end())
        return;
    std::shared_ptr<Player> p = (*it).second;
    p->Logout();
}

void PlayerManager::KickAllPlayers()
{
    while (!players_.empty())
    {
        auto it = players_.begin();
        std::shared_ptr<Player> p = (*it).second;
        p->Logout();
    }
}

}

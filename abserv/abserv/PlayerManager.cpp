#include "stdafx.h"
#include "PlayerManager.h"
#include "Player.h"

#include "DebugNew.h"

namespace Game {

PlayerManager PlayerManager::Instance;

std::shared_ptr<Player> PlayerManager::GetPlayerByName(const std::string& name)
{
    auto it = playerNames_.find(name);
    if (it != playerNames_.end())
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

std::shared_ptr<Player> PlayerManager::GetPlayerByAccountId(uint32_t id)
{
    auto it = std::find_if(players_.begin(), players_.end(), [&](const std::pair<uint32_t, std::shared_ptr<Player>>& current)
    {
        return current.second->data_.accountId == id;
    });
    if (it != players_.end())
        return (*it).second;

    return std::shared_ptr<Player>();
}

uint32_t PlayerManager::GetPlayerId(const std::string& name)
{
    auto it = playerNames_.find(name);
    if (it != playerNames_.end())
        return (*it).second->id_;
    return 0;
}

std::shared_ptr<Player> PlayerManager::CreatePlayer(const std::string& name,
    std::shared_ptr<Net::ProtocolGame> client)
{
    std::shared_ptr<Player> result = std::make_shared<Player>(client);

    {
        std::lock_guard<std::recursive_mutex> lockClass(lock_);
        players_[result->id_] = result;
        playerNames_[name] = result.get();
    }

    return result;
}

void PlayerManager::RemovePlayer(uint32_t playerId)
{
    auto it = players_.find(playerId);
    if (it != players_.end())
    {
        std::string name = (*it).second->GetName();
        std::lock_guard<std::recursive_mutex> lockClass(lock_);
        playerNames_.erase(name);
        players_.erase(it);
    }
}

void PlayerManager::CleanPlayers()
{
    if (players_.size() == 0)
        return;

    // Delete all assets that are only owned by the cache
    auto i = players_.begin();
    while ((i = std::find_if(i, players_.end(), [](const auto& current) -> bool
    {
        // Disconnect after 10sec
        return current.second->GetInactiveTime() > PLAYER_INACTIVE_TIME_KICK;
    })) != players_.end())
    {
        std::shared_ptr<Player> p = (*i).second;
        i++;
        std::string name = p->GetName();
        // Calls PlayerManager::RemovePlayer()
        p->Logout();
   }
}

}

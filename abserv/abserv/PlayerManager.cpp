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

std::shared_ptr<Player> PlayerManager::CreatePlayer(const std::string& name,
    std::shared_ptr<Net::ProtocolGame> client)
{
    std::shared_ptr<Player> result(new Player(client));

    players_[result->id_] = result;
    playerNames_[name] = result.get();

    return result;
}

void PlayerManager::RemovePlayer(uint32_t playerId)
{
    auto it = players_.find(playerId);
    if (it != players_.end())
    {
        players_.erase(it);
    }
}

}

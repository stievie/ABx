#pragma once

#include <limits>
#include "Utils.h"

namespace Net {
class ProtocolGame;
}

namespace Game {

class Player;

class PlayerManager
{
private:
    std::recursive_mutex lock_;
    std::map<std::string, Player*> playerUuids_;
    int64_t idleTime_;
    /// The owner of players
    std::map<uint32_t, std::shared_ptr<Player>> players_;
public:
    PlayerManager() :
        idleTime_(Utils::AbTick())
    {}
    ~PlayerManager()
    {
        playerUuids_.clear();
        players_.clear();
    }

    const std::map<uint32_t, std::shared_ptr<Player>>& GetPlayers() const
    {
        return players_;
    }
    std::shared_ptr<Player> GetPlayerByName(const std::string& name);
    /// GEt player by player UUID
    std::shared_ptr<Player> GetPlayerByUuid(const std::string& uuid);
    /// Get player by in game ID
    std::shared_ptr<Player> GetPlayerById(uint32_t id);
    std::shared_ptr<Player> GetPlayerByAccountUuid(const std::string& uuid);
    /// Get player ID by name
    uint32_t GetPlayerIdByName(const std::string& name);
    std::shared_ptr<Player> CreatePlayer(const std::string& playerUuid, std::shared_ptr<Net::ProtocolGame> client);
    void RemovePlayer(uint32_t playerId);
    void CleanPlayers();
    void KickPlayer(uint32_t playerId);
    void KickAllPlayers();
    void BroadcastNetMessage(const Net::NetworkMessage& msg);

    size_t GetPlayerCount() const
    {
        return players_.size();
    }

    int64_t GetIdleTime() const
    {
        if (players_.size() != 0)
            return 0;
        return Utils::AbTick() - idleTime_;
    }
};

}

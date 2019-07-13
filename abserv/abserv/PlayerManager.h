#pragma once

#include <limits>
#include "Iteration.h"

namespace Net {
class ProtocolGame;
}

namespace Game {

class Player;

class PlayerManager
{
private:
    /// Player UUID -> id
    std::unordered_map<std::string, uint32_t> playerUuids_;
    int64_t idleTime_;
    /// The owner of players
    std::map<uint32_t, std::shared_ptr<Player>> players_;
public:
    PlayerManager() :
        idleTime_(Utils::Tick())
    {}
    ~PlayerManager()
    {
        playerUuids_.clear();
        players_.clear();
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
        return Utils::Tick() - idleTime_;
    }
    template<typename Callback>
    inline void VisitPlayers(Callback&& callback)
    {
        for (auto& player : players_)
        {
            if (player.second)
            {
                if (callback(*player.second) != Iteration::Continue)
                    break;
            }
        }
    }
};

}

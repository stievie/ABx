#pragma once

#include <memory>
#include <string>
#include <map>
#include <limits>
#include <mutex>

namespace Net {
class ProtocolGame;
}

namespace Game {

class Player;

class PlayerManager
{
private:
    std::recursive_mutex lock_;
    std::map<std::string, Player*> playerNames_;
    /// The owner of players
    std::map<uint32_t, std::shared_ptr<Player>> players_;
public:
    PlayerManager() = default;
    ~PlayerManager()
    {
        playerNames_.clear();
        players_.clear();
    }

    std::shared_ptr<Player> GetPlayerByName(const std::string& name);
    std::shared_ptr<Player> GetPlayerById(uint32_t id);
    /// Get player ID by name
    uint32_t GetPlayerId(const std::string& name);
    std::shared_ptr<Player> CreatePlayer(const std::string& name, std::shared_ptr<Net::ProtocolGame> client);
    void RemovePlayer(uint32_t playerId);

    static PlayerManager Instance;
};

}

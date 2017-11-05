#pragma once

#include <memory>
#include <string>
#include <map>
#include <limits>

namespace Net {
class ProtocolGame;
}

namespace Game {

class Player;

class PlayerManager
{
private:
    std::map<std::string, Player*> playerNames_;
    /// The owner of players
    std::map<uint32_t, std::shared_ptr<Player>> players_;
    uint32_t playerIds_ = 0;
    uint32_t GetNewPlayerId()
    {
        if (playerIds_ >= std::numeric_limits<uint32_t>::max())
            playerIds_ = 0;
        return playerIds_++;
    }
public:
    PlayerManager() = default;

    std::shared_ptr<Player> GetPlayerByName(const std::string& name);
    std::shared_ptr<Player> CreatePlayer(const std::string& name, std::shared_ptr<Net::ProtocolGame> client);
    void RemovePlayer(uint32_t playerId);

    static PlayerManager Instance;
};

}

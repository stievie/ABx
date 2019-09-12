#pragma once

#include <unordered_map>
#include <map>
#include <memory>
#include <limits>
#include "Utils.h"
#include <sa/Iteration.h>
#include <multi_index_container.hpp>
#include <multi_index/hashed_index.hpp>
#include <multi_index/ordered_index.hpp>
#include <multi_index/member.hpp>

namespace Net {
class ProtocolGame;
class NetworkMessage;
}

namespace Game {

class Player;

class PlayerManager
{
private:
    struct PlayerIndexItem
    {
        uint32_t id;
        std::string playerUuid;
        std::string playerName;
        std::string accountUuid;
    };

    using PlayerIndex = multi_index::multi_index_container <
        PlayerIndexItem,
        multi_index::indexed_by <
            multi_index::hashed_unique<
                multi_index::member<PlayerIndexItem, uint32_t, &PlayerIndexItem::id>
            >,
            multi_index::hashed_unique<
                multi_index::member<PlayerIndexItem, std::string, &PlayerIndexItem::playerUuid>
            >,
            multi_index::hashed_unique<
                multi_index::member<PlayerIndexItem, std::string, &PlayerIndexItem::playerName>
            >,
            multi_index::hashed_unique<
                multi_index::member<PlayerIndexItem, std::string, &PlayerIndexItem::accountUuid>
            >
        >
    >;

    PlayerIndex playerIndex_;
    int64_t idleTime_;
    /// The owner of players
    std::map<uint32_t, std::shared_ptr<Player>> players_;
public:
    PlayerManager() :
        idleTime_(Utils::Tick())
    {}
    ~PlayerManager()
    {
        playerIndex_.clear();
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
    std::shared_ptr<Player> CreatePlayer(std::shared_ptr<Net::ProtocolGame> client);
    void UpdatePlayerIndex(const Player& player);
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
    inline void VisitPlayers(const Callback& callback)
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

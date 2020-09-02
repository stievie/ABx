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

#pragma once

#include <unordered_map>
#include <map>
#include <memory>
#include <limits>
#include <abscommon/Utils.h>
#include <eastl.hpp>
#include <sa/Iteration.h>
#include <multi_index_container.hpp>
#include <multi_index/hashed_index.hpp>
#include <multi_index/ordered_index.hpp>
#include <multi_index/member.hpp>
#include <sa/time.h>

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

    struct IdIndexTag {};
    struct PlayerUuidIndexTag {};
    struct PlayerNameIndexTag {};
    struct AccountUuidIndexTag {};

    using PlayerIndex = multi_index::multi_index_container <
        PlayerIndexItem,
        multi_index::indexed_by <
            multi_index::hashed_unique<
                multi_index::tag<IdIndexTag>,
                multi_index::member<PlayerIndexItem, uint32_t, &PlayerIndexItem::id>
            >,
            multi_index::hashed_unique<
                multi_index::tag<PlayerUuidIndexTag>,
                multi_index::member<PlayerIndexItem, std::string, &PlayerIndexItem::playerUuid>
            >,
            multi_index::hashed_unique<
                multi_index::tag<PlayerNameIndexTag>,
                multi_index::member<PlayerIndexItem, std::string, &PlayerIndexItem::playerName>
            >,
            multi_index::hashed_unique<
                multi_index::tag<AccountUuidIndexTag>,
                multi_index::member<PlayerIndexItem, std::string, &PlayerIndexItem::accountUuid>
            >
        >
    >;

    /// Index to lookup players
    PlayerIndex playerIndex_;
    /// Time with no players
    int64_t idleTime_;
    /// The owner of players
    ea::map<uint32_t, ea::shared_ptr<Player>> players_;
public:
    PlayerManager() :
        idleTime_(sa::time::tick())
    {}
    ~PlayerManager()
    {
        playerIndex_.clear();
        players_.clear();
    }

    ea::shared_ptr<Player> GetPlayerByName(const std::string& name);
    /// Get player by player UUID
    ea::shared_ptr<Player> GetPlayerByUuid(const std::string& uuid);
    /// Get player by in game ID
    ea::shared_ptr<Player> GetPlayerById(uint32_t id);
    ea::shared_ptr<Player> GetPlayerByAccountUuid(const std::string& uuid);
    /// Get player ID by name
    uint32_t GetPlayerIdByName(const std::string& name);
    ea::shared_ptr<Player> CreatePlayer(std::shared_ptr<Net::ProtocolGame> client);
    void UpdatePlayerIndex(const Player& player);
    void RemovePlayer(uint32_t playerId);
    void CleanPlayers();
    void RefreshAuthTokens();
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
        return sa::time::tick() - idleTime_;
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

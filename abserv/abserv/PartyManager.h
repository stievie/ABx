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

#include "Party.h"
#include <sa/Iteration.h>
#include <multi_index_container.hpp>
#include <multi_index/hashed_index.hpp>
#include <multi_index/ordered_index.hpp>
#include <multi_index/member.hpp>
#include <unordered_map>

namespace Game {

class PartyManager
{
private:
    /// The owner of Parties
    std::unordered_map<uint32_t, std::shared_ptr<Party>> parties_;

    // https://stackoverflow.com/questions/39510143/how-to-use-create-boostmulti-index
    struct PartyIndexItem
    {
        uint32_t partyId;
        uint32_t gameId;
        std::string partyUuid;
    };
    struct PartyIdTag {};
    struct PartyUuidTag {};
    struct GameIdTag {};
    using PartyIndex = multi_index::multi_index_container<
        PartyIndexItem,
        multi_index::indexed_by<
            multi_index::hashed_unique<
                multi_index::tag<PartyIdTag>,
                multi_index::member<PartyIndexItem, uint32_t, &PartyIndexItem::partyId>
            >,
            multi_index::hashed_unique<
                multi_index::tag<PartyUuidTag>,
                multi_index::member<PartyIndexItem, std::string, &PartyIndexItem::partyUuid>
            >,
            multi_index::hashed_non_unique<
                multi_index::tag<GameIdTag>,
                multi_index::member<PartyIndexItem, uint32_t, &PartyIndexItem::gameId>
            >
        >
    >;
    PartyIndex partyIndex_;
    void AddToIndex(const Party& party);
public:
    PartyManager() = default;
    ~PartyManager() = default;
    /// Returns an existing party or creates a new one
    std::shared_ptr<Party> GetByUuid(const std::string& uuid);
    std::shared_ptr<Party> Get(uint32_t partyId) const;
    void Remove(uint32_t partyId);
    // Update parties game ID
    void SetPartyGameId(uint32_t partyId, uint32_t gameId);
    /// Get all parties in a game. Used by Lua.
    std::vector<Party*> GetByGame(uint32_t gameId) const;
    template <typename Callback>
    void VisitGameParties(uint32_t gameId, Callback&& callback) const
    {
        if (gameId == 0)
            return;

        auto& idIndex = partyIndex_.get<GameIdTag>();
        auto its = idIndex.equal_range(gameId);
        while (its.first != its.second)
        {
            auto it = parties_.find((*its.first).partyId);
            if ((*it).second)
                if (callback(*(*it).second) != Iteration::Continue)
                    return;
            ++its.first;
        }
    }
};

}

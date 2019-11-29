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
        std::string partyUuid;
        uint32_t gameId;
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
    ~PartyManager()
    {
        parties_.clear();
    }
    /// Returns an existing party or creates a new one
    std::shared_ptr<Party> GetByUuid(const std::string& uuid);
    std::shared_ptr<Party> Get(uint32_t partyId) const;
    void Remove(uint32_t partyId);
    void SetPartyGameId(uint32_t partyId, uint32_t gameId);
    /// Get all parties in a game. Used by Lua.
    std::vector<Party*> GetByGame(uint32_t gameId) const;
    template <typename Callback>
    void VisitGameParties(uint32_t gameId, const Callback& callback) const
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

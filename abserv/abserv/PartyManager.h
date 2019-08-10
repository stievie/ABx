#pragma once

#include "Party.h"

namespace Game {

class PartyManager
{
private:
    /// The owner of Parties
    std::map<std::string, std::shared_ptr<Party>> parties_;
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
    /// Get all parties in a game
    std::vector<Party*> GetByGame(uint32_t id) const;
};

}

#include "stdafx.h"
#include "PartyManager.h"

namespace Game {

std::shared_ptr<Party> PartyManager::GetParty(std::shared_ptr<Player> leader, const std::string& uuid)
{
    auto it = parties_.find(uuid);
    if (it != parties_.end())
        return (*it).second;

    std::string _uuid(uuid);
    if (uuids::uuid(_uuid).nil())
    {
        const uuids::uuid guid = uuids::uuid_system_generator{}();
        _uuid = guid.to_string();
    }
    std::shared_ptr<Party> result = std::make_shared<Party>(leader);
    result->data_.uuid = _uuid;
    return result;
}

std::shared_ptr<Party> PartyManager::GetPartyById(uint32_t partyId)
{
    auto it = std::find_if(parties_.begin(), parties_.end(), [&](const auto& o) -> bool
    {
        return o.second->id_ == partyId;
    });
    if (it != parties_.end())
        return (*it).second;

    return std::shared_ptr<Party>();
}

}

#include "stdafx.h"
#include "PartyManager.h"
#include "Subsystems.h"
#include "DataClient.h"

namespace Game {

std::shared_ptr<Party> PartyManager::GetByUuid(const std::string& uuid)
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
    AB::Entities::Party p;
    p.uuid = _uuid;
    auto cli = GetSubsystem<IO::DataClient>();
    if (!cli->Read(p))
        cli->Create(p);

    std::shared_ptr<Party> result = std::make_shared<Party>();
    result->data_ = p;
    parties_[_uuid] = result;
    return result;
}

std::shared_ptr<Party> PartyManager::Get(uint32_t partyId)
{
    auto it = std::find_if(parties_.begin(), parties_.end(), [partyId](const auto& o) -> bool
    {
        return o.second->id_ == partyId;
    });
    if (it != parties_.end())
        return (*it).second;

    return std::shared_ptr<Party>();
}

void PartyManager::Remove(uint32_t partyId)
{
    auto it = std::find_if(parties_.begin(), parties_.end(), [partyId](const auto& o) -> bool
    {
        return o.second->id_ == partyId;
    });
    if (it != parties_.end())
        parties_.erase(it);
}

}

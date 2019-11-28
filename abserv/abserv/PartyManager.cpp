#include "stdafx.h"
#include "PartyManager.h"
#include "Subsystems.h"
#include "DataClient.h"
#include "UuidUtils.h"

namespace Game {

void PartyManager::AddToIndex(const Party& party)
{
    partyIndex_.insert({
       party.id_,
       party.data_.uuid,
       party.gameId_
   });
}

std::shared_ptr<Party> PartyManager::GetByUuid(const std::string& uuid)
{
    auto& idIndex = partyIndex_.get<PartyUuidTag>();
    auto indexIt = idIndex.find(uuid);
    if (indexIt != idIndex.end())
    {
        auto it = parties_.find((*indexIt).partyId);
        if (it != parties_.end())
            return (*it).second;
    }

    std::string _uuid(uuid);
    if (uuids::uuid(_uuid).nil())
        _uuid = Utils::Uuid::New();
    AB::Entities::Party p;
    p.uuid = _uuid;
    auto* cli = GetSubsystem<IO::DataClient>();
    if (!cli->Read(p))
        cli->Create(p);

    std::shared_ptr<Party> result = std::make_shared<Party>();
    result->data_ = std::move(p);
    parties_[result->id_] = result;
    AddToIndex(*result);
    return result;
}

std::shared_ptr<Party> PartyManager::Get(uint32_t partyId) const
{
    const auto it = parties_.find(partyId);
    if (it == parties_.end())
        return std::shared_ptr<Party>();
    return (*it).second;
}

void PartyManager::Remove(uint32_t partyId)
{
    auto it = parties_.find(partyId);
    if (it != parties_.end())
        parties_.erase(it);
}

std::vector<Party*> PartyManager::GetByGame(uint32_t gameId)
{
    std::vector<Party*> result;
    if (gameId == 0)
        return result;

    VisitGameParties(gameId, [&result] (Party& party)
    {
        result.push_back(&party);
        return Iteration::Continue;
    });
    return result;
}

}

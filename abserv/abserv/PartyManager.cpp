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

#include "PartyManager.h"
#include <abscommon/StringUtils.h>
#include <abscommon/UuidUtils.h>

namespace Game {

void PartyManager::AddToIndex(const Party& party)
{
    partyIndex_.insert({
        party.GetId(),
        party.gameId_,
        party.data_.uuid
   });
}

void PartyManager::SetPartyGameId(uint32_t partyId, uint32_t gameId)
{
    auto& idIndex = partyIndex_.get<PartyIdTag>();
    auto its = idIndex.find(partyId);
    if (its != idIndex.end())
    {
        idIndex.replace(its, {
            its->partyId,
            gameId,
            its->partyUuid
        });
    }

    auto it = parties_.find(partyId);
    if (it != parties_.end())
    {
        (*it).second->gameId_ = gameId;
    }
}

ea::shared_ptr<Party> PartyManager::GetByUuid(const std::string& uuid)
{
    auto& idIndex = partyIndex_.get<PartyUuidTag>();
    auto indexIt = idIndex.find(uuid);
    if (indexIt != idIndex.end())
        return Get((*indexIt).partyId);

    std::string _uuid(uuid);
    if (uuids::uuid(_uuid).nil())
        _uuid = Utils::Uuid::New();
    AB::Entities::Party p;
    p.uuid = _uuid;
    auto* cli = GetSubsystem<IO::DataClient>();
    if (!cli->Read(p))
        cli->Create(p);

    ea::shared_ptr<Party> result = ea::make_shared<Party>();
    result->data_ = std::move(p);
    parties_[result->GetId()] = result;
    AddToIndex(*result);
    return result;
}

ea::shared_ptr<Party> PartyManager::Get(uint32_t partyId) const
{
    const auto it = parties_.find(partyId);
    if (it == parties_.end())
        return ea::shared_ptr<Party>();
    return (*it).second;
}

void PartyManager::Remove(uint32_t partyId)
{
    auto it = parties_.find(partyId);
    if (it != parties_.end())
        parties_.erase(it);
}

std::vector<Party*> PartyManager::GetByGame(uint32_t gameId) const
{
    std::vector<Party*> result;

    VisitGameParties(gameId, [&result] (Party& party)
    {
        result.push_back(&party);
        return Iteration::Continue;
    });
    return result;
}

}

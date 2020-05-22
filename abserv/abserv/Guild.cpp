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


#include "Guild.h"
#include <AB/Entities/GuildMembers.h>
#include <abscommon/DataClient.h>
#include <abscommon/UuidUtils.h>

namespace Game {

Guild::Guild(AB::Entities::Guild&& data) :
    data_(std::move(data))
{
}

size_t Guild::GetAccounts(ea::vector<std::string>& uuids) const
{
    AB::Entities::GuildMembers members;
    members.uuid = data_.uuid;
    auto* client = GetSubsystem<IO::DataClient>();
    if (!client->Read(members))
        return 0;
    uuids.reserve(members.members.size());
    for (const auto& mu : members.members)
        uuids.push_back(mu.accountUuid);
    return uuids.size();
}

bool Guild::GetMembers(AB::Entities::GuildMembers& members) const
{
    members.uuid = data_.uuid;
    auto* client = GetSubsystem<IO::DataClient>();
    if (!client->Read(members))
        return false;
    return true;
}

bool Guild::IsMember(const std::string& accountUuid) const
{
    AB::Entities::GuildMembers members;
    members.uuid = data_.uuid;
    auto* client = GetSubsystem<IO::DataClient>();
    if (!client->Read(members))
        return false;
    for (const auto& mu : members.members)
        if (Utils::Uuid::IsEqual(accountUuid, mu.accountUuid))
            return true;
    return false;
}

bool Guild::GetMember(const std::string& accountUuid, AB::Entities::GuildMember& member) const
{
    AB::Entities::GuildMembers members;
    members.uuid = data_.uuid;
    auto* client = GetSubsystem<IO::DataClient>();
    if (!client->Read(members))
        return false;
    for (const auto& mu : members.members)
    {
        if (Utils::Uuid::IsEqual(accountUuid, mu.accountUuid))
        {
            member = mu;
            return true;
        }
    }
    return false;
}

}

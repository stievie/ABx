#include "stdafx.h"
#include "Guild.h"
#include "DataClient.h"
#include "Subsystems.h"
#include <AB/Entities/GuildMembers.h>
#include "UuidUtils.h"

namespace Game {

Guild::Guild(AB::Entities::Guild&& data) :
    data_(std::move(data))
{
}

size_t Guild::GetAccounts(std::vector<std::string>& uuids) const
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
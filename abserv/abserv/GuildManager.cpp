#include "stdafx.h"
#include "GuildManager.h"
#include "DataClient.h"
#include "Subsystems.h"

namespace Game {

std::shared_ptr<Guild> GuildManager::Get(const std::string& guildUuid)
{
    const auto it = guilds_.find(guildUuid);
    if (it != guilds_.end())
        return (*it).second;

    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Guild g;
    g.uuid = guildUuid;
    if (!client->Read(g))
        return std::shared_ptr<Guild>();

    std::shared_ptr<Guild> guild = std::make_shared<Guild>(std::move(g));
    guilds_.emplace(guildUuid, guild);
    return guild;
}

}
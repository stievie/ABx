#include "stdafx.h"
#include "GuildManager.h"
#include "DataClient.h"
#include "Subsystems.h"
#include "StringUtils.h"

namespace Game {

void GuildManager::AddGuildToIndex(const AB::Entities::Guild& guild)
{
    guildIndex_.insert({
        guild.uuid,
        Utils::Utf8ToLower(guild.name)
    });
}

std::shared_ptr<Guild> GuildManager::GetByName(const std::string& name)
{
    auto& index = guildIndex_.get<GuildNameIndexTag>();
    // Guild names are case insensitive
    const auto it = index.find(Utils::Utf8ToLower(name));
    if (it == index.end())
    {
        auto* client = GetSubsystem<IO::DataClient>();
        AB::Entities::Guild g;
        g.name = name;
        if (!client->Read(g))
            return std::shared_ptr<Guild>();
        AddGuildToIndex(g);

        std::shared_ptr<Guild> guild = std::make_shared<Guild>(std::move(g));
        guilds_.emplace(g.uuid, guild);
        return guild;
    }

    return Get((*it).guildUuid);
}

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

    AddGuildToIndex(g);
    std::shared_ptr<Guild> guild = std::make_shared<Guild>(std::move(g));
    guilds_.emplace(guildUuid, guild);
    return guild;
}

}

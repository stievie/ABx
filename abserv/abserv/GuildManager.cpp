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

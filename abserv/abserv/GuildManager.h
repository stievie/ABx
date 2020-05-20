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

#pragma once

#include <AB/Entities/Guild.h>
#include <eastl.hpp>
#include <multi_index_container.hpp>
#include <multi_index/hashed_index.hpp>
#include <multi_index/ordered_index.hpp>
#include <multi_index/member.hpp>

namespace Game {

class Guild;

class GuildManager
{
private:
    struct GuildIndexItem
    {
        std::string guildUuid;
        std::string guildName;
    };
    struct GuildUuidIndexTag {};
    struct GuildNameIndexTag {};
    using GuildIndex = multi_index::multi_index_container<
        GuildIndexItem,
        multi_index::indexed_by<
            multi_index::hashed_unique<
                multi_index::tag<GuildUuidIndexTag>,
                multi_index::member<GuildIndexItem, std::string, &GuildIndexItem::guildUuid>
            >,
            multi_index::hashed_unique<
                multi_index::tag<GuildNameIndexTag>,
                multi_index::member<GuildIndexItem, std::string, &GuildIndexItem::guildName>
            >
        >
    >;
    GuildIndex guildIndex_;
    ea::unordered_map<std::string, ea::shared_ptr<Guild>> guilds_;
    void AddGuildToIndex(const AB::Entities::Guild& guild);
public:
    ea::shared_ptr<Guild> Get(const std::string& guildUuid);
    ea::shared_ptr<Guild> GetByName(const std::string& name);
};

}

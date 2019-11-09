#pragma once

#include "Guild.h"
#include <unordered_map>
#include <memory>
#include <multi_index_container.hpp>
#include <multi_index/hashed_index.hpp>
#include <multi_index/ordered_index.hpp>
#include <multi_index/member.hpp>

namespace Game {

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
    std::unordered_map<std::string, std::shared_ptr<Guild>> guilds_;
    void AddGuildToIndex(const AB::Entities::Guild& guild);
public:
    std::shared_ptr<Guild> Get(const std::string& guildUuid);
    std::shared_ptr<Guild> GetByName(const std::string& name);
};

}

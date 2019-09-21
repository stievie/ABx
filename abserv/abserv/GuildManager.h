#pragma once

#include "Guild.h"

namespace Game {

class GuildManager
{
private:
    std::unordered_map<std::string, std::shared_ptr<Guild>> guilds_;
public:
    std::shared_ptr<Guild> Get(const std::string& guildUuid);
};

}

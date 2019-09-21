#pragma once

#include "Guild.h"
#include <unordered_map>
#include <memory>

namespace Game {

class GuildManager
{
private:
    std::unordered_map<std::string, std::shared_ptr<Guild>> guilds_;
public:
    std::shared_ptr<Guild> Get(const std::string& guildUuid);
};

}

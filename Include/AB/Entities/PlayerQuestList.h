#pragma once

#include <AB/Entities/Entity.h>
#include <bitsery/ext/inheritance.h>
#include <AB/Entities/Limits.h>

using bitsery::ext::BaseClass;

namespace AB {
namespace Entities {

static constexpr auto KEY_PLAYERQUSTLIST = "player_quest_list";

/// A list of quests a player has running, completed or not but still not collected reward.
/// UUID is the player UUID
struct PlayerQuestList : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_PLAYERQUSTLIST;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.container(questUuids, Limits::MAX_PLAYERQUESTS, [&s](std::string& c)
        {
            s.text1b(c, Limits::MAX_UUID);
        });
    }

    std::vector<std::string> questUuids;
};

}
}

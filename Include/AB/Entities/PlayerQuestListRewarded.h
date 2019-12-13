#pragma once

#include <AB/Entities/Entity.h>
#include <bitsery/ext/inheritance.h>
#include <AB/Entities/Limits.h>

using bitsery::ext::BaseClass;

namespace AB {
namespace Entities {

static constexpr auto KEY_PLAYERQUSTLIST_REWARDED = "player_quest_list_rewarded";

/// A list of quests a player has completed and collected the reward. Keeping track
/// of this is necessary for e.g. Quest chains.
/// UUID is the player UUID
struct PlayerQuestListRewarded : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_PLAYERQUSTLIST_REWARDED;
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

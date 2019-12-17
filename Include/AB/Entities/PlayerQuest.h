#pragma once

#include <AB/Entities/Entity.h>
#include <bitsery/ext/inheritance.h>
#include <AB/Entities/Limits.h>

using bitsery::ext::BaseClass;

namespace AB {
namespace Entities {

static constexpr auto KEY_PLAYERQUEST = "player_quests";

struct PlayerQuest : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_PLAYERQUEST;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(playerUuid, Limits::MAX_UUID);
        s.text1b(questUuid, Limits::MAX_UUID);
        s.value1b(completed);
        s.value1b(rewarded);
        s.value1b(deleted);
        s.value8b(pickupTime);
        s.value8b(completeTime);
        s.value8b(rewardTime);
        s.text1b(progress, Limits::MAX_QUESTPROGRESS);
    }

    std::string playerUuid{ EMPTY_GUID };
    std::string questUuid{ EMPTY_GUID };
    bool completed{ false };   //!< Player completed the quest
    bool rewarded{ false };    //!< Player collected reward for this quest -> no longer shown in quest log
    bool deleted{ false };
    timestamp_t pickupTime{ 0 };
    timestamp_t completeTime{ 0 };
    timestamp_t rewardTime{ 0 };
    std::string progress;
};

}
}

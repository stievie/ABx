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

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
#include <vector>

using bitsery::ext::BaseClass;

namespace AB {
namespace Entities {

static constexpr auto KEY_QUEST = "game_quests";

struct Quest : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_QUEST;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.value4b(index);
        s.value1b(repeatable);
        s.value4b(rewardXp);
        s.value4b(rewardMoney);
        s.text1b(name, Limits::MAX_QUESTNAME);
        s.text1b(script, Limits::MAX_FILENAME);
        s.text1b(description, Limits::MAX_QUESTDESCR);
        s.text1b(dependsOn, Limits::MAX_UUID);
        s.container(rewardItems, Limits::MAX_QUEST_REWARDITEMS, [&s](std::string& c)
        {
            s.text1b(c, Limits::MAX_UUID);
        });
    }

    uint32_t index{ INVALID_INDEX };
    bool repeatable{ false };
    int32_t rewardXp{ 0 };
    int32_t rewardMoney{ 0 };
    std::string name;
    std::string script;
    std::string description;
    std::string dependsOn{ EMPTY_GUID };
    std::vector<std::string> rewardItems;
};

}
}

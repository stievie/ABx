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
    std::vector<std::string> rewardItems;
};

}
}

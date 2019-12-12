#pragma once

#include <AB/Entities/Entity.h>
#include <bitsery/ext/inheritance.h>
#include <AB/Entities/Limits.h>

using bitsery::ext::BaseClass;

namespace AB {
namespace Entities {

static constexpr auto KEY_QUESTLIST = "quest_list";

struct QuestList : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_QUESTLIST;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.container(questUuids, Limits::MAX_QUESTS, [&s](std::string& c)
        {
            s.text1b(c, Limits::MAX_UUID);
        });
    }

    std::vector<std::string> questUuids;
};

}
}

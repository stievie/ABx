#pragma once

#include <AB/Entities/Entity.h>
//include inheritance extension
//this header contains two extensions, that specifies inheritance type of base class
//  BaseClass - normal inheritance
//  VirtualBaseClass - when virtual inheritance is used
//in order for virtual inheritance to work, InheritanceContext is required.
//it can be created either internally (via configuration) or externally (pointer to context).
#include <bitsery/ext/inheritance.h>
#include <AB/Entities/Limits.h>

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
        s.text1b(name, Limits::MAX_QUESTNAME);
        s.text1b(script, Limits::MAX_FILENAME);
        s.text1b(description, Limits::MAX_QUESTDESCR);
    }

    uint32_t index{ INVALID_INDEX };
    bool repeatable{ false };
    std::string name;
    std::string script;
    std::string description;
};

}
}

#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

static constexpr auto KEY_EFFECTS = "game_effects";

enum EffectCategory : uint8_t
{
    EffectNone = 0,
    // From skills ---------------
    EffectCondition = 1,
    EffectEnchantment = 2,
    EffectHex = 3,
    EffectPreparation = 4,
    EffectShout = 5,
    EffectSpirit = 6,
    EffectStance = 7,
    EffectWard = 8,
    EffectWell = 9,
    // ---------------------------
    EffectGeneral = 20,
    EffectEnvironment = 254
};

struct Effect : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_EFFECTS;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.value4b(index);
        s.text1b(name, Limits::MAX_EFFECT_NAME);
        s.value1b(category);
        s.text1b(script, Limits::MAX_FILENAME);
        s.text1b(icon, Limits::MAX_FILENAME);
    }

    uint32_t index = 0;
    std::string name;
    EffectCategory category;
    std::string script;
    std::string icon;
};

}
}

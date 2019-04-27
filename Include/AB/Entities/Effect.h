#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

static constexpr auto KEY_EFFECTS = "game_effects";

enum EffectCategory : uint8_t
{
    EffectNone = 0,
    // From skills -------------------------------------------------------------
    EffectCondition = 1,
    EffectEnchantment = 2,
    EffectHex = 3,
    EffectShout = 4,
    EffectSpirit = 5,
    EffectWard = 6,
    EffectWell = 7,
    // Single
    EffectPreparation = 8,
    EffectStance = 9,
    EffectForm = 10,
    EffectGlyphe = 11,
    EffectPetAttack = 12,
    EffectWeaponSpell = 13,
    // /Single
    // Other -------------------------------------------------------------------
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
        s.text1b(soundEffect, Limits::MAX_FILENAME);
        s.text1b(particleEffect, Limits::MAX_FILENAME);
    }

    uint32_t index = 0;
    std::string name;
    EffectCategory category = EffectNone;
    std::string script;
    std::string icon;
    std::string soundEffect;
    std::string particleEffect;
};

}
}

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

namespace AB {
namespace Entities {

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
    // Single, e.g. only one stance at a time, other stances remove previous stances
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

inline constexpr uint32_t EFFECT_INDEX_MORALE = 1001;

struct Effect : Entity
{
    MAKE_ENTITY(Effect)
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

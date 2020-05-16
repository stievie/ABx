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

#include <map>
#include <memory>
#include <AB/Entities/Effect.h>
#include <sa/StringHash.h>
#include <eastl.hpp>

namespace Game {

class Effect;

static constexpr size_t EFFECTTCAT_CONDITION = sa::StringHash("Condition");
static constexpr size_t EFFECTTCAT_ENCHANTMENT = sa::StringHash("Enchantment");
static constexpr size_t EFFECTTCAT_HEX = sa::StringHash("Hex");
static constexpr size_t EFFECTTCAT_SHOUT = sa::StringHash("Shout");
static constexpr size_t EFFECTTCAT_SPIRIT = sa::StringHash("Spirit");
static constexpr size_t EFFECTTCAT_WARD = sa::StringHash("Ward");
static constexpr size_t EFFECTTCAT_WELL = sa::StringHash("Well");
static constexpr size_t EFFECTTCAT_PREPARATION = sa::StringHash("Preparation");
static constexpr size_t EFFECTTCAT_STANCE = sa::StringHash("Stance");
static constexpr size_t EFFECTTCAT_FORM = sa::StringHash("Form");
static constexpr size_t EFFECTTCAT_GLYPHE = sa::StringHash("Glyphe");
static constexpr size_t EFFECTTCAT_PETATTTACK = sa::StringHash("PetAttack");
static constexpr size_t EFFECTTCAT_WEAPONSPELL = sa::StringHash("WeaponSpell");

AB::Entities::EffectCategory EffectCatNameToEffectCat(const std::string& name);

class EffectManager
{
private:
    ea::map<uint32_t, AB::Entities::Effect> effects_;
public:
    EffectManager() = default;
    ~EffectManager() = default;

    std::shared_ptr<Effect> Get(uint32_t index);
};

}

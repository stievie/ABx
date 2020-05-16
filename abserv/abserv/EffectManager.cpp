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

#include "stdafx.h"
#include "EffectManager.h"
#include "Effect.h"
#include "DataProvider.h"

namespace Game {

AB::Entities::EffectCategory EffectCatNameToEffectCat(const std::string& name)
{
    const size_t hash = sa::StringHashRt(name.c_str());
    switch (hash) {
    case EFFECTTCAT_CONDITION:
        return AB::Entities::EffectCondition;
    case EFFECTTCAT_ENCHANTMENT:
        return AB::Entities::EffectEnchantment;
    case EFFECTTCAT_HEX:
        return AB::Entities::EffectHex;
    case EFFECTTCAT_SHOUT:
        return AB::Entities::EffectShout;
    case EFFECTTCAT_SPIRIT:
        return AB::Entities::EffectSpirit;
    case EFFECTTCAT_WARD:
        return AB::Entities::EffectWard;
    case EFFECTTCAT_WELL:
        return AB::Entities::EffectWell;
    case EFFECTTCAT_PREPARATION:
        return AB::Entities::EffectPreparation;
    case EFFECTTCAT_STANCE:
        return AB::Entities::EffectStance;
    case EFFECTTCAT_FORM:
        return AB::Entities::EffectForm;
    case EFFECTTCAT_GLYPHE:
        return AB::Entities::EffectGlyphe;
    case EFFECTTCAT_PETATTTACK:
        return AB::Entities::EffectPetAttack;
    case EFFECTTCAT_WEAPONSPELL:
        return AB::Entities::EffectWeaponSpell;
    default:
        return AB::Entities::EffectNone;
    }
}

ea::shared_ptr<Effect> EffectManager::Get(uint32_t index)
{
    ea::shared_ptr<Effect> result;
    auto it = effects_.find(index);
    if (it != effects_.end())
    {
        result = ea::make_shared<Effect>((*it).second);
    }
    else
    {
        IO::DataClient* client = GetSubsystem<IO::DataClient>();
        AB::Entities::Effect effect;
        effect.index = index;
        if (!client->Read(effect))
        {
            LOG_ERROR << "Error reading effect with index " << index << std::endl;
            return ea::shared_ptr<Effect>();
        }
        result = ea::make_shared<Effect>(effect);
        // Move to cache
        effects_.emplace(index, effect);
    }

    if (result)
    {
        if (result->LoadScript(result->data_.script))
            return result;
    }

    return ea::shared_ptr<Effect>();
}

}

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
#include "ItemStats.h"

namespace Game {

ItemStats::ItemStats() = default;

DamageType ItemStats::GetDamageType() const
{
    return static_cast<DamageType>(GetValue<int>(ItemStatIndex::DamageType, (int)DamageType::Unknown));
}

int32_t ItemStats::GetMinDamage() const
{
    return GetValue<int32_t>(ItemStatIndex::MinDamage, 0);
}

int32_t ItemStats::GetMaxDamage() const
{
    return GetValue<int32_t>(ItemStatIndex::MaxDamage, 0);
}

uint32_t ItemStats::GetRequirement() const
{
    return GetValue<uint32_t>(ItemStatIndex::AttributeValue, 0);
}

Attribute ItemStats::GetAttribute() const
{
    return static_cast<Attribute>(GetValue<int>(ItemStatIndex::Attribute, (int)Attribute::None));
}

int ItemStats::GetArmor(DamageType damageType) const
{
    int value = 0;
    // General
    value += GetValue(ItemStatIndex::Armor, 0);

    switch (damageType)
    {
    case DamageType::Unknown:
        break;
    case DamageType::Fire:
    {
        value += GetValue(ItemStatIndex::ArmorElemental, 0);
        value += GetValue(ItemStatIndex::ArmorFire, 0);
        break;
    }
    case DamageType::Cold:
    {
        value += GetValue(ItemStatIndex::ArmorElemental, 0);
        value += GetValue(ItemStatIndex::ArmorCold, 0);
        break;
    }
    case DamageType::Lightning:
    {
        value += GetValue(ItemStatIndex::ArmorElemental, 0);
        value += GetValue(ItemStatIndex::ArmorLightning, 0);
        break;
    }
    case DamageType::Earth:
    {
        value += GetValue(ItemStatIndex::ArmorElemental, 0);
        value += GetValue(ItemStatIndex::ArmorEarth, 0);
        break;
    }
    case DamageType::Blunt:
    case DamageType::Piercing:
    case DamageType::Slashing:
    {
        value += GetValue(ItemStatIndex::ArmorPhysical, 0);
        break;
    }
    case DamageType::Holy:
    case DamageType::Shadow:
    case DamageType::Chaos:
    case DamageType::LifeDrain:
    case DamageType::Typeless:
    case DamageType::Dark:
        // Ignoring armor
        break;
    default:
        ASSERT_FALSE();
    }

    return value;
}

uint32_t ItemStats::GetAttributeIncrease(Attribute index) const
{
    return GetValue(static_cast<size_t>(ItemStatIndex::AttributeOffset) + static_cast<size_t>(index), 0u);
}

int ItemStats::GetHealth() const
{
    return GetValue<int>(ItemStatIndex::Health, 0);
}

int ItemStats::GetEnergy() const
{
    return GetValue<int>(ItemStatIndex::Energy, 0);
}

bool ItemStats::Load(sa::PropReadStream& stream)
{
    return Utils::VariantMapRead(stats_, stream);
}

void ItemStats::Save(sa::PropWriteStream& stream)
{
    Utils::VariantMapWrite(stats_, stream);
}

}

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

#include "Damage.h"
#include <abscommon/PropStream.h>
#include <abscommon/Variant.h>
#include <abshared/Attributes.h>

namespace Game {

enum class Stat : size_t
{
    None = 0,
    MinDamage = 1,
    MaxDamage,
    DamageType,
    Attribute,           // Weapon requires this attribute
    AttributeValue,
    Armor,               // General armor
    HealthRegen,
    EnergyRegen,
    Health,              // +/- Health
    Energy,              // +/- Energy

    PhysicalDamageReduction,
    HexDurationReduction,
    ConditionDurationReduction,
    BlindnessDurationReduction,
    WeaknessDurationReduction,
    DeseaseDurationReduction,
    PoisionDurationReduction,
    DazedDurationReduction,
    DeepWoundDurationReduction,
    BleedingDurationReduction,
    CrippledDurationReduction,

    // Type specific armor
    ArmorElemental,
    ArmorFire,
    ArmorCold,
    ArmorLightning,
    ArmorEarth,
    ArmorPhysical,
    // Special
    ArmorHoly,
    ArmorShadow,
    ArmorTypeless,
    // Other
    ArmorDark,
    ArmorChaos,

    // Attributes
    AttributeOffset = 1000
};

class ItemStats
{
public:
    ItemStats();
    // non-copyable
    ItemStats(const ItemStats&) = delete;
    ItemStats& operator=(const ItemStats&) = delete;
    ~ItemStats() = default;

    DamageType GetDamageType() const;
    int32_t GetMinDamage() const;
    int32_t GetMaxDamage() const;
    uint32_t GetRequirement() const;
    Attribute GetAttribute() const;
    int GetArmor(DamageType damageType) const;
    uint32_t GetAttributeIncrease(Attribute index) const;
    int GetHealth() const;
    int GetEnergy() const;
    template <typename T>
    T GetValue(Stat index, T def) const
    {
        return GetValue<T>(static_cast<size_t>(index), def);
    }
    template <typename T>
    T GetValue(size_t index, T def) const
    {
        const auto it = stats_.find(index);
        if (it != stats_.end())
            return static_cast<T>((*it).second);
        return def;
    }
    template<typename T>
    void SetValue(Stat index, T value)
    {
        SetValue(static_cast<size_t>(index), value);
    }
    template<typename T>
    void SetValue(size_t index, T value)
    {
        stats_[index] = value;
    }

    bool Load(IO::PropReadStream& stream);
    void Save(IO::PropWriteStream& stream);

    Utils::VariantMap stats_;
};

}

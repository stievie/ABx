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

#include <abshared/Damage.h>
#include <sa/PropStream.h>
#include <abscommon/Variant.h>
#include <abshared/Attributes.h>
#include <abshared/Items.h>

namespace Game {

class ItemStats
{
public:
    ItemStats();
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
    int GetUsages() const;
    void DescreaseUsages();
    template <typename T>
    T GetValue(ItemStatIndex index, T def) const
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
    void SetValue(ItemStatIndex index, T value)
    {
        SetValue(static_cast<size_t>(index), value);
    }
    template<typename T>
    void SetValue(size_t index, T value)
    {
        stats_[index] = value;
    }

    bool Load(sa::PropReadStream& stream);
    void Save(sa::PropWriteStream& stream) const;
    bool LoadFromString(const std::string& value);
    std::string SaveToString() const;

    Utils::VariantMap stats_;
};

}

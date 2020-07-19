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

namespace Game {

enum class DamageType
{
    Unknown = 0,
    // Elemental
    Fire,
    Cold,
    Lightning,
    Earth,
    // Physical
    Blunt,
    Piercing,
    Slashing,
    // Special
    Holy,
    Shadow,
    Typeless,
    // Other
    Dark,
    Chaos,
    // Not a real damage
    LifeDrain = 99
};

inline const char* GetDamageTypeName(DamageType type)
{
    switch (type)
    {
    case DamageType::Fire: return "Fire";
    case DamageType::Cold: return "Cold";
    case DamageType::Lightning: return "Lightning";
    case DamageType::Earth: return "Earth";
    case DamageType::Blunt: return "Blunt";
    case DamageType::Piercing: return "Piercing";
    case DamageType::Slashing: return "Slashing";
    case DamageType::Holy: return "Holy";
    case DamageType::Shadow: return "Shadow";
    case DamageType::Typeless: return "";
    case DamageType::Dark: return "Dark";
    case DamageType::Chaos: return "Chaos";
    default: return nullptr;
    }
}

enum class DamageTypeCategory
{
    Any = 0,
    Elemental = 1,
    Physical,
    ArmorIgnoring,
    Typeless
};

inline bool IsPhysicalDamage(DamageType type)
{
    return type == DamageType::Blunt || type == DamageType::Piercing || type == DamageType::Slashing;
}

inline bool IsElementalDamage(DamageType type)
{
    return type == DamageType::Fire || type == DamageType::Cold || type == DamageType::Lightning ||
        type == DamageType::Earth;
}

inline bool IsArmorIgnoringDamage(DamageType type)
{
    return type == DamageType::Chaos || type == DamageType::Holy;
}

inline bool IsTypelessDamage(DamageType type)
{
    return type == DamageType::Typeless;
}

inline bool IsDamageCategory(DamageType type, DamageTypeCategory category)
{
    switch (category)
    {
    case DamageTypeCategory::Elemental:
        return IsElementalDamage(type);
    case DamageTypeCategory::Physical:
        return IsPhysicalDamage(type);
    case DamageTypeCategory::ArmorIgnoring:
        return IsArmorIgnoringDamage(type);
    case DamageTypeCategory::Typeless:
        return IsTypelessDamage(type);
    case DamageTypeCategory::Any:
        return type != DamageType::Unknown;
    }
    return false;
}

enum class DamagePos
{
    Head = 0,
    Chest,
    Hands,
    Legs,
    Feet,

    NoPos = 99
};

}

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

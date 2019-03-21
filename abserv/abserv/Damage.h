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

enum class DamagePos
{
    Head = 0,
    Chest,
    Hands,
    Legs,
    Feet,
};

}
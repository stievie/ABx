#pragma once

#include "Variant.h"
#include "Damage.h"
#include "PropStream.h"
#include "Attributes.h"

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
    AttributeIndices GetAttribute() const;
    int GetArmor(DamageType damageType) const;
    uint32_t GetAttributeIncrease(uint32_t index) const;

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
            return (T)(*it).second;
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

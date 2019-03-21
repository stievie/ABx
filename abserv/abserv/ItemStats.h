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
    Energy,
    Armor,               // General armor
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
    ~ItemStats() = default;

    DamageType GetDamageType() const;
    int32_t GetMinDamage() const;
    int32_t GetMaxDamage() const;
    uint32_t GetRequirement() const;
    AttributeIndices GetAttribute() const;
    int GetArmor(DamageType damageType) const;
    uint32_t GetAttributeIncrease(uint32_t index) const;

    bool Load(IO::PropReadStream& stream);
    void Save(IO::PropWriteStream& stream);

    Utils::VariantMap stats_;
};

}

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


    bool Load(IO::PropReadStream& stream);
    void Save(IO::PropWriteStream& stream);

    Utils::VariantMap stats_;
};

}

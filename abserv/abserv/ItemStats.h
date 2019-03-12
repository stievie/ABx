#pragma once

#include "Variant.h"
#include "Damage.h"
#include "PropStream.h"

namespace Game {

enum class Stat : size_t
{
    None = 0,
    MinDamage = 1,
    MaxDamage,
    DamageType,
    Attribute,           // Weapon requires this attribute
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

    bool Load(IO::PropReadStream& stream);
    void Save(IO::PropWriteStream& stream);

    Utils::VariantMap stats_;
};

}

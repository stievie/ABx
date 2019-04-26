#include "stdafx.h"
#include "ItemStats.h"

namespace Game {

ItemStats::ItemStats() = default;

DamageType ItemStats::GetDamageType() const
{
    return static_cast<DamageType>(GetValue<int>(Stat::DamageType, (int)DamageType::Unknown));
}

int32_t ItemStats::GetMinDamage() const
{
    return GetValue<int32_t>(Stat::MinDamage, 0);
}

int32_t ItemStats::GetMaxDamage() const
{
    return GetValue<int32_t>(Stat::MaxDamage, 0);
}

uint32_t ItemStats::GetRequirement() const
{
    return GetValue<uint32_t>(Stat::AttributeValue, 0);
}

AttributeIndices ItemStats::GetAttribute() const
{
    return static_cast<AttributeIndices>(GetValue<int>(Stat::Attribute, (int)AttributeIndices::None));
}

int ItemStats::GetArmor(DamageType damageType) const
{
    int value = 0;
    // General
    value += GetValue(Stat::Armor, 0);

    switch (damageType)
    {
    case Game::DamageType::Unknown:
        break;
    case Game::DamageType::Fire:
    {
        value += GetValue(Stat::ArmorElemental, 0);
        value += GetValue(Stat::ArmorFire, 0);
        break;
    }
    case Game::DamageType::Cold:
    {
        value += GetValue(Stat::ArmorElemental, 0);
        value += GetValue(Stat::ArmorCold, 0);
        break;
    }
    case Game::DamageType::Lightning:
    {
        value += GetValue(Stat::ArmorElemental, 0);
        value += GetValue(Stat::ArmorLightning, 0);
        break;
    }
    case Game::DamageType::Earth:
    {
        value += GetValue(Stat::ArmorElemental, 0);
        value += GetValue(Stat::ArmorEarth, 0);
        break;
    }
    case Game::DamageType::Blunt:
    case Game::DamageType::Piercing:
    case Game::DamageType::Slashing:
    {
        value += GetValue(Stat::ArmorPhysical, 0);
        break;
    }
    case Game::DamageType::Holy:
    case Game::DamageType::Shadow:
    case Game::DamageType::Chaos:
    case Game::DamageType::LifeDrain:
    case Game::DamageType::Typeless:
    case Game::DamageType::Dark:
        // Ignoring armor
        break;
    default:
        assert(false);
        break;
    }

    return value;
}

uint32_t ItemStats::GetAttributeIncrease(uint32_t index) const
{
    return GetValue(static_cast<size_t>(Stat::AttributeOffset) + index, 0);
}

int ItemStats::GetHealth() const
{
    return GetValue<int>(Stat::Health, 0);
}

int ItemStats::GetEnergy() const
{
    return GetValue<int>(Stat::Energy, 0);
}

bool ItemStats::Load(IO::PropReadStream& stream)
{
    return Utils::VariantMapRead(stats_, stream);
}

void ItemStats::Save(IO::PropWriteStream& stream)
{
    Utils::VariantMapWrite(stats_, stream);
}

}

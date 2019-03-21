#include "stdafx.h"
#include "ItemStats.h"

namespace Game {

ItemStats::ItemStats() = default;

DamageType ItemStats::GetDamageType() const
{
    auto it = stats_.find(static_cast<size_t>(Stat::DamageType));
    if (it == stats_.end())
        return DamageType::Unknown;
    return static_cast<DamageType>((*it).second.GetInt());
}

int32_t ItemStats::GetMinDamage() const
{
    auto it = stats_.find(static_cast<size_t>(Stat::MinDamage));
    if (it == stats_.end())
        return 0;
    return (*it).second.GetInt();
}

int32_t ItemStats::GetMaxDamage() const
{
    auto it = stats_.find(static_cast<size_t>(Stat::MaxDamage));
    if (it == stats_.end())
        return 0;
    return (*it).second.GetInt();
}

uint32_t ItemStats::GetRequirement() const
{
    auto it = stats_.find(static_cast<size_t>(Stat::AttributeValue));
    if (it == stats_.end())
        return 0;
    return static_cast<uint32_t>((*it).second.GetInt());
}

AttributeIndices ItemStats::GetAttribute() const
{
    auto it = stats_.find(static_cast<size_t>(Stat::Attribute));
    if (it == stats_.end())
        return AttributeIndices::None;
    return static_cast<AttributeIndices>((*it).second.GetInt());
}

int ItemStats::GetArmor(DamageType damageType) const
{
    // TODO: + type specific armor
    int value = 0;
    {
        // General
        auto it = stats_.find(static_cast<size_t>(Stat::Armor));
        if (it != stats_.end())
            value += (*it).second.GetInt();
    }

    switch (damageType)
    {
    case Game::DamageType::Unknown:
        break;
    case Game::DamageType::Fire:
    {
        {
            auto it = stats_.find(static_cast<size_t>(Stat::ArmorElemental));
            if (it != stats_.end())
                value += (*it).second.GetInt();
        }
        {
            auto it = stats_.find(static_cast<size_t>(Stat::ArmorFire));
            if (it != stats_.end())
                value += (*it).second.GetInt();
        }
        break;
    }
    case Game::DamageType::Cold:
    {
        {
            auto it = stats_.find(static_cast<size_t>(Stat::ArmorElemental));
            if (it != stats_.end())
                value += (*it).second.GetInt();
        }
        {
            auto it = stats_.find(static_cast<size_t>(Stat::ArmorCold));
            if (it != stats_.end())
                value += (*it).second.GetInt();
        }
        break;
    }
    case Game::DamageType::Lightning:
    {
        {
            auto it = stats_.find(static_cast<size_t>(Stat::ArmorElemental));
            if (it != stats_.end())
                value += (*it).second.GetInt();
        }
        {
            auto it = stats_.find(static_cast<size_t>(Stat::ArmorLightning));
            if (it != stats_.end())
                value += (*it).second.GetInt();
        }
        break;
    }
    case Game::DamageType::Earth:
    {
        {
            auto it = stats_.find(static_cast<size_t>(Stat::ArmorElemental));
            if (it != stats_.end())
                value += (*it).second.GetInt();
        }
        {
            auto it = stats_.find(static_cast<size_t>(Stat::ArmorEarth));
            if (it != stats_.end())
                value += (*it).second.GetInt();
        }
        break;
    }
    case Game::DamageType::Blunt:
    case Game::DamageType::Piercing:
    case Game::DamageType::Slashing:
    {
        auto it = stats_.find(static_cast<size_t>(Stat::ArmorPhysical));
        if (it != stats_.end())
            value += (*it).second.GetInt();
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
    auto it = stats_.find(static_cast<size_t>(Stat::AttributeOffset) + index);
    if (it == stats_.end())
        return 0;
    return static_cast<uint32_t>((*it).second.GetInt());
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

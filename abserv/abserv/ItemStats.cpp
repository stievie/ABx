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

bool ItemStats::Load(IO::PropReadStream& stream)
{
    return Utils::VariantMapRead(stats_, stream);
}

void ItemStats::Save(IO::PropWriteStream& stream)
{
    Utils::VariantMapWrite(stats_, stream);
}

}

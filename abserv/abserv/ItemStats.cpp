#include "stdafx.h"
#include "ItemStats.h"

namespace Game {

ItemStats::ItemStats() = default;

DamageType ItemStats::GetDamageType() const
{
    auto it = stats_.find(static_cast<size_t>(Stat::DamageType));
    if (it == stats_.end())
        return DamageType::Unknown;
    return static_cast<DamageType>((*it).second.GetInt64());
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

}

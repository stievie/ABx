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

void ItemStats::Load(IO::PropReadStream& stream)
{
    if (stream.GetSize() == 0)
        return;

    size_t count = 0;
    if (!stream.Read<size_t>(count))
        return;
    for (size_t i = 0; i < count; ++i)
    {
        size_t stat = 0;
        if (!stream.Read<size_t>(stat))
            return;

        Utils::VariantType t = Utils::VAR_NONE;
        if (!stream.Read<Utils::VariantType>(t))
            return;
        if (t == Utils::VAR_NONE || t == Utils::VAR_VOIDPTR)
            continue;

        switch (t)
        {
        case Utils::VAR_INT:
        {
            int value = 0;
            if (stream.Read<int>(value))
                stats_[stat] = value;
            break;
        }
        case Utils::VAR_INT64:
        {
            long long value = 0;
            if (stream.Read<long long>(value))
                stats_[stat] = value;
            break;
        }
        case Utils::VAR_BOOL:
        {
            uint8_t value = 0;
            if (stream.Read<uint8_t>(value))
                stats_[stat] = value == 0 ? false : true;
            break;
        }
        case Utils::VAR_FLOAT:
        {
            float value = 0.0f;
            if (stream.Read<float>(value))
                stats_[stat] = value;
            break;
        }
        case Utils::VAR_STRING:
        {
            std::string value;
            if (stream.ReadString(value))
                stats_[stat] = value;
            break;
        }
        default:
            break;
        }
    }
}

void ItemStats::Save(IO::PropWriteStream& stream)
{
    stream.Write<size_t>(stats_.size());
    for (const auto& s : stats_)
    {
        Utils::VariantType t = s.second.GetType();
        if (t == Utils::VAR_NONE || t == Utils::VAR_VOIDPTR)
            continue;

        stream.Write<size_t>(s.first);
        stream.Write<Utils::VariantType>(t);
        switch (t)
        {
        case Utils::VAR_INT:
            stream.Write<int>(s.second.GetInt());
            break;
        case Utils::VAR_INT64:
            stream.Write<long long>(s.second.GetInt64());
            break;
        case Utils::VAR_BOOL:
            stream.Write<uint8_t>(s.second.GetBool() ? 1 : 0);
            break;
        case Utils::VAR_FLOAT:
            stream.Write<float>(s.second.GetFloat());
            break;
        case Utils::VAR_STRING:
            stream.WriteString(s.second.GetString());
            break;
        default:
            break;
        }
    }
}

}

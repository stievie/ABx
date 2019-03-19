#include "stdafx.h"
#include "ProgressComp.h"
#include "Actor.h"

namespace Game {
namespace Components {

void ProgressComp::Write(Net::NetworkMessage& message)
{
    if (items_.size() == 0)
        return;

    for (const auto& i : items_)
    {
        switch (i.type)
        {
        case ProgressType::XPIncrease:
            message.AddByte(AB::GameProtocol::GameObjectXPIncreased);
            message.Add<uint32_t>(owner_.id_);
            message.Add<int16_t>(static_cast<int16_t>(i.value));
            break;
        case ProgressType::GotSkillPoint:
            message.AddByte(AB::GameProtocol::GameObjectGotSkillPoint);
            message.Add<uint32_t>(owner_.id_);
            message.Add<int16_t>(static_cast<int16_t>(i.value));
            break;
        }
    }
    items_.clear();
}

void ProgressComp::Died()
{
    auto enemies = owner_.GetEnemiesInRange(Ranges::Aggro);
    for (const auto& e : enemies)
    {
        if (!e->IsDead())
            e->progressComp_.AddXpForKill(&owner_);
    }
}

void ProgressComp::AddXp(int value)
{
    owner_.AddXp(value);
    items_.push_back({ ProgressType::XPIncrease, value });
}

void ProgressComp::AddXpForKill(Actor* victim)
{
    size_t allyCount = owner_.GetAllyCountInRange(Ranges::Aggro);
    int a = static_cast<int>(victim->GetLevel());
    int b = static_cast<int>(owner_.GetLevel());
    int xp = (100 + 4 * std::abs(a - b) + 16 * (a - b)) / static_cast<int>(allyCount);
    AddXp(xp);
}

}
}

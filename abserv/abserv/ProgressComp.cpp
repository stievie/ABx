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
            message.AddByte(AB::GameProtocol::GameObjectProgress);
            message.Add<uint32_t>(owner_.id_);
            message.AddByte(AB::GameProtocol::ObjectProgressXPIncreased);
            message.Add<int16_t>(static_cast<int16_t>(i.value));
            break;
        case ProgressType::GotSkillPoint:
            message.AddByte(AB::GameProtocol::GameObjectProgress);
            message.Add<uint32_t>(owner_.id_);
            message.AddByte(AB::GameProtocol::ObjectProgressGotSkillPoint);
            message.Add<int16_t>(static_cast<int16_t>(i.value));
            break;
        case ProgressType::LevelAdvance:
            message.AddByte(AB::GameProtocol::GameObjectProgress);
            message.Add<uint32_t>(owner_.id_);
            message.AddByte(AB::GameProtocol::ObjectProgressLevelAdvance);
            message.Add<int16_t>(static_cast<int16_t>(i.value));
            break;
        }
    }
    items_.clear();
}

void ProgressComp::Died()
{
    // When we die all Enemies in range get XP for that
    owner_.VisitEnemiesInRange(Ranges::Aggro, [this](const Actor* actor)
    {
        if (!actor->IsDead())
            // Actors get only XP for kills when they are not dead
            actor->progressComp_->AddXpForKill(&owner_);
    });
}

void ProgressComp::AddXp(int value)
{
    // Max XP
    if (value > SKILLPOINT_ADVANCE_XP)
        value = SKILLPOINT_ADVANCE_XP;

    const uint32_t oldXp = owner_.GetXp();
    owner_.AddXp(value);
    items_.push_back({ ProgressType::XPIncrease, value });

    const uint32_t level = owner_.GetLevel();
    if (level < LEVEL_CAP)
    {
        unsigned nextLevel = level * 600 + 1400;
        if (nextLevel > SKILLPOINT_ADVANCE_XP)
            nextLevel = SKILLPOINT_ADVANCE_XP;
        if (owner_.GetXp() >= nextLevel)
        {
            AdvanceLevel();
            AddSkillPoint();
        }
    }
    else
    {
        const uint32_t x = oldXp / SKILLPOINT_ADVANCE_XP;
        const uint32_t y = owner_.GetXp() / SKILLPOINT_ADVANCE_XP;
        const uint32_t z = y - x;
        if (z > 0)
            AddSkillPoint();
    }
}

void ProgressComp::AddXpForKill(Actor* victim)
{
    const size_t allyCount = owner_.GetAllyCountInRange(Ranges::Aggro);
    const int a = static_cast<int>(victim->GetLevel());
    const int b = static_cast<int>(owner_.GetLevel());
    const int xp = (100 + 4 * std::abs(a - b) + 16 * (a - b)) / static_cast<int>(allyCount);
    AddXp(xp);
}

void ProgressComp::AddSkillPoint()
{
    items_.push_back({ ProgressType::GotSkillPoint, 1 });
    owner_.AddSkillPoint();
}

void ProgressComp::AdvanceLevel()
{
    if (owner_.GetLevel() > LEVEL_CAP)
    {
        items_.push_back({ ProgressType::LevelAdvance, 1 });
        owner_.AdvanceLevel();
    }
}

}
}

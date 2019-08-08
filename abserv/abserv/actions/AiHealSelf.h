
#pragma once

#include "../AiTask.h"
#include "../Npc.h"
#include "../SkillBar.h"

namespace AI {

AI_TASK(HealSelf)
{
    (void)deltaMillis;
    if (chr.currentTask_ == this)
    {
        if (auto s = chr.currentSkill_.lock())
        {
            if (s->IsUsing())
                return ai::RUNNING;
            chr.currentTask_ = nullptr;
            chr.currentSkill_.reset();
            return ai::FINISHED;
        }
        // WTF?
        chr.currentTask_ = nullptr;
        return ai::FINISHED;
    }
    // Not currently using

    Game::Npc& npc = chr.GetNpc();
    auto currSkill = npc.skills_->GetCurrentSkill();
    if (currSkill)
        // Some other skill currently using
        return ai::FAILED;

    auto skills = npc.skills_->GetSkillsWithEffectTarget(Game::SkillEffectHeal, Game::SkillTargetSelf, true);
    if (skills.size() == 0)
        return ai::FAILED;

    for (uint32_t i : skills)
    {
        // Find skill that can be used
        auto skill = npc.skills_->GetSkill(i);
        if (!skill)
            continue;
        if (npc.IsDead() || !npc.resourceComp_->HaveEnoughResources(skill.get()))
            continue;

        npc.SetSelectedObjectById(npc.GetId());
        npc.UseSkill(i);
        chr.currentSkill_ = skill;
        chr.currentTask_ = this;
        return ai::RUNNING;
    }

    // Nothing found
    return ai::TreeNodeStatus::FAILED;
}

}

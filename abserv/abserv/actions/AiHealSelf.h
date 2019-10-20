
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

    int skillIndex = npc.GetBestSkillIndex(Game::SkillEffectHeal, Game::SkillTargetSelf);
    if (skillIndex == -1)
        return ai::FAILED;

    auto skill = npc.skills_->GetSkill(skillIndex);
    if (!skill)
        return ai::FAILED;
    if (npc.IsDead() || !npc.resourceComp_->HaveEnoughResources(skill.get()))
        return ai::FAILED;

    if (skill->NeedsTarget())
        npc.SetSelectedObjectById(npc.GetId());
    npc.UseSkill(skillIndex);
    chr.currentSkill_ = skill;
    chr.currentTask_ = this;
    return ai::RUNNING;
}

}

#pragma once

#include "../AiTask.h"
#include "../Npc.h"
#include "../Game.h"
#include "../Skill.h"

namespace AI {

AI_TASK(ResurrectSelection)
{
    (void)deltaMillis;
    if (chr.currentTask_ == this)
    {
        if (auto s = chr.currentSkill_.lock())
        {
            if (s->IsUsing())
                return ai::RUNNING;
            chr.currentSkill_.reset();
            chr.currentTask_ = nullptr;
            return ai::FINISHED;
        }
        // WTF?
        chr.currentTask_ = nullptr;
        return ai::FINISHED;
    }

    Game::Npc& npc = chr.GetNpc();

    auto currSkill = npc.skills_->GetCurrentSkill();
    if (currSkill)
        // Some other skill currently using
        return ai::FAILED;

    auto skills = npc.skills_->GetSkillsWithEffect(Game::SkillEffectResurrect, true);
    if (skills.size() == 0)
        return ai::FAILED;
    // Possible heal targets
    const ai::FilteredEntities& selection = npc.GetAi()->getFilteredEntities();
    if (selection.empty())
        return ai::TreeNodeStatus::FAILED;

    auto target = npc.GetGame()->GetObjectById(selection[0]);
    if (!target)
        return ai::FAILED;
    Game::Actor* actor = dynamic_cast<Game::Actor*>(target.get());
    if (!actor || !actor->IsDead())
        return ai::FAILED;

    for (auto i : skills)
    {
        auto skill = npc.skills_->GetSkill(i);
        if (!skill)
            continue;
        if (!npc.resourceComp_->HaveEnoughResources(skill.get()))
            continue;

        npc.SetSelectedObjectById(selection[0]);
        npc.UseSkill(i);
        chr.currentSkill_ = skill;
        chr.currentTask_ = this;
        return ai::RUNNING;
    }
    return ai::FAILED;
}

}

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

    int skillIndex = npc.GetBestSkillIndex(Game::SkillEffectResurrect, Game::SkillTargetTarget);
    if (skillIndex == -1)
        return ai::FAILED;
    // Possible heal targets
    const ai::FilteredEntities& selection = npc.GetAi()->getFilteredEntities();
    if (selection.empty())
        return ai::TreeNodeStatus::FAILED;

    auto target = npc.GetGame()->GetObjectById(selection[0]);
    if (!target)
        return ai::FAILED;
    if (!target->IsActorType())
        return ai::FAILED;

    Game::Actor* actor = Game::To<Game::Actor>(target.get());
    if (!actor->IsDead())
        return ai::FAILED;

    auto skill = npc.skills_->GetSkill(skillIndex);
    if (!skill)
        return ai::FAILED;
    if (!npc.resourceComp_->HaveEnoughResources(skill.get()))
        return ai::FAILED;

    npc.SetSelectedObjectById(selection[0]);
    npc.UseSkill(skillIndex);
    chr.currentSkill_ = skill;
    chr.currentTask_ = this;
    return ai::RUNNING;
}

}

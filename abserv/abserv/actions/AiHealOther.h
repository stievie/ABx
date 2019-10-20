
#pragma once

#include "../AiTask.h"
#include "../Npc.h"
#include "../SkillBar.h"
#include "../Game.h"
#include "../Actor.h"

namespace AI {

AI_TASK(HealOther)
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
    // Possible heal targets
    const ai::FilteredEntities& selection = npc.GetAi()->getFilteredEntities();
    if (selection.empty())
        return ai::FAILED;

    // Lambda to use the given skills on the target
    const auto useSkill = [&npc, &chr](int skillIndex, uint32_t targetId) -> bool
    {
        auto skill = npc.skills_->GetSkill(skillIndex);
        if (!skill)
            return false;
        if (!npc.resourceComp_->HaveEnoughResources(skill.get()))
            return false;
        npc.SetSelectedObjectById(targetId);
        npc.UseSkill(skillIndex);
        chr.currentSkill_ = skill;
        return true;
    };

    if (selection.size() > 4)
    {
        // Many to heal try party healing
        int skillIndex = npc.GetBestSkillIndex(Game::SkillEffectHeal, Game::SkillTargetParty);
        if (skillIndex != -1)
        {
            if (useSkill(skillIndex, selection[0]))
            {
                chr.currentTask_ = this;
                return ai::RUNNING;
            }
        }
    }

    // Highest priorioty target is on top
    auto target = npc.GetGame()->GetObjectById(selection[0]);
    if (!target)
        return ai::FAILED;
    if (!target->IsActorType())
        return ai::FAILED;

    Game::Actor* actor = Game::To<Game::Actor>(target.get());
    if (actor->IsDead())
        return ai::FAILED;

    int skillIndex = npc.GetBestSkillIndex(Game::SkillEffectHeal, Game::SkillTargetTarget);
    if (skillIndex == -1)
        return ai::FAILED;

    if (useSkill(skillIndex, selection[0]))
    {
        chr.currentTask_ = this;
        return ai::RUNNING;
    }
    return ai::FAILED;
}

}

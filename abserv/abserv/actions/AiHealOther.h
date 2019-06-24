
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

    // Lambda to use one of the given skills on the target
    const auto useSkill = [&npc, &chr](const std::vector<uint32_t>& skills, uint32_t targetId) -> bool
    {
        for (uint32_t i : skills)
        {
            // Try all skills until success
            auto skill = npc.skills_->GetSkill(i);
            if (!skill)
                continue;
            if (!npc.resourceComp_.HaveEnoughResources(skill.get()))
                continue;

            npc.SetSelectedObjectById(targetId);
            npc.UseSkill(i);
            chr.currentSkill_ = skill;
            return true;
        }
        return false;
    };

    if (selection.size() > 4)
    {
        // Many to heal try party healing
        auto partySkills = npc.skills_->GetSkillsWithEffectTarget(Game::SkillEffectHeal, Game::SkillTargetParty, true);
        if (partySkills.size() != 0)
        {
            if (useSkill(partySkills, selection[0]))
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
    Game::Actor* actor = dynamic_cast<Game::Actor*>(target.get());
    if (!actor || actor->IsDead())
        return ai::FAILED;

    auto skills = npc.skills_->GetSkillsWithEffectTarget(Game::SkillEffectHeal, Game::SkillTargetTarget, true);
    if (skills.size() == 0)
        return ai::FAILED;

    if (useSkill(skills, selection[0]))
    {
        chr.currentTask_ = this;
        return ai::RUNNING;
    }
    return ai::FAILED;
}

}

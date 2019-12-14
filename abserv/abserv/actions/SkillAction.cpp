#include "stdafx.h"
#include "SkillAction.h"
#include "../Actor.h"
#include "Agent.h"
#include "../Game.h"
#include "../AiAgent.h"
#include "../Npc.h"
#include <AB/ProtocolCodes.h>

//#define DEBUG_AI

namespace AI {
namespace Actions {

bool SkillAction::TestSkill(int index, Game::Actor& source, Game::Actor* target)
{
    auto skill = source.skills_->GetSkill(index);
    if (!skill)
        return false;
    if (!source.resourceComp_->HaveEnoughResources(skill.get()))
        return false;
    if (skill->NeedsTarget())
    {
        if (!target)
            return false;
        if (target->IsDead())
        {
            if (!skill->HasEffect(Game::SkillEffectResurrect))
                return false;
            if (!source.IsAlly(target))
                return false;
        }
    }
    auto res = skill->CanUse(&source, target);
#ifdef DEBUG_AI
    if (res != AB::GameProtocol::SkillErrorNone)
        LOG_DEBUG << skill->data_.name << "::CanUse() returned " << static_cast<int>(res) << std::endl;
#endif
    // Out of range is okay we will move to the target
    return res == AB::GameProtocol::SkillErrorNone || res == AB::GameProtocol::SkillErrorOutOfRange;
}

int SkillAction::GetSkillIndex(std::vector<int>& candidates, Game::Actor& source, Game::Actor* target)
{
    int skillIndex = -1;
    for (int index : candidates)
    {
        if (TestSkill(index, source, target))
        {
            skillIndex = index;
            break;
        }
    }
    return skillIndex;
}

}

}

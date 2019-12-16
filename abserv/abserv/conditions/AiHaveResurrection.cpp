#include "stdafx.h"
#include "AiHaveResurrection.h"
#include "../AiAgent.h"
#include "../Npc.h"

namespace AI {
namespace Conditions {

bool HaveResurrection::Evaluate(Agent& agent, const Node&)
{
    Game::Npc& npc = GetNpc(agent);
    std::vector<int> skills;
    return npc.GetSkillCandidates(skills, Game::SkillEffectResurrect, Game::SkillTargetTarget);
}

}
}

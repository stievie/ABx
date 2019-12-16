#include "stdafx.h"
#include "AiHaveSkill.h"
#include "../AiAgent.h"
#include "../Npc.h"

namespace AI {
namespace Conditions {

HaveSkill::HaveSkill(const ArgumentsType& arguments) :
    Condition(arguments)
{
    if (arguments.size() > 0)
        effect_ = static_cast<Game::SkillEffect>(atoi(arguments[0].c_str()));
    if (arguments.size() > 1)
        effectTarget_ = static_cast<Game::SkillEffectTarget>(atoi(arguments[1].c_str()));
}

bool HaveSkill::Evaluate(Agent& agent, const Node&)
{
    Game::Npc& npc = GetNpc(agent);
    std::vector<int> skills;
    return npc.GetSkillCandidates(skills, effect_, effectTarget_);
}

}
}

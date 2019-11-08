#include "stdafx.h"
#include "AiIsInAOE.h"
#include "../Game.h"
#include "../Npc.h"
#include "../AreaOfEffect.h"

namespace AI {
namespace Conditions {

bool IsInAOE::Evaluate(Agent& agent)
{
    auto& npc = AI::GetNpc(agent);
    bool result = false;
    npc.VisitInRange(Game::Ranges::Aggro, [&result, &npc](const Game::GameObject& object)
    {
        if (Game::Is<Game::AreaOfEffect>(object))
        {
            const auto& aoe = Game::To<Game::AreaOfEffect>(object);
            if (aoe.IsEnemy(&npc) && aoe.HasEffect(Game::SkillEffectDamage) && aoe.IsInRange(&npc))
            {
                result = true;
                return Iteration::Break;
            }
        }
        return Iteration::Continue;
    });
    return result;
}

}
}

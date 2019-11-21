#include "stdafx.h"
#include "AiIsEnergyLow.h"
#include "../Game.h"
#include "../Npc.h"

namespace AI {
namespace Conditions {

bool IsEnergyLow::Evaluate(Agent& agent, const Node&)
{
    auto& npc = AI::GetNpc(agent);
    const auto& skillBar = *npc.skills_;
    int skillCount = 0;
    int sumEnergy = 0;
    // Get avg. skill cost
    skillBar.VisitSkills([&](int, const Game::Skill& current)
    {
        ++skillCount;
        sumEnergy += current.energy_;
        return Iteration::Continue;
    });

    if (skillCount > 0)
    {
        int avgEnergy = sumEnergy / skillCount;
        return npc.resourceComp_->GetEnergy() <= avgEnergy;
    }
    return npc.resourceComp_->GetEnergyRatio() <= LOW_ENERGY_THRESHOLD;
}

}
}

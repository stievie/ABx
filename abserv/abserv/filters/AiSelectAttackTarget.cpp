#include "stdafx.h"
#include "AiSelectAttackTarget.h"
#include "../Npc.h"
#include "Random.h"
#include "Subsystems.h"
#include "Utils.h"

namespace AI {
namespace Filters {

void SelectAttackTarget::Execute(Agent& agent)
{
    AgentIds candidates;
    Game::Npc& chr = GetNpc(agent);
    chr.VisitEnemiesInRange(Game::Ranges::Aggro, [&candidates](const Game::Actor* o)
    {
        candidates.push_back(o->id_);
    });

    auto& entities = agent.filteredAgents_;
    entities.clear();
    if (candidates.size() == 0)
        return;

    // For the moment randomly select one in range. But it should select a target
    // with low HP, low armor, close to us etc. So basically a target which is
    // easy to kill.
    auto* rnd = GetSubsystem<Crypto::Random>();
    const float rn = rnd->GetFloat();
    auto it = Utils::SelectRandomly(candidates.begin(), candidates.end(), rn);
    entities.push_back(*it);
}

}
}

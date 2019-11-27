#include "stdafx.h"
#include "AiIsSelectionAlive.h"
#include "../Game.h"
#include "../Npc.h"
#include "../GameObject.h"
#include "Agent.h"

namespace AI {
namespace Conditions {

bool IsSelectionAlive::Evaluate(Agent& agent, const Node&)
{
    const auto& selection = agent.filteredAgents_;
    if (selection.empty())
        return false;

    auto& npc = AI::GetNpc(agent);
    auto game = npc.GetGame();

    for (auto id : selection)
    {
        auto* sel = game->GetObject<Game::Actor>(id);
        if (sel)
        {
            if (sel->IsDead())
                return false;
        }
    }
    return true;
}

}
}

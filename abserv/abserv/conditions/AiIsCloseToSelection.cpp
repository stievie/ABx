#include "stdafx.h"
#include "AiIsCloseToSelection.h"
#include "../Game.h"
#include "../Npc.h"
#include "../GameObject.h"

namespace AI {
namespace Conditions {

IsCloseToSelection::IsCloseToSelection(const ArgumentsType& arguments) :
    Condition(arguments)
{
    if (arguments.size() == 1)
        distance_ = static_cast<float>(atof(arguments[0].c_str()));
}

bool IsCloseToSelection::Evaluate(Agent& agent, const Node&)
{
    const auto& selection = agent.filteredAgents_;
    if (selection.empty())
        return false;

    auto& npc = AI::GetNpc(agent);
    auto game = npc.GetGame();

    const auto& ownPos = npc.GetPosition();
    for (auto id : selection)
    {
        auto sel = game->GetObjectById(id);
        const Math::Vector3& _pos = sel->GetPosition();
        const float distance = ownPos.Distance(_pos);
        if (distance > distance_)
            return false;
    }
    return true;
}

}
}

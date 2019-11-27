#include "stdafx.h"
#include "AiAttackSelection.h"
#include "../Npc.h"
#include "../AiAgent.h"

namespace AI {
namespace Actions {

Node::Status AttackSelection::DoAction(Agent& agent, uint32_t)
{
    Game::Npc& npc = GetNpc(agent);

    const auto& selection = agent.filteredAgents_;
    if (selection.empty())
    {
        return Status::Failed;
    }
    for (auto id : selection)
    {
        auto* target = npc.GetGame()->GetObject<Game::Actor>(id);
        if (!target)
            continue;

        if (target->IsDead())
            return Status::Finished;
        if (npc.attackComp_->IsAttackingTarget(target))
            return Status::Running;

        if (npc.AttackById(id))
        {
            if (npc.GetSpeed() < 1.0f)
                npc.SetSpeed(1.0f);
            return Status::Running;
        }
    }
    return Status::Finished;
}

AttackSelection::AttackSelection(const ArgumentsType& arguments) :
    Action(arguments)
{
}

}
}

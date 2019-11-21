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
        auto target = npc.GetGame()->GetObjectById(id);
        if (!target)
            continue;
        if (!target->IsActorType())
            continue;

        const Game::Actor& actor = Game::To<Game::Actor>(*target);
        if (actor.IsDead())
            return Status::Finished;
        if (npc.attackComp_->IsAttackingTarget(&actor))
            return Status::Running;

        if (npc.AttackById(id))
            return Status::Running;
    }
    return Status::Finished;
}

AttackSelection::AttackSelection(const ArgumentsType& arguments) :
    Action(arguments)
{
}

}
}

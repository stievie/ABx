#include "stdafx.h"
#include "Agent.h"
#include "HasSelection.h"

namespace AI {
namespace Conditions {

bool HasSelection::Evaluate(Agent& agent, const Node&)
{
    return agent.filteredAgents_.size() != 0;
}

}

}

#include "stdafx.h"
#include "AiIdle.h"

namespace AI {
namespace Actions {

Node::Status Idle::ExecuteStart(Agent& agent, uint32_t timeElapsed)
{
    // TODO: Play some random animations
    return TimedNode::ExecuteStart(agent, timeElapsed);
}

}
}

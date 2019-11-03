#include "stdafx.h"
#include "Action.h"

namespace AI {

Action::Action(const NodeFactoryContext& ctx) :
    Node(ctx)
{ }

Action::~Action() = default;

Node::Status Action::Execute(Agent& agent, uint32_t timeElapsed)
{
    if (Node::Execute(agent, timeElapsed) == Status::CanNotExecute)
        return Status::CanNotExecute;
    return DoAction(agent, timeElapsed);
}

}

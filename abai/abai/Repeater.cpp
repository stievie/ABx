#include "stdafx.h"
#include "Repeater.h"

namespace AI {

Node::Status AI::Repeater::Execute(Agent& agent, uint32_t timeElapsed)
{
    if (Node::Execute(agent, timeElapsed) == Node::Status::CanNotExecute)
        return Node::Status::CanNotExecute;
    if (!child_)
        return Node::Status::CanNotExecute;
    child_->Execute(agent, timeElapsed);
    return Status::Running;
}

}

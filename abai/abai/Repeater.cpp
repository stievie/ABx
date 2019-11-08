#include "stdafx.h"
#include "Repeater.h"

namespace AI {

Node::Status AI::Repeater::Execute(Agent& agent, uint32_t timeElapsed)
{
    if (Node::Execute(agent, timeElapsed) == Node::Status::CanNotExecute)
        return ReturnStatus(Node::Status::CanNotExecute);
    if (!child_)
        return ReturnStatus(Node::Status::CanNotExecute);
    child_->Execute(agent, timeElapsed);
    return ReturnStatus(Status::Running);
}

}

#include "stdafx.h"
#include "UntilFail.h"

namespace AI {

Node::Status UntilFail::Execute(Agent & agent, uint32_t timeElapsed)
{
    if (Node::Execute(agent, timeElapsed) == Node::Status::CanNotExecute)
        return ReturnStatus(Node::Status::CanNotExecute);
    if (!child_)
        return ReturnStatus(Node::Status::CanNotExecute);
    auto status = child_->Execute(agent, timeElapsed);
    if (status == Status::Failed)
        return ReturnStatus(Status::Finished);
    return ReturnStatus(Status::Running);
}

}

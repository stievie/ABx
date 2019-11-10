#include "stdafx.h"
#include "UntilFail.h"

namespace AI {

Node::Status UntilFail::Execute(Agent & agent, uint32_t timeElapsed)
{
    if (Node::Execute(agent, timeElapsed) == Node::Status::CanNotExecute)
        return Node::Status::CanNotExecute;
    if (!child_)
        return Node::Status::CanNotExecute;
    auto status = child_->Execute(agent, timeElapsed);
    if (status == Status::Failed)
        return Status::Finished;
    return Status::Running;
}

}

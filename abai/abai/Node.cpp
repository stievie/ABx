#include "stdafx.h"
#include "Node.h"
#include "Agent.h"

namespace AI {

sa::IdGenerator<Id> Node::sIds_;

Node::Node(const ArgumentsType&) :
    id_(sIds_.Next())
{ }

Node::~Node() = default;

bool Node::AddNode(std::shared_ptr<Node>)
{
    return false;
}

void Node::SetCondition(std::shared_ptr<Condition> condition)
{
    condition_ = condition;
}

Node::Status Node::Execute(Agent& agent, uint32_t)
{
    if (status_ != Status::Running)
        Initialize();

    if (condition_ && !condition_->Evaluate(agent))
        return ReturnStatus(Status::CanNotExecute);
    return ReturnStatus(Status::Finished);
}

}

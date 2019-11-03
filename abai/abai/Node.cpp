#include "stdafx.h"
#include "Node.h"
#include "Agent.h"

namespace AI {

Node::Node(const NodeFactoryContext& ctx) :
    condition_(ctx.condition),
    id_(sIds_.Next())
{ }

Node::~Node() = default;

bool Node::AddChild(std::shared_ptr<Node>)
{
    return false;
}

void Node::SetCondition(std::shared_ptr<Condition> condition)
{
    condition_ = condition;
}

Node::Status Node::Execute(Agent& agent, uint32_t)
{
    if (condition_ && !condition_->Evaluate(agent))
        return Status::CanNotExecute;
    return Status::Finished;
}

}

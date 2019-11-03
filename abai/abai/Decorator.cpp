#include "stdafx.h"
#include "Decorator.h"

namespace AI {

Decorator::Decorator(const NodeFactoryContext& ctx) :
    Node(ctx)
{ }

void Decorator::Initialize()
{
    child_->Initialize();
}

bool Decorator::AddChild(std::shared_ptr<Node> node)
{
    if (child_)
        return false;
    child_ = node;
    return true;
}

}

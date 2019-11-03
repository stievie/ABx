#include "stdafx.h"
#include "Composite.h"

namespace AI {

Composite::Composite(const NodeFactoryContext& ctx) :
    Node(ctx)
{ }

bool Composite::AddChild(std::shared_ptr<Node> node)
{
    children_.push_back(node);
    return true;
}

}

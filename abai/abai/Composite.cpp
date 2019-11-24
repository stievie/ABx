#include "stdafx.h"
#include "Composite.h"

namespace AI {

Composite::Composite(const ArgumentsType& arguments) :
    Node(arguments)
{ }

bool Composite::AddNode(std::shared_ptr<Node> node)
{
    if (!node)
        return false;
    children_.push_back(node);
    return true;
}

void Composite::VisitChildren(const std::function<Iteration (const Node&)>& callback) const
{
    for (const auto& nd : children_)
        if (callback(*nd) != Iteration::Continue)
            break;
}

}

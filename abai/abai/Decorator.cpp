#include "stdafx.h"
#include "Decorator.h"

namespace AI {

Decorator::Decorator(const ArgumentsType& arguments) :
    Node(arguments)
{ }

bool Decorator::AddNode(std::shared_ptr<Node> node)
{
    if (!node)
        return false;
    if (child_)
        return false;
    child_ = node;
    return true;
}

void Decorator::VisitChildren(const std::function<Iteration (const Node&)>& callback) const
{
    if (child_)
        callback(*child_);
}

}

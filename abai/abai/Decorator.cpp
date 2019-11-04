#include "stdafx.h"
#include "Decorator.h"

namespace AI {

Decorator::Decorator(const ArgumentsType& arguments) :
    Node(arguments)
{ }

bool Decorator::AddNode(std::shared_ptr<Node> node)
{
    if (child_)
        return false;
    child_ = node;
    return true;
}

}

#pragma once

#include "Node.h"

namespace AI {

// A node with one or more children
class Composite : public Node
{
protected:
    Nodes children_;
public:
    explicit Composite(const ArgumentsType& arguments);
    bool AddNode(std::shared_ptr<Node> node) override;
};

}

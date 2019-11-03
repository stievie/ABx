#pragma once

#include "Node.h"

namespace AI {

// A node with one or more children
class Composite : public Node
{
protected:
    Nodes children_;
public:
    explicit Composite(const NodeFactoryContext& ctx);
    bool AddChild(std::shared_ptr<Node> node) override;
};

}

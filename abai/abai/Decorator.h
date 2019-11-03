#pragma once

#include "Node.h"

namespace AI {

// A node with exactly one child
class Decorator : public Node
{
protected:
    std::shared_ptr<Node> child_;
public:
    explicit Decorator(const NodeFactoryContext& ctx);
    void Initialize() override;
    bool AddChild(std::shared_ptr<Node> node) override;
};

}

#pragma once

#include "Node.h"

namespace AI {

// A node with exactly one child
class Decorator : public Node
{
protected:
    std::shared_ptr<Node> child_;
public:
    explicit Decorator(const ArgumentsType& arguments);
    bool AddNode(std::shared_ptr<Node> node) override;
    void VisitChildren(const std::function<Iteration(const Node&)>&) const override;
};

}

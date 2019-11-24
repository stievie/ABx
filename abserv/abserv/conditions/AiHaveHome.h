#pragma once

#include "Condition.h"

namespace AI {
namespace Conditions {

class HaveHome final : public Condition
{
    CONDITON_CLASS(HaveHome)
public:
    explicit HaveHome(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(Agent& agent, const Node& node) override;
};

}
}

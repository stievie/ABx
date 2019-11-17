#pragma once

#include "Condition.h"

namespace AI {
namespace Conditions {

class HaveHome : public Condition
{
public:
    CONDITON_CLASS(HaveHome)
    explicit HaveHome(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(Agent& agent, const Node& node) override;
};

}
}

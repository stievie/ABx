#pragma once

#include "Condition.h"

namespace AI {
namespace Conditions {

class HaveWanderRoute : public Condition
{
    CONDITON_CLASS(HaveWanderRoute)
public:
    explicit HaveWanderRoute(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(Agent& agent, const Node& node) override;
};

}
}

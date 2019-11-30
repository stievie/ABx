#pragma once

#include "Condition.h"

namespace AI {
namespace Conditions {

class IsFighting final : public Condition
{
    CONDITON_CLASS(IsFighting)
public:
    explicit IsFighting(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(Agent& agent, const Node& node) override;
};

}
}

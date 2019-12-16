#pragma once

#include "Condition.h"

namespace AI {
namespace Conditions {

class HaveResurrection final : public Condition
{
    CONDITON_CLASS(HaveResurrection)
public:
    explicit HaveResurrection(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(Agent& agent, const Node& node) override;
};

}
}

#pragma once

#include "Condition.h"

namespace AI {
namespace Conditions {

// Evaluates to true when the NPC has a weapon equipped.
class HaveWeapon final : public Condition
{
    CONDITON_CLASS(HaveWeapon)
public:
    explicit HaveWeapon(const ArgumentsType& arguments) :
        Condition(arguments)
    { }
    bool Evaluate(Agent& agent, const Node& node) override;
};

}
}

#pragma once

#include "Condition.h"

namespace AI {
namespace Conditions {

class RandomCondition final : public Condition
{
    CONDITON_CLASS(RandomCondition)
private:
    float weight_{ 1.0f };
public:
    explicit RandomCondition(const ArgumentsType& arguments);
    bool Evaluate(Agent&, const Node&) override;
};

}
}

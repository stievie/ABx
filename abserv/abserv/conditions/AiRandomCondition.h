#pragma once

#include "Condition.h"

namespace AI {
namespace Conditions {

class RandomCondition : public Condition
{
private:
    float weight_{ 1.0f };
public:
    CONDITON_CLASS(RandomCondition)
    explicit RandomCondition(const ArgumentsType& arguments);
    bool Evaluate(Agent&) override;
};

}
}

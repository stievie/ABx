#include "stdafx.h"
#include "AiRandomCondition.h"
#include "Random.h"
#include "Subsystems.h"

namespace AI {
namespace Conditions {

RandomCondition::RandomCondition(const ArgumentsType& arguments) :
    Condition(arguments)
{
    if (arguments.size() != 0)
        weight_ = static_cast<float>(atof(arguments[0].c_str()));
}

bool RandomCondition::Evaluate(Agent&)
{
    auto* rnd = GetSubsystem<Crypto::Random>();
    float rand = rnd->GetFloat() * weight_;
    return rand > 0.5f;
}

}
}

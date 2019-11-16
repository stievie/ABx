#pragma once

#include "Filter.h"
#include <AB/Entities/Effect.h>

namespace AI {
namespace Filters {

// Select friend|foe|all which have an effect of the effect category.
//
// 2 arguments:
// 1. EffectCategory name as defined in EffectManager.h
// 2. Optional friend|foe|all (default)
class SelectWithEffect : public Filter
{
private:
    enum class TargetClass
    {
        All,
        Friend,
        Foe
    };
    TargetClass class_;
    AB::Entities::EffectCategory effectCat_;
public:
    FILTER_CLASS(SelectWithEffect)
    explicit SelectWithEffect(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}


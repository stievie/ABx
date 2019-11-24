#pragma once

#include "Filter.h"
#include <AB/Entities/Effect.h>
#include "../Actor.h"

namespace AI {
namespace Filters {

// Select friend|foe|all which have an effect of the effect category.
//
// 2 Arguments:
//  1. EffectCategory name as defined in EffectManager.h
//  2. Optional friend|foe|all (default)
class SelectWithEffect final : public Filter
{
    FILTER_CLASS(SelectWithEffect)
private:
    Game::TargetClass class_{ Game::TargetClass::All };
    AB::Entities::EffectCategory effectCat_{ AB::Entities::EffectNone };
public:
    explicit SelectWithEffect(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}

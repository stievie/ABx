#pragma once

#include "Filter.h"
#include "../Damage.h"
#include "../Actor.h"

namespace AI {
namespace Filters {

// Select actors that got damage of certain type.
class SelectGettingDamage final : public Filter
{
    FILTER_CLASS(SelectGettingDamage)
private:
    Game::TargetClass class_{ Game::TargetClass::All };
    Game::DamageTypeCategory category_{ Game::DamageTypeCategory::Any };
public:
    explicit SelectGettingDamage(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}

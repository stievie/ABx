#pragma once

#include "Filter.h"
#include "../Mechanic.h"
#include "../Actor.h"

namespace AI {
namespace Filters {

// Select all objects that are visible (not obstructed) to the owner.
class SelectVisible final : public Filter
{
    FILTER_CLASS(SelectVisible)
private:
    Game::Ranges range_{ Game::Ranges::Aggro };
    Game::TargetClass class_{ Game::TargetClass::All };
public:
    explicit SelectVisible(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}

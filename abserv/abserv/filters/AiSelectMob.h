#pragma once

#include "Filter.h"
#include "../Mechanic.h"

namespace AI {
namespace Filters {

// Select the foe with most other foes around
class SelectMob final : public Filter
{
    FILTER_CLASS(SelectMob)
private:
    Game::Ranges range_{ Game::Ranges::Adjecent };
public:
    explicit SelectMob(const ArgumentsType& arguments) :
        Filter(arguments)
    {
        if (arguments.size() > 0)
            range_ = static_cast<Game::Ranges>(atoi(arguments[0].c_str()));
    }
    void Execute(Agent& agent) override;
};

}
}

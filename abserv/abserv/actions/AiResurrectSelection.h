#pragma once

#include "Action.h"

namespace AI {
namespace Actions {

class ResurrectSelection : public Action
{
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    NODE_CLASS(ResurrectSelection)
    explicit ResurrectSelection(const ArgumentsType& arguments) :
        Action(arguments)
    { }
};

}
}

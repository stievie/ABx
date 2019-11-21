#pragma once

#include "Action.h"

namespace AI {
namespace Actions {

class ResurrectSelection : public Action
{
    NODE_CLASS(ResurrectSelection)
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit ResurrectSelection(const ArgumentsType& arguments) :
        Action(arguments)
    {
        mustComplete_ = true;
    }
};

}
}

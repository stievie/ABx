#pragma once

#include "Action.h"

namespace AI {
namespace Actions {

class Follow final : public Action
{
    NODE_CLASS(Follow)
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit Follow(const ArgumentsType& arguments) :
        Action(arguments)
    { }
};

}
}

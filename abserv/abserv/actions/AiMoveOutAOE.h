#pragma once

#include "Action.h"
#include "Vector3.h"

namespace AI {
namespace Actions {

class MoveOutAOE : public Action
{
    NODE_CLASS(MoveOutAOE)
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit MoveOutAOE(const ArgumentsType& arguments) :
        Action(arguments)
    { }
};

}
}

#pragma once

#include "Action.h"
#include "Vector3.h"

namespace AI {
namespace Actions {

class MoveOutAOE : public Action
{
private:
    Math::Vector3 destination_;
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    NODE_FACTORY(MoveOutAOE)
    explicit MoveOutAOE(const ArgumentsType& arguments) :
        Action(arguments)
    { }
};

}
}

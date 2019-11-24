#pragma once

#include "Action.h"

namespace AI {
namespace Actions {

class MoveTo final : public Action
{
    NODE_CLASS(MoveTo)
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit MoveTo(const ArgumentsType& arguments) :
        Action(arguments)
    { }
};

}
}

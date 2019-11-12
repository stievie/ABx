#pragma once

#include "Action.h"
#include "Vector3.h"

namespace AI {
namespace Actions {

class Flee : public Action
{
private:
    Math::Vector3 destination_;
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    NODE_CLASS(Flee)
    explicit Flee(const ArgumentsType& arguments) :
        Action(arguments)
    { }
};

}
}

#pragma once

#include "Action.h"
#include "Vector3.h"

namespace AI {
namespace Actions {

class Flee : public Action
{
    NODE_CLASS(Flee)
private:
    Math::Vector3 destination_;
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit Flee(const ArgumentsType& arguments) :
        Action(arguments)
    { }
};

}
}

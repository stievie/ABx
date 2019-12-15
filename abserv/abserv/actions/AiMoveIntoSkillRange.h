#pragma once

#include "Action.h"

namespace AI {
namespace Actions {

class MoveIntoSkillRange final : public Action
{
    NODE_CLASS(MoveIntoSkillRange)
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit MoveIntoSkillRange(const ArgumentsType& arguments) :
        Action(arguments)
    { }
};

}

}


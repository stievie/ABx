#pragma once

#include "Action.h"
#include "../Npc.h"
#include "../Game.h"

namespace AI {
namespace Actions {

class AttackSelection : public Action
{
    NODE_CLASS(AttackSelection)
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit AttackSelection(const ArgumentsType& arguments);
};

}
}

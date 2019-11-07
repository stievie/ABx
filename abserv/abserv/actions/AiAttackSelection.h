#pragma once

#include "Action.h"
#include "../Npc.h"
#include "../Game.h"

namespace AI {
namespace Actions {

class AttackSelection : public Action
{
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    NODE_FACTORY(AttackSelection)
    explicit AttackSelection(const ArgumentsType& arguments);
};

}
}

#pragma once

#include "Action.h"
#include "Vector3.h"

namespace Game {
class Npc;
class AreaOfEffect;
}

namespace AI {
namespace Actions {

class MoveOutAOE final : public Action
{
    NODE_CLASS(MoveOutAOE)
private:
    bool TryMove(Game::Npc& npc, Game::AreaOfEffect& damager, Math::Vector3& destination);
protected:
    Status DoAction(Agent& agent, uint32_t timeElapsed) override;
public:
    explicit MoveOutAOE(const ArgumentsType& arguments) :
        Action(arguments)
    { }
};

}
}

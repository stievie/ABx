#pragma once

#include "Action.h"

namespace Game {
class Actor;
}

namespace AI {
namespace Actions {

class SkillAction : public Action
{
protected:
    bool TestSkill(int index, Game::Actor& source, Game::Actor* target);
    int GetSkillIndex(std::vector<int>& candidates, Game::Actor& source, Game::Actor* target);
public:
    explicit SkillAction(const ArgumentsType& arguments) :
        Action(arguments)
    {
        mustComplete_ = true;
    }
};

}
}

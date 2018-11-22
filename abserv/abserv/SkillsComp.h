#pragma once

namespace Game {

class Actor;

namespace Components {

class SkillsComp
{
private:
    Actor& owner_;
public:
    explicit SkillsComp(Actor& owner) :
        owner_(owner)
    { }
    ~SkillsComp() = default;
    void Update(uint32_t timeElapsed);
};

}
}

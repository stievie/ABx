#pragma once

namespace Game {

class Actor;

namespace Components {

class CollisionComp
{
private:
    Actor& owner_;
public:
    CollisionComp(Actor& owner) :
        owner_(owner)
    { }
    ~CollisionComp() = default;

    void DoCollisions();
};

}
}

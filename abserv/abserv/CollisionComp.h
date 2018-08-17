#pragma once

namespace Game {

class Creature;

namespace Components {

class CollisionComp
{
private:
    Creature& owner_;
public:
    CollisionComp(Creature& owner) :
        owner_(owner)
    { }
    ~CollisionComp() = default;

    void DoCollisions();
};

}
}

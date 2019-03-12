#pragma once

namespace Game {

class Actor;

namespace Components {

class CollisionComp
{
private:
    Actor& owner_;
public:
    CollisionComp() = delete;
    explicit CollisionComp(Actor& owner) :
        owner_(owner)
    { }
    // non-copyable
    CollisionComp(const CollisionComp&) = delete;
    CollisionComp& operator=(const CollisionComp&) = delete;
    ~CollisionComp() = default;

    void DoCollisions();
};

}
}

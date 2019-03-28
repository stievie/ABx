#pragma once

namespace Game {

class Actor;

namespace Components {

/// Only an Actor can have a CollisionComp, because only moving objects need it.
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

    void Update(uint32_t timeElapsed);
    void ResolveCollisions();
};

}
}

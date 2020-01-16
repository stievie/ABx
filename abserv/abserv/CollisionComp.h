#pragma once

#include <stdint.h>
#include <sa/Iteration.h>
#include <sa/Noncopyable.h>

namespace Math {
class BoundingBox;
class Vector3;
}

namespace Game {

class Actor;
class GameObject;

namespace Components {

/// Only an Actor can have a CollisionComp, because only moving objects need it.
class CollisionComp
{
    NON_COPYABLE(CollisionComp)
private:
    Actor& owner_;
    bool isCollidingWithPlayers_;
    Iteration CollisionCallback(const Math::BoundingBox& myBB, GameObject& other, const Math::Vector3& move, bool& updateTrans);
    bool Slide(const Math::BoundingBox& myBB, const GameObject& other);
    void GotoSafePosition();
    static Math::Vector3 GetBodyCenter(const Math::Vector3& pos);
public:
    CollisionComp() = delete;
    explicit CollisionComp(Actor& owner) :
        owner_(owner)
    { }
    ~CollisionComp() = default;

    void Update(uint32_t timeElapsed);
    void ResolveCollisions();
};

}
}

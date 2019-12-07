#include "stdafx.h"
#include "CollisionComp.h"
#include "Actor.h"
#include "Game.h"
#include "MoveComp.h"
#include "BoundingBox.h"
#include "VectorMath.h"
#include "Plane.h"

#define DEBUG_COLLISION

namespace Game {
namespace Components {

static constexpr float UnitScale = 1.0f / 100.0f;
static constexpr float CloseDistance = 0.005f * UnitScale;

bool CollisionComp::Slide(const Math::BoundingBox& myBB, const GameObject& other)
{
    AB_PROFILE;
    MoveComp& mc = *owner_.moveComp_;
    const Math::Vector3& oldPos = mc.GetOldPosition();

    // That's us
    Math::CollisionManifold manifold;
    manifold.velocity = mc.velocity_;
    // Center of body
    manifold.position = GetBodyCenter(oldPos);
    manifold.radius = myBB.Extends();
    // Since we collide with `other` let's get some more information from `other`
    bool foundSolution = other.GetCollisionShape()->GetManifold(manifold, other.transformation_.GetMatrix());
    if (!foundSolution || manifold.stuck)
    {
#ifdef DEBUG_COLLISION
        if (!foundSolution)
            LOG_DEBUG << "No solution found ";
        if (manifold.stuck)
            LOG_DEBUG << "Stuck! ";
        LOG_DEBUG << "going back to " << oldPos.ToString() << std::endl;
#endif
        GotoSafePosition();
        owner_.CallEvent<void(void)>(EVENT_ON_STUCK);
        return false;
    }

    const Math::Vector3 destinationPoint = owner_.transformation_.position_;
    if (manifold.distance >= CloseDistance)
    {
        Math::Vector3 V = mc.velocity_;
        V.SetLength(manifold.distance - CloseDistance);
        V.Normalize();
        manifold.intersectionPoint -= CloseDistance * V;
    }

    const Math::Vector3 slidingPlaneOrigin = manifold.intersectionPoint;
    const Math::Vector3 slidingPlaneNormal = (oldPos - manifold.intersectionPoint).Normal();
    const Math::Plane slidingPlane(slidingPlaneNormal, slidingPlaneOrigin);

    const Math::Vector3 newDestinationPoint = destinationPoint - slidingPlane.Distance(destinationPoint) * slidingPlaneNormal;
    const Math::Vector3 newVelocityVector = newDestinationPoint - manifold.intersectionPoint;

    const Math::Vector3 newPos = oldPos + newVelocityVector;

#ifdef DEBUG_COLLISION
    LOG_DEBUG << "Sliding from " << oldPos.ToString() << " to "
        << newPos.ToString() <<
        " intersection at " << manifold.intersectionPoint.ToString() << std::endl;
#endif

    // TODO:
#if 1
    owner_.transformation_.position_ = newPos;
    mc.StickToGround();
    mc.moved_ = true;
    mc.forcePosition_ = true;
    return true;
#else
    (void)newPos;
    return false;
#endif
}

void CollisionComp::GotoSafePosition()
{
    MoveComp& mc = *owner_.moveComp_;
    owner_.transformation_.position_ = mc.GetOldPosition();
    mc.moved_ = false;
}

Math::Vector3 CollisionComp::GetBodyCenter(const Math::Vector3& pos)
{
    return pos + GameObject::BodyOffset;
}

Iteration CollisionComp::CollisionCallback(const Math::BoundingBox& myBB,
    GameObject& other, const Math::Vector3& move,
    bool& updateTrans)
{
    if (!isCollidingWithPlayers_ && (other.GetType() == AB::GameProtocol::ObjectTypePlayer))
        return Iteration::Continue;
    if (Is<Actor>(other))
    {
        // Not colliding with dead actors
        if (To<Actor>(other).IsDead())
            return Iteration::Continue;
    }

    if ((other.collisionMask_ != 0) && (owner_.collisionMask_ != 0))
    {
        // Don't move the character when the object actually does not collide,
        // but we may still need the trigger stuff.

        // There was no simple sliding solution, let's try the complicated one
        if (move.Equals(Math::Vector3::Zero))
        {
            updateTrans = Slide(myBB, other);
        }
        else
        {
            // There was a simple solution, e.g. an AABB can do that
            owner_.transformation_.position_ += move;
            updateTrans = true;
        }
    }

    // Need to notify both, because we test collisions only for moving objects
    // Notify ci for colliding with us
    other.CallEvent<void(GameObject*)>(EVENT_ON_COLLIDE, &owner_);
    // Notify us for colliding with ci
    owner_.CallEvent<void(GameObject*)>(EVENT_ON_COLLIDE, &other);

    return Iteration::Continue;
}

void CollisionComp::ResolveCollisions()
{
    // Players don't collide with other players in outposts.
    // GetType() is virtual so call it just once.
    isCollidingWithPlayers_ = (owner_.GetType() != AB::GameProtocol::ObjectTypePlayer) ||
        !AB::Entities::IsOutpost(owner_.GetGame()->data_.type);

    // Actor always has a MoveComp
    MoveComp& mc = *owner_.moveComp_;
    std::vector<GameObject*> c;
    const Math::BoundingBox box = owner_.GetWorldBoundingBox();
    if (owner_.QueryObjects(c, box))
    {
        if (c.size() == 0)
            return;

        owner_.Collides(&c[0], c.size(), mc.velocity_,
            std::bind(&CollisionComp::CollisionCallback, this, box,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }
}

void CollisionComp::Update(uint32_t)
{
    assert(owner_.moveComp_);
    if (owner_.moveComp_->moved_)
        ResolveCollisions();
}

}
}

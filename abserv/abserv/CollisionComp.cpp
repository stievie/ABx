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

void CollisionComp::Slide(const Math::BoundingBox& myBB, GameObject& other)
{
    AB_PROFILE;
    MoveComp& mc = *owner_.moveComp_;
    // That's us
    Math::CollisionManifold manifold;
    manifold.velocity = mc.velocity_;
    // Center of body
    manifold.position = GetBodyCenter(mc.GetOldPosition());
    manifold.radius = myBB.Extends();
    // Since we collide with `other` let's get some more information from `other`
    bool foundSolution = other.GetCollisionShape()->GetManifold(manifold, other.transformation_.GetMatrix());
    if (!foundSolution || manifold.stuck)
    {
        GotoSafePosition();
#ifdef DEBUG_COLLISION
        if (!foundSolution)
            LOG_DEBUG << "No solution found ";
        if (manifold.stuck)
            LOG_DEBUG << "Stuck!";
        LOG_DEBUG << std::endl;
#endif
        return;
    }

    Math::Vector3 destinationPoint = owner_.transformation_.position_;
    Math::Vector3 newBasePoint = mc.GetOldPosition();
    if (manifold.distance >= 0.005f)
    {
        Math::Vector3 V = mc.velocity_;
        V.SetLength(manifold.distance - 0.005f);
        V.Normalize();
        manifold.intersectionPoint -= 0.005f * V;
    }

    Math::Vector3 slidingPlaneOrigin = manifold.intersectionPoint;
    Math::Vector3 slidingPlaneNormal = (newBasePoint - manifold.intersectionPoint).Normal();
    Math::Plane slidingPlane(slidingPlaneNormal, slidingPlaneOrigin);

    Math::Vector3 newDestinationPoint = destinationPoint - slidingPlane.Distance(destinationPoint) * slidingPlaneNormal;
    Math::Vector3 newVelocityVector = newDestinationPoint - manifold.intersectionPoint;

    Math::Quaternion rot = Math::Quaternion::FromAxisAngle(Math::Vector3::UnitY, other.transformation_.GetYRotation());
    const Math::Vector3 newPos = owner_.transformation_.position_ + (rot * (newVelocityVector / 2.0f));

#ifdef DEBUG_COLLISION
    LOG_DEBUG << "Sliding from " << mc.GetOldPosition().ToString() << " to "
        << newPos.ToString() <<
        " intersection " << manifold.intersectionPoint.ToString() << std::endl;
#endif

    owner_.transformation_.position_ = newPos;
    mc.StickToGround();
    mc.moved_ = true;
    mc.forcePosition_ = true;
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

void CollisionComp::ResolveOne(const Math::BoundingBox& myBB, GameObject& other)
{
    // Actor always has a MoveComp
    MoveComp& mc = *owner_.moveComp_;
    Math::Vector3 move;
    if (owner_.Collides(&other, mc.velocity_, move))
    {
        if ((other.collisionMask_ != 0) && (owner_.collisionMask_ != 0))
        {
            // Don't move the character when the object actually does not collide,
            // but we may still need the trigger stuff.

            // There was no simple sliding solution, let's try the complicated one
            if (move.Equals(Math::Vector3::Zero))
                Slide(myBB, other);
            else
                // There was a simple solution, e.g. an AABB can do that
                owner_.transformation_.position_ += move;
        }

        // Need to notify both, because we test collisions only for moving objects
        // Notify ci for colliding with us
        other.CallEvent<void(GameObject*)>(EVENT_ON_COLLIDE, &owner_);
        // Notify us for colliding with ci
        owner_.CallEvent<void(GameObject*)>(EVENT_ON_COLLIDE, &other);
    }
}

void CollisionComp::ResolveCollisions()
{
    // Players don't collide with other players in outposts
    const bool isCollidingWithPlayers = (owner_.GetType() != AB::GameProtocol::ObjectTypePlayer) ||
        !AB::Entities::IsOutpost(owner_.GetGame()->data_.type);

    std::vector<GameObject*> c;
    const Math::BoundingBox box = owner_.GetWorldBoundingBox();
    if (owner_.QueryObjects(c, box))
    {
        for (auto& ci : c)
        {
            if (ci != &owner_)
            {
                if (!isCollidingWithPlayers && (ci->GetType() == AB::GameProtocol::ObjectTypePlayer))
                    // In outposts players don't collide with players
                    continue;
                ResolveOne(box, *ci);
            }
        }
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

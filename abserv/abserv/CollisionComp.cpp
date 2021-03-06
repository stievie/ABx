/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "CollisionComp.h"
#include "Actor.h"
#include "Game.h"
#include "MoveComp.h"
#include <absmath/Plane.h>
#include <sa/Assert.h>
#include <absmath/VectorMath.h>

//#define DEBUG_COLLISION

namespace Game {
namespace Components {

static constexpr float UnitScale = 1.0f / 100.0f;
static constexpr float CloseDistance = 0.005f * UnitScale;

bool CollisionComp::Slide(const Math::BoundingBox& myBB, const GameObject& other)
{
//    AB_PROFILE;
    MoveComp& mc = *owner_.moveComp_;
    // We try to go from last safe position to Actors current position, because we do this after moving
    const Math::Vector3& oldPos = mc.GetOldPosition();
    const Math::Vector3 destinationPoint = owner_.GetPosition();
    const Math::Vector3 velocity = oldPos - destinationPoint;

    Math::CollisionManifold manifold;
    manifold.radius = myBB.Extends();
    manifold.velocity = velocity / manifold.radius;
    manifold.position = oldPos / manifold.radius;
    // Since we collide with `other` let's get some more information from `other`
    bool foundSolution = other.GetCollisionShape()->GetManifold(manifold, other.transformation_.GetMatrix());
    if (!manifold.foundCollision)
    {
#ifdef DEBUG_COLLISION
        LOG_DEBUG << "No collisions found" << std::endl;
#endif
        return true;
    }

#ifdef DEBUG_COLLISION
    LOG_DEBUG << "Manifold:" << std::endl;
    LOG_DEBUG << "  position: " << manifold.position * manifold.radius << std::endl;
    LOG_DEBUG << "  velocity: " << manifold.velocity * manifold.radius << std::endl;
    LOG_DEBUG << "  radius: " << manifold.radius << std::endl;

    LOG_DEBUG << "  nearestIntersectionPoint: " << manifold.nearestIntersectionPoint * manifold.radius << std::endl;
    LOG_DEBUG << "  nearestPolygonIntersectionPoint: " << manifold.nearestPolygonIntersectionPoint * manifold.radius << std::endl;
    LOG_DEBUG << "  nearestDistance: " << manifold.nearestDistance << std::endl;
    LOG_DEBUG << "  stuck: " << manifold.stuck << std::endl;
    LOG_DEBUG << "  foundCollision: " << manifold.foundCollision << std::endl;
#endif

    if (!foundSolution || manifold.stuck)
    {
#ifdef DEBUG_COLLISION
        if (!foundSolution)
            LOG_DEBUG << "No solution found ";
        if (manifold.stuck)
            LOG_DEBUG << "Stuck! ";
        LOG_DEBUG << "Going back to " << mc.GetSafePosition() << std::endl;
#endif
        GotoSafePosition();
        owner_.CallEvent<void(void)>(EVENT_ON_STUCK);
        return false;
    }

    const Math::Vector3 eDest = destinationPoint / manifold.radius;

    Math::Vector3 newSourcePoint;
    if (manifold.nearestDistance >= CloseDistance)
    {
        Math::Vector3 V = velocity;
        V.SetLength(manifold.nearestDistance - CloseDistance);
        V.Normalize();
        newSourcePoint = manifold.position + V;
    }
    else
        newSourcePoint = manifold.position;

    const Math::Vector3 slidingPlaneOrigin = manifold.nearestPolygonIntersectionPoint;
    const Math::Vector3 slidingPlaneNormal = (newSourcePoint - manifold.nearestPolygonIntersectionPoint).Normal();
    const float l = Math::IntersectsRayPlane(eDest, slidingPlaneNormal, slidingPlaneOrigin, slidingPlaneNormal);
    const Math::Vector3 newDestinationPoint = eDest - l * slidingPlaneNormal;

#ifdef DEBUG_COLLISION
    LOG_DEBUG << "Sliding from " << oldPos << " to "
        << newDestinationPoint * manifold.radius <<
        " intersection at " << manifold.nearestPolygonIntersectionPoint * manifold.radius << std::endl;
#endif

    owner_.transformation_.position_ = newDestinationPoint * manifold.radius;
    mc.StickToGround();
    mc.moved_ = true;
    mc.forcePosition_ = true;
    return true;
}

void CollisionComp::GotoSafePosition()
{
    MoveComp& mc = *owner_.moveComp_;
#ifdef DEBUG_COLLISION
    LOG_DEBUG << "Going back to safe position " << mc.GetSafePosition() << std::endl;
#endif
    owner_.transformation_.position_ = mc.GetSafePosition();
    mc.moved_ = false;
    mc.forcePosition_ = true;
}

Math::Vector3 CollisionComp::GetBodyCenter(const Math::Vector3& pos)
{
    return pos + GameObject::BodyOffset;
}

Iteration CollisionComp::CollisionCallback(const Math::BoundingBox& myBB,
    GameObject& other, const Math::Vector3& move,
    bool& updateTrans)
{
    if (owner_.CollisionMaskMatches(other.GetCollisionLayer()))
    {
#ifdef DEBUG_COLLISION
        LOG_DEBUG << owner_ << "(" << owner_.GetCollisionMask() << ")" << " colliding with " <<
            other << "(" << other.GetCollisionLayer() << ")" << std::endl;
#endif
        // If it's a triangle mesh then it's most likely a building and we may be able to step on it
        // FIXME: I think this isn't a great way doing this
        if (owner_.CollisionNeedsAdjustment(other))
        {
#ifdef DEBUG_COLLISION
            LOG_DEBUG << "Adjusting position" << std::endl;
#endif
            // Don't move the character when the object actually does not collide,
            // but we may still need the trigger stuff.
            if (move.Equals(Math::Vector3::Zero))
            {
                // There was no simple sliding solution, let's try the complicated one
                updateTrans = Slide(myBB, other);
            }
            else
            {
                // There was a simple solution, e.g. an AABB can do that
                owner_.transformation_.position_ += move;
                updateTrans = true;
            }
        }
    }

    // Need to notify both, because we test collisions only for moving objects
    // Notify other for colliding with us
    other.CallEvent<void(GameObject*)>(EVENT_ON_COLLIDE, &owner_);
    // Notify us for colliding with other
    owner_.CallEvent<void(GameObject*)>(EVENT_ON_COLLIDE, &other);

    return Iteration::Continue;
}

void CollisionComp::ResolveCollisions()
{
    // Players don't collide with other players in outposts.
    // GetType() is virtual so call it just once.
    const bool isCollidingWithPlayers = (owner_.GetType() != AB::GameProtocol::GameObjectType::Player) ||
        !AB::Entities::IsOutpost(owner_.GetGame()->data_.type);

    const Math::CallbackOctreeMatcher matcher([isCollidingWithPlayers](const Math::OctreeObject* object) -> bool
    {
        const GameObject* gameObject = static_cast<const GameObject*>(object);
        if (!gameObject->GetCollisionShape())
            return false;

        if (!isCollidingWithPlayers && (gameObject->GetType() == AB::GameProtocol::GameObjectType::Player))
            return false;
        if (const auto* actor = To<Actor>(gameObject))
        {
            // Not colliding with dead actors
            if (actor->IsDead())
                return false;
        }
        return true;
    });

    // Actor always has a MoveComp
    MoveComp& mc = *owner_.moveComp_;
    ea::vector<Math::OctreeObject*> c;
    const Math::BoundingBox box = owner_.GetWorldBoundingBox();
    if (owner_.QueryObjects(c, box, &matcher))
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
    ASSERT(owner_.moveComp_);
    if (owner_.moveComp_->moved_)
    {
        ResolveCollisions();
    }
}

}
}

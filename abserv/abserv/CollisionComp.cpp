#include "stdafx.h"
#include "CollisionComp.h"
#include "GameObject.h"
#include "Actor.h"
#include "Game.h"

namespace Game {
namespace Components {

void CollisionComp::ResolveCollisions()
{
    std::vector<GameObject*> c;
    Math::BoundingBox box = owner_.GetWorldBoundingBox();
    if (owner_.QueryObjects(c, box))
    {
        for (auto& ci : c)
        {
            if (ci != &owner_ && ((owner_.collisionMask_ & ci->collisionMask_) == ci->collisionMask_))
            {
                Math::Vector3 move;
                if (owner_.Collides(ci, owner_.moveComp_.velocity_, move))
                {
                    if (move != Math::Vector3::Zero)
                        owner_.transformation_.position_ += move;
                    else
                    {
                        owner_.transformation_.position_ = owner_.moveComp_.GetOldPosition();
                        owner_.moveComp_.moved_ = false;
                    }

                    // Need to notify both, because we test collisions only for moving objects
                    // Notify ci for colliding with us
                    ci->OnCollide(&owner_);
                    // Notify us for colliding with ci
                    owner_.OnCollide(ci);
                }
            }
        }
    }
}

void CollisionComp::Update(uint32_t)
{
    if (owner_.moveComp_.moved_)
        ResolveCollisions();
}

}
}

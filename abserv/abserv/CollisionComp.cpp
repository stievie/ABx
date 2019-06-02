#include "stdafx.h"
#include "CollisionComp.h"
#include "Actor.h"
#include "Game.h"
#include "MoveComp.h"

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
                // Actor always has a MoveComp
                MoveComp* mc = owner_.moveComp_.get();
                assert(mc);
                Math::Vector3 move;
                if (owner_.Collides(ci, mc->velocity_, move))
                {
                    // Doing now client side collisions as well, so only do it for NPCs
                    if (move != Math::Vector3::Zero && owner_.GetType() == AB::GameProtocol::ObjectTypeNpc)
                        owner_.transformation_.position_ += move;
                    else
                    {
                        owner_.transformation_.position_ = mc->GetOldPosition();
                        mc->moved_ = false;
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
    assert(owner_.moveComp_);
    if (owner_.moveComp_->moved_)
        ResolveCollisions();
}

}
}

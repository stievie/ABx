#include "stdafx.h"
#include "CollisionComp.h"
#include "GameObject.h"
#include "Creature.h"
#include "Game.h"

namespace Game {
namespace Components {

void CollisionComp::DoCollisions()
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
                if (owner_.Collides(ci, move))
                {
#ifdef DEBUG_COLLISION
                    //                    LOG_DEBUG << GetName() << " collides with " << ci->GetName() << std::endl;
#endif
                    if (move != Math::Vector3::Zero)
                        owner_.transformation_.position_ += move;
                    else
                        owner_.transformation_.position_ = owner_.moveComp_.GetOldPosition();

                    // Need to notify both, because we test collisions only for moving objects
                    // Notify ci for colliding with us
                    ci->OnCollide(owner_.GetThis<Creature>());
                    // Notify us for colliding with ci
                    owner_.OnCollide(ci->GetThis<Creature>());
                }
            }
        }
    }

    // Keep on ground
    float y = owner_.GetGame()->map_->GetTerrainHeight(owner_.transformation_.position_);
    owner_.transformation_.position_.y_ = y;
}

}
}

#include "stdafx.h"
#include "AutoRunComp.h"
#include "Creature.h"
#include "Game.h"
#include "MathUtils.h"

namespace Game {
namespace Components {

bool AutoRunComp::FindPath(const Math::Vector3& dest)
{
    wayPoints_.clear();
    bool succ = owner_.GetGame()->map_->FindPath(wayPoints_, owner_.transformation_.position_,
        dest);
#ifdef DEBUG_NAVIGATION
    LOG_DEBUG << "Goto from " << transformation_.position_.ToString() <<
        " to " << dest.ToString() << " via " << wayPoints_.size() << " waypoints:";
    for (const auto& wp : wayPoints_)
        LOG_DEBUG << " " << wp.ToString();
    LOG_DEBUG << std::endl;
#endif
    if (succ && wayPoints_.size() != 0)
    {
        return true;
    }
    return false;
}

void AutoRunComp::Remove()
{
    wayPoints_.erase(wayPoints_.begin());
}

void AutoRunComp::MoveToNext(uint32_t timeElapsed)
{
    const Math::Vector3& pt = Next();
    const float distance = pt.Distance(owner_.transformation_.position_);

    if (distance > NAVIGATION_MIN_DIST)
    {
        float worldAngle = -Math::DegToRad(owner_.transformation_.position_.AngleY(pt) - 180.0f);
        if (worldAngle < 0.0f)
            worldAngle += Math::M_PIF;
#ifdef DEBUG_NAVIGATION
        LOG_DEBUG << "From " << transformation_.position_.ToString() << " to " << pt.ToString() <<
            " angle " << worldAngle << " old angle " << Math::RadToDeg(transformation_.rotation_) <<
            ", distance " << distance << std::endl;
#endif
        if (fabs(owner_.transformation_.rotation_ - worldAngle) > 0.05f)
        {
            owner_.moveComp_.SetDirection(worldAngle);
        }
        owner_.moveComp_.Move(((float)(timeElapsed) / owner_.moveComp_.BaseSpeed) * owner_.GetActualMoveSpeed(),
            Math::Vector3::UnitZ);
    }
    else
    {
        // If we are close to this point remove it from the list
        Remove();
    }
}

}
}

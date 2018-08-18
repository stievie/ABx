#include "stdafx.h"
#include "AutoRunComp.h"
#include "Creature.h"
#include "Game.h"
#include "MathUtils.h"

namespace Game {
namespace Components {

bool AutoRunComp::FindPath(const Math::Vector3& dest)
{
    destination_ = dest;
    wayPoints_.clear();
    static const Math::Vector3 extends(1.0f, 4.0f, 1.0f);

    const Math::Vector3& pos = owner_.transformation_.position_;
    bool succ = owner_.GetGame()->map_->FindPath(wayPoints_, pos,
        dest, extends);
#ifdef DEBUG_NAVIGATION
    LOG_DEBUG << "Goto from " << pos.ToString() <<
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

void AutoRunComp::Pop()
{
    wayPoints_.erase(wayPoints_.begin());
}

void AutoRunComp::MoveTo(uint32_t timeElapsed, const Math::Vector3& dest)
{
    const Math::Vector3& pos = owner_.transformation_.position_;
#ifdef DEBUG_NAVIGATION
    const float distance = dest.Distance(pos);
#endif
    float worldAngle = -Math::DegToRad(pos.AngleY(dest) - 180.0f);
    if (worldAngle < 0.0f)
        worldAngle += Math::M_PIF;
#ifdef DEBUG_NAVIGATION
    LOG_DEBUG << "From " << pos.ToString() << " to " << dest.ToString() <<
        " angle " << worldAngle << " old angle " << Math::RadToDeg(owner_.transformation_.rotation_) <<
        ", distance " << distance << std::endl;
#endif
    if (fabs(owner_.transformation_.rotation_ - worldAngle) > 0.02f)
    {
        owner_.moveComp_.SetDirection(worldAngle);
    }
    owner_.moveComp_.Move(((float)(timeElapsed) / owner_.moveComp_.BaseSpeed) * owner_.GetActualMoveSpeed(),
        Math::Vector3::UnitZ);
}

void AutoRunComp::Update(uint32_t timeElapsed)
{
    if (!autoRun_)
        return;

    const Math::Vector3& pos = owner_.transformation_.position_;
    if (!HasWaypoints())
    {
        // Still auto running but no more waypoints, move close to dest
        const float distance = destination_.Distance(pos);
        // Remaining distance
        if (distance > Creature::MAX_INTERACTION_DIST)
        {
            MoveTo(timeElapsed, destination_);
            return;
        }
        else
        {
            // If at dest reset
            owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateIdle);
            autoRun_ = false;
        }
        return;
    }

    const Math::Vector3& pt = Next();
    const float distance = pt.Distance(pos);

    if (distance > Creature::SWITCH_WAYPOINT_DIST)
    {
        MoveTo(timeElapsed, pt);
    }
    else
    {
        // If we are close to this point remove it from the list
        Pop();
    }
}

}
}

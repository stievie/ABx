#include "stdafx.h"
#include "AutoRunComp.h"
#include "Actor.h"
#include "Game.h"
#include "MathUtils.h"
#include "ConfigManager.h"
#include "Player.h"

namespace Game {
namespace Components {

bool AutoRunComp::Follow(std::shared_ptr<GameObject> object, bool ping)
{
    auto actor = object->GetThisDynamic<Actor>();
    if (!actor)
        return false;
    following_ = actor;
    maxDist_ = RANGE_TOUCH;
    if (auto f = following_.lock())
    {
        bool succ = FindPath(f->transformation_.position_);
        if (succ && ping)
            owner_.CallEvent<void(uint32_t,AB::GameProtocol::ObjectCallType,int)>(EVENT_ON_PINGOBJECT, actor->id_, AB::GameProtocol::ObjectCallTypeFollow, 0);
        return succ;
    }
    return false;
}

bool AutoRunComp::Goto(const Math::Vector3& dest)
{
    maxDist_ = 0.2f;
    return FindPath(dest);
}

bool AutoRunComp::FindPath(const Math::Vector3& dest)
{
    destination_ = dest;
    static constexpr Math::Vector3 EXTENDS(1.0f, 8.0f, 1.0f);

    const Math::Vector3& pos = owner_.transformation_.position_;
    if (pos.Distance(dest) < maxDist_)
        return false;

    std::vector<Math::Vector3> wp;
    bool succ = owner_.GetGame()->map_->FindPath(wp, pos, dest, EXTENDS);
#ifdef DEBUG_NAVIGATION
    std::stringstream ss;
    ss << "Goto from " << pos.ToString() <<
        " to " << dest.ToString() << " via " << wp.size() << " waypoints:";
    for (const auto& _wp : wp)
        ss << " " << _wp.ToString();
    LOG_DEBUG << ss.str() << std::endl;
#endif
    if (succ && wp.size() != 0)
    {
        wayPoints_ = wp;
        lastCalc_ = Utils::Tick();
        return true;
    }
    lastCalc_ = 0;
    return false;
}

void AutoRunComp::Pop()
{
    wayPoints_.erase(wayPoints_.begin());
}

void AutoRunComp::MoveTo(uint32_t timeElapsed, const Math::Vector3& dest)
{
    owner_.moveComp_->HeadTo(dest);
    owner_.moveComp_->Move(
        owner_.moveComp_->GetSpeed(timeElapsed, BASE_MOVE_SPEED),
        Math::Vector3::UnitZ);
}

void AutoRunComp::Update(uint32_t timeElapsed)
{
    if (!autoRun_)
        return;

    const Math::Vector3& pos = owner_.transformation_.position_;
    if (auto f = following_.lock())
    {
        if ((lastCalc_ != 0 && Utils::TimeElapsed(lastCalc_) > 1000)
            && (destination_.Distance(f->transformation_.position_) > SWITCH_WAYPOINT_DIST))
        {
            // Find new path when following object moved and enough time passed
            FindPath(f->transformation_.position_);
        }
    }

    if (!HasWaypoints())
    {
        // Still auto running but no more waypoints, move close to dest
        const float distance = destination_.Distance(pos);
        // Remaining distance
        if (distance > maxDist_)
        {
            MoveTo(timeElapsed, destination_);
            return;
        }
        else
        {
            // If at dest reset
            owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateIdle);
            SetAutoRun(false);
            owner_.CallEvent<void(void)>(EVENT_ON_ARRIVED);
        }
        return;
    }

    const Math::Vector3& pt = Next();
    const float distance = pt.Distance(pos);

    if (distance > SWITCH_WAYPOINT_DIST)
    {
        MoveTo(timeElapsed, pt);
    }
    else
    {
        // If we are close to this point remove it from the list
        Pop();
    }
}

void AutoRunComp::SetAutoRun(bool value)
{
    if (autoRun_ != value)
    {
        autoRun_ = value;
        if (owner_.GetType() == AB::GameProtocol::ObjectTypePlayer)
        {
            // This tells the players client to switch off client prediction and use server positions instead
            auto nmsg = Net::NetworkMessage::GetNew();
            nmsg->AddByte(AB::GameProtocol::PlayerAutoRun);
            nmsg->Add<uint8_t>(autoRun_ ? 1 : 0);
            Player* player = static_cast<Player*>(&owner_);
            player->WriteToOutput(*nmsg);
        }
    }
}

}
}

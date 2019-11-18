#include "stdafx.h"
#include "WanderComp.h"
#include "Npc.h"
#include "AutoRunComp.h"

namespace Game {
namespace Components {

WanderComp::WanderComp(Npc& owner) :
    owner_(owner)
{ }

int WanderComp::FindCurrentPointIndex() const
{
    const auto& ownerPos = owner_.GetPosition();
    int result = -1;
    float dist = Math::M_INFINITE;
    int i = 0;
    for (const auto& pt : route_)
    {
        if (pt.Distance(ownerPos) < dist)
            result = i;
        ++i;
    }
    return result;
}

int WanderComp::GetNextIndex()
{
    int result = currentIndex_;
    if (direction_ == Direction::Forward)
    {
        if (currentIndex_ >= static_cast<int>(route_.size()) - 1)
            direction_ = Direction::Backeward;
    }
    else if (direction_ == Direction::Backeward)
    {
        if (currentIndex_ <= 0)
            direction_ = Direction::Forward;
    }

    switch (direction_)
    {
    case Direction::Forward:
        ++result;
        break;
    case Direction::Backeward:
        --result;
        break;
    }
    return result;
}

bool WanderComp::CheckDestination() const
{
    const auto& ownerPos = owner_.GetPosition();
    const Math::Vector3& point = route_[static_cast<size_t>(currentIndex_)];
    if (ownerPos.Distance(point) <= AutoRunComp::SWITCH_WAYPOINT_DIST)
        return true;
    return false;
}

bool WanderComp::GotoCurrentPoint()
{
    if (currentIndex_ >= 0 && currentIndex_ < static_cast<int>(route_.size()))
        return owner_.autorunComp_->Goto(route_.at(static_cast<size_t>(currentIndex_)));
    return false;
}

void WanderComp::AddRoutePoint(const Math::Vector3& point)
{
    route_.push_back(point);
}

void WanderComp::Update(uint32_t)
{
    if (!wandering_)
        return;

    if (CheckDestination())
        currentIndex_ = GetNextIndex();
    if (!GotoCurrentPoint())
        wandering_ = false;
}

bool WanderComp::Wander(bool value)
{
    if (route_.size() == 0)
        return false;

    if (value != wandering_)
        return wandering_;
    wandering_ = value;
    if (wandering_)
    {
        currentIndex_ = FindCurrentPointIndex();
        if (!GotoCurrentPoint())
            wandering_ = false;
    }

    return wandering_;
}

}
}

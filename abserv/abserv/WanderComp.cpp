#include "stdafx.h"
#include "WanderComp.h"
#include "Npc.h"
#include "AutoRunComp.h"
#include "Game.h"
#include "Map.h"

//#define DEBUG_NAVIGATION

namespace Game {
namespace Components {

WanderComp::WanderComp(Npc& owner) :
    owner_(owner)
{ }

void WanderComp::Initialize()
{
    // Calculate Y value once
    const auto& map = *owner_.GetGame()->map_;
    for (auto& p : route_)
        map.UpdatePointHeight(p);
    initialized_ = true;
}

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
            direction_ = Direction::Backward;
    }
    else if (direction_ == Direction::Backward)
    {
        if (currentIndex_ <= 0)
            direction_ = Direction::Forward;
    }

    switch (direction_)
    {
    case Direction::Forward:
        ++result;
        break;
    case Direction::Backward:
        --result;
        break;
    }
    return result;
}

bool WanderComp::CheckDestination() const
{
    const auto& ownerPos = owner_.GetPosition();
    const Math::Vector3& point = route_[static_cast<size_t>(currentIndex_)];
    if (ownerPos.Equals(point, AT_POSITION_THRESHOLD))
        return true;
    return false;
}

bool WanderComp::GotoCurrentPoint()
{
    if (currentIndex_ >= 0 && currentIndex_ < static_cast<int>(route_.size()))
    {
        const Math::Vector3& pt = route_[static_cast<size_t>(currentIndex_)];
        bool res = owner_.autorunComp_->Goto(pt);
        if (res)
        {
            owner_.autorunComp_->SetAutoRun(true);
            owner_.stateComp_.SetState(AB::GameProtocol::CreatureState::Moving);
        }
#ifdef DEBUG_NAVIGATION
        else
        {
            LOG_WARNING << "Failed to go to current point " <<
                pt.ToString() << std::endl;
        }
#endif
        return res;
    }
    return false;
}

void WanderComp::AddRoutePoint(const Math::Vector3& point)
{
    route_.push_back(point);
    initialized_ = false;
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
    {
#ifdef DEBUG_NAVIGATION
        LOG_WARNING << "No route points" << std::endl;
#endif
        return false;
    }

    if (value == wandering_)
        return wandering_;
    wandering_ = value;
    if (wandering_)
    {
        if (!initialized_)
            Initialize();
        currentIndex_ = FindCurrentPointIndex();
        if (!GotoCurrentPoint())
        {
            wandering_ = false;
        }
    }

#ifdef DEBUG_NAVIGATION
    LOG_DEBUG << owner_.GetName() << (wandering_ ? " starting" : "stopped") <<
        " wandering" << std::endl;
#endif
    return wandering_;
}

}
}

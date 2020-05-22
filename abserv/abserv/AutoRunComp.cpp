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


#include "Actor.h"
#include "AutoRunComp.h"
#include "Game.h"
#include "Player.h"
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>
#include <absmath/VectorMath.h>
#include <optional>

//#define DEBUG_NAVIGATION

namespace Game {
namespace Components {

AutoRunComp::AutoRunComp(Actor& owner) :
    owner_(owner)
{
    owner_.SubscribeEvent<void(GameObject*)>(EVENT_ON_COLLIDE, std::bind(&AutoRunComp::OnCollide, this, std::placeholders::_1));
    owner_.SubscribeEvent<void(void)>(EVENT_ON_STUCK, std::bind(&AutoRunComp::OnStuck, this));
}

bool AutoRunComp::Follow(ea::shared_ptr<GameObject> object, bool ping, float maxDist /* = RANGE_TOUCH */)
{
    auto actor = object->GetPtr<Actor>();
    if (!actor)
        return false;
    if (auto f = following_.lock())
    {
        // Already following this guy
        if (f->id_ == actor->id_)
            return true;
    }

    following_ = actor;
    maxDist_ = maxDist;
    if (auto f = following_.lock())
    {
        wayPoints_.clear();
        bool succ = FindPath(f->transformation_.position_);
        if (succ && ping)
            owner_.CallEvent<void(uint32_t,AB::GameProtocol::ObjectCallType,int)>(EVENT_ON_PINGOBJECT, actor->id_, AB::GameProtocol::ObjectCallType::Follow, 0);
        return succ;
    }
    return false;
}

bool AutoRunComp::Goto(const Math::Vector3& dest)
{
    maxDist_ = AT_POSITION_THRESHOLD;
    following_.reset();
    return FindPath(dest);
}

bool AutoRunComp::GotoDirection(const Math::Quaternion& direction, float distance)
{
    Math::Vector3 pos = Math::GetPosFromDirectionDistance(owner_.transformation_.position_,
        direction, distance);
    return Goto(pos);
}

void AutoRunComp::Reset()
{
    wayPoints_.clear();
    SetAutoRun(false);
    following_.reset();
}

bool AutoRunComp::FindPath(const Math::Vector3& dest)
{
    if (destination_.Equals(dest, AT_POSITION_THRESHOLD) && wayPoints_.size() != 0)
        return true;

    static constexpr Math::Vector3 EXTENDS{ 1.0f, 8.0f, 1.0f };

    const Math::Vector3& pos = owner_.transformation_.position_;
    if (pos.Distance(dest) < maxDist_)
        return false;

    ea::vector<Math::Vector3> wp;
    bool succ = owner_.GetGame()->map_->FindPath(wp, pos, dest, EXTENDS);
#ifdef DEBUG_NAVIGATION
    std::stringstream ss;
    ss << "Goto from " << pos << " to " << dest << " via " << wp.size() << " waypoints:";
    for (const auto& _wp : wp)
        ss << " " << _wp;
    LOG_DEBUG << ss.str() << std::endl;
#endif
    if (succ && wp.size() != 0)
    {
        wayPoints_ = wp;
        destination_ = dest;
        lastCalc_ = Utils::Tick();
        return true;
    }
    lastCalc_ = 0;
    return false;
}

void AutoRunComp::StopAutoRun()
{
    if (IsAutoRun())
    {
        if (owner_.stateComp_.IsMoving())
            owner_.stateComp_.SetState(AB::GameProtocol::CreatureState::Idle);
        SetAutoRun(false);
    }
}

void AutoRunComp::OnStuck()
{
    StopAutoRun();
}

void AutoRunComp::OnCollide(GameObject* other)
{
    if (auto fo = following_.lock())
    {
        if (other->id_ == fo->id_)
        {
            // If at dest reset
            StopAutoRun();
            owner_.CallEvent<void(void)>(EVENT_ON_ARRIVED);
        }
        else
            // If that's not the target just stop
            Reset();
    }
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

Math::Vector3 AutoRunComp::Next()
{
    return AvoidObstacles(wayPoints_[0]);
}

Math::Vector3 AutoRunComp::AvoidObstaclesInternal(const Math::Vector3& destination, unsigned recursionLevel)
{
    if (recursionLevel >= MAX_OBSTACLE_AVOID_RECURSION_LEVEL)
    {
#ifdef DEBUG_NAVIGATION
    LOG_DEBUG << "Stuck: new destination " << newDest << std::endl;
#endif
        owner_.CallEvent<void(void)>(EVENT_ON_STUCK);
        return destination;
    }

    const Math::Vector3& pos = owner_.transformation_.position_;

    // Raycast to the point and see if there is a hit.
    const auto raycast = [&pos, this](const Math::Vector3& dest) -> std::optional<Math::RayQueryResult>
    {
        ea::vector<Math::RayQueryResult> result;
        float dist = pos.Distance(dest);
        if (!owner_.RaycastWithResult(result, pos, dest - pos, dist))
            // No Octree (shouldn't happen)
            return {};
        if (result.size() == 0)
            // Lucky, no obstacles
            return {};

        // The first non-TerrainPatch hit
        Math::RayQueryResult* hit = nullptr;
        for (auto& r : result)
        {
            if (!Is<TerrainPatch>(r.object_) && owner_.CollisionMaskMatches(r.object_->GetCollisionMask()))
            {
                hit = &r;
                break;
            }
        }
        if (hit)
        {
            // May be the object we are moving to.
            // A bit more than the approx. extends of an average creature BB.
            if (hit->object_->transformation_.position_.Distance(dest) < AVERAGE_BB_EXTENDS * 2.0f)
                return {};
            // We need a copy of the hit, because when result runs out of scope the hit becomes invalid,
            // so we can't return just a pointer to the hit.
            return *hit;
        }
        return {};
    };

    const auto hit = raycast(destination);
    if (!hit.has_value())
        // Nothing or only TerrainPatches on the way
        return destination;

#ifdef DEBUG_NAVIGATION
//    LOG_DEBUG << "Obstacle " << *hit->object_ << " on the way to " << destination << std::endl;
#endif

    Math::BoundingBox bb = hit->object_->GetWorldBoundingBox();
    bb.AddSize(1.0f);
    Math::Vector3 newDest = bb.GetClosestCorner2D(hit->position_);
    if (!owner_.GetGame()->map_->CanStepOn(newDest))
        return AvoidObstaclesInternal(newDest, recursionLevel + 1);

    const auto newHit = raycast(newDest);
    if (newHit.has_value())
        return AvoidObstaclesInternal(newDest, recursionLevel + 1);
#ifdef DEBUG_NAVIGATION
    LOG_DEBUG << "New destination " << newDest << std::endl;
#endif
    return newDest;
}


Math::Vector3 AutoRunComp::AvoidObstacles(const Math::Vector3& destination)
{
    return AvoidObstaclesInternal(destination, 0);
}

void AutoRunComp::Update(uint32_t timeElapsed)
{
    if (!autoRun_)
        return;

    const Math::Vector3& pos = owner_.transformation_.position_;
    if (auto f = following_.lock())
    {
        if ((lastCalc_ != 0 && Utils::TimeElapsed(lastCalc_) > RECALCULATE_PATH_TIME)
            && (!destination_.Equals(f->transformation_.position_, AT_POSITION_THRESHOLD)))
        {
            // Find new path when following object moved and enough time passed
            FindPath(f->transformation_.position_);
        }
    }

    if (!HasWaypoints())
    {
#ifdef DEBUG_NAVIGATION
        LOG_DEBUG << owner_ << " has no (more) waypoints" << std::endl;
#endif
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
            StopAutoRun();
            owner_.CallEvent<void(void)>(EVENT_ON_ARRIVED);
        }
        return;
    }

    // Here we must have some waypoints left
    const Math::Vector3 currWp = Next();
    if (!pos.Equals(currWp, AT_POSITION_THRESHOLD))
    {
#ifdef DEBUG_NAVIGATION
        LOG_DEBUG << owner_ << " moving to way point " << currWp << std::endl;
#endif
        MoveTo(timeElapsed, currWp);
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
        if (Is<Player>(owner_))
        {
            // This tells the players client to switch off client prediction and use server positions instead
            auto nmsg = Net::NetworkMessage::GetNew();
            nmsg->AddByte(AB::GameProtocol::ServerPacketType::PlayerAutoRun);
            AB::Packets::Server::PlayerAutorun packet = {
                autoRun_
            };
            AB::Packets::Add(packet, *nmsg);
            Player& player = To<Player>(owner_);
            player.WriteToOutput(*nmsg);
        }
    }
}

bool AutoRunComp::IsFollowing(const Actor& actor) const
{
    if (!IsAutoRun())
        return false;
    if (auto f = following_.lock())
        return f->id_ == actor.id_;
    return false;
}

}
}

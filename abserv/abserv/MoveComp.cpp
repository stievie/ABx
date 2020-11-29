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


#include "MoveComp.h"
#include "Actor.h"
#include "CollisionComp.h"
#include "Game.h"
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>

namespace Game {
namespace Components {

void MoveComp::Update(uint32_t timeElapsed, uint32_t flags)
{
    if (owner_.stateComp_.GetState() != AB::GameProtocol::CreatureState::Moving)
        return;
    if (flags == 0 && !autoMove_)
        return;

    forcePosition_ = false;
    // Lets turn first
    if ((flags & UpdateFlagTurn) == UpdateFlagTurn)
        UpdateTurn(timeElapsed);
    if (autoMove_ || ((flags & UpdateFlagMove) == UpdateFlagMove))
    {
        StoreOldPosition();
        UpdateMove(timeElapsed);
        CalculateVelocity(timeElapsed);
        if (!velocity_.Equals(Math::Vector3::Zero))
            moved_ = true;
    }
}

bool MoveComp::SetPosition(const Math::Vector3& pos)
{
    StoreOldPosition();

    HeadTo(pos);
    owner_.transformation_.position_ = pos;

    if (owner_.GetType() != AB::GameProtocol::GameObjectType::Projectile)
    {
        // Keep on ground
        StickToGround();
    }
    if (owner_.collisionComp_)
        // We need to do it here because this is not called from Update()
        owner_.collisionComp_->ResolveCollisions();

    const bool moved = oldPosition_ != owner_.transformation_.position_;

    if (moved && owner_.octant_)
    {
        // We need to do it here because this is not called from Update()
        Math::Octree* octree = owner_.octant_->GetRoot();
        octree->AddObjectUpdate(&owner_);
    }

    if (moved)
        moved_ = true;

    return moved;
}

void MoveComp::StickToGround()
{
    owner_.GetGame()->map_->UpdatePointHeight(owner_.transformation_.position_);
}

void MoveComp::HeadTo(const Math::Vector3& pos)
{
    const float worldAngle = -owner_.transformation_.position_.AngleY(pos) - Math::M_PIF;
    SetDirection(worldAngle);
}

void MoveComp::Move(float speed, const Math::Vector3& amount)
{
/*
    // The client prediction calculates the position in the following way:
    const Math::Matrix4 m = Math::Matrix4::FromQuaternion(owner_.transformation_.oriention_.Inverse());
    const Math::Vector3 a = amount * speed;
    const Math::Vector3 v = m * a;
    Math::Vector3 oldPos = owner_.transformation_.position_;
    oldPos += v;
    LOG_INFO << "seepd " << speed << " amount " << amount << " v " << v << " new pos " << oldPos << std::endl;
*/

    owner_.transformation_.Move(speed, amount);

    if (owner_.GetType() != AB::GameProtocol::GameObjectType::Projectile)
    {
        auto& map = *owner_.GetGame()->map_;
        if (owner_.autorunComp_->IsAutoRun() || (!checkStepOn_ || map.CanStepOn(owner_.transformation_.position_)))
        {
            // Keep on ground, except projectiles, they usually fly...
            const float y = map.GetTerrainHeight(owner_.transformation_.position_);
            owner_.transformation_.position_.y_ = y;
        }
        else
        {
            owner_.transformation_.position_ = oldPosition_;
            forcePosition_ = true;
        }
    }

    const bool moved = !oldPosition_.Equals(owner_.transformation_.position_);
    if (moved)
        moved_ = true;
}

void MoveComp::UpdateMove(uint32_t timeElapsed)
{
    if (moveDir_ == 0)
        return;

    const float speed = GetSpeed(timeElapsed, BASE_MOVE_SPEED);
    if ((moveDir_ & AB::GameProtocol::MoveDirectionNorth) == AB::GameProtocol::MoveDirectionNorth)
        Move(speed, Math::Vector3::UnitZ);
    if ((moveDir_ & AB::GameProtocol::MoveDirectionSouth) == AB::GameProtocol::MoveDirectionSouth)
        // Move slower backward
        Move(speed / 2.0f, Math::Vector3::Back);
    if ((moveDir_ & AB::GameProtocol::MoveDirectionWest) == AB::GameProtocol::MoveDirectionWest)
        Move(speed / 2.0f, Math::Vector3::Left);
    if ((moveDir_ & AB::GameProtocol::MoveDirectionEast) == AB::GameProtocol::MoveDirectionEast)
        Move(speed / 2.0f, Math::Vector3::UnitX);
}

void MoveComp::Turn(float angle)
{
    owner_.transformation_.Turn(angle);
}

void MoveComp::UpdateTurn(uint32_t timeElapsed)
{
    const float speed = GetSpeed(timeElapsed, BASE_TURN_SPEED);
    if ((turnDir_ & AB::GameProtocol::TurnDirectionLeft) == AB::GameProtocol::TurnDirectionLeft)
    {
        Turn(speed);
        turned_ = true;
    }
    if ((turnDir_ & AB::GameProtocol::TurnDirectionRight) == AB::GameProtocol::TurnDirectionRight)
    {
        Turn(-speed);
        turned_ = true;
    }
}

void MoveComp::SetDirection(float worldAngle)
{
    const float ang = owner_.transformation_.GetYRotation();
    Math::NormalizeAngle(worldAngle);
    if (!Math::Equals(ang, worldAngle, 0.00001f))
    {
        owner_.transformation_.SetYRotation(worldAngle);
        directionSet_ = true;
    }
}

void MoveComp::Write(Net::NetworkMessage& message)
{
    if (speedDirty_)
    {
        speedDirty_ = false;
        message.AddByte(AB::GameProtocol::ServerPacketType::ObjectSpeedChanged);
        AB::Packets::Server::ObjectSpeedChanged packet = {
            owner_.id_,
            GetSpeedFactor()
        };
        AB::Packets::Add(packet, message);
    }

    if (moved_ || forcePosition_)
    {
        if (forcePosition_)
            message.AddByte(AB::GameProtocol::ServerPacketType::ObjectForcePosition);
        else
            message.AddByte(AB::GameProtocol::ServerPacketType::ObjectPositionUpdate);
        AB::Packets::Server::ObjectPositionUpdate packet = {
            owner_.id_,
            {
                owner_.transformation_.position_.x_,
                owner_.transformation_.position_.y_,
                owner_.transformation_.position_.z_
            }
        };
        AB::Packets::Add(packet, message);
        moved_ = false;
        forcePosition_ = false;
    }

    // The rotation may change in 2 ways: Turn and SetWorldDirection
    if (turned_ || directionSet_)
    {
        message.AddByte(AB::GameProtocol::ServerPacketType::ObjectRotationUpdate);
        AB::Packets::Server::ObjectRotationUpdate packet = {
            owner_.id_,
            owner_.transformation_.GetYRotation(),
            directionSet_
        };
        AB::Packets::Add(packet, message);
        turned_ = false;
        directionSet_ = false;
    }
}

void MoveComp::StoreOldPosition()
{
    oldPosition_ = owner_.transformation_.position_;
    safePosition_ = owner_.transformation_.position_;
}

void MoveComp::StoreSafePosition()
{
    safePosition_ = owner_.transformation_.position_;
}

Math::Vector3& MoveComp::CalculateVelocity(uint32_t timeElapsed)
{
    velocity_ = ((oldPosition_ - owner_.transformation_.position_) / (static_cast<float>(timeElapsed) / 1000.0f));
    return velocity_;
}

}
}

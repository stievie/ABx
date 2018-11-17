#include "stdafx.h"
#include "MoveComp.h"
#include "Actor.h"
#include "CollisionComp.h"
#include "MathUtils.h"
#include "Game.h"
#include "MathUtils.h"

namespace Game {
namespace Components {

void MoveComp::Update(uint32_t timeElapsed)
{
    MoveTo(timeElapsed);
    TurnTo(timeElapsed);
}

bool MoveComp::SetPosition(const Math::Vector3& pos)
{
    oldPosition_ = owner_.transformation_.position_;

    HeadTo(pos);
    owner_.transformation_.position_ = pos;
    // Keep on ground
    float y = owner_.GetGame()->map_->GetTerrainHeight(owner_.transformation_.position_);
    owner_.transformation_.position_.y_ = y;
    owner_.collisionComp_.DoCollisions();

    bool moved = oldPosition_ != owner_.transformation_.position_;

    if (moved && owner_.octant_)
    {
        Math::Octree* octree = owner_.octant_->GetRoot();
        octree->AddObjectUpdate(&owner_);
    }

    if (moved)
        moved_ = true;
    return moved;
}

void MoveComp::HeadTo(const Math::Vector3& pos)
{
    float worldAngle = -Math::DegToRad(oldPosition_.AngleY(pos) - 180.0f);
    if (worldAngle < 0.0f)
        worldAngle += Math::M_PIF;
    SetDirection(worldAngle);
}

bool MoveComp::Move(float speed, const Math::Vector3& amount)
{
    // new position = position + direction * speed (where speed = amount * speed)

    oldPosition_ = owner_.transformation_.position_;

    // It's as easy as:
    // 1. Create a matrix from the rotation,
    // 2. multiply this matrix with the moving vector and
    // 3. add the resulting vector to the current position
#ifdef HAVE_DIRECTX_MATH
    DirectX::XMMATRIX m = DirectX::XMMatrixRotationAxis(Math::Vector3::UnitY, -owner_.transformation_.rotation_);
    Math::Vector3 a = amount * speed;
    DirectX::XMVECTOR v = DirectX::XMVector3Transform(a, m);
    owner_.transformation_.position_.x_ += v.m128_f32[0];
    owner_.transformation_.position_.y_ += v.m128_f32[1];
    owner_.transformation_.position_.z_ += v.m128_f32[2];
#else
    Matrix4 m = Matrix4::CreateFromQuaternion(transformation_.GetQuaternion());
    Vector3 a = amount * speed;
    Vector3 v = m * a;
    transformation_.position_ += v;
#endif

    // Keep on ground
    float y = owner_.GetGame()->map_->GetTerrainHeight(owner_.transformation_.position_);
    owner_.transformation_.position_.y_ = y;

    owner_.collisionComp_.DoCollisions();

    bool moved = oldPosition_ != owner_.transformation_.position_;

    if (moved && owner_.octant_)
    {
        Math::Octree* octree = owner_.octant_->GetRoot();
        octree->AddObjectUpdate(&owner_);
    }

    if (moved)
        moved_ = true;
    return moved;
}

bool MoveComp::MoveTo(uint32_t timeElapsed)
{
    if (owner_.autorunComp_.autoRun_)
        return false;

    bool moved = false;
    if ((moveDir_ & AB::GameProtocol::MoveDirectionNorth) == AB::GameProtocol::MoveDirectionNorth)
    {
        moved |= Move(((float)(timeElapsed) / BaseSpeed) * speedFactor_, Math::Vector3::UnitZ);
    }
    if ((moveDir_ & AB::GameProtocol::MoveDirectionSouth) == AB::GameProtocol::MoveDirectionSouth)
    {
        // Move slower backward
        moved |= Move(((float)(timeElapsed) / BaseSpeed) * speedFactor_, Math::Vector3::Back / 2.0f);
    }
    if ((moveDir_ & AB::GameProtocol::MoveDirectionWest) == AB::GameProtocol::MoveDirectionWest)
    {
        moved |= Move(((float)(timeElapsed) / BaseSpeed) * speedFactor_, Math::Vector3::Left / 2.0f);
    }
    if ((moveDir_ & AB::GameProtocol::MoveDirectionEast) == AB::GameProtocol::MoveDirectionEast)
    {
        moved |= Move(((float)(timeElapsed) / BaseSpeed) * speedFactor_, Math::Vector3::UnitX / 2.0f);
    }

    moved_ |= moved;
    return moved;
}

void MoveComp::Turn(float angle)
{
    owner_.transformation_.rotation_ += angle;
    Math::NormalizeAngle(owner_.transformation_.rotation_);
}

void MoveComp::TurnTo(uint32_t timeElapsed)
{
    if ((turnDir_ & AB::GameProtocol::TurnDirectionLeft) == AB::GameProtocol::TurnDirectionLeft)
    {
        Turn(((float)(timeElapsed) / 2000.0f) * speedFactor_);
        turned_ = true;
    }
    if ((turnDir_ & AB::GameProtocol::TurnDirectionRight) == AB::GameProtocol::TurnDirectionRight)
    {
        Turn(-((float)(timeElapsed) / 2000.0f) * speedFactor_);
        turned_ = true;
    }
}

void MoveComp::SetDirection(float worldAngle)
{
    if (!Math::Equals(owner_.transformation_.rotation_, worldAngle))
    {

        owner_.transformation_.rotation_ = worldAngle;
        Math::NormalizeAngle(owner_.transformation_.rotation_);
        directionSet_ = true;
    }
}

}
}

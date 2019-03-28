#pragma once

#include "Vector3.h"
#include <AB/ProtocolCodes.h>

namespace Game {

class GameObject;

namespace Components {

class MoveComp
{
private:
    GameObject& owner_;
    Math::Vector3 oldPosition_;
    float speedFactor_;
    /// Move to moveDir_
    void UpdateMove(uint32_t timeElapsed);
    /// Turn to turnDir_
    void UpdateTurn(uint32_t timeElapsed);
public:
    enum UpdateFlags : uint32_t
    {
        UpdateFlagMove = 1,
        UpdateFlagTurn = 1 << 1
    };
    MoveComp() = delete;
    explicit MoveComp(GameObject& owner) :
        owner_(owner),
        moveDir_(AB::GameProtocol::MoveDirectionNone),
        turnDir_(AB::GameProtocol::TurnDirectionNone),
        oldPosition_(Math::Vector3::Zero),
        speedFactor_(1.0f),
        turned_(false),
        moved_(false),
        directionSet_(false),
        newAngle_(false),
        speedDirty_(false),
        velocity_(Math::Vector3::Zero)
    { }
    // non-copyable
    MoveComp(const MoveComp&) = delete;
    MoveComp& operator=(const MoveComp&) = delete;
    ~MoveComp() = default;

    void Update(uint32_t timeElapsed, uint32_t flags);
    bool SetPosition(const Math::Vector3& pos);
    void HeadTo(const Math::Vector3& pos);
    /// Move in direction of rotation
    void Move(float speed, const Math::Vector3& amount);
    void Turn(float angle);
    void SetDirection(float worldAngle);
    float GetSpeedFactor() const
    {
        return speedFactor_;
    }
    void SetSpeedFactor(float value)
    {
        if (speedFactor_ != value)
        {
            speedFactor_ = value;
            speedDirty_ = true;
        }
    }
    void AddSpeed(float value)
    {
        speedFactor_ += value;
        speedDirty_ = true;
    }

    const Math::Vector3& GetOldPosition() const
    {
        return oldPosition_;
    }
    bool IsMoving() const { return velocity_ != Math::Vector3::Zero; }
    void Write(Net::NetworkMessage& message);

    uint32_t moveDir_;
    uint32_t turnDir_;
    bool turned_;
    bool moved_;
    bool speedDirty_;
    /// Manual direction set
    bool directionSet_;
    bool newAngle_;
    /// Velocity in Units/s.
    Math::Vector3 velocity_;
};

}
}
